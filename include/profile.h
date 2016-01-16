#ifndef __HYP_PROFILE_H__
#define __HYP_PROFILE_H__

#include <stdint.h>

typedef struct _profile Profile;

#ifndef __PROFILE_IMPLEMENTATION__
ANONYMOUS_STRUCT_DUMMY(_profile)
#endif

typedef struct {
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
		char *hyptop_file;
		char *marker_path;          /* Default file to save mark files to */
		short startup;				/* 0=show selector, 1=load default_file, 2=load last file */
		char *last_file;
		char *printer;              /* external print application */
		short text_color;           /* Displays text in the specified color */
		short link_effect;          /* Text Effect for references */
		short link_color;           /* Displays references in the specified color */
		short xref_color;           /* Displays external references in the specified color */
		short popup_color;          /* Displays references to popups in the specified color */
		short system_color;         /* Displays references to {@ system } in the specified color */
		short rexx_color;           /* Displays references to {@ rx } in the specified color */
		short quit_color;           /* Displays references to {@ quit } in the specified color */
		short background_color;     /* window background color */
#ifdef WITH_GUI_GEM
		short font_id;              /* GDOS font ID */
		short font_pt;              /* Size of font, in points */
		short xfont_id;             /* Alternate font ID */
		short xfont_pt;             /* Alternate font size */
#endif
#if defined(WITH_GUI_GTK) || defined(WITH_GUI_WIN32)
		char *font_name;
		char *xfont_name;
		char *color[16];			/* actual color values for the first 16 GEM colors (any format understood by gdk_color_parse()) */
#endif
		short win_x, win_y, win_w, win_h; /* initial window position */
		short text_offset;          /* vertical separation in pixels of the window contents */
		short binary_columns;       /* # of calumns to show for binary data */
		gboolean expand_spaces;		/* wether to expand multiple spaces when using proportional font */
		short ascii_tab_size;       /* The number of spaces a tab will be expanded to */
		short ascii_break_len;      /* break lines in ASCII files after this many chars */
		gboolean rightback;         /* if set, right mouse button means "BACK" */
		gboolean backwind;          /* if set, right mouse button will top a background window */
		gboolean arrowpatch;        /* work around a AES bug of old TOS versions */
		gboolean adjust_winsize;    /* automatically adjust window size */
		gboolean transparent_pics;  /* display pictures transparent */
		gboolean check_time;        /* watch modification times of files */
		short va_start_newwin;      /* AV_START opens a new window */
		gboolean intelligent_fuller; /* INTELLIGENT_FULLER                              INTELLIGENT_FULLER */
		gboolean clipbrd_new_window; /* if set, pasting a block opens new window */
		gboolean av_window_cycle;   /* send window cycles to AV-Server */
		gboolean marken_save_ask;   /* ask before saving "marks.dat" */
		gboolean refonly;           /* if set, clicking on 'normal' text does not search for references */
		gboolean norefbox;          /* if set, prevents the Find box with a word in a REF-file, from being displayed */
	} viewer;
	
	struct {
		char *path;                 /* Path for REMARKER */
		gboolean run_on_startup;    /* Wether to start REMARKER automatically on startup */
	} remarker;
	
	struct {
		char *options;              /* default options passed to HCP compiler */
	} hcp;
	
	struct {
		char *options;              /* options that are used automatically when Hyptree is called */
		short win_x, win_y, win_w, win_h; /* default size of the HypTree window */
		gboolean openall;           /* whether all pages are visible */
		gboolean maclike;           /* position of the triangle in the window */
		char *stg_start;            /* command line sent to HypTree from the viewer */
		char *stg_stop;             /* command line sent to viewer from HypTree */
		gboolean usequotes;			/* quote/dequote VA_START command line */
		short debug;				/* debug options */
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

/*
 * generic functions
 */

/*
 * like strtod(), but always run in "C" locale
 */
double c_strtod(const char *s, const char **end);
char *c_dtostr(double value);

const char *x_get_tmp_dir(void);
const char *x_get_home_dir(void);
const char *x_get_user_name(void);
const char *x_get_real_name(void);
const char *x_get_user_appdata(void);
const char *x_get_system_appdata(void);

void x_subst_homedir(char **dirp);
void x_free_resources(void);

#ifdef G_PLATFORM_WIN32
char *get_special_folder(int csidl);
#endif


Profile *Profile_New(const char *filename);
void Profile_Delete(Profile *profile);
gboolean Profile_Read(Profile *profile);

Profile *Profile_Load(const char *filename);
gboolean Profile_Save(Profile *profile);
const char *Profile_GetFilename(Profile *profile);

char *Profile_GetSectionNames(Profile *profile, unsigned long *pNumSections);
char *Profile_GetKeyNames(Profile *profile, const char *section, unsigned long *pNumKeys);
gboolean Profile_DeleteKey(Profile *profile, const char *section, const char *key);


gboolean Profile_ReadString(Profile *profile, const char *section, const char *key, char **strp);
gboolean Profile_ReadByte(Profile *profile, const char *section, const char *key, unsigned char *pval);
gboolean Profile_ReadInt(Profile *profile, const char *section, const char *key, short *pval);
gboolean Profile_ReadLong(Profile *profile, const char *section, const char *key, intmax_t *pval);
gboolean Profile_ReadCard(Profile *profile, const char *section, const char *key, unsigned short *pval);
gboolean Profile_ReadLongCard(Profile *profile, const char *section, const char *key, uintmax_t *pval);
gboolean Profile_ReadBool(Profile *profile, const char *section, const char *key, gboolean *pval);
gboolean Profile_ReadChar(Profile *profile, const char *section, const char *key, char *pval);

void Profile_WriteString(Profile *profile, const char *section, const char *key, const char *str);
void Profile_WriteByte(Profile *profile, const char *section, const char *key, unsigned char val);
void Profile_WriteInt(Profile *profile, const char *section, const char *key, short val);
void Profile_WriteCard(Profile *profile, const char *section, const char *key, unsigned short val);
void Profile_WriteLong(Profile *profile, const char *section, const char *key, intmax_t val);
void Profile_WriteLongCard(Profile *profile, const char *section, const char *key, uintmax_t val);
void Profile_WriteBool(Profile *profile, const char *section, const char *key, gboolean val);
void Profile_WriteChar(Profile *profile, const char *section, const char *key, char val);

/*
 * functions specific to the package
 */

extern HypProfile gl_profile;

char *path_subst(const char *path);

void HypProfile_Load(void);
gboolean HypProfile_Save(gboolean report_error);
void HypProfile_Delete(void);
void Profile_ValidateColors(_WORD display_colors);

#endif
