/*
 * gemx.h - main header file for new gem-lib extensions
 *
 * This lib contains all GEM extensions of MagiC and NVDI/FSM/Speedo
 *
 */

#ifndef _MT_GEMLIB_X_H_
#define _MT_GEMLIB_X_H_

#include <portaes.h>
#include <portvdi.h>

EXTERN_C_BEG

#ifndef __MTDIALOG
#define __MTDIALOG
typedef struct _dialog { int dummy; } DIALOG;
#endif

/*******************************************************************************
 * The AES extensions of MagiC
 */

#ifndef __EVNT
#define __EVNT
typedef struct
{
	_WORD		mwhich;		/**< Type of events */
	_WORD		mx;			/**< X-coordinate of the mouse cursor */
	_WORD		my;			/**< Y-coordinate of the mouse cursor */
	_WORD		mbutton; 	/**< Pressed mouse button */
	_WORD		kstate;		/**< Status of the 'special' keys (kbshift) */
	_WORD		key;		/**< Scancode of the pressed key */
	_WORD		mclicks; 	/**< Number of mouse clicks */
	_WORD		reserved[9];/**< Reserved */
	_WORD		msg[16]; 	/**< Message-buffer */
} EVNT;
#endif


/*
 * Extensions to the form library (MagiC only)
 */

/** @addtogroup a_form
 *  @{
 */

#ifndef _SCANX
#define _SCANX
typedef struct
{
	char		scancode;
	char		nclicks;
	_WORD		objnr;
} SCANX;
#endif

#ifndef _XDO_INF
#define _XDO_INF
typedef struct
{
	SCANX		*unsh;		/**< Tabellen f�r UnShift-Kombinationen */
	SCANX		*shift;		/**< Tabellen f�r Shift-Kombinationen */
	SCANX		*ctrl;		/**< Tabellen f�r Control-Kombinationen */
	SCANX		*alt; 		/**< Tabellen f�r Alternate-Kombinationen */
	void		*resvd;		/**< reserviert */
} XDO_INF;
#endif

/** parameters for the init callback function (7th parameter of mt_xfrm_popup() )
 */
struct POPUP_INIT_args
{
	OBJECT *tree;
	_WORD scrollpos;
	_WORD nlines;
	void *param;
};

_WORD	form_popup 	(OBJECT *tree, _WORD x, _WORD y);
_WORD	form_wbutton	(OBJECT *fo_btree, _WORD fo_bobject, _WORD fo_bclicks, _WORD *fo_bnxtobj, _WORD whandle);
_WORD	form_wkeybd	(OBJECT *fo_ktree, _WORD fo_kobject, _WORD fo_kobnext, _WORD fo_kchar, _WORD *fo_knxtobject, _WORD *fo_knxtchar, _WORD whandle);
_WORD	form_xdial 	(_WORD fo_diflag, _WORD fo_dilittlx, _WORD fo_dilittly, _WORD fo_dilittlw, _WORD fo_dilittlh, _WORD fo_dibigx, _WORD fo_dibigy, _WORD fo_dibigw, _WORD fo_dibigh, void **flydial);
_WORD	form_xdial_grect (_WORD fo_diflag, const GRECT *fo_dilittl, const GRECT *fo_dibig, void **flydial);
_WORD	form_xdo		(OBJECT *tree, _WORD startob, _WORD *lastcrsr, XDO_INF *tabs, void *flydial);
_WORD	form_xerr	(_LONG errcode, const char *errfile);
_WORD	xfrm_popup 	(OBJECT *tree, _WORD x, _WORD y, _WORD firstscrlob, _WORD lastscrlob, _WORD nlines,
						 void _CDECL (*init)(OBJECT *tree, _WORD scrollpos, _WORD nlines, void *param),
						 void *param, _WORD *lastscrlpos);
/**@}*/

/*
 * Extensions to the object library (MagiC only)
 */

/** @addtogroup a_objc
 *  @{
 */
void	objc_wchange	(OBJECT *tree, _WORD obj, _WORD new_state, GRECT *clip, _WORD whandle);
void	objc_wdraw	(OBJECT *tree, _WORD start, _WORD depth, GRECT *clip, _WORD whandle);
_WORD	objc_wedit	(OBJECT *tree, _WORD obj, _WORD key, _WORD *idx, _WORD kind, _WORD whandle);
_WORD	objc_xedit	(OBJECT *tree, _WORD obj, _WORD key, _WORD *xpos, _WORD subfn, GRECT *r);
/**@}*/


/*
 * fnts_* font selection (MagiC/WDIALOG only)
 */

/** @addtogroup x_fnts
 *  @{
 */

/** opaque structure (internal management structure) */
typedef void *FNT_DIALOG;

/** parameters for UTXT_FN callback functions
 */
struct UTXT_FN_args
{
	_WORD x;
	_WORD y;
	_WORD *clip_rect;
	int32_t id;
	int32_t pt;
	int32_t ratio;
	char *string;
};

typedef void _CDECL (*UTXT_FN)(struct UTXT_FN_args);

typedef struct _fnts_item FNTS_ITEM;

/** FNTS_ITEM data structure */
struct _fnts_item
{
	FNTS_ITEM	*next;		  /**< Pointer to the next font or 0L (end of the list) */
	UTXT_FN		display;	  /**< Pointer to the display function for the user fonts */
	int32_t		id; 		  /**< Font ID, >= 65536 for user fonts */
	_WORD 		index;		  /**< Must be 0, as not a VDI font */
	char		mono;		  /**< Flag for mono-spaced font */
	char		outline;	  /**< Flag for vector font */
	_WORD 		npts;		  /**< Number of predefined point sizes */
	char		*full_name;   /**< Pointer to the complete name */
	char		*family_name; /**< Pointer to the family name */
	char		*style_name;  /**< Pointer to the style name */
	char		*pts; 		  /**< Pointer to field with point sizes */
	int32_t		reserved[4];  /**< Reserved, must be 0 */
};

/* Definition of <font_flags> in fnts_create() */
#define FNTS_BTMP 		1		    /**< Display bitmap fonts */
#define FNTS_OUTL 		2		    /**< Display vector fonts */
#define FNTS_MONO 		4		    /**< Display mono-spaced fonts */
#define FNTS_PROP 		8		    /**< Display proportional fonts */

/* Definition  of <dialog_flags> in fnts_create() */
#define FNTS_3D			1			/**< Display selector in 3D-look */

/* Definition of <button_flags> in fnts_open() */
#define FNTS_SNAME		0x01		/**< Select checkbox for names */
#define FNTS_SSTYLE		0x02		/**< Select checkbox for style */
#define FNTS_SSIZE		0x04		/**< Select checkbox for height  */
#define FNTS_SRATIO		0x08		/**< Select checkbox for width/height ratio */

#define FNTS_CHNAME		0x0100		/**< Display checkbox for names */
#define FNTS_CHSTYLE 	0x0200		/**< Display checkbox for style */
#define FNTS_CHSIZE		0x0400		/**< Display checkbox for height */
#define FNTS_CHRATIO 	0x0800		/**< Display checkbox for width/height ratio */
#define FNTS_RATIO		0x1000		/**< Width/height ratio adjustable */
#define FNTS_BSET 		0x2000		/**< Button "Set" selectable */
#define FNTS_BMARK		0x4000		/**< Button "Mark" selectable */

/* Definition of <button> in fnts_evnt() */
#define FNTS_CANCEL		1
#define FNTS_OK			2
#define FNTS_SET		3
#define FNTS_MARK 		4
#define FNTS_OPT		5
#define FNTS_OPTION		FNTS_OPT	/**< see FNTS_OPT */

