#if defined(PRIVATE_VDI)

#include "../vdi/vdi.h"

long vq_vgdos(void)
{
	return vdi_vq_vgdos();
}

#elif defined(__GNUC__) && defined(__mc68000__)

#include "gem_vdiP.h"

long vq_vgdos(void)
{
	register long x __asm__("%d0");
	
	__asm__ volatile (
		"moveq	#-2,%0\n\t"
		"trap	#2"
		: "=r"(x)
		:
		: "d1","d2","a0","a1","a2","cc","memory"
	);
	return x;
}

#elif defined(__VBCC__) && defined(__mc68000__)

#include "gem_vdiP.h"

__regsused("d0/d1/a0/a1") long vq_vgdos(void) =
  "\tmove.l\td2,-(sp)\n"
  "\tmove.l\ta2,-(sp)\n"
  "\tmoveq\t#-2,d0\n"
  "\ttrap\t#2\n"
  "\tmove.l\t(sp)+,a2\n"
  "\tmove.l\t(sp)+,d2";

#else

extern int _I_dont_care_that_ISO_C_forbids_an_empty_source_file_;

#endif
