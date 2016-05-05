#include "hypdefs.h"
#include "hypdebug.h"

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

void hyp_decode_gfx(HYP_DOCUMENT *hyp, const unsigned char *src, struct hyp_gfx *gfx, FILE *errorfile, gboolean read_images)
{
	unsigned char hyp_pic_raw[SIZEOF_HYP_PICTURE];
	_UBYTE *data;
	hyp_nodenr node;
	HYP_IMAGE *pic;
	unsigned long data_size;
		
	gfx->type = *src;

	switch (*src)
	{
	case HYP_ESC_PIC:
		gfx->extern_node_index = node = DEC_255(&src[1]);
		gfx->x_offset = src[3];
		gfx->y_offset = DEC_255(&src[4]);
		gfx->width = src[6];
		gfx->height = src[7];
		gfx->islimage = hyp->comp_vers >= 3 && src[6] == 1;
		gfx->format = HYP_PIC_UNKNOWN;
		gfx->pixwidth = gfx->width * HYP_PIC_FONTW;
		gfx->pixheight = gfx->height * HYP_PIC_FONTH;
		gfx->planes = 1;
		if (!hypnode_valid(hyp, node) ||
			hyp->indextable[node]->type != HYP_NODE_IMAGE)
		{
			HYP_DBG(("%s:%d: node %u not an image", hyp_basename(__FILE__), __LINE__, node));
			break;
		}
		pic = (HYP_IMAGE *)AskCache(hyp, node);
		if (pic == NULL)
		{
			pic = g_new0(HYP_IMAGE, 1);
			if (pic == NULL)
				break;
			data = hyp_loaddata(hyp, node);
			if (data == NULL)
			{
				g_free(pic);
				if (errorfile)
					hyp_utf8_fprintf(errorfile, ("failed to load image node %u\n"), node);
				break;
			}
			data_size = GetDataSize(hyp, node);
			if (data_size < SIZEOF_HYP_PICTURE ||
				!GetEntryBytes(hyp, node, data, hyp_pic_raw, SIZEOF_HYP_PICTURE))
			{
				g_free(pic);
				g_free(data);
				if (errorfile)
					hyp_utf8_fprintf(errorfile, _("failed to decode image header for %u\n"), node);
				break;
			}
			pic->data_size = data_size;
			pic->number = node;
			pic->pic.fd_addr = data;
			if (hyp_pic_get_header(pic, hyp_pic_raw, errorfile) == FALSE)
			{
				g_free(pic);
				g_free(data);
				break;
			}
			gfx->pixwidth = pic->pic.fd_w;
			gfx->pixheight = pic->pic.fd_h;
			gfx->planes = pic->pic.fd_nplanes;
			gfx->format = (hyp_pic_format)hyp->indextable[node]->toc_index;
			if (!TellCache(hyp, node, (HYP_NODE *)pic))
			{
				g_free(pic);
				g_free(data);
				if (errorfile && read_images)
					hyp_utf8_fprintf(errorfile, _("failed to cache compressed image data for %u\n"), node);
			}
		} else
		{
			ASSERT(pic->decompressed);
			gfx->pixwidth = pic->pic.fd_w;
			gfx->pixheight = pic->pic.fd_h;
			gfx->planes = pic->pic.fd_nplanes;
		}
		break;

	case HYP_ESC_LINE:
		gfx->x_offset = src[1];
		gfx->y_offset = DEC_255(&src[2]);
		gfx->width = (src[4] - 128);
		gfx->height = src[5] - 1;
		gfx->attr = src[6] - 1;
		gfx->begend = gfx->attr & 7;
		gfx->style = min(max((gfx->attr >> 3), 0), 6) + 1;
		break;

	case HYP_ESC_BOX:
	case HYP_ESC_RBOX:
		gfx->x_offset = src[1];
		gfx->y_offset = DEC_255(&src[2]);
		gfx->width = src[4];
		gfx->height = src[5];
		gfx->attr = src[6] - 1;
		gfx->style = gfx->attr;
		break;
	}
}

/* ------------------------------------------------------------------------- */

/*
 * this function is only for completeness:
 * none of the files i found needs this,
 * and the new compiler does not generate images that would need it
 */
void hyp_pic_apply_planemasks(HYP_IMAGE *pic, unsigned char *buf)
{
	if (pic->incomplete)
	{
		int plane;
		unsigned char *dst = buf + pic->image_size;
		unsigned char *src = buf + pic->data_size;
		unsigned long planesize = (unsigned long)(pic->pic.fd_wdwidth << 1) * pic->pic.fd_h;
		
		/* Copy the present planes data */
		for (plane = pic->pic.fd_nplanes; --plane >= 0; )
		{
			dst -= planesize;
			if (pic->plane_pic & (1u << plane))
			{
				src -= planesize;
				if (dst != src)
				{
					memcpy(dst, src, planesize);
					memset(src, 0, planesize);
				}
			} else if (dst != src)
			{
				memset(dst, 0, planesize);
			}
		}
	}
	
	/* Fill the totally filled planes */
	if (pic->plane_on_off != 0 && pic->pic.fd_nplanes >= 1 && pic->pic.fd_nplanes <= 8)
	{
		int plane;
		unsigned char *dst = buf + SIZEOF_HYP_PICTURE;
		unsigned long planesize = (unsigned long)(pic->pic.fd_wdwidth << 1) * pic->pic.fd_h;
		
		for (plane = 0; plane < pic->pic.fd_nplanes; plane++)
		{
			if (pic->plane_on_off & (1u << plane))
			{
				memset(dst, 0xff, planesize);
			}
			dst += planesize;
		}
	}
}