_WORD		fnts_add			(FNT_DIALOG *fnt_dialog, FNTS_ITEM *user_fonts);
_WORD		fnts_close		(FNT_DIALOG *fnt_dialog, _WORD *x, _WORD *y);
FNT_DIALOG *fnts_create		(_WORD vdi_handle, _WORD no_fonts, _WORD font_flags, _WORD dialog_flags, const char *sample, const char *opt_button);
_WORD		fnts_delete		(FNT_DIALOG *fnt_dialog, _WORD vdi_handle);
_WORD		fnts_do			(FNT_DIALOG *fnt_dialog, _WORD button_flags, int32_t id_in, int32_t pt_in, int32_t ratio_in, _WORD *check_boxes, int32_t *id, int32_t *pt, int32_t *ratio);
_WORD		fnts_evnt		(FNT_DIALOG *fnt_dialog, EVNT *events, _WORD *button, _WORD *check_boxes, int32_t *id, int32_t *pt, int32_t *ratio);
_WORD		fnts_get_info	(FNT_DIALOG *fnt_dialog, int32_t id, _WORD *mono, _WORD *outline);
_WORD		fnts_get_name	(FNT_DIALOG *fnt_dialog, int32_t id, char *full_name, char *family_name, char *style_name);
_WORD		fnts_get_no_styles (FNT_DIALOG *fnt_dialog, int32_t id);
int32_t		fnts_get_style	(FNT_DIALOG *fnt_dialog, int32_t id, _WORD __index);
_WORD		fnts_open		(FNT_DIALOG *fnt_dialog, _WORD button_flags, _WORD x, _WORD y, int32_t id, int32_t pt, int32_t ratio);
void		fnts_remove		(FNT_DIALOG *fnt_dialog);
_WORD		fnts_update		(FNT_DIALOG *fnt_dialog, _WORD button_flags, int32_t id, int32_t pt, int32_t ratio);
/**@}*/

/*
 * fslx_* file selection (MagiC only)
 */

#ifndef GEMLIB_XATTR
/* purec pctoslib defined __TOS in the file that defines the structure XATTR */
/* sozobonx xdlibs defined _file_h_ or _filesys_h_ in both files where the structure XATTR is defined */
/* in other case (XATTR not defined at this point), we go the old way and use "void" instead */
#  if defined(__TOS) || defined(_file_h_) || defined(_filesys_h_)
#    define GEMLIB_XATTR XATTR
#  else /* struct XATTR defined */
#    define GEMLIB_XATTR void
#  endif /* struct XATTR defined */
#endif /* GEMLIB_XATTR */

/** @addtogroup x_fslx
 *  @{
 */

typedef _WORD _CDECL (*XFSL_FILTER)(char *path, char *name, GEMLIB_XATTR *xattr);

/* Sortiermodi */
#define SORTBYNAME		0
#define SORTBYDATE		1
#define SORTBYSIZE		2
#define SORTBYTYPE		3
#define SORTBYNONE		4
#define SORTDEFAULT		(-1)

/* Flags f�r Dateiauswahl */
#define DOSMODE			1
#define NFOLLOWSLKS		2
#define GETMULTI		8

/* fslx_set_flags */
#define SHOW8P3			1

typedef void *XFSL_DIALOG;

_WORD	fslx_close		(XFSL_DIALOG *fsd);
XFSL_DIALOG *fslx_do			(const char *title, char *path, _WORD pathlen, char *fname, _WORD fnamelen, char *patterns, XFSL_FILTER filter, char *paths, _WORD *sort_mode, _WORD flags, _WORD *button, _WORD *nfiles, char **pattern);
_WORD	fslx_evnt		(XFSL_DIALOG *fsd, EVNT *events, char *path, char *fname, _WORD *button, _WORD *nfiles, _WORD *sort_mode, char **pattern);
_WORD	fslx_getnxtfile	(XFSL_DIALOG *fsd, char *fname);
XFSL_DIALOG *fslx_open		(const char *title, _WORD x, _WORD y, _WORD *handle, char *path, _WORD pathlen, char *fname, _WORD fnamelen, const char *patterns, XFSL_FILTER filter, char *paths, _WORD sort_mode, _WORD flags);
_WORD	fslx_set_flags 	(_WORD flags, _WORD *oldval);
/**@}*/

/*
 * pdlg_* printer configuration dialogs (WDIALOG only)
 */

/** @addtogroup x_pdlg
 *  @{
 */

typedef void *PRN_DIALOG;

typedef struct _prn_tray		PRN_TRAY;
typedef struct _media_size		MEDIA_SIZE;
typedef struct _media_type		MEDIA_TYPE;
typedef struct _prn_mode		PRN_MODE;
typedef struct _prn_entry		PRN_ENTRY;
typedef struct _dither_mode 	DITHER_MODE;
typedef struct _drv_entry		DRV_ENTRY;
typedef struct _pdlg_sub		PDLG_SUB;
typedef struct _prn_settings	PRN_SETTINGS;

/** Description of a feed/output tray */
struct _prn_tray
{
	PRN_TRAY	*next;		/**< Pointer to next output tray description */
	int32_t		tray_id; 	/**< Number of the feed or output tray */
	char		name[32];	/**< Name of the tray */
};

/** Description of a paper format */
struct _media_size
{
	MEDIA_SIZE	*next;		/**< Pointer to next paper format description */
	int32_t		size_id; 	/**< Paper format size ID */
	char		name[32];	/**< Name of the paper format */
};

/** Description of a paper type/print medium */
struct _media_type
{
	MEDIA_TYPE	*next;		/**< Pointer to next print medium description */
	int32_t		type_id; 	/**< Paper format type ID */
	char		name[32];	/**< Name of the paper format */
};

/** Description of a print mode */
struct _prn_mode
{
	PRN_MODE	*next; 				/**< Pointer to the next print mode */
	int32_t		mode_id; 			/**< Mode ID (index within the file) */
	_WORD 		hdpi; 				/**< Horizontal resolution in dpi */
	_WORD 		vdpi; 				/**< Vertical resolution in dpi */
	int32_t		mode_capabilities;	/**< Mode capabilities */
	int32_t		color_capabilities;	/**< Colour capabilities */
	int32_t		dither_flags;		/**< Flags specifying whether the
	                                     corresponding colour mode is accessible
										 with or without dithering */
	MEDIA_TYPE	*paper_types;		/**< Suitable paper types */
	int32_t		reserved;			/**< Reserved */
	char		name[32];			/**< Mode name  */
};

/* sub_flags */
#define	PRN_STD_SUBS	0x0001			/**< Standard-Unterdialoge f�r NVDI-Drucker */
#define	PRN_FSM_SUBS	0x0002			/**< Standard-Unterdialoge f�r FSM-Drucker */
#define	PRN_QD_SUBS 	0x0004			/**< Standard-Unterdialoge f�r QuickDraw-Drucker */

/** old_printer can also be 0L */
typedef int32_t _CDECL (*PRN_SWITCH)(DRV_ENTRY *drivers, PRN_SETTINGS *settings, PRN_ENTRY *old_printer, PRN_ENTRY *new_printer);

/** Device description */
struct _prn_entry
{
	PRN_ENTRY	*next;					/**< Pointer to next device description */
	int32_t		length; 				/**< Structure length */
	int32_t		format; 				/**< Data format */
	int32_t		reserved;				/**< Reserved */
	_WORD 		driver_id;				/**< Driver ID */
	_WORD 		driver_type;			/**< Driver type */
	int32_t		printer_id; 			/**< Printer ID */
	int32_t		printer_capabilities;	/**< Printer capabilities */
	int32_t		reserved1;      /**< reserved */
	int32_t		sub_flags;
	PDLG_SUB	*sub_dialogs;	/**< Pointer to the list of sub-dialogs for this printer */
	PRN_SWITCH	setup_panel;	/**< Initialise sub-dialog at printer change  */
	PRN_SWITCH	close_panel;	/**< Close sub-dialog at printer change */
	PRN_MODE 	*modes;			/**< List of available resolutions */
	MEDIA_SIZE	*papers; 		/**< List of available paper formats */
	PRN_TRAY 	*input_trays;	/**< List of feed trays */
	PRN_TRAY 	*output_trays; 	/**< List of output trays */
	char		name[32];		/**< Name of the printer */
};

