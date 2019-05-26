#include "gem_aesP.h"

/** return non-zero value if both area are equal
 *
 *  @param p1 an area
 *  @param p2 an other area
 *
 *  @return 0 if \p p1 and \p p2 are not the same area, any other
 *          value (not 0) otherwise (\p p1 equal \p p2).
 *
 */
 
short
rc_equal (const GRECT * p1, const GRECT * p2)
{
	return p1->g_x == p2->g_x &&
		   p1->g_y == p2->g_y &&
		   p1->g_w == p2->g_w &&
		   p1->g_h == p2->g_h;
}
