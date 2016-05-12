#include "hypdefs.h"
#include "hypdebug.h"
#include "picture.h"

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

gboolean hyp_pic_get_header(HYP_IMAGE *image, const unsigned char *hyp_pic_raw, FILE *errorfile)
{
	/* image->pic.fd_addr must be set by caller */
	/* image->node must be set by caller */
	/* image->data_size must be set by caller */

	image->pic.fd_w = short_from_chars(hyp_pic_raw);
	image->pic.fd_h = short_from_chars(hyp_pic_raw + 2);
	image->pic.fd_nplanes = hyp_pic_raw[4];
	image->pic.fd_wdwidth = (image->pic.fd_w + 15) >> 4;
	image->pic.fd_stand = FALSE;
	image->plane_pic = hyp_pic_raw[5];
	image->plane_on_off = hyp_pic_raw[6];
	/* image->filler = hyp_pic_raw[7]; unused */
	image->decompressed = FALSE;

	if ((image->pic.fd_nplanes != 1 &&
		 image->pic.fd_nplanes != 2 &&
		 image->pic.fd_nplanes != 4 &&
		 image->pic.fd_nplanes != 8 &&
		 image->pic.fd_nplanes != 15 &&
		 image->pic.fd_nplanes != 16 &&
		 image->pic.fd_nplanes != 24 &&
		 image->pic.fd_nplanes != 32) ||
		 image->pic.fd_w <= 0 ||
		 image->pic.fd_h <= 0)
	{
		if (errorfile)
		{
			char *colors = pic_colornameformat(image->pic.fd_nplanes);
			hyp_utf8_fprintf(errorfile, _("unsupported picture format %dx%d%s in node %u\n"), image->pic.fd_w, image->pic.fd_h, colors, image->number);
			g_free(colors);
		}
		return FALSE;
	}

	if (image->pic.fd_nplanes >= 1 && image->pic.fd_nplanes <= 8)
	{
		unsigned long planesize = (unsigned long)(image->pic.fd_wdwidth << 1) * image->pic.fd_h;
		int plane;
		unsigned long image_size = SIZEOF_HYP_PICTURE;
		unsigned char planemask = (1u << image->pic.fd_nplanes) - 1;
		
		image->plane_pic &= planemask;
		image->plane_on_off &= planemask;
		image->incomplete = image->plane_pic != planemask;
		for (plane = 0; plane < image->pic.fd_nplanes; plane++)
			if (image->plane_pic & (1 << plane))
				image_size += planesize;
		if (image_size != image->data_size)
		{
			/*
			 * happens e.g. in icfs.hyp, created by compiler version 2.
			 */
			if (errorfile && !image->warned)
			{
				/* FIXME: won't see any message in the GUI */
				hyp_utf8_fprintf(errorfile, _("wrong image size for node %u: %lu, should be %lu%s\n"),
					image->number, image->data_size, image_size,
					image_size > image->data_size ? _(" (corrupted)") : "");
				image->warned = TRUE;
			}
			if (image_size > image->data_size)
			{
				/*
				 * data in file is too short, reject it
				 */
				return FALSE;
			}
			/*
			 * data in file is longer than expected.
			 * Must update the datasize because we later allocate
			 * only the needed size for the image buffer,
			 * and would otherwise try to decode too much data.
			 */
			image->data_size = image_size;
		}
		image->image_size = SIZEOF_HYP_PICTURE + planesize * image->pic.fd_nplanes;
	} else
	{
		image->image_size = image->data_size;
		image->incomplete = FALSE;
	}
	
	return TRUE;
}