/* ------------------------------------------------------------------------- */

static gboolean transform_image(HYP_DOCUMENT *hyp, struct hyp_gfx *gfx)
{
	hyp_nodenr node = gfx->extern_node_index;
	unsigned long data_size;
	unsigned char *buf;
	void *image;
	HYP_IMAGE *pic;
	gboolean inplace;
	
	pic = (HYP_IMAGE *)AskCache(hyp, node);
	if (pic == NULL || pic->pic.fd_addr == NULL)
		return FALSE;
	if (pic->decompressed)
		return TRUE;
	
	data_size = pic->data_size;
	if (data_size > pic->image_size)
	{
		/*
		 * something went wrong. should have been checked in hyp_pic_get_header already.
		 * Just update it so that we dont try to decode more bytes than we allocated.
		 */
		HYP_DBG(("%s:%d: trying to decode %ld bytes for image only %ld long", hyp_basename(__FILE__), __LINE__, data_size, pic->image_size));
		data_size = pic->image_size;
	}
	buf = g_new(unsigned char, pic->image_size);
	if (buf == NULL)
	{
		g_free(pic->pic.fd_addr);
		g_free(pic);
		TellCache(hyp, node, NULL);
		return FALSE;
	}
	if (!GetEntryBytes(hyp, node, (const unsigned char *)pic->pic.fd_addr, buf, data_size))
	{
		g_free(buf);
		g_free(pic->pic.fd_addr);
		g_free(pic);
		TellCache(hyp, node, NULL);
		HYP_DBG(("%s:%d: failed to decode image header for %u", hyp_basename(__FILE__), __LINE__, node));
		return FALSE;
	}
	hyp_pic_apply_planemasks(pic, buf);
	image = buf + SIZEOF_HYP_PICTURE;
	inplace = W_Fix_Bitmap(&image, pic->pic.fd_w, pic->pic.fd_h, pic->pic.fd_nplanes);
	if (image == NULL)
	{
		g_free(buf);
		g_free(pic->pic.fd_addr);
		g_free(pic);
		TellCache(hyp, node, NULL);
		HYP_DBG(("%s:%d: failed transform image for %u", hyp_basename(__FILE__), __LINE__, node));
		return FALSE;
	}
	if (inplace)
	{
		/*
		 * device data is either monochrome (which does not need conversion),
		 * or was converted in place.
		 */
		g_free(pic->pic.fd_addr);
		pic->pic.fd_addr = image;
	} else
	{
		/*
		 * image now contains device dependent data
		 */
		g_free(buf);
		g_free(pic->pic.fd_addr);
		pic->pic.fd_addr = image;
	}
	pic->decompressed = TRUE;
	return TRUE;
}

/* ------------------------------------------------------------------------- */

gboolean hyp_prep_graphics(HYP_DOCUMENT *hyp, HYP_NODE *node)
{
	const unsigned char *src, *end;
	gboolean retval = TRUE;
	struct hyp_gfx *gfx, **last;
	unsigned short dithermask;
	
	ASSERT(node->gfx == NULL);
	
	src = node->start;
	end = node->end;
	dithermask = 0;
	
	/*
	 * scan through esc commands, gathering graphic commands
	 */
	while (retval && src < end && *src == HYP_ESC)
	{
		switch (src[1])
		{
		case HYP_ESC_PIC:
		case HYP_ESC_LINE:
		case HYP_ESC_BOX:
		case HYP_ESC_RBOX:
			last = &node->gfx;
			while (*last != NULL)
				last = &(*last)->next;
			gfx = g_new0(struct hyp_gfx, 1);
			if (gfx == NULL)
			{
				retval = FALSE;
				break;
			}
			*last = gfx;
			hyp_decode_gfx(hyp, src + 1, gfx, NULL, TRUE);
			if (gfx->type == HYP_ESC_PIC)
			{
				gfx->dithermask = dithermask;
				transform_image(hyp, gfx);
				dithermask = 0;
			}
			break;
		case HYP_ESC_DITHERMASK:
			if (src[2] == 5u)
				dithermask = short_from_chars(&src[3]);
			break;
		}
		src = hyp_skip_esc(src);
	}
	
	return retval;
}

/* ------------------------------------------------------------------------- */

void hyp_free_graphics(HYP_NODE *node)
{
	struct hyp_gfx *gfx, *next;
	
	for (gfx = node->gfx; gfx != NULL; gfx = next)
	{
		next = gfx->next;
		g_free(gfx);
	}
	node->gfx = NULL;
}