struct _dither_mode
{
	DITHER_MODE	*next;
	int32_t		length;			/**< Structure length */
	int32_t		format;			/**< Data format */
	int32_t		reserved;		/**< Reserved */
	int32_t		dither_id;		/**< Dither ID */
	int32_t		color_modes;	/**< Colour depths supported */
	int32_t		reserved1;		/**< Reserved */
	int32_t		reserved2;		/**< Reserved */
	char		name[32];		/**< Name of the dither process */
};

typedef struct
{
	int32_t		magic;			/**< 'pdnf' */
	int32_t		length;			/**< Structure length */
	int32_t		format;			/**< Data format */
	int32_t		reserved;		/**< Reserved */
	_WORD 		driver_id;		/**< Driver number for the VDI */
	_WORD 		driver_type;	/**< Driver type */
	int32_t		reserved1;		/**< Reserved */
	int32_t		reserved2;		/**< Reserved */
	int32_t		reserved3;		/**< Reserved */
	PRN_ENTRY	*printers;		/**< List of printers belonging to the driver */
	DITHER_MODE	*dither_modes; 	/**< List of dither processes supported by the driver */
	int32_t		reserved4;		/**< Reserved */
	int32_t		reserved5;		/**< Reserved */
	int32_t		reserved6;		/**< Reserved */
	int32_t		reserved7;		/**< Reserved */
	int32_t		reserved8;		/**< Reserved */
	int32_t		reserved9;		/**< Reserved */
	char		device[128];	/**< Printer driver output file */
} DRV_INFO;

struct _drv_entry
{
	 DRV_ENTRY	*next;
};

#define	PDLG_CHG_SUB	0x80000000L
#define	PDLG_IS_BUTTON	0x40000000L

#define	PDLG_PREBUTTON	0x20000000L
#define	PDLG_PB_OK		1
#define	PDLG_PB_CANCEL	2
#define	PDLG_PB_DEVICE	3

#define	PDLG_BUT_OK 	(PDLG_PREBUTTON | PDLG_PB_OK)
#define	PDLG_BUT_CNCL	(PDLG_PREBUTTON | PDLG_PB_CANCEL)
#define	PDLG_BUT_DEV	(PDLG_PREBUTTON | PDLG_PB_DEVICE)

typedef int32_t _CDECL (*PDLG_INIT)(PRN_SETTINGS *settings, PDLG_SUB *sub);

/** parameters for PDLG_HNDL callback functions
 */
struct PDLG_HNDL_args
{
	PRN_SETTINGS *settings;
	PDLG_SUB *sub;
	_WORD exit_obj;
};

typedef int32_t _CDECL (*PDLG_HNDL)(struct PDLG_HNDL_args);

typedef int32_t _CDECL (*PDLG_RESET)(PRN_SETTINGS *settings, PDLG_SUB *sub);

/** Sub-dialog for setting device */
struct _pdlg_sub
{
	PDLG_SUB	*next; 			/**< Pointer to the successor in the list */
	int32_t		length;			/**< Structure length */
	int32_t		format;			/**< Data format */
	int32_t		reserved;		/**< Reserved */
	void		*drivers;		/**< Only for internal dialogs */
	_WORD 		option_flags;	/**< Flags, inc. PDLG_PRINTING, PDLG_PREFS */
	_WORD 		sub_id;			/**< Sub-dialog ID, entered for global
                                     sub-dialogs of pdlg_add() */
	void		*dialog; 		/**< Pointer to the structure of the window
                                     dialog or 0L */
	OBJECT		*tree;			/**< Pointer to the assembled object tree */
	_WORD 		index_offset;	/**< Index offset of the sub-dialog */
	_WORD 		reserved1;		/**< Reserved */
	int32_t		reserved2;		/**< Reserved */
	int32_t		reserved3;		/**< Reserved */
	int32_t		reserved4;		/**< Reserved */
	PDLG_INIT	init_dlg;		/**< Initialisation function */
	PDLG_HNDL	do_dlg;			/**< Handling function */
	PDLG_RESET	reset_dlg;		/**< Reset function */
	int32_t		reserved5;		/**< Reserved */
	OBJECT		*sub_icon;		/**< Pointer to the icon for the list box */
	OBJECT		*sub_tree;		/**< Pointer to the object tree of the
                                     sub-dialog */
	int32_t		reserved6;		/**< Reserved */
	int32_t		reserved7;		/**< Reserved */
	int32_t		private1;		/**< Dialog's private information */
	int32_t		private2;		/**< Dialog's private information */
	int32_t		private3;		/**< Dialog's private information */
	int32_t		private4;		/**< Dialog's private information */
};


/*----------------------------------------------------------------------------------------*/
/* einstellbare Farbmodi eines Druckermodus																*/
/*----------------------------------------------------------------------------------------*/
#define	CC_MONO			0x0001		/**< 2 Graut�ne */
#define	CC_4_GREY		0x0002		/**< 4 Graut�ne */
#define	CC_8_GREY		0x0004		/**< 8 Graut�ne */
#define	CC_16_GREY		0x0008		/**< 16 Graut�ne */
#define	CC_256_GREY 	0x0010		/**< 256 Graut�ne */
#define	CC_32K_GREY 	0x0020		/**< 32768 Farben in Graut�ne wandeln */
#define	CC_65K_GREY 	0x0040		/**< 65536 Farben in Graut�ne wandeln */
#define	CC_16M_GREY 	0x0080		/**< 16777216 Farben in Graut�ne wandeln */

#define	CC_2_COLOR		0x0100		/**< 2 Farben */
#define	CC_4_COLOR		0x0200		/**< 4 Farben */
#define	CC_8_COLOR		0x0400		/**< 8 Farben */
#define	CC_16_COLOR 	0x0800		/**< 16 Farben */
#define	CC_256_COLOR	0x1000		/**< 256 Farben */
#define	CC_32K_COLOR	0x2000		/**< 32768 Farben */
#define	CC_65K_COLOR	0x4000		/**< 65536 Farben */
#define	CC_16M_COLOR	0x8000		/**< 16777216 Farben */

#define	NO_CC_BITS		16

/*----------------------------------------------------------------------------------------*/
/* einstellbare Rasterverfahren																				*/
/*----------------------------------------------------------------------------------------*/
#define	DC_NONE			0			/**< keine Rasterverfahren */
#define	DC_FLOYD 		1			/**< einfacher Floyd-Steinberg */
#define	NO_DC_BITS		1

/*----------------------------------------------------------------------------------------*/
/* Druckereigenschaften 																						*/
/*----------------------------------------------------------------------------------------*/
#define	PC_FILE			0x0001		/**< Drucker kann �ber GEMDOS-Dateien angesprochen werden */
#define	PC_SERIAL		0x0002		/**< Drucker kann auf der seriellen Schnittstelle angesteuert werden */
#define	PC_PARALLEL 	0x0004		/**< Drucker kann auf der parallelen Schnittstelle angesteuert werden */
#define	PC_ACSI			0x0008		/**< Drucker kann auf der ACSI-Schnittstelle ausgeben */
#define	PC_SCSI			0x0010		/**< Drucker kann auf der SCSI-Schnittstelle ausgeben */

