/*
 * resource set indices for hypview
 *
 * created by ORCS 2.14
 */

/*
 * Number of Strings:        136
 * Number of Bitblks:        0
 * Number of Iconblks:       0
 * Number of Color Iconblks: 15
 * Number of Color Icons:    29
 * Number of Tedinfos:       21
 * Number of Free Strings:   20
 * Number of Free Images:    0
 * Number of Objects:        89
 * Number of Trees:          8
 * Number of Userblks:       0
 * Number of Images:         0
 * Total file size:          26532
 */

#undef RSC_NAME
#define RSC_NAME "hypview"
#undef RSC_ID
#ifdef hypview
#define RSC_ID hypview
#else
#define RSC_ID 0
#endif

#if !defined(RSC_STATIC_FILE) || !RSC_STATIC_FILE
#define NUM_STRINGS 136
#define NUM_FRSTR 20
#define NUM_UD 0
#define NUM_IMAGES 0
#define NUM_BB 0
#define NUM_FRIMG 0
#define NUM_IB 0
#define NUM_CIB 15
#define NUM_TI 21
#define NUM_OBS 89
#define NUM_TREE 8
#endif



#define DIAL_LIBRARY                       0 /* form/dialog */
#define DI_ICON                            1 /* CICON in tree DIAL_LIBRARY */ /* max len 1 */

#define TOOLBAR                            1 /* form/dialog */
#define TO_BACKGRND                        0 /* BOX in tree TOOLBAR */
#define TO_BUTTONBOX                       1 /* IBOX in tree TOOLBAR */
#define TO_BACK                            2 /* CICON in tree TOOLBAR */ /* max len 4 */
#define TO_HISTORY                         3 /* CICON in tree TOOLBAR */ /* max len 0 */
#define TO_BOOKMARKS                       4 /* CICON in tree TOOLBAR */ /* max len 1 */
#define TO_PREV                            5 /* CICON in tree TOOLBAR */ /* max len 1 */
#define TO_HOME                            6 /* CICON in tree TOOLBAR */ /* max len 1 */
#define TO_NEXT                            7 /* CICON in tree TOOLBAR */ /* max len 1 */
#define TO_INDEX                           8 /* CICON in tree TOOLBAR */ /* max len 1 */
#define TO_CATALOG                         9 /* CICON in tree TOOLBAR */ /* max len 1 */
#define TO_REFERENCES                     10 /* CICON in tree TOOLBAR */ /* max len 1 */
#define TO_HELP                           11 /* CICON in tree TOOLBAR */ /* max len 0 */
#define TO_INFO                           12 /* CICON in tree TOOLBAR */ /* max len 0 */
#define TO_LOAD                           13 /* CICON in tree TOOLBAR */ /* max len 1 */
#define TO_SAVE                           14 /* CICON in tree TOOLBAR */ /* max len 1 */
#define TO_MENU                           15 /* CICON in tree TOOLBAR */ /* max len 1 */
#define TO_SEARCHBOX                      16 /* BOX in tree TOOLBAR */
#define TO_SEARCH                         17 /* FTEXT in tree TOOLBAR */ /* max len 25 */
#define TO_STRNOTFOUND                    18 /* TEXT in tree TOOLBAR */ /* max len 15 */

#define CONTEXT                            2 /* form/dialog */
#define CO_BACK                            1 /* STRING in tree CONTEXT */
#define CO_COPY                            2 /* STRING in tree CONTEXT */
#define CO_PASTE                           3 /* STRING in tree CONTEXT */
#define CO_SELECT_ALL                      5 /* STRING in tree CONTEXT */
#define CO_SAVE                            7 /* STRING in tree CONTEXT */
#define CO_SEARCH                          9 /* STRING in tree CONTEXT */
#define CO_SEARCH_AGAIN                   10 /* STRING in tree CONTEXT */
#define CO_DELETE_STACK                   11 /* STRING in tree CONTEXT */
#define CO_PRINT                          12 /* STRING in tree CONTEXT */

