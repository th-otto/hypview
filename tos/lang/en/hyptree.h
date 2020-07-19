/*
 * resource set indices for hyptree
 *
 * created by ORCS 2.17
 */

/*
 * Number of Strings:        24
 * Number of Bitblks:        0
 * Number of Iconblks:       0
 * Number of Color Iconblks: 1
 * Number of Color Icons:    1
 * Number of Tedinfos:       0
 * Number of Free Strings:   8
 * Number of Free Images:    0
 * Number of Objects:        24
 * Number of Trees:          3
 * Number of Userblks:       0
 * Number of Images:         0
 * Total file size:          2198
 */

#undef RSC_NAME
#ifndef __ALCYON__
#define RSC_NAME "hyptree"
#endif
#undef RSC_ID
#ifdef hyptree
#define RSC_ID hyptree
#else
#define RSC_ID 0
#endif

#ifndef RSC_STATIC_FILE
# define RSC_STATIC_FILE 0
#endif
#if !RSC_STATIC_FILE
#define NUM_STRINGS 24
#define NUM_FRSTR 8
#define NUM_UD 0
#define NUM_IMAGES 0
#define NUM_BB 0
#define NUM_FRIMG 0
#define NUM_IB 0
#define NUM_CIB 1
#define NUM_TI 0
#define NUM_OBS 24
#define NUM_TREE 3
#endif



#define DIAL_LIBRARY                       0 /* form/dialog */
#define DI_ICON                            0 /* CICON in tree DIAL_LIBRARY */

#define MAINMENU                           1 /* menu */
#define ME_FILE                            4 /* TITLE in tree MAINMENU */
#define ME_ABOUT                           7 /* STRING in tree MAINMENU */
#define ME_OPEN                           16 /* STRING in tree MAINMENU */
#define ME_CLOSE                          17 /* STRING in tree MAINMENU */
#define ME_QUIT                           19 /* STRING in tree MAINMENU */

#define ABOUT_DIALOG                       2 /* form/dialog */
#define PR_OK                              1 /* BUTTON in tree ABOUT_DIALOG */

#define FSLX_LOAD                          0 /* Free string */

#define DI_MEMORY_ERROR                    1 /* Alert string */

#define DI_WDIALOG_ERROR                   2 /* Alert string */

#define DI_VDI_WKS_ERROR                   3 /* Alert string */

#define PROGINFO_FROM                      4 /* Free string */

#define PROGINFO_TITLE                     5 /* Free string */

#define HV_ERR_NOTREE                      6 /* Alert string */

#define WDLG_ABOUT                         7 /* Free string */




#ifdef __STDC__
#ifndef _WORD
#  ifdef WORD
#    define _WORD WORD
#  else
#    define _WORD short
#  endif
#endif
extern _WORD hyptree_rsc_load(_WORD wchar, _WORD hchar);
extern _WORD hyptree_rsc_gaddr(_WORD type, _WORD idx, void *gaddr);
extern _WORD hyptree_rsc_free(void);
#endif
