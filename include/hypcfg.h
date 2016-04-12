#ifndef __HYP_CFG_H__
#define __HYP_CFG_H__ 1

#include "profile.h"

typedef struct {
	gboolean changed;
	Profile *profile;
	struct {
		char *path_list;            /* Default search path */
		char *bindir;               /* directory for external tools */
		char *hypfold;              /* default directory */
		char *hcp_path;             /* path to hypertext compiler */
		char *all_ref;              /* The file used to find references */
		char *hypfind_path;         /* Hypfind is called to search for search masks in all pages */
	} general;
	
	struct {
		char *extview;              /* external viewer */
		char *skin_path;            /* custom skin RSC for toolbar */
		char *default_file;         /* default filename if called without parameters */
		char *catalog_file;         /* The hypertext file to be loaded via the 'Catalog' option */
		char *marker_path;          /* Default file to save mark files to */
		int startup;				/* 0=show selector, 1=load default_file, 2=load last file */
		char *last_file;
		unsigned short last_node;
		char *printer;              /* external print application */
#ifdef WITH_GUI_GEM
		int font_id;                /* GDOS font ID */
		int font_pt;                /* Size of font, in points */
		int xfont_id;               /* Alternate font ID */
		int xfont_pt;               /* Alternate font size */
#else
		char *font_name;
		char *xfont_name;
#endif
		gboolean use_xfont;
		int win_x, win_y, win_w, win_h; /* initial window position */
		int text_xoffset;           /* horizontal separation in pixels of the window contents */
		int text_yoffset;           /* vertical separation in pixels of the window contents */
		int binary_columns;         /* # of calumns to show for binary data */
		gboolean expand_spaces;		/* wether to expand multiple spaces when using proportional font */
		int ascii_tab_size;         /* The number of spaces a tab will be expanded to */
		int ascii_break_len;        /* break lines in ASCII files after this many chars */
		gboolean rightback;         /* if set, right mouse button means "BACK" */
		gboolean backwind;          /* if set, right mouse button will top a background window */
		gboolean arrowpatch;        /* work around a AES bug of old TOS versions */
		gboolean adjust_winsize;    /* automatically adjust window size */
		gboolean transparent_pics;  /* display pictures transparent */
		gboolean scale_bitmaps;
		gboolean check_time;        /* watch modification times of files */
		int va_start_newwin;        /* VA_START opens a new window */
		gboolean alink_newwin;		/* ALINKs are opened in a new window */
		gboolean intelligent_fuller; /* Resize window to maximal document size when fuller is clicked */
		gboolean clipbrd_new_window; /* if set, pasting a block opens new window */
		gboolean av_window_cycle;   /* send window cycles to AV-Server */
		gboolean marken_save_ask;   /* ask before saving "marks.dat" */
		gboolean refonly;           /* if set, clicking on 'normal' text does not search for references */
		gboolean norefbox;          /* if set, prevents the Find box with a word in a REF-file, from being displayed */
		gboolean find_casesensitive;
		gboolean find_word;
		gboolean detail_info;
#ifdef WITH_GUI_GEM
		char *applname;
#endif
	} viewer;

	struct {
		char *output_dir;
		HYP_CHARSET output_charset;
		gboolean bracket_links;
		gboolean all_links;
		gboolean output_index;
	} output;
	
	struct {
		char *background;           /* window background color */
		char *text;                 /* Displays text in the specified color */
		int link_effect;            /* Text Effect for references */
		char *link;                 /* Displays references in the specified color */
		char *popup;                /* Displays references to popups in the specified color */
		char *xref;                 /* Displays external references in the specified color */
		char *system;               /* Displays references to {@ system } in the specified color */
		char *rx;                   /* Displays references to {@ rx } in the specified color */
		char *rxs;                  /* Displays references to {@ rxs } in the specified color */
		char *quit;                 /* Displays references to {@ quit } in the specified color */
		char *close;                /* Displays references to {@ close } in the specified color */
		char *ghosted;				/* for "ghosted" effect (attribute @{G}) */
	} colors;
	
	struct {
		char *path;                 /* Path for REMARKER */
		gboolean run_on_startup;    /* Wether to start REMARKER automatically on startup */
	} remarker;
	
	struct {
		char *options;              /* default options passed to HCP compiler */
	} hcp;
	
	struct {
		char *options;              /* options that are used automatically when Hyptree is called */
		int win_x, win_y, win_w, win_h; /* default size of the HypTree window */
		gboolean openall;           /* whether all pages are visible */
		gboolean maclike;           /* position of the triangle in the window */
		char *stg_start;            /* command line sent to HypTree from the viewer */
		char *stg_stop;             /* command line sent to viewer from HypTree */
		gboolean usequotes;			/* quote/dequote VA_START command line */
		int debug;				    /* debug options */
	} hyptree;
	
	struct {
		gboolean short_filenames;   /* Write filenames without extensions */
		gboolean lower_filenames;   /* Convert filenames to lowercase */
	} katmaker;
	
	struct {
		char *options;              /* options that are used automatically when RefCheck is called */
		char *path_list;
	} refcheck;
	
	struct {
		char *database;
		char *subject;
		char *title;
		char *wordchars;
	} hypfind;

	struct {
		char *stool_path;           /* STOOL is used to recompile the Katalog file */
	} tools;
} HypProfile;


extern HypProfile gl_profile;

char *path_subst(const char *path);
char *path_unsubst(const char *path, gboolean subst_hypfold);

void HypProfile_Load(void);
gboolean HypProfile_Save(gboolean report_error);
void HypProfile_Delete(void);
void HypProfile_SetChanged(void);

#endif /* __HYP_CFG_H__ */