#define	PC_BACKGROUND	0x0080		/**< Treiber kann im Hintergrund ausdrucken */

#define	PC_SCALING		0x0100		/**< Treiber kann Seite skalieren */
#define	PC_COPIES		0x0200		/**< Treiber kann Kopien einer Seite erstellen */

/*----------------------------------------------------------------------------------------*/
/* Moduseigenschaften																							*/
/*----------------------------------------------------------------------------------------*/
#define	MC_PORTRAIT 	0x0001		/**< Seite kann im Hochformat ausgegeben werden */
#define	MC_LANDSCAPE	0x0002		/**< Seite kann im Querformat ausgegeben werden */
#define	MC_REV_PTRT 	0x0004		/**< Seite kann um 180 Grad gedreht im Hochformat ausgegeben werden */
#define	MC_REV_LNDSCP	0x0008		/**< Seite kann um 180 Grad gedreht im Querformat ausgegeben werden */
#define	MC_ORIENTATION	0x000f

#define	MC_SLCT_CMYK	0x0400		/**< Treiber kann bestimmte Farbebenen ausgeben */
#define	MC_CTRST_BRGHT	0x0800		/**< Treiber kann Kontrast und Helligkeit ver�ndern */

/*----------------------------------------------------------------------------------------*/
/* plane_flags 																									*/
/*----------------------------------------------------------------------------------------*/
#define	PLANE_BLACK 	0x0001
#define	PLANE_YELLOW	0x0002
#define	PLANE_MAGENTA	0x0004
#define	PLANE_CYAN		0x0008


/* <driver_mode> */
#define	DM_BG_PRINTING	0x0001		/**< Flag f�r Hintergrunddruck */

/*----------------------------------------------------------------------------------------*/

/* <page_flags> */
#define  PG_EVEN_PAGES  0x0001  	/**< Only output pages with even page numbers */
#define  PG_ODD_PAGES   0x0002  	/**< Only output pages with odd page numbers */

/* <first_page/last_page> */
#define	PG_MIN_PAGE 	1
#define	PG_MAX_PAGE 	9999

/* <orientation> */
#define  PG_UNKNOWN     0x0000  	/**< Orientation unknown and not adjustable */
#define  PG_PORTRAIT    0x0001  	/**< Output page in portrait format */
#define  PG_LANDSCAPE   0x0002  	/**< Output page in landscape format */

/** printer settings
 *
 *  The following structure items can be read by the application:
 *  - length
 *  - page_flags
 *  - first_page
 *  - last_page
 *  - no_copies
 *  - orientation
 *  - scale
 *  - driver_id
 *  .
 *  All other entries should not be accessed. Data such as the printer
 *  resolution or colour planes, for instance, should not be taken from the
 *  settings structure but requested from the printer at the start of printing
 *  (it is possible, for instance, that the printer driver is forced by a
 *  shortage of memory to reduce the print resolution below the value entered
 *  in PRN_SETTINGS).
 */
struct _prn_settings
{
	int32_t		magic;			/**< 'pset' */
	int32_t		length;			/**< Structure length */
	int32_t		format;			/**< Structure type */
	int32_t		reserved;

	int32_t		page_flags; 	/**< Flags, inc. even pages, odd pages */
	_WORD		first_page; 	/**< First page to be printed */
	_WORD		last_page;		/**< Last page to be printed */
	_WORD		no_copies;		/**< Number of copies */
	_WORD		orientation;	/**< Orientation */
	int32_t		scale;			/**< Scaling: 0x10000L corresponds to 100%  */

	_WORD		driver_id;		/**< VDI device number */
	_WORD		driver_type;	/**< Type of driver set */
	int32_t		driver_mode;	/**< Flags, inc. for background printing */
	int32_t		reserved1;		/**< Reserved */
	int32_t		reserved2;		/**< Reserved */

	int32_t		printer_id; 	/**< Printer number */
	int32_t		mode_id; 		/**< Mode number */
	_WORD		mode_hdpi;		/**< Horizontal resolution in dpi */
	_WORD		mode_vdpi;		/**< Vertical resolution in dpi */
	int32_t		quality_id; 	/**< Print mode (hardware-dependent quality,
                                     e.g. Microweave or Econofast) */

	int32_t		color_mode; 	/**< Colour mode */
	int32_t		plane_flags;	/**< Flags for colour planes to be output
                                     (e.g. cyan only) */
	int32_t		dither_mode;	/**< Dither process */
	int32_t		dither_value;	/**< Parameter for the dither process */

	int32_t		size_id; 		/**< Paper format */
	int32_t		type_id; 		/**< Paper type (normal, glossy) */
	int32_t		input_id;		/**< Paper feed channel */
	int32_t		output_id;		/**< Paper output channel */

	int32_t		contrast;		/**< Contrast: 0x10000L corresponds to the
                                     normal setting */
	int32_t		brightness; 	/**< Brightness: 0x1000L corresponds to the
                                     normal setting */
	int32_t		reserved3;		/**< Reserved */
	int32_t		reserved4;		/**< Reserved */

	int32_t		reserved5;		/**< Reserved */
	int32_t		reserved6;		/**< Reserved */
	int32_t		reserved7;		/**< Reserved */
	int32_t		reserved8;		/**< Reserved */

	char		device[128];	/**< File name to be printed  */

	struct	             		/**< Settings of the Mac printer driver */
	{
		char	inside[120];
	} mac_settings;             /**< Settings of the Mac printer driver */
};


/* <dialog_flags> for pdlg_create() */
#define PDLG_3D			0x0001      /**< Use 3D-look */

/* <option_flags> for pdlg_open/do() */
#define PDLG_PREFS          0x0000  /**< Display settings dialog */
#define PDLG_PRINT          0x0001  /**< Display print dialog */

#define PDLG_ALWAYS_COPIES  0x0010  /**< Always offer No. of copies */
#define PDLG_ALWAYS_ORIENT  0x0020  /**< Always offer landscape format */
#define PDLG_ALWAYS_SCALE   0x0040  /**< Always offer scaling */

#define PDLG_EVENODD        0x0100  /**< Offer option for even and odd pages */

/* <button> for pdlg_evnt()/pdlg_do() */
#define	PDLG_CANCEL 1				/**< "Abbruch" wurde angew�hlt */
#define	PDLG_OK		2				/**< "OK" wurde gedr�ckt */

_WORD		   pdlg_add_printers 		(PRN_DIALOG *prn_dialog, DRV_INFO *drv_info);
_WORD		   pdlg_add_sub_dialogs		(PRN_DIALOG *prn_dialog, PDLG_SUB *sub_dialogs);
_WORD		   pdlg_close	    		(PRN_DIALOG *prn_dialog, _WORD *x, _WORD *y);
PRN_DIALOG *   pdlg_create				(_WORD dialog_flags);
_WORD		   pdlg_delete		    	(PRN_DIALOG *prn_dialog);
_WORD		   pdlg_dflt_settings    	(PRN_DIALOG *prn_dialog, PRN_SETTINGS *settings);
_WORD		   pdlg_do			    	(PRN_DIALOG *prn_dialog, PRN_SETTINGS *settings, const char *document_name, _WORD option_flags);
_WORD		   pdlg_evnt 		    	(PRN_DIALOG *prn_dialog, PRN_SETTINGS *settings, EVNT *events, _WORD *button);
_WORD		   pdlg_free_settings    	(PRN_SETTINGS *settings);
int32_t		   pdlg_get_setsize      	(_WORD *global);
PRN_SETTINGS * pdlg_new_settings		(PRN_DIALOG *prn_dialog);
_WORD		   pdlg_open 			    (PRN_DIALOG *prn_dialog, PRN_SETTINGS *settings, const char *document_name, _WORD option_flags, _WORD x, _WORD y);
_WORD		   pdlg_remove_printers      (PRN_DIALOG *prn_dialog);
_WORD		   pdlg_remove_sub_dialogs   (PRN_DIALOG *prn_dialog);
_WORD		   pdlg_save_default_settings(PRN_DIALOG *prn_dialog, PRN_SETTINGS *settings);
_WORD		   pdlg_update			    (PRN_DIALOG *prn_dialog, const char *document_name);
_WORD		   pdlg_use_settings 	    (PRN_DIALOG *prn_dialog, PRN_SETTINGS *settings);
_WORD		   pdlg_validate_settings    (PRN_DIALOG *prn_dialog, PRN_SETTINGS *settings);
/**@}*/


