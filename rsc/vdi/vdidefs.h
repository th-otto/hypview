#ifndef __VDIDEFS_H__
#define __VDIDEFS_H__ 1

#include "config.h"

#define	MAX_PAL		4096	/* palette size                     */

/*
 * set to 1 for supporting pixel-packed VIDEL modes, including hicolor and truecolor
 */
#define VIDEL_SUPPORT 0

/*
 * set to 1 for supporting hardware accelerated blitter routines
 */
#define BLITTER_SUPPORT 0

/*
 * set to 1 for supporting 8 planes interleaved video modes
 * (and corresponding 256 entries color palette)
 */
#define PLANES8 1

#if VIDEL_SUPPORT
#define MU_PLANES 32
#else
#if PLANES8
#define MU_PLANES 8
#else
#define MU_PLANES 4
#endif
#endif


/*
 * PTSIN maximum length
 */
#define MAX_PTSIN 1024

#endif /* __VDIDEFS_H__ */
