#include "hypdefs.h"
#include "hypdebug.h"

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

void hyp_pic_get_header(HYP_PICTURE *hyp_pic, const unsigned char *hyp_pic_raw)
{
	hyp_pic->width = short_from_chars(hyp_pic_raw);
	hyp_pic->height = short_from_chars(hyp_pic_raw + 2);
	hyp_pic->planes = hyp_pic_raw[4];
	hyp_pic->plane_pic = hyp_pic_raw[5];
	hyp_pic->plane_on_off = hyp_pic_raw[6];
	hyp_pic->filler = hyp_pic_raw[7];
}