/******************************************************************************
 * Listbox definitions
 */

/** @addtogroup x_lbox
 *  @{
 */

 /** opaque structure */
typedef void * LIST_BOX;

typedef struct lbox_item LBOX_ITEM;

struct lbox_item
{
	LBOX_ITEM	*next;		/**< Pointer to the next entry in the list */
	_WORD		selected;	/**< Specifies if the object is selected */

	_WORD		data1;		/**< Data for the program... */
	void		*data2; 	/**< Data for the program... */
	void		*data3; 	/**< Data for the program... */

};

/** parameters for SLCT_ITEM callback function */
struct SLCT_ITEM_args
{
	LIST_BOX *box;
	OBJECT *tree;
	struct lbox_item *item;
	void *user_data;
	_WORD obj_index;
	_WORD last_state;
};

/** parameters for SET_ITEM callback function */
struct SET_ITEM_args
{
	LIST_BOX *box;
	OBJECT *tree;
	struct lbox_item *item;
	_WORD obj_index;
	void *user_data;
	GRECT *rect;
	_WORD first;
};

typedef void  _CDECL (*SLCT_ITEM)(struct SLCT_ITEM_args);
typedef _WORD _CDECL (*SET_ITEM)(struct SET_ITEM_args);

#define	LBOX_VERT		1	/**< Listbox with vertical slider */
#define	LBOX_AUTO		2	/**< Auto-scrolling */
#define	LBOX_AUTOSLCT		4	/**< Automatic display during auto-scrolling */
#define	LBOX_REAL		8	/**< Real-time slider */
#define	LBOX_SNGL		16	/**< Only a selectable entry */
#define	LBOX_SHFT		32	/**< Multi-selection with Shift */
#define	LBOX_TOGGLE		64	/**< Toggle status of an entry at selection */
#define	LBOX_2SLDRS		128	/**< Listbox has a horiz. and a vertical slider */

LIST_BOX *	lbox_create (OBJECT *tree, SLCT_ITEM slct, SET_ITEM set,
		    LBOX_ITEM *items, _WORD visible_a, _WORD first_a,
		    const _WORD *ctrl_objs, const _WORD *objs, _WORD flags,
		    _WORD pause_a, void *user_data, DIALOG *dialog,
		    _WORD visible_b, _WORD first_b, _WORD entries_b,
		    _WORD pause_b);
void		lbox_update (LIST_BOX *box, GRECT *rect);
_WORD		lbox_do (LIST_BOX *box, _WORD obj);
_WORD		lbox_delete (LIST_BOX *box);
_WORD		lbox_cnt_items (LIST_BOX *box);
OBJECT *	lbox_get_tree (LIST_BOX *box);
_WORD		lbox_get_visible (LIST_BOX *box);
void *		lbox_get_udata (LIST_BOX *box);
_WORD		lbox_get_afirst (LIST_BOX *box);
_WORD		lbox_get_slct_idx (LIST_BOX *box);
LBOX_ITEM *	lbox_get_items (LIST_BOX *box);
LBOX_ITEM *	lbox_get_item (LIST_BOX *box, _WORD n);
LBOX_ITEM *	lbox_get_slct_item (LIST_BOX *box);
_WORD		lbox_get_idx (LBOX_ITEM *items, LBOX_ITEM *search);
_WORD		lbox_get_bvis (LIST_BOX *box);
_WORD		lbox_get_bentries (LIST_BOX *box);
_WORD		lbox_get_bfirst (LIST_BOX *box);
void		lbox_set_asldr (LIST_BOX *box, _WORD first, GRECT *rect);
void		lbox_set_items (LIST_BOX *box, LBOX_ITEM *items);
void		lbox_free_items (LIST_BOX *box);
void		lbox_free_list (LBOX_ITEM *items);
void		lbox_ascroll_to (LIST_BOX *box, _WORD first, GRECT *box_rect,
		    GRECT *slider_rect);
void		lbox_set_bsldr (LIST_BOX *box, _WORD first, GRECT *rect);
void		lbox_set_bentries (LIST_BOX *box, _WORD entries);
void		lbox_bscroll_to (LIST_BOX *box, _WORD first, GRECT *box_rect,
   		    GRECT *slider_rect);

/* #defines for listboxes with only one slider */
#define	lbox_get_avis			lbox_get_visible	/**< another name for lbox_get_avis */
#define	lbox_get_first			lbox_get_afirst	/**< another name for lbox_get_first */
#define	lbox_set_slider			lbox_set_asldr	/**< another name for lbox_set_slider */
#define	lbox_scroll_to			lbox_ascroll_to	/**< another name for lbox_scroll_to */
/**@}*/

/******************************************************************************
 * Wdialog definitions
 */

/** @addtogroup x_wdlg
 *  @{
 */

/** an opaque structure. One should not access the
    structure directly. The wdlg_xx functions should be used! */

/** parameters of HNDL_OBJ callback functions */
struct HNDL_OBJ_args
{
	DIALOG *dialog;
	EVNT *events;
	_WORD obj;
	_WORD clicks;
	void *data;
};

/** service routine that is called, among others, by wdlg_evnt().
 *
 *  This function may be called if an exit or touchexit
 *  object was clicked on (in that case \p obj is a positive object number)
 *  or when an event has occurred that affects the dialog (in that case
 *  \p obj is negative and contains a corresponding function number such as
 *  HNDL_CLSD, for instance).
 *
 *  If \p obj is an object number (>= 0), then \p events points to
 *  the EVNT structure that was passed by wdlg_evnt().
 *  Otherwise \p events is basically 0L and can  not be used for addressing.
 *
 *  \p clicks contains then number of mouse clicks (if \p obj is an object number)
 *
 *  Here is a list of event (value given in the \p obj parameter):
 *  - HNDL_INIT (-1) : \n
 *	  \p data is the variable passed by wdlg_create.
 *	  If handle_exit() returns 0, wdlg_create() does not
 *	  create a dialog structure (error).
 *	  The variable \p code is passed in \p clicks.
 *  - HNDL_OPEN (-5) : \n
 *    \p data is the variable passed by wdlg_open.
 *    The variable \p code is passed in \p clicks.
 *  - HNDL_CLSD (-3) : \n
 *    \p data is \p user_data. If handle_exit() returns 0,
 *    the dialog will be closed -- wdlg_evnt() returns 0
 *    \p events points to the EVNT structure passed by
 *    wdlg_evnt().
 *  - HNDL_MOVE (-9) : \n
 *    \p data is \p user_data. If handle_exit() returns 0,
 *    the dialog will be closed -- wdlg_evnt() returns 0.
 *    \p events points to the EVNT structure passed by
 *    wdlg_evnt().
 *  - HNDL_TOPW (-10) : \n
 *    \p data is \p user_data. If handle_exit() returns 0,
 *    the dialog will be closed -- wdlg_evnt() returns 0.
 *    \p events points to the EVNT structure passed by
 *    wdlg_evnt().
 *  - HNDL_UNTP (-11) : \n
 *    \p data is \p user_data. If handle_exit() returns 0,
 *    the dialog will be closed -- wdlg_evnt() returns 0.
 *    \p events points to the EVNT structure passed by
 *    wdlg_evnt().
 *  - HNDL_EDIT (-6) : \n
 *    \p data points to a word with the key code.
 *    If handle_exit() returns 1, the key press will be
 *    evaluated, if 0 ignored.
 *    \p events points to the EVNT structure passed by
 *    wdlg_evnt().
 *  - HNDL_EDDN (-7) : \n
 *    \p data points to a word with the key code.
 *    \p events points to the EVNT structure passed by
 *    wdlg_evnt().
 *  - HNDL_EDCH (-8) : \n
 *    \p data points to a word with the object number of
 *    the new editable field.
 *  - HNDL_MESG (-2) : \n
 *    \p data is \p user_data. If handle_exit() returns 0,
 *    the dialog will be closed -- wdlg_evnt() returns 0.
 *    \p events points to the EVNT structure passed by
 *    wdlg_evnt().	\n
 *    HNDL_MESG is only passed if a message code between
 *    20 and 39 was received that is not handled by other
 *    opcodes.
 *    Is required for iconification, for instance.\n
 *    Warning: This opcode is only present from MagiC 4.5
 *    of 18.4.96
 *  .
 *  Of these function numbers one only has to react to HNDL_CLSD. All other
 *  events need only be paid attention to when needed.\n
 *  If handle_exit is called with an unknown function number in \p obj, or
 *  one of the above function numbers is to be ignored, then 1 has to be
 *  returned.
 *
 *  The parameters are passed via the stack and the routine may alter
 *  registers d0-d2/a0-a2.
 */