#define EMPTYPOPUP                         3 /* form/dialog */
#define EM_BACK                            0 /* BOX in tree EMPTYPOPUP */
#define EM_1                               1 /* STRING in tree EMPTYPOPUP */
#define EM_2                               2 /* STRING in tree EMPTYPOPUP */
#define EM_3                               3 /* STRING in tree EMPTYPOPUP */
#define EM_4                               4 /* STRING in tree EMPTYPOPUP */
#define EM_5                               5 /* STRING in tree EMPTYPOPUP */
#define EM_6                               6 /* STRING in tree EMPTYPOPUP */
#define EM_7                               7 /* STRING in tree EMPTYPOPUP */
#define EM_8                               8 /* STRING in tree EMPTYPOPUP */
#define EM_9                               9 /* STRING in tree EMPTYPOPUP */
#define EM_10                             10 /* STRING in tree EMPTYPOPUP */
#define EM_11                             11 /* STRING in tree EMPTYPOPUP */
#define EM_12                             12 /* STRING in tree EMPTYPOPUP */

#define PROGINFO                           4 /* form/dialog */
#define PROG_OK                            5 /* BUTTON in tree PROGINFO */
#define PROG_NAME                          6 /* STRING in tree PROGINFO */
#define PROG_FILE                          7 /* TEXT in tree PROGINFO */ /* max len 40 */
#define PROG_DATABASE                      8 /* TEXT in tree PROGINFO */ /* max len 40 */
#define PROG_AUTHOR                        9 /* TEXT in tree PROGINFO */ /* max len 40 */
#define PROG_VERSION                      10 /* TEXT in tree PROGINFO */ /* max len 40 */
#define PROG_DATE                         11 /* STRING in tree PROGINFO */

#define SEARCH_RESULT                      5 /* form/dialog */
#define SR_FSTL_UP                         1 /* BOXCHAR in tree SEARCH_RESULT */
#define SR_FSTL_BACK                       2 /* BOX in tree SEARCH_RESULT */
#define SR_FSTL_WHITE                      3 /* BOX in tree SEARCH_RESULT */
#define SR_FSTL_DOWN                       4 /* BOXCHAR in tree SEARCH_RESULT */
#define SR_BOX                             5 /* IBOX in tree SEARCH_RESULT */
#define SR_FSTL_0                          6 /* TEXT in tree SEARCH_RESULT */ /* max len 70 */
#define SR_FSTL_1                          7 /* TEXT in tree SEARCH_RESULT */ /* max len 70 */
#define SR_FSTL_2                          8 /* TEXT in tree SEARCH_RESULT */ /* max len 70 */
#define SR_FSTL_3                          9 /* TEXT in tree SEARCH_RESULT */ /* max len 70 */
#define SR_FSTL_4                         10 /* TEXT in tree SEARCH_RESULT */ /* max len 70 */
#define SR_FSTL_5                         11 /* TEXT in tree SEARCH_RESULT */ /* max len 70 */
#define SR_FSTL_6                         12 /* TEXT in tree SEARCH_RESULT */ /* max len 70 */
#define SR_FSTL_7                         13 /* TEXT in tree SEARCH_RESULT */ /* max len 70 */
#define SR_FSTL_8                         14 /* TEXT in tree SEARCH_RESULT */ /* max len 70 */
#define SR_FSTL_9                         15 /* TEXT in tree SEARCH_RESULT */ /* max len 70 */
#define SR_ABORT                          16 /* BUTTON in tree SEARCH_RESULT */

