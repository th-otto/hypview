#include "config.h"
#include <stdint.h>
#include <gem.h>
#include <getopt.h>
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>
#include "portvdi.h"
#include "fileio.h"
#include "rsc.h"
#include "nls.h"
#include "ws.h"
#include "debug.h"
#ifdef _WIN32
#include <direct.h>
#endif
#include "../vdi/writepng.h" /* for writepng_version_info */

#define RSCVIEW_VERSION "1.05"
#define RSCVIEW_DATE "25.07.2018"

char const gl_program_name[] = "rscview";
char const gl_program_version[] = RSCVIEW_VERSION;
char const gl_program_date[] = RSCVIEW_DATE;

/*
 * gui variables
 */
static _WORD gl_hchar;
static _WORD gl_wchar;
static _WORD gl_wbox;
static _WORD gl_hbox;							/* system sizes */
static GRECT desk;
static _WORD phys_handle;						/* physical workstation handle */
static _WORD vdi_handle;						/* virtual screen handle */
static WS ws;
static _WORD xworkout[57];

/*
 * program options
 */
static _BOOL verbose = FALSE;
static _BOOL use_timestamps = FALSE;
static const char *pngdir;
static const char *htmlout_name;
static const char *html_dir;
static FILE *htmlout_file;
static _BOOL gen_imagemap;

#define ARRAY_SIZE(array) ((int)(sizeof(array)/sizeof(array[0])))

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