typedef _WORD _CDECL (*HNDL_OBJ)(struct HNDL_OBJ_args);

DIALOG * wdlg_create			(HNDL_OBJ handle_exit, OBJECT *tree, void *user_data, _WORD code, void *data, _WORD flags);
_WORD	 wdlg_open			(DIALOG *dialog, const char *title, _WORD kind, _WORD x, _WORD y, _WORD code, void *data);
_WORD	 wdlg_close			(DIALOG *dialog, _WORD *x, _WORD *y);
_WORD	 wdlg_delete			(DIALOG *dialog);
_WORD	 wdlg_get_tree		(DIALOG *dialog, OBJECT **tree, GRECT *r);
_WORD	 wdlg_get_edit		(DIALOG *dialog, _WORD *cursor);
void *	 wdlg_get_udata		(DIALOG *dialog);
_WORD	 wdlg_get_handle		(DIALOG *dialog);
_WORD	 wdlg_set_edit		(DIALOG *dialog, _WORD obj);
_WORD	 wdlg_set_tree		(DIALOG *dialog, OBJECT *tree);
_WORD	 wdlg_set_size		(DIALOG *dialog, GRECT *size);
_WORD	 wdlg_set_iconify	(DIALOG *dialog, GRECT *g, const char *title, OBJECT *tree, _WORD obj);
_WORD	 wdlg_set_uniconify	(DIALOG *dialog, GRECT *g, const char *title, OBJECT *tree);
_WORD	 wdlg_evnt			(DIALOG *dialog, EVNT *events );
void	 wdlg_redraw			(DIALOG *dialog, GRECT *rect, _WORD obj, _WORD depth);

/* Definitions for <flags> */
#define  WDLG_BKGD   0x0001           /**< Permit background operation */

/* Function numbers for <obj> with handle_exit(...) */
#define  HNDL_INIT   (-1)          /**< Initialise dialog */
#define  HNDL_MESG   (-2)          /**< Initialise dialog */
#define  HNDL_CLSD   (-3)          /**< Dialog window was closed */
#define  HNDL_OPEN   (-5)          /**< End of dialog initialisation (second  call at end of wdlg_init) */
#define  HNDL_EDIT   (-6)          /**< Test characters for an edit-field */
#define  HNDL_EDDN   (-7)          /**< Character was entered in edit-field */
#define  HNDL_EDCH   (-8)          /**< Edit-field was changed */
#define  HNDL_MOVE   (-9)          /**< Dialog was moved */
#define  HNDL_TOPW   (-10)         /**< Dialog-window has been topped */
#define  HNDL_UNTP   (-11)         /**< Dialog-window is not active */
/**@}*/


/*
 *    Editor extensions for Magic
 */

/** @addtogroup x_edit
 *  @{
 */
typedef void XEDITINFO; /**< opaque data structure use by edit_create() and edit_delete() */
XEDITINFO *	edit_create 	(_WORD *global);
void	edit_delete 		(XEDITINFO *xi);
_WORD	edit_open		(OBJECT *tree, _WORD obj);
void	edit_close		(OBJECT *tree, _WORD obj);
_WORD	edit_cursor 		(OBJECT *tree, _WORD obj, _WORD whdl, _WORD show);
_WORD	edit_evnt		(OBJECT *tree, _WORD obj, _WORD whdl,	EVNT *ev, long *errc);
_WORD	edit_get_buf		(OBJECT *tree, _WORD obj, char **buf, long *buflen, long *txtlen);
_WORD	edit_get_format	(OBJECT *tree, _WORD obj, _WORD *tabwidth, _WORD *autowrap);
_WORD	edit_get_colour	(OBJECT *tree, _WORD obj, _WORD *tcolour, _WORD *bcolour);
/** another name, with "color" instead of "colour" to be consistent with AES/VDI function naming rules */
#define edit_get_color( tree, obj, tcolor, bcolor, aes_global ) edit_get_colour(tree, obj, tcolor, bcolor, aes_global)
_WORD	edit_get_cursor	(OBJECT *tree, _WORD obj, char **cursorpos);
_WORD	edit_get_font	(OBJECT *tree, _WORD obj, _WORD *fontID, _WORD *fontH, _WORD *fontPix, _WORD *mono);
void	edit_set_buf		(OBJECT *tree, _WORD obj, char *buf, long buflen);
void	edit_set_format	(OBJECT *tree, _WORD obj, _WORD tabwidth, _WORD autowrap);
void	edit_set_font	(OBJECT *tree, _WORD obj, _WORD fontID, _WORD fontH, _WORD fontPix, _WORD mono);
void	edit_set_colour	(OBJECT *tree, _WORD obj, _WORD tcolour, _WORD bcolour);
/** another name, with "color" instead of "colour" to be consistent with AES/VDI function naming rules */
#define edit_set_color( tree, obj, tcolor, bcolor, aes_global ) edit_set_colour(tree, obj, tcolor, bcolor, aes_global)
void	edit_set_cursor	(OBJECT *tree, _WORD obj, char *cursorpos);
_WORD	edit_resized		(OBJECT *tree, _WORD obj, _WORD *oldrh, _WORD *newrh);
_WORD	edit_get_dirty	(OBJECT *tree, _WORD obj);
void	edit_set_dirty	(OBJECT *tree, _WORD obj, _WORD dirty);
void	edit_get_sel		(OBJECT *tree, _WORD obj, char **bsel, char **esel);
void	edit_get_pos		(OBJECT *tree, _WORD obj, _WORD *xscroll, long *yscroll, char **cyscroll, char **cursorpos, _WORD *cx, _WORD *cy);
void	edit_set_pos		(OBJECT *tree, _WORD obj, _WORD xscroll, long yscroll, char *cyscroll, char *cursorpos, _WORD cx, _WORD cy);
_WORD	edit_scroll		(OBJECT *tree, _WORD obj, _WORD whdl, long yscroll, _WORD xscroll);
void	edit_get_scrollinfo (OBJECT *tree, _WORD obj, long *nlines, long *yscroll, _WORD *yvis, _WORD *yval, _WORD *ncols, _WORD *xscroll, _WORD *xvis);
/**@}*/