#define HYPFIND                            6 /* form/dialog */
#define HYPFIND_STRING                     2 /* FTEXT in tree HYPFIND */ /* max len 30 */
#define HYPFIND_TEXT                       3 /* BUTTON in tree HYPFIND */
#define HYPFIND_PAGES                      4 /* BUTTON in tree HYPFIND */
#define HYPFIND_REF                        5 /* BUTTON in tree HYPFIND */
#define HYPFIND_ABORT                      6 /* BUTTON in tree HYPFIND */
#define HYPFIND_ALL_PAGE                   7 /* BUTTON in tree HYPFIND */
#define HYPFIND_ALL_HYP                    8 /* BUTTON in tree HYPFIND */

#define REFBOX                             7 /* form/dialog */
#define REFBOX_STRING                      2 /* TEXT in tree REFBOX */ /* max len 31 */

#define WARN_FEXIST                        0 /* Alert string */
/* [2][Die Datei existiert schon.|Soll sie ersetzt werden?][Ersetzen|Abbruch] */

#define WARN_ERASEMARK                     1 /* Alert string */
/* [2][Wollen Sie wirklich die Marke|%s|lîschen?][  Ja  | Nein ] */

#define ASK_SETMARK                        2 /* Alert string */
/* [2][Wollen Sie eine Marke auf|%s|setzen?][  Ja  | Nein ] */

#define ASK_SAVEMARKFILE                   3 /* Alert string */
/* [2][Marken speichern?][  Ja  | Nein ] */

#define WARN_NORESULT                      4 /* Alert string */
/* [1][HypView:|<%s>|konnte nicht gefunden werden.][ Abbruch ] */

#define FSLX_LOAD                          5 /* Free string */
/* Bitte Hypertext wÑhlen: */

#define FSLX_SAVE                          6 /* Free string */
/* ASCII Text speichern unter: */

#define WDLG_SEARCH_PATTERN                7 /* Free string */
/* Pattern Suchen... */

#define DI_MEMORY_ERROR                    8 /* Alert string */
/* [1][Konnte Befehl nicht ausfÅhren,|da nicht genÅgend Speicher frei|ist.][Abbruch] */

#define DI_WDIALOG_ERROR                   9 /* Alert string */
/* [1][Bitte starten Sie zuvor die|Systemerweiterung WDIALOG.PRG][Abbruch] */

#define DI_VDI_WKS_ERROR                  10 /* Alert string */
/* [1][Konnte keine VDI-Workstation|îffnen.][Abbruch] */

#define HV_ERR_NO_HOSTNAME                11 /* Alert string */
/* [1][Kein Hostname im Hypertext|angegeben.][Abbruch] */

#define HV_ERR_HOST_NOT_FOUND             12 /* Alert string */
/* [1][Host Applikation nicht gefunden.][Abbruch] */

#define HV_ERR_NOT_IMPLEMENTED            13 /* Alert string */
/* [1][Nicht implementiert.][Abbruch] */

#define DI_WDIALOG_FONTSEL_ERROR          14 /* Alert string */
/* [1][Kann keinen Fontselector|anzeigen. Mîglicherweise nicht|unterstÅtzt?][Abbruch] */

#define PROGINFO_FROM                     15 /* Free string */
/* vom: %s */

#define PROGINFO_TITLE                    16 /* Free string */
/* Programminfo... */

#define HV_ERR_EXEC                       17 /* Alert string */
/* [1][Kann|<%s>|nicht ausfÅhren][Abbruch] */

#define HV_ERR_HYPFIND                    18 /* Alert string */
/* [1][HypFind beendet mit Code %d][Abbruch] */

#define HV_ERR_NO_REMARKER                19 /* Alert string */
/* [1][Kein Pfad fÅr REMARKER konfiguriert|in HYPVIEW.INI.][Abbruch] */




#ifdef __STDC__
#ifndef _WORD
#  ifdef WORD
#    define _WORD WORD
#  else
#    define _WORD short
#  endif
#endif
extern _WORD hypview_rsc_load(void);
extern _WORD hypview_rsc_gaddr(_WORD type, _WORD idx, void *gaddr);
extern _WORD hypview_rsc_free(void);
#endif
