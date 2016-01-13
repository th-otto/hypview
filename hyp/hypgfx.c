#include "hypdefs.h"
#include "hypdebug.h"

/* ------------------------------------------------------------------------- */

void hyp_decode_gfx(HYP_DOCUMENT *hyp, const unsigned char *src, struct hyp_gfx *gfx)
{
	unsigned char hyp_pic_raw[SIZEOF_HYP_PICTURE];
	_UBYTE *data;
	HYP_PICTURE hyp_pic;
	hyp_nodenr node;
	HYP_IMAGE *pic;
	
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
			pic = g_new(HYP_IMAGE, 1);
			if (pic == NULL)
				break;
			data = hyp_loaddata(hyp, node);
			if (data == NULL)
			{
				g_free(pic);
				HYP_DBG(("%s:%d: failed to load image node %u", hyp_basename(__FILE__), __LINE__, node));
				break;
			}
			if (GetDataSize(hyp, node) < SIZEOF_HYP_PICTURE ||
				!GetEntryBytes(hyp, node, data, hyp_pic_raw, SIZEOF_HYP_PICTURE))
			{
				g_free(pic);
				g_free(data);
				HYP_DBG(("%s:%d: failed to decode image header for %u", hyp_basename(__FILE__), __LINE__, node));
				break;
			}
			hyp_pic_get_header(&hyp_pic, hyp_pic_raw);
			gfx->pixwidth = hyp_pic.width;
			gfx->pixheight = hyp_pic.height;
			gfx->planes = hyp_pic.planes;
			gfx->format = (hyp_pic_format)hyp->indextable[node]->toc_index;
			pic->pic.fd_addr = data;
			pic->pic.fd_w = gfx->pixwidth;
			pic->pic.fd_h = gfx->pixheight;
			pic->pic.fd_nplanes = gfx->planes;
			pic->pic.fd_wdwidth = (pic->pic.fd_w + 15) >> 4;
			pic->pic.fd_stand = FALSE;
			pic->decompressed = FALSE;
			pic->number = node;
			if (!TellCache(hyp, node, (HYP_NODE *)pic))
			{
				g_free(pic);
				g_free(data);
				HYP_DBG(("%s:%d: failed to cache compressed image data for %u", hyp_basename(__FILE__), __LINE__, node));
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
		gfx->style = min(gfx->attr, 8);
		break;
	}
}

/* ------------------------------------------------------------------------- */

static gboolean transform_image(HYP_DOCUMENT *hyp, struct hyp_gfx *gfx)
{
	hyp_nodenr node = gfx->extern_node_index;
	HYP_PICTURE hyp_pic;
	long data_size;
	unsigned char *buf;
	void *image;
	HYP_IMAGE *pic;
	gboolean inplace;
	
	pic = (HYP_IMAGE *)AskCache(hyp, node);
	if (pic == NULL || pic->pic.fd_addr == NULL)
		return FALSE;
	if (pic->decompressed)
		return TRUE;
	
	data_size = GetDataSize(hyp, node);
	buf = g_new(unsigned char, data_size);
	if (buf == NULL)
	{
		g_free(pic->pic.fd_addr);
		g_free(pic);
		TellCache(hyp, node, NULL);
		return FALSE;
	}
	if (!GetEntryBytes(hyp, node, pic->pic.fd_addr, buf, data_size))
	{
		g_free(buf);
		g_free(pic->pic.fd_addr);
		g_free(pic);
		TellCache(hyp, node, NULL);
		HYP_DBG(("%s:%d: failed to decode image header for %u", hyp_basename(__FILE__), __LINE__, node));
		return FALSE;
	}
	hyp_pic_get_header(&hyp_pic, buf);
	image = buf + SIZEOF_HYP_PICTURE;
	inplace = W_Fix_Bitmap(&image, hyp_pic.width, hyp_pic.height, hyp_pic.planes);
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

	ASSERT(node->gfx == NULL);
	
	src = node->start;
	end = node->end;
	
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
			hyp_decode_gfx(hyp, src + 1, gfx);
			if (gfx->type == HYP_ESC_PIC)
			{
				transform_image(hyp, gfx);
			}
			break;
		}
		src = hyp_skip_esc(src);
	}
	
	return retval;
}
