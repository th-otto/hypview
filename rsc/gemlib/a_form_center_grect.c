#include "gem_aesP.h"

/** see mt_form_center()
 *
 *  @param tree mt_form_center()
 *  @param r mt_form_center()\n
 *             [option CHECK_NULLPTR] r may be NULL
 *
 *  @return  see mt_form_center()
 *
 *  @since  see mt_form_center()
 *
 */

short form_center_grect( OBJECT *tree, GRECT *r)
{
	AES_PARAMS(54,0,5,1,0);

	aes_addrin[0] = tree;

	AES_TRAP(aes_params);

#if CHECK_NULLPTR
	if (r)
#endif
	*r = aes_intout_ptr(1, GRECT);

 	return aes_intout[0];
}
