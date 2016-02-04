#include "hv_defs.h"
#include "picture.h"
#define RSC_NAMED_FUNCTIONS 1
#include "lang/en/hypview.rsh"
#include <mint/arch/nf_ops.h>

#define PROGRAM_NAME "HypView"
char const gl_program_name[] = PROGRAM_NAME;
char const gl_compile_date[12] = __DATE__;

struct _viewer_colors viewer_colors;

/* ------------------------------------------------------------------------- */

char *gl_program_version(void)
{
	return g_strdup(HYPVIEW_VERSION);
}

/*** ---------------------------------------------------------------------- ***/

void GetTextSize(_WORD *width, _WORD *height)
{
	_WORD dummy;
	
	graf_handle(width, height, &dummy, &dummy);
}

/*** ---------------------------------------------------------------------- ***/

static unsigned char parse_hex(const char *str)
{
	unsigned char val;
	if (str[0] >= '0' && str[0] <= '9')
		val = str[0] - '0';
	else if (str[0] >= 'a' && str[0] <= 'f')
		val = str[0] - 'a' + 10;
	else if (str[0] >= 'A' && str[0] <= 'F')
		val = str[0] - 'A' + 10;
	else
		val = 0;
	val <<= 4;
	if (str[1] >= '0' && str[1] <= '9')
		val |= str[1] - '0';
	else if (str[1] >= 'a' && str[1] <= 'f')
		val |= str[1] - 'a' + 10;
	else if (str[1] >= 'A' && str[1] <= 'F')
		val |= str[1] - 'A' + 10;
	return val;
}

/*** ---------------------------------------------------------------------- ***/

static void parse_color(const char *name, _WORD rgb[3])
{
	unsigned char val;
	if (name == NULL || *name != '#' || strlen(name) != 7)
	{
		rgb[0] = rgb[1] = rgb[2] = 0;
		return;
	}
	val = parse_hex(name + 1);
	rgb[0] = pic_rgb_to_vdi(val);
	val = parse_hex(name + 3);
	rgb[1] = pic_rgb_to_vdi(val);
	val = parse_hex(name + 5);
	rgb[2] = pic_rgb_to_vdi(val);
}

/*** ---------------------------------------------------------------------- ***/

static _WORD get_color(const char *name)
{
	_WORD rgb[3];
	_WORD color[3];
	_WORD i, display_colors;
	_WORD closest = G_BLACK;
	long closest_dist = 1000l * 1000l + 1000l * 1000l + 1000l * 1000l;
	long dist, diff;
	
	parse_color(name, rgb);
	display_colors = GetNumColors();
	if (display_colors > 256)
		display_colors = 256;
	for (i = 0; i < display_colors; i++)
	{
		vq_color(vdi_handle, i, 1, color);
		diff = color[0] - rgb[0];
		dist = diff * diff;
		diff = color[1] - rgb[1];
		dist += diff * diff;
		diff = color[2] - rgb[2];
		dist += diff * diff;
		if (dist < closest_dist)
		{
			closest_dist = dist;
			closest = i;
		}
	}
	return closest;
}

/*** ---------------------------------------------------------------------- ***/

static void ValidateColors(void)
{
	_WORD display_colors = GetNumColors();
	
	viewer_colors.background = get_color(gl_profile.colors.background);
	viewer_colors.text = get_color(gl_profile.colors.text);
	viewer_colors.link = get_color(gl_profile.colors.link);
	viewer_colors.xref = get_color(gl_profile.colors.xref);
	viewer_colors.popup = get_color(gl_profile.colors.popup);
	viewer_colors.system = get_color(gl_profile.colors.system);
	viewer_colors.rx = get_color(gl_profile.colors.rx);
	viewer_colors.rxs = get_color(gl_profile.colors.rxs);
	viewer_colors.quit = get_color(gl_profile.colors.quit);
	viewer_colors.close = get_color(gl_profile.colors.close);
	viewer_colors.error = get_color("#ff0000"); /* used to display invalid links in hypertext files */
	
	if (viewer_colors.background == viewer_colors.text)
		viewer_colors.background = viewer_colors.text ^ 1;
	if (display_colors < 16)
		viewer_colors.link =
		viewer_colors.popup =
		viewer_colors.xref =
		viewer_colors.system =
		viewer_colors.rx =
		viewer_colors.rxs =
		viewer_colors.quit =
		viewer_colors.close =
		viewer_colors.error = viewer_colors.text;
}

/*** ---------------------------------------------------------------------- ***/

static void LoadConfig(void)
{
	HypProfile_Load();
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
#define g_ttp_get_bindir g_gem_get_bindir
#include "hypmain.h"


int main(int argc, const char **argv)
{
	WINDOW_DATA *win = NULL;
	
	if (DoAesInit() == FALSE)
		return 1;

	if (DoInitSystem() == FALSE)
	{
		singletos_fail_loop();
		return 1;
	}
	
	LoadConfig();						/* load configuration */

	hv_init();							/* remaining initialization */
	ValidateColors();

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
				win = OpenFileInWindow(NULL, filename, hyp_default_main_node_name, HYP_NOINDEX, TRUE, TRUE, FALSE);
				g_free(filename);
			} else if (gl_profile.viewer.startup == 2 &&
				!empty(gl_profile.viewer.last_file))
			{
				char *filename = path_subst(gl_profile.viewer.last_file);
				win = OpenFileInWindow(NULL, filename, hyp_default_main_node_name, HYP_NOINDEX, TRUE, TRUE, FALSE);
				g_free(filename);
			}
		}
	} else
	{
		if (argc == 2 && hyp_guess_filetype(argv[1]) != HYP_FT_HYP)
		{
			win = search_allref(win, argv[1], FALSE);
		} else
		{
			/* ...load this file (incl. chapter) */
			win = OpenFileInWindow(NULL, argv[1], (argc > 2 ? argv[2] : hyp_default_main_node_name), HYP_NOINDEX, TRUE, TRUE, FALSE);
		}
	}
	if (win == NULL && _app)
		SelectFileLoad(NULL);						/* use file selector */

	while (!_app || (!doneFlag && all_list))
	{
		DoEvent();
		if (quitApp)
			break;
	}

	RemoveItems();

	hv_exit();

	DoExitSystem();

	HypProfile_Delete();
	x_free_resources();

	return 0;
}
