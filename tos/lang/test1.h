/*
 * resource set indices for test1
 *
 * created by ORCS 2.12
 */

/*
 * Number of Strings:        7
 * Number of Bitblks:        0
 * Number of Iconblks:       2
 * Number of Color Iconblks: 5
 * Number of Color Icons:    6
 * Number of Tedinfos:       0
 * Number of Free Strings:   0
 * Number of Free Images:    0
 * Number of Objects:        8
 * Number of Trees:          1
 * Number of Userblks:       0
 * Number of Images:         4
 * Total file size:          638
 */

#undef RSC_NAME
#define RSC_NAME "test1"
#undef RSC_ID
#ifdef test1
#define RSC_ID test1
#else
#define RSC_ID 0
#endif

#if !defined(RSC_STATIC_FILE) || !RSC_STATIC_FILE
#define NUM_STRINGS 7
#define NUM_FRSTR 0
#define NUM_UD 0
#define NUM_IMAGES 4
#define NUM_BB 0
#define NUM_FRIMG 0
#define NUM_IB 2
#define NUM_CIB 5
#define NUM_TI 0
#define NUM_OBS 8
#define NUM_TREE 1
#endif



#define TREE002                            0 /* form/dialog */




#ifdef __STDC__
#ifndef _WORD
#  ifdef WORD
#    define _WORD WORD
#  else
#    define _WORD short
#  endif
#endif
extern _WORD test1_rsc_load(void);
extern _WORD test1_rsc_gaddr(_WORD type, _WORD idx, void *gaddr);
extern _WORD test1_rsc_free(void);
#endif
