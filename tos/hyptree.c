#include "tv_defs.h"
#define RSC_NAMED_FUNCTIONS 1
#include "lang/en/hyptree.rsh"
#include <mint/arch/nf_ops.h>
#include "hv_vers.h"

#define PROGRAM_NAME "HypTree"
char const gl_program_name[] = PROGRAM_NAME;
char const gl_program_version[] = HYPVIEW_VERSION;
char const gl_compile_date[12] = __DATE__;

_WORD aes_handle;
OBJECT *about_tree;
OBJECT *menu_tree;

static void tv_init(void)
{
	_WORD dummy;
	_WORD font_w, font_h;
	_WORD fid, pt;
	
	/*
	 * initialize VDI
	 */

	/* set default value for text putput */
	vswr_mode(vdi_handle, MD_TRANS);

	if (vq_gdos())						/* GDOS avaiablable? */
		vst_load_fonts(vdi_handle, 0);	/* load fonts */

	fid = gl_profile.viewer.use_xfont ? gl_profile.viewer.xfont_id : gl_profile.viewer.font_id;
	pt = gl_profile.viewer.use_xfont ? gl_profile.viewer.xfont_pt : gl_profile.viewer.font_pt;
	if (fid == 0)
	{
		fid = aes_fontid;
		pt = aes_fontsize;
	}
	
	vst_font(vdi_handle, fid);		/* select font */
	vst_point(vdi_handle, pt, &font_w, &font_h, &font_cw, &font_ch);

	/* set default alignment */
	vst_alignment(vdi_handle, TA_LEFT, TA_TOP, &dummy, &dummy);

	/* set default fill attributes */
	vsf_color(vdi_handle, G_WHITE);
	vsf_interior(vdi_handle, FIS_SOLID);
	vsf_perimeter(vdi_handle, 0);

	/* set default line attributes */
	vsl_udsty(vdi_handle, 0xAAAA);		/* dotted line */
	vsl_width(vdi_handle, 1);
	vsl_ends(vdi_handle, 0, 0);
	vsl_type(vdi_handle, SOLID);
	vsl_color(vdi_handle, G_BLACK);
}

/*** ---------------------------------------------------------------------- ***/

static void tv_exit(void)
{
	if (vq_gdos() && vdi_handle)
		vst_unload_fonts(vdi_handle, 0);
}

/*** ---------------------------------------------------------------------- ***/

void *hfix_objs(RSHDR *hdr, OBJECT *objects, _WORD num_objs)
{
	UNUSED(hdr);
	UNUSED(objects);
	UNUSED(num_objs);
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

void hrelease_objs(OBJECT *tree, _WORD num_objs)
{
	/* no-op here; will be released above */
	UNUSED(tree);
	UNUSED(num_objs);
}

/*** ---------------------------------------------------------------------- ***/

static void LoadConfig(void)
{
	HypProfile_Load(TRUE);
	/*
	 * as long as there are no configuration dialogs,
	 * throw away the contents of the ini file,
	 * and only keep the settings
	 */
	Profile_Delete(gl_profile.profile);
	gl_profile.profile = NULL;
}


/*** ---------------------------------------------------------------------- ***/

const char *_argv0;


int main(int argc, const char **argv)
{
	WINDOW_DATA *win = NULL;
	const char *applname;
	
	static _DTA mydta;
	Fsetdta(&mydta);
	
	Pdomain(1); /* DOM_MINT */

	gl_apid = appl_init();
	if (gl_apid < 0)
		return 1;
	acc_memsave = !_app && _AESnumapps == 1;

	_mallocChunkSize(0);
	
	_argv0 = argv[0];
	g_tos_get_bindir = g_gem_get_bindir;

	if (DoAesInit() == FALSE)
		return 1;

	about_tree = rs_tree(ABOUT_DIALOG);
	menu_tree = rs_tree(MAINMENU);

	if (DoInitSystem() == FALSE)
	{
		singletos_fail_loop();
		appl_exit();
		return 1;
	}
	
#if USE_MENU
	menu_bar(menu_tree, 1);
#endif

	LoadConfig();						/* load configuration */

	applname = gl_program_name;
	menu_register(-1, applname);
	va_proto_init(applname);

	tv_init();							/* remaining initialization */
	hv_init_colors();

	if (!_app)							/* running as ACC? */
		menu_register(gl_apid, "  " PROGRAM_NAME);	/* ...register to menu */
	
	if (argc <= 1)						/* parameters specified? */
	{
		if (_app)						/* running as program? */
		{
			/* default-Hypertext specified? */
			if (gl_profile.viewer.startup == 1 &&
				(!empty(gl_profile.viewer.default_file) || !empty(gl_profile.viewer.catalog_file)))
			{
				char *filename = path_subst(empty(gl_profile.viewer.default_file) ? gl_profile.viewer.catalog_file : gl_profile.viewer.default_file);
				win = tv_open_window(filename);
				g_free(filename);
			} else if (gl_profile.viewer.startup == 2 &&
				!empty(gl_profile.viewer.last_file))
			{
				char *filename = path_subst(gl_profile.viewer.last_file);
				win = tv_open_window(filename);
				g_free(filename);
			}
		}
	} else
	{
		/* ...load this file */
		win = tv_open_window(argv[1]);
	}
	if (win == NULL && _app)
		SelectFileLoad(NULL);						/* use file selector */

	while (!_app || !doneFlag)
	{
		DoEvent();
		if (quitApp)
			break;
	}

	RemoveItems();

#if USE_MENU
	menu_bar(menu_tree, 0);
#endif

	tv_exit();

	DoExitSystem();

	HypProfile_Delete();
	x_free_resources();

	_crtexit();

	return 0;
}
