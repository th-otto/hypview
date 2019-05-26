#include "gem_vdiP.h"

#if defined(__GNUC__) && defined(__mc68000__) && !defined(PRIVATE_VDI)

void vdi(VDIPB *pb)
{
	__asm__ volatile (
		"move.l	%0,%%d1\n\t"	/* &vdipb */
		"moveq	#115,%%d0\n\t"
		"trap	#2"
		:
		: "g"(pb)
		: "d0","d1","d2","a0","a1","a2","memory","cc"
	);
}

#else

void vdi(VDIPB *pb)
{
	vditrap(pb);
}

#endif