__attribute__((format(printf, 1, 2)))
static void warn(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	errout(_("Warning: "));
	erroutv(fmt, ap);
	errout("\n");
	va_end(ap);
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static void open_screen(void)
{
	int i;
	_WORD pxy[8];
	_WORD workin[11];

	vdi_handle = phys_handle;
	for (i = 0; i < 10; i++)
		workin[i] = 1;
	workin[10] = 2;
	v_opnvwk(workin, &vdi_handle, &ws.ws_xres);
	vq_extnd(vdi_handle, 1, xworkout);
	vsf_interior(vdi_handle, FIS_SOLID);
	vsf_perimeter(vdi_handle, FALSE);
	vswr_mode(vdi_handle, MD_REPLACE);
	vsf_color(vdi_handle, G_GREEN);
	
	pxy[0] = 0;
	pxy[1] = 0;
	pxy[2] = ws.ws_xres;
	pxy[3] = ws.ws_yres;
	vr_recfl(vdi_handle, pxy);

	vsf_color(vdi_handle, G_WHITE);
}

/* ------------------------------------------------------------------------- */

static void close_screen(void)
{
	v_clsvwk(vdi_handle);
}

/* ------------------------------------------------------------------------- */

static void clear_screen(char *title)
{
	static char empty[1] = { 0 };
	static TEDINFO tedinfo = {
		NULL, empty, empty, IBM, 1, TE_CNTR, 0x1100, 0x0, 1, 2,1
	};
	static OBJECT desktop[] = {
		{ NIL, 1, 2, G_BOX, OF_NONE, OS_NORMAL, { OBSPEC_MAKE(0, 0, G_BLACK, G_BLACK, TRUE, IP_SOLID, G_GREEN) }, 0, 0, 0, 0 },
		{ 2, NIL, NIL, G_BOX, OF_NONE, OS_NORMAL, { OBSPEC_MAKE(0, -1, G_BLACK, G_BLACK, FALSE, IP_HOLLOW, G_WHITE) }, 0, 0, 0, 0 },
		{ 0, NIL, NIL, G_TEXT, OF_LASTOB, OS_NORMAL, { (_LONG_PTR)&tedinfo }, 0, 0, 0, 0 },
	};
	int i;
	
	for (i = 0; i < 3; i++)
		desktop[i].ob_width = desk.g_x + desk.g_w;
	desktop[ROOT].ob_height = desk.g_y + desk.g_h;
	desktop[1].ob_height = gl_hchar + 2;
	desktop[2].ob_height = gl_hchar + 3;
	tedinfo.te_ptext = title;
	
	objc_draw(desktop, ROOT, MAX_DEPTH, 0, 0, desk.g_x + desk.g_w, desk.g_y + desk.g_h);
}

/* ------------------------------------------------------------------------- */

static void str_lwr(char *name)
{
	while (*name)
	{
		*name = tolower(*name);
		name++;
	}
}

/* ------------------------------------------------------------------------- */

static void generate_imagemap(RSCTREE *tree)
{
	OBJECT *obj;
	_WORD j;
	GRECT gr;
	_WORD dx, dy;
	
	fprintf(htmlout_file, "<map name=\"%s\">\n", tree->rt_name);
	if (tree->rt_type == RT_MENU)
	{
		obj = tree->rt_objects.menu.mn_tree;
		objc_offset(obj, ROOT, &gr.g_x, &gr.g_y);
		gr.g_w = obj[ROOT].ob_width;
		gr.g_h = obj[ROOT].ob_height;
		dx = dy = 0;
	} else
	{
		obj = tree->rt_objects.dial.di_tree;
		form_center_grect(obj, &gr);
		objc_offset(obj, ROOT, &dx, &dy);
		dx = dx - gr.g_x;
		dy = dy - gr.g_y;
	}
	
	/*
	 * if area definitions overlap, the first one will be used,
	 * so we have to output them in reverse order
	 */
	for (j = 0; !(obj[j].ob_flags & OF_LASTOB); j++)
	{
	}
	
	while (j >= 0)
	{
		const char *name;
		_WORD x, y, w, h;
		_WORD type;
		char sbuf[128];
		
		objc_offset(obj, j, &x, &y);
		x -= gr.g_x;
		y -= gr.g_y;
		w = obj[j].ob_width;
		h = obj[j].ob_height;
		type = obj[j].ob_type & OBTYPEMASK;
		/* if object #1 is a dialog title, recalc the width */
		if (tree->rt_file->rsc_emutos != EMUTOS_NONE && j == 1 && type == G_STRING && obj[j].ob_y == gl_hchar)
			w = strlen(obj[j].ob_spec.free_string) * gl_wchar;
		fprintf(htmlout_file, "<area shape=\"rect\" coords=\"%d,%d,%d,%d\" title=\"Object #%d",
			x, y,
			x + w - 1, y + h - 1,
			j);
		name = ob_name(tree->rt_file, tree, j);
		if (name)
		{
			fprintf(htmlout_file, "&#10;%s", name);
		}
		x = x - dx;
		y = y - dy;
		fprintf(htmlout_file, "&#10;type = %s", type_name(type));
		fprintf(htmlout_file, "&#10;x = %d", x / gl_wchar);
		if (x % gl_wchar != 0)
			fprintf(htmlout_file, " + %d", x % gl_wchar);
		fprintf(htmlout_file, "&#10;y = %d", y / gl_hchar);
		if (y % gl_hchar != 0)
			fprintf(htmlout_file, " + %d", y % gl_wchar);
		fprintf(htmlout_file, "&#10;w = %d", w / gl_wchar);
		if (w % gl_wchar != 0)
			fprintf(htmlout_file, " + %d", w % gl_wchar);
		fprintf(htmlout_file, "&#10;h = %d", h / gl_hchar);
		if (h % gl_hchar != 0)
			fprintf(htmlout_file, " + %d", h % gl_wchar);
		fprintf(htmlout_file, "&#10;flags = %s", flags_name(sbuf, obj[j].ob_flags, tree->rt_file->rsc_emutos));
		fprintf(htmlout_file, "&#10;state = %s", state_name(sbuf, obj[j].ob_state));
		
		fprintf(htmlout_file, "\" />\n");
		j--;
	}
	fprintf(htmlout_file, "</map>\n");
}

/* ------------------------------------------------------------------------- */

static _WORD write_png(RSCTREE *tree, _WORD x, _WORD y, _WORD w, _WORD h, _BOOL write_imagemap)
{
	_WORD pxy[4];
	char basename[PATH_MAX];
	char filename[PATH_MAX];
	_WORD err;
	char *p;
	
	if (verbose)
		printf("%s %ld %s: %dx%d\n", rtype_name(tree->rt_type), tree->rt_number, tree->rt_name, w, h);
	pxy[0] = x;
	pxy[1] = y;
	pxy[2] = x + w - 1;
	pxy[3] = y + h - 1;
	vs_clip(vdi_handle, 1, pxy);
	if (use_timestamps)
	{
		time_t t = time(NULL);
		struct tm *tp = gmtime(&t);
		sprintf(basename, "%s_%04d%02d%02d%02d%02d%02d",
			tree->rt_name,
			tp->tm_year + 1900,
			tp->tm_mon + 1,
			tp->tm_mday,
			tp->tm_hour,
			tp->tm_min,
			tp->tm_sec);
	} else
	{
		strcpy(basename, tree->rt_name);
	}
	str_lwr(basename);
	p = filename;
	if (pngdir)
	{
		int len;
		
#ifdef _WIN32
		(void) _mkdir(pngdir);
#else
		(void) mkdir(pngdir, 0755);
#endif
		strcpy(p, pngdir);
		len = strlen(p);
		p += len;
		if (len > 0 && p[-1] != '/')
			*p++ = '/';
	}
	sprintf(p, "%03ld_%s.png", tree->rt_number, basename);
	err = v_write_png(vdi_handle, filename);
	if (err != 0)
	{
		KINFO(("write_png: %s: %s\n", filename, strerror(err)));
	}
	if (htmlout_file)
	{
		fprintf(htmlout_file, "<p>%s:<br /><img src=\"%s/%s\" alt=\"%s\"",
			tree->rt_name,
			html_dir ? html_dir : ".", p,
			tree->rt_name);
		if (write_imagemap)
		{
			fprintf(htmlout_file, " usemap=\"#%s\"", tree->rt_name);
		}
		fprintf(htmlout_file, " /></p>\n");
		if (write_imagemap)
		{
			generate_imagemap(tree);
		}
	}
	return err;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static _BOOL draw_dialog(RSCTREE *tree)
{
	OBJECT *ob;
	GRECT gr;
	_WORD err;
	
	ob = tree->rt_objects.dial.di_tree;
	if (ob == NULL)
		return FALSE;
	form_center_grect(ob, &gr);

	wind_update(BEG_UPDATE);
	form_dial_grect(FMD_START, &gr, &gr);
	
	clear_screen(tree->rt_name);
	
	objc_draw_grect(ob, ROOT, MAX_DEPTH, &gr);
	
	err = write_png(tree, gr.g_x, gr.g_y, gr.g_w, gr.g_h, gen_imagemap);

	form_dial_grect(FMD_FINISH, &gr, &gr);
	wind_update(END_UPDATE);

	return err == 0;
}

/* ------------------------------------------------------------------------- */

static _BOOL draw_menu(RSCTREE *tree)
{
	OBJECT *ob;
	_WORD thebar;
	_WORD theactive;
	_WORD themenus;
	_WORD title, menubox;
	_WORD x;
	GRECT gr;
	_WORD err;
	_WORD maxx, maxy;
	
	ob = tree->rt_objects.menu.mn_tree;
	if (ob == NULL)
		return FALSE;
	/*
	 * set the width of the root object, the titlebar, and the background box
	 * to screen width
	 */
	ob[ROOT].ob_width = ob[menu_the_bar(ob)].ob_width = ob[menu_the_menus(ob)].ob_width = desk.g_x + desk.g_w;
	/*
	 * adjust the height of the root object and the
	 * background box to fill up the screen
	 */
	ob[ROOT].ob_height = desk.g_y + desk.g_h;
	ob[menu_the_menus(ob)].ob_height = ob[ROOT].ob_height - ob[menu_the_menus(ob)].ob_y;

	objc_offset(ob, ROOT, &gr.g_x, &gr.g_y);
	gr.g_w = ob[ROOT].ob_width;
	gr.g_h = ob[ROOT].ob_height;

	wind_update(BEG_UPDATE);
	
	/*
	 * draw the menu titles
	 */
	clear_screen(tree->rt_name);
	menu_bar(ob, TRUE);

	/*
	 * reposition the submenu boxes so that they don't overlap
	 */
	thebar = menu_the_bar(ob);
	if (thebar == NIL)
		return FALSE;
	themenus = ob[thebar].ob_next;
	if (themenus == thebar)
		return FALSE;
	theactive = ob[thebar].ob_head;
	if (theactive == NIL)
		return FALSE;
	title = ob[theactive].ob_head;
	if (title == NIL)
		return FALSE;
	menubox = ob[themenus].ob_head;
	if (menubox == NIL)
		return FALSE;
	x = ob[menubox].ob_x;
	do
	{
		ob[menubox].ob_x = x;
		/* ob[title].ob_x = x + gl_wchar; */
		x += ob[menubox].ob_width + 1;
		title = ob[title].ob_next;
		menubox = ob[menubox].ob_next;
	} while (title != theactive && menubox != themenus);
	
	/*
	 * draw the boxes
	 */
	maxx = maxy = 0;
	menubox = ob[themenus].ob_head;
	do
	{
		_WORD mx, my, mw, mh;
		
		objc_offset(ob, menubox, &mx, &my);
		mw = ob[menubox].ob_width;
		mh = ob[menubox].ob_height;
		mx -= 1;
		my -= 1;
		mw += 2;
		mh += 2;
		objc_draw(ob, menubox, MAX_DEPTH, mx, my, mw, mh);
		menubox = ob[menubox].ob_next;
		mx = mx + mw;
		my = my + mh;
		if (mx > maxx)
			maxx = mx;
		if (my > maxy)
			maxy = my;
	} while (menubox != themenus);
	
	err = write_png(tree, 0, 0, maxx, maxy, gen_imagemap);

	menu_bar(ob, FALSE);
	form_dial_grect(FMD_FINISH, &gr, &gr);
	wind_update(END_UPDATE);
	
	return err == 0;
}

/* ------------------------------------------------------------------------- */

static _BOOL draw_string(RSCTREE *tree)
{
	const char *str;
	_WORD err;
	GRECT gr;
	TEDINFO ted = { NULL, NULL, NULL, IBM, 0, TE_CNTR, COLSPEC_MAKE(G_BLACK, G_BLACK, TEXT_OPAQUE, 0, G_WHITE), 0, 0, 0, 0 };
	OBJECT string[1] = { { NIL, NIL, NIL, G_TEXT, OF_LASTOB, OS_NORMAL, { 0 }, 0, 0, 0, 0 } };
	_WORD len;
	
	str = tree->rt_objects.str.fr_str;
	if (str == NULL)
		return FALSE;
	
	ted.te_ptext = (char *)NO_CONST(str);
	string[0].ob_spec.tedinfo = &ted;
	len = strlen(str);
	string[0].ob_width = len * gl_wchar;
	string[0].ob_height = gl_hchar;
	form_center(string, &string[ROOT].ob_x, &string[ROOT].ob_y, &gr.g_w, &gr.g_h);
	gr.g_x = string[ROOT].ob_x;
	gr.g_y = string[ROOT].ob_y;
	
	clear_screen(tree->rt_name);
	objc_draw_grect(string, ROOT, MAX_DEPTH, &gr);

	err = write_png(tree, gr.g_x, gr.g_y, gr.g_w, gr.g_h, FALSE);

	form_dial_grect(FMD_FINISH, &gr, &gr);
	
	return err == 0;
}

/* ------------------------------------------------------------------------- */

static _BOOL draw_alert(RSCTREE *tree)
{
	const char *str;
	_WORD err;
	_WORD wo[57];
	GRECT gr;
	
	str = tree->rt_objects.alert.al_str;
	if (str == NULL)
		return FALSE;
	
	clear_screen(tree->rt_name);
	/*
	 * call our special version that only displays the dialog,
	 * and does not restore the screen background.
	 */
	form_alert_ex(1, str, 1 | (tree->rt_file->rsc_emutos != EMUTOS_NONE ? 2 : 0));
	/*
	 * get clipping rect
	 */
	vq_extnd(phys_handle, 1, wo);
	gr.g_x = wo[45];
	gr.g_y = wo[46];
	gr.g_w = wo[47] - gr.g_x + 1;
	gr.g_h = wo[48] - gr.g_y + 1;
	
	err = write_png(tree, gr.g_x, gr.g_y, gr.g_w, gr.g_h, FALSE);

	form_dial_grect(FMD_FINISH, &gr, &gr);
	
	return err == 0;
}

/* ------------------------------------------------------------------------- */

static _BOOL draw_image(RSCTREE *tree)
{
	_WORD err;
	BITBLK *bit;
	_WORD *data;
	_WORD width;
	_WORD height;
	GRECT gr;
	_WORD pxy[8];
	_WORD colors[2];
	MFDB src, dst;
	
	bit = tree->rt_objects.bit;
	data = bit->bi_pdata;
	width = bit->bi_wb * 8;
	height = bit->bi_hl;
	if (is_mouseform(bit))
	{
		data += 5;
		height -= 5;
	}

	clear_screen(tree->rt_name);

	gr.g_x = (desk.g_w - width) / 2 + desk.g_x;
	gr.g_y = (desk.g_h - height) / 2 + desk.g_y;
	gr.g_w = width;
	gr.g_h = height;
	
	pxy[0] = gr.g_x;
	pxy[1] = gr.g_y;
	pxy[2] = gr.g_x + gr.g_w - 1;
	pxy[3] = gr.g_y + gr.g_h - 1;
	vs_clip(vdi_handle, 1, pxy);
	vswr_mode(vdi_handle, MD_REPLACE);
	vsf_color(vdi_handle, G_WHITE);
	vr_recfl(vdi_handle, pxy);

	pxy[0] = 0;
	pxy[1] = 0;
	pxy[2] = gr.g_w - 1;
	pxy[3] = gr.g_h - 1;
	pxy[4] = gr.g_x;
	pxy[5] = gr.g_y;
	pxy[6] = gr.g_x + gr.g_w - 1;
	pxy[7] = gr.g_y + gr.g_h - 1;

	src.fd_w = width;
	src.fd_h = height;
	src.fd_nplanes = 1;
	src.fd_wdwidth = (src.fd_w + 15) >> 4;
	src.fd_stand = FALSE;
	src.fd_addr = data;
	
	dst.fd_w = ws.ws_xres + 1;
	dst.fd_h = ws.ws_yres + 1;
	dst.fd_nplanes = xworkout[4];
	dst.fd_wdwidth = (dst.fd_w + 15) >> 4;
	dst.fd_stand = FALSE;
	dst.fd_addr = 0;
	
	colors[0] = G_BLACK;
	colors[1] = G_WHITE;
	vrt_cpyfm(vdi_handle, MD_TRANS, pxy, &src, &dst, colors);
	
	err = write_png(tree, gr.g_x, gr.g_y, gr.g_w, gr.g_h, FALSE);

	return err == 0;
}

/* ------------------------------------------------------------------------- */

static _BOOL draw_all_trees(RSCFILE *file)
{
	RSCTREE *tree;
	_BOOL ret = TRUE;
	
	FOR_ALL_RSC(file, tree)
	{
		switch (tree->rt_type)
		{
		case RT_DIALOG:
		case RT_FREE:
		case RT_UNKNOWN:
			ret &= draw_dialog(tree);
			break;
		case RT_MENU:
			ret &= draw_menu(tree);
			break;
		case RT_FRSTR:
			ret &= draw_string(tree);
			break;
		case RT_ALERT:
			ret &= draw_alert(tree);
			break;
		case RT_FRIMG:
		case RT_MOUSE:
			ret &= draw_image(tree);
			break;
		case RT_BUBBLEMORE:
		case RT_BUBBLEUSER:
			break;
		}
	}
	return ret;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/*
 * charsets
 */

struct charset_alias
{
	const char *name;
	int id;
};

static struct charset_alias const charsets[] = {
 	{ "ISO-8859-1", CHARSET_L1 },
 	{ "latin1", CHARSET_L1 },
	{ "ISO-8859-2", CHARSET_L2 },
	{ "latin2", CHARSET_L2 },
	{ "ISO-8859-15", CHARSET_L9 },
	{ "latin9", CHARSET_L9 },
	{ "russian-atarist", CHARSET_RU },
	{ "cp737", CHARSET_GR },
	{ "atarist", CHARSET_ST }
};


/* resolve any known alias */
static int po_get_charset_id(const char *name)
{
	int i;
	int n = ARRAY_SIZE(charsets);

	for (i = 0; i < n; i++)
	{
		if (strcmp(charsets[i].name, name) == 0)
		{
			return charsets[i].id;
		}
	}
	warn(_("unknown charset name %s"), name);
	errout(_("known charsets are:\n"));
	for (i = 0; i < n; i++)
	{
		errout("  %s\n", charsets[i].name);
	}
	return -1;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

enum rscview_opt {
	OPT_VERBOSE = 'v',
	OPT_PNGDIR = 'P',
	OPT_CHARSET = 'c',
	OPT_VERSION = 'V',
	OPT_HELP = 'h',
	
	OPT_SETVAR = 0,
	OPT_OPTERROR = '?',
	
	OPT_CREATE_HTML = 256,
	OPT_HTML_DIR,
	OPT_IMAGEMAP,
	OPT_TIMESTAMPS
};

static struct option const long_options[] = {
	{ "verbose", no_argument, NULL, OPT_VERBOSE },
	{ "pngdir", required_argument, NULL, OPT_PNGDIR },
	{ "charset", required_argument, NULL, OPT_CHARSET },
	{ "create-html", required_argument, NULL, OPT_CREATE_HTML },
	{ "imagemap", no_argument, NULL, OPT_IMAGEMAP },
	{ "html-dir", required_argument, NULL, OPT_HTML_DIR },
	{ "timestamps", no_argument, NULL, OPT_TIMESTAMPS },
	{ "version", no_argument, NULL, OPT_VERSION },
	{ "help", no_argument, NULL, OPT_HELP },
	{ NULL, no_argument, NULL, 0 }
};

/* ------------------------------------------------------------------------- */

static void usage(FILE *fp)
{
	fprintf(fp, _("%s - Create png files from GEM resource files\n"), gl_program_name);
	fprintf(fp, _("Usage: %s [<options>] <file...>\n"), gl_program_name);
	fprintf(fp, _("Options:\n"));
	fprintf(fp, _("   -v, --verbose        emit some progress messages\n"));
	fprintf(fp, _("   -c, --charset <name> use <charset> for display\n"));
	fprintf(fp, _("   -P, --pngdir <dir>   write output files to <dir>\n"));
	fprintf(fp, _("   -T, --timestamps     use timestamps in filenames\n"));
	fprintf(fp, _("       --version        print version and exit\n"));
	fprintf(fp, _("       --help           print this help and exit\n"));
}

/* ------------------------------------------------------------------------- */

static void stdout_handler(void *data, const char *fmt, va_list args)
{
	vfprintf((FILE *)data, fmt, args);
}

/* ------------------------------------------------------------------------- */

static void print_version(void)
{
	printf(_("%s version %s, %s\n"), gl_program_name, gl_program_version, gl_program_date);
	set_errout_handler(stdout_handler, stdout);
	writepng_version_info();
}

/* ------------------------------------------------------------------------- */

int main(int argc, char **argv)
{
	int c;
	RSCFILE *file;
	const char *filename;
	int exit_status = EXIT_SUCCESS;
	const char *charset = NULL;
	_UWORD load_flags = XRSC_SAFETY_CHECKS;
	
	while ((c = getopt_long_only(argc, argv, "c:l:P:TvhV", long_options, NULL)) != EOF)
	{
		switch ((enum rscview_opt) c)
		{
		case OPT_CHARSET:
			charset = optarg;
			break;
			
		case OPT_PNGDIR:
			pngdir = optarg;
			break;
		
		case OPT_CREATE_HTML:
			htmlout_name = optarg;
			break;
		
		case OPT_HTML_DIR:
			html_dir = optarg;
			break;
		
		case OPT_IMAGEMAP:
			gen_imagemap = TRUE;
			break;
			
		case OPT_VERBOSE:
			verbose = TRUE;
			break;
		
		case OPT_TIMESTAMPS:
			use_timestamps = TRUE;
			break;
		
		case OPT_VERSION:
			print_version();
			return EXIT_SUCCESS;
		
		case OPT_HELP:
			usage(stdout);
			return EXIT_SUCCESS;

		case OPT_SETVAR:
			/* option which just sets a var */
			break;
		
		case OPT_OPTERROR:
		default:
			usage(stderr);
			return EXIT_FAILURE;
		}
	}
	
	if (optind >= argc)
	{
		errout(_("%s: missing arguments\n"), gl_program_name);
		return EXIT_FAILURE;
	}
	
	if (htmlout_name)
	{
		htmlout_file = fopen(htmlout_name, "w");
		if (htmlout_file == NULL)
		{
			errout(_("%s: %s: %s\n"), gl_program_name, htmlout_name, strerror(errno));
			return EXIT_FAILURE;
		}
	}
	
	appl_init();
	
	menu_register(-1, gl_program_name);
	phys_handle = graf_handle(&gl_wchar, &gl_hchar, &gl_wbox, &gl_hbox);
	wind_get(DESK, WF_WORKXYWH, &desk.g_x, &desk.g_y, &desk.g_w, &desk.g_h);

	while (optind < argc)
	{
		filename = argv[optind++];
		file = load_all(filename, gl_wchar, gl_hchar, load_flags);
		if (file != NULL)
		{
			if (charset)
			{
				int cset = po_get_charset_id(charset);
				if (cset >= 0)
					file->fontset = cset;
			}
			open_screen();
			vst_font(vdi_handle, file->fontset);
			vst_font(phys_handle, file->fontset);
			
			if (draw_all_trees(file) == FALSE)
				exit_status = EXIT_FAILURE;
			
			vst_font(phys_handle, 1);
			close_screen();
							
			rsc_file_delete(file, FALSE);
			xrsrc_free(file);
		} else
		{
			exit_status = EXIT_FAILURE;
		}
	}
	
	appl_exit();
		
	if (htmlout_file != NULL)
	{
		fclose(htmlout_file);
		htmlout_file = NULL;
	}
	
	return exit_status;
}