/*******************************************************************************
 * The VDI extentsions of NVDI/FSM/Speedo
 */

/*
 * The following functions requires EdDI version 1.x or higher
 */
void	v_clsbm		(_WORD handle);
void	v_opnbm		(_WORD *work_in, MFDB *bitmap, _WORD *handle, _WORD *work_out);
_WORD	vq_scrninfo	(_WORD handle, _WORD *work_out);


/*
 * The following functions requires NVDI version 2.x or higher
 */
_WORD	v_bez_on	(_WORD handle);
void	v_bez_off	(_WORD handle);
_WORD v_bez_con(_WORD handle, _WORD onoff);
void	v_bez		(_WORD handle, _WORD count, _WORD *xyarr, char *bezarr, _WORD *extent, _WORD *totpts, _WORD *totmoves);
void	v_bez_fill	(_WORD handle, _WORD count, _WORD *xyarr, char *bezarr, _WORD *extent, _WORD *totpts, _WORD *totmoves);
_WORD 	v_bez_qual	(_WORD handle, _WORD percent, _WORD *actual);


/*
 * The following functions requires NVDI version 3.x or higher
 */

/** structure to store information about a font */
#ifndef __XFNT_INFO
#define __XFNT_INFO
typedef struct
{
	int32_t		size;				/**< length of the structure, initialize this entry before
	                                     calling vqt_xfntinfo() */
	_WORD		format;				/**< font format, e.g. 4 for TrueType */
	_WORD		id;					/**< font ID, e.g. 6059 */
	_WORD		index;				/**< index */
	char		font_name[50];		/**< font name, e.g. "Century 725 Italic BT" */
	char		family_name[50];	/**< name of the font family, e.g. "Century725 BT" */
	char		style_name[50];		/**< name of the font style, e.g. "Italic" */
	char		file_name1[200];	/**< name of the first font file,
	                                     e.g. "C:\\FONTS\\TT1059M_.TTF" */
	char		file_name2[200];	/**< name of the 2nd font file */
	char		file_name3[200];	/**< name of the 3rd font file */
	_WORD		pt_cnt;				/**< number of available point sizes (vst_point()),
	                                     e.g. 10 */
	_WORD		pt_sizes[64];		/**< available point sizes,
                                         e.g. { 8, 9, 10, 11, 12, 14, 18, 24, 36, 48 } */
} XFNT_INFO;
#endif

#ifndef fix31_to_point
#define fix31_to_point(a) ((_WORD)((((a) + 32768L) >> 16)))
#define point_to_fix31(a) (((fix31)(a)) << 16)
#endif


void	v_ftext         (VdiHdl, _WORD x, _WORD y, const char  *str);
void	v_ftextn         (VdiHdl, _WORD x, _WORD y, const char  *str, _WORD num);
void	v_ftext16       (VdiHdl, _WORD x, _WORD y, const vdi_wchar_t *wstr);
void	v_ftext16n      (VdiHdl, _WORD x, _WORD y, const vdi_wchar_t *wstr, _WORD num);
void	v_ftext_offset  (VdiHdl, _WORD x, _WORD y,
                               const char  *str,  const _WORD *offset);
void	v_ftext_offset16(VdiHdl, _WORD x, _WORD y,
                               const vdi_wchar_t *wstr, const _WORD *offset);
void	v_getbitmap_info(VdiHdl, _WORD ch, fix31 *advancex, fix31 *advancey,
                               fix31 *xoffset, fix31 *yoffset, _WORD *width,
                               _WORD *height, _WORD **bitmap);
void	v_getoutline    (VdiHdl, _WORD ch, _WORD *xyarray, char *bezarray,
                               _WORD maxverts, _WORD *numverts);

_WORD	vq_devinfo     (VdiHdl, _WORD device, _WORD *dev_exists,
                              char *file_name, char *device_name);
void	vqt_devinfo    (VdiHdl, _WORD device, _WORD *dev_busy,
                              char *file_name, char *device_name);
_WORD	vq_ext_devinfo (VdiHdl, _WORD device, _WORD *dev_exists,
                              char *file_path, char *file_name, char *name);

void	vqt_advance     (VdiHdl, _WORD ch, _WORD *advx, _WORD *advy,
                               _WORD *xrem, _WORD *yrem);
void	vqt_advance32   (VdiHdl, _WORD ch, fix31 *advx, fix31 *advy);
_WORD	vqt_ext_name    (VdiHdl, _WORD __index,
                               char *name, _WORD *font_format, _WORD *flags);
void	vqt_f_extent    (VdiHdl, const char  *str, _WORD extent[]);
void	vqt_f_extentn    (VdiHdl, const char  *str, _WORD num, _WORD extent[]);
void	vqt_f_extent16  (VdiHdl, const vdi_wchar_t *str, _WORD extent[]);
void	vqt_f_extent16n (VdiHdl, const vdi_wchar_t *str, _WORD num, _WORD extent[]);
void	vqt_fontheader  (VdiHdl, char *buffer, char *pathname);
_WORD	vqt_name_and_id (VdiHdl, _WORD font_format,
                               char *font_name, char *ret_name);
void	vqt_pairkern    (VdiHdl, _WORD ch1, _WORD ch2, fix31 *x, fix31 *y);
void	vqt_real_extent (VdiHdl, _WORD x, _WORD y, const char *string, _WORD extent[]);
void	vqt_real_extentn (VdiHdl, _WORD x, _WORD y, const char *string, _WORD num, _WORD extent[]);
void	vqt_trackkern   (VdiHdl, fix31 *x, fix31 *y);
_WORD	vqt_xfntinfo    (VdiHdl, _WORD flags, _WORD id,
                               _WORD __index, XFNT_INFO *info);

_WORD vst_arbpt 	(VdiHdl, _WORD point, _WORD *wchar, _WORD *hchar, _WORD *wcell, _WORD *hcell);
fix31  vst_arbpt32 	(VdiHdl, fix31 point, _WORD *wchar, _WORD *hchar, _WORD *wcell, _WORD *hcell);
void  vst_charmap 	(VdiHdl, _WORD mode);
void  vst_kern	(VdiHdl, _WORD tmode, _WORD pmode, _WORD *tracks, _WORD *pairs);
_WORD vst_name 	(VdiHdl, _WORD font_format, char *font_name, char *ret_name);
_WORD vst_setsize 	(VdiHdl, _WORD point, _WORD *wchar, _WORD *hchar, _WORD *wcell, _WORD *hcell);
fix31  vst_setsize32 	(VdiHdl, fix31 point, _WORD *wchar, _WORD *hchar, _WORD *wcell, _WORD *hcell);
_WORD vst_skew 	(VdiHdl, _WORD skew);
void  vst_track_offset(VdiHdl, fix31 offset, _WORD pairmode, _WORD *tracks, _WORD *pairs);
/* another name for vst_track_offset */
#define vst_kern_info vst_track_offset
void  vst_width	(VdiHdl, _WORD width, _WORD *char_width, _WORD *char_height, _WORD *cell_width, _WORD *cell_height);
void v_killoutline (_WORD handle, fsm_component_t *component);


/*
 * The following functions requires NVDI version 4.x or higher
 */
_WORD	vqt_char_index (_WORD handle, _WORD scr_index, _WORD scr_mode, _WORD dst_mode);
_WORD vst_map_mode   (_WORD handle, _WORD mode);

#define vqt_is_char_available(handle,unicode) \
	(vqt_char_index(handle,unicode,CHARIDX_UNICODE,CHARIDX_DIRECT)!=0xFFFF)

/*
 * The following functions requires NVDI version 5.x or higher
 */

/*----------------------------------------------------------------------------------------*/
/* Function witch use for the printer dialog from WDialog											*/
/*----------------------------------------------------------------------------------------*/

DRV_INFO *v_create_driver_info( _WORD handle, _WORD driver_id );
_WORD v_delete_driver_info( _WORD handle, DRV_INFO *drv_info );
_WORD v_read_default_settings( _WORD handle, PRN_SETTINGS *settings );
_WORD v_write_default_settings( _WORD handle, PRN_SETTINGS *settings );


int32_t		v_color2nearest		(_WORD handle, int32_t color_space, COLOR_ENTRY *color, COLOR_ENTRY *nearest_color);
uint32_t	v_color2value		(_WORD handle, int32_t color_space, COLOR_ENTRY *color);
COLOR_TAB *	v_create_ctab		(_WORD handle, int32_t color_space, uint32_t px_format);
ITAB_REF	v_create_itab		(_WORD handle, COLOR_TAB *ctab, _WORD bits );
uint32_t	v_ctab_idx2value	(_WORD handle, _WORD __index );
_WORD		v_ctab_idx2vdi		(_WORD handle, _WORD __index);
_WORD		v_ctab_vdi2idx		(_WORD handle, _WORD vdi_index);
_WORD		v_delete_ctab		(_WORD handle, COLOR_TAB *ctab);
_WORD		v_delete_itab		(_WORD handle, ITAB_REF itab);
int32_t		v_get_ctab_id		(_WORD handle);
_WORD		v_get_outline		(_WORD handle, _WORD __index, _WORD x_offset, _WORD y_offset, _WORD *pts, char *flags, _WORD max_pts);
_WORD		v_opnprn		(_WORD aes_handle, PRN_SETTINGS *settings, _WORD work_out[]);
_WORD		v_open_bm		(_WORD base_handle, GCBITMAP *bitmap, _WORD color_flags, _WORD unit_flags, _WORD pixel_width, _WORD pixel_height);
_WORD		v_resize_bm		(_WORD handle, _WORD width, _WORD height, int32_t b_width, unsigned char *addr);
void		v_setrgb		(_WORD handle, _WORD type, _WORD r, _WORD g, _WORD b);
int32_t		v_value2color		(_WORD handle, uint32_t value, COLOR_ENTRY *color);
_WORD		vq_ctab			(_WORD handle, int32_t ctab_length, COLOR_TAB *ctab);
int32_t		vq_ctab_entry		(_WORD handle, _WORD __index, COLOR_ENTRY *color);
int32_t		vq_ctab_id		(_WORD handle);
_WORD		vq_dflt_ctab		(_WORD handle, int32_t ctab_length, COLOR_TAB *ctab);
int32_t		vq_hilite_color		(_WORD handle, COLOR_ENTRY *hilite_color);
_WORD		vq_margins		(_WORD handle, _WORD *top_margin, _WORD *bottom_margin, _WORD *left_margin, _WORD *right_margin, _WORD *hdpi, _WORD *vdpi);
int32_t		vq_max_color		(_WORD handle, COLOR_ENTRY *hilite_color);
int32_t		vq_min_color		(_WORD handle, COLOR_ENTRY *hilite_color);
int32_t		vq_prn_scaling		(_WORD handle);
int32_t		vq_px_format		(_WORD handle, uint32_t *px_format);
int32_t		vq_weight_color		(_WORD handle, COLOR_ENTRY *hilite_color);
int32_t		vqf_bg_color		(_WORD handle, COLOR_ENTRY *fg_color);
int32_t		vqf_fg_color		(_WORD handle, COLOR_ENTRY *fg_color);
int32_t		vql_bg_color		(_WORD handle, COLOR_ENTRY *fg_color);
int32_t		vql_fg_color		(_WORD handle, COLOR_ENTRY *fg_color);
int32_t		vqm_bg_color		(_WORD handle, COLOR_ENTRY *fg_color);
int32_t		vqm_fg_color		(_WORD handle, COLOR_ENTRY *fg_color);
int32_t		vqr_bg_color		(_WORD handle, COLOR_ENTRY *fg_color);
int32_t		vqr_fg_color		(_WORD handle, COLOR_ENTRY *fg_color);
int32_t		vqt_bg_color		(_WORD handle, COLOR_ENTRY *fg_color);
int32_t		vqt_fg_color		(_WORD handle, COLOR_ENTRY *fg_color);
void		vr_transfer_bits	(_WORD handle, GCBITMAP *src_bm, GCBITMAP *dst_bm, const _WORD *src_rect, const _WORD *dst_rect, _WORD mode);
_WORD		vs_ctab			(_WORD handle, COLOR_TAB *ctab);
_WORD		vs_ctab_entry		(_WORD handle, _WORD __index, int32_t color_space, COLOR_ENTRY *color);
_WORD		vs_dflt_ctab		(_WORD handle);
_WORD		vs_document_info	(_WORD vdi_handle, _WORD type, const char *s, _WORD wchar);
_WORD		vs_hilite_color		(_WORD handle, int32_t color_space, COLOR_ENTRY *hilite_color);
_WORD		vs_max_color		(_WORD handle, int32_t color_space, COLOR_ENTRY *min_color);
_WORD		vs_min_color		(_WORD handle, int32_t color_space, COLOR_ENTRY *min_color);
_WORD		vs_weight_color		(_WORD handle, int32_t color_space, COLOR_ENTRY *weight_color);
_WORD		vsf_bg_color		(_WORD handle, int32_t color_space, COLOR_ENTRY *bg_color);
_WORD		vsf_fg_color		(_WORD handle, int32_t color_space, COLOR_ENTRY *fg_color);
_WORD		vsl_bg_color		(_WORD handle, int32_t color_space, COLOR_ENTRY *bg_color);
_WORD		vsl_fg_color		(_WORD handle, int32_t color_space, COLOR_ENTRY *fg_color);
_WORD		vsm_bg_color		(_WORD handle, int32_t color_space, COLOR_ENTRY *bg_color);
_WORD		vsm_fg_color		(_WORD handle, int32_t color_space, COLOR_ENTRY *fg_color);
_WORD		vsr_bg_color		(_WORD handle, int32_t color_space, COLOR_ENTRY *bg_color);
_WORD		vsr_fg_color		(_WORD handle, int32_t color_space, COLOR_ENTRY *fg_color);
_WORD		vst_bg_color		(_WORD handle, int32_t color_space, COLOR_ENTRY *bg_color);
_WORD		vst_fg_color		(_WORD handle, int32_t color_space, COLOR_ENTRY *fg_color);

_WORD vr_clip_rects_by_src ( _WORD handle, const RECT16 *src_clip_rect, const RECT16 *src_rect, const RECT16 *dst_rect, RECT16 *clipped_src_rect, RECT16 *clipped_dst_rect );
_WORD vr_clip_rects_by_dst ( _WORD handle, const RECT16 *src_clip_rect, const RECT16 *src_rect, const RECT16 *dst_rect, RECT16 *clipped_src_rect, RECT16 *clipped_dst_rect );
_WORD vr_clip_rects32_by_src ( _WORD handle, const RECT32 *src_clip_rect, const RECT32 *src_rect, const RECT32 *dst_rect, RECT32 *clipped_src_rect, RECT32 *clipped_dst_rect );
_WORD vr_clip_rects32_by_dst ( _WORD handle, const RECT32 *src_clip_rect, const RECT32 *src_rect, const RECT32 *dst_rect, RECT32 *clipped_src_rect, RECT32 *clipped_dst_rect );

EXTERN_C_END

#endif /* _MT_GEMLIB_X_H_ */
