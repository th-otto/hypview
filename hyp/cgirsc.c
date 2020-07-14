/*
 * HypView - (c)      - 2019 Thorsten Otto
 *
 * A replacement hypertext viewer
 *
 * This file is part of HypView.
 *
 * HypView is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * HypView is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HypView; if not, see <http://www.gnu.org/licenses/>.
 */

#include "hypdefs.h"
#include "hypdebug.h"
#include "xgetopt.h"
#include "hcp_opts.h"
#include "picture.h"
#include "png.h"
#include "hcp.h"
#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif
#include "cgic.h"
#include "outcomm.h"
#include "outhtml.h"
#include "stat_.h"
#include "../rsc/include/portvdi.h"
#include "../rsc/include/rsrcload.h"
#include "../rsc/src/fileio.h"
#include "../rsc/include/rsc.h"
#include "../rsc/include/ws.h"
#include "cgirsc.h"

static hcp_opts *err_opts;
static GString *err_out;
static gboolean goterr;

static _WORD gl_hchar;
static _WORD gl_wchar;
static _WORD gl_wbox;
static _WORD gl_hbox;							/* system sizes */
static GRECT desk, screen;
static _WORD phys_handle;						/* physical workstation handle */
static _WORD vdi_handle;						/* virtual screen handle */
static WS ws;
static _WORD xworkout[57];

typedef struct _writepng_info {
	png_uint_32 width;
	png_uint_32 height;
	GString *out;
	png_structp png_ptr;
	png_infop info_ptr;
	jmp_buf jmpbuf;
} writepng_info;



/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

void err_fcreate(const char *filename)
{
	if (err_out->len == 0)
		html_out_header(NULL, err_opts, err_out, _("404 Not Found"), HYP_NOINDEX, FALSE, NULL, NULL, TRUE, NULL);
	hyp_utf8_sprintf_charset(err_out, err_opts->output_charset, NULL, _("Can't create %s: %s\n"), hyp_basename(filename), hyp_utf8_strerror(errno));
	goterr = TRUE;
}

/*** ---------------------------------------------------------------------- ***/

void err_fopen(const char *filename)
{
	if (err_out->len == 0)
		html_out_header(NULL, err_opts, err_out, _("404 Not Found"), HYP_NOINDEX, FALSE, NULL, NULL, TRUE, NULL);
	hyp_utf8_sprintf_charset(err_out, err_opts->output_charset, NULL, _("Can't open %s: %s\n"), hyp_basename(filename), hyp_utf8_strerror(errno));
	goterr = TRUE;
}

/*** ---------------------------------------------------------------------- ***/

void err_fread(const char *filename)
{
	hyp_utf8_fprintf(err_opts->errorfile, _("Error reading %s: %s\n"), hyp_basename(filename), hyp_utf8_strerror(errno));
	goterr = TRUE;
}

/*** ---------------------------------------------------------------------- ***/

void err_fwrite(const char *filename)
{
	hyp_utf8_fprintf(err_opts->errorfile, _("Error writing %s: %s\n"), hyp_basename(filename), hyp_utf8_strerror(errno));
	goterr = TRUE;
}

/*** ---------------------------------------------------------------------- ***/

void err_rename(const char *oldname, const char *newname)
{
	hyp_utf8_fprintf(err_opts->errorfile, _("Can't rename %s to %s: %s\n"), hyp_basename(oldname), hyp_basename(newname), hyp_utf8_strerror(errno));
	goterr = TRUE;
}

/*** ---------------------------------------------------------------------- ***/

void err_nota_rsc(const char *filename)
{
	hyp_utf8_fprintf(err_opts->errorfile, _("Not a resource file: %s\n"), hyp_basename(filename));
	goterr = TRUE;
}

/*** ---------------------------------------------------------------------- ***/

void warn_damaged(const char *filename, const char *where)
{
	hyp_utf8_fprintf(err_opts->errorfile, _("problems in %s while scanning %s\n"), hyp_basename(filename), where);
}

/*** ---------------------------------------------------------------------- ***/

void warn_cicons(void)
{
	hyp_utf8_fprintf(err_opts->errorfile, _("I couldn't find any color icons although the flag is set the header\n"));
}

/*** ---------------------------------------------------------------------- ***/

void warn_crc_mismatch(const char *filename, RSC_RSM_CRC header_crc, RSC_RSM_CRC file_crc)
{
	hyp_utf8_fprintf(err_opts->errorfile, _("%s: CRC $%04x does not match resource file $%04x\n"), hyp_basename(filename), header_crc, file_crc);
}

/*** ---------------------------------------------------------------------- ***/

void warn_crc_string_mismatch(const char *filename)
{
	hyp_utf8_fprintf(err_opts->errorfile, _("%s: embedded string CRC does not match resource file\n"), hyp_basename(filename));
}

/*** ---------------------------------------------------------------------- ***/

void warn_def_damaged(const char *filename)
{
	hyp_utf8_fprintf(err_opts->errorfile, _("%s: illegal definition file\n"), hyp_basename(filename));
}

/*** ---------------------------------------------------------------------- ***/

void warn_names_truncated(_WORD maxlen)
{
	hyp_utf8_fprintf(err_opts->errorfile, _("Names truncated (maxlen = %d)\n"), maxlen);
}

/*** ---------------------------------------------------------------------- ***/

void warn_interface_flags(const char *filename)
{
	hyp_utf8_fprintf(err_opts->errorfile, _("%s: some flags have been interpreted as being written by INTRFACE\n"), hyp_basename(filename));
}

/*** ---------------------------------------------------------------------- ***/

_BOOL ask_tree_notfound(_WORD trindex)
{
	hyp_utf8_fprintf(err_opts->errorfile, _("Tree %d not found.\n"), trindex);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

_BOOL ask_object_notfound(_LONG ob_index, char *tree_name)
{
	hyp_utf8_fprintf(err_opts->errorfile, _("No object #%ld in tree %s.\n"), ob_index, tree_name);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

void warn_rso_toonew(void)
{
	hyp_utf8_fprintf(err_opts->errorfile, _("RSO-File created by newer Version of ORCS\n"));
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

/*** ---------------------------------------------------------------------- ***/

static void writepng_error_handler(png_structp png_ptr, png_const_charp msg)
{
	writepng_info *wpnginfo;

	(void) msg;
	wpnginfo = (writepng_info *)png_get_error_ptr(png_ptr);
	if (wpnginfo == NULL)
	{									/* we are completely hosed now */
		/* errout("writepng severe error:  jmpbuf not recoverable; terminating.\n"); */
		exit(99);
	}
	longjmp(wpnginfo->jmpbuf, 1);
}

/*** ---------------------------------------------------------------------- ***/

static void writepng_warning_handler(png_structp png_ptr, png_const_charp msg)
{
	/*
	 * Silently ignore any warning messages from libpng.
	 * They stupidly tend to introduce new warnings with every release,
	 * with the default warning handler writing to stdout and/or stderr,
	 * messing up the output of the CGI scripts.
	 */
	(void) png_ptr;
	(void) msg;
}

/*** ---------------------------------------------------------------------- ***/

static void PNGCBAPI image_memory_flush(png_structp png_ptr)
{
	UNUSED(png_ptr);
}

static void PNGCBAPI image_memory_write(png_structp png_ptr, png_bytep data, size_t size)
{
	writepng_info *wpnginfo = (writepng_info *)png_get_io_ptr(png_ptr);

	/* I don't think libpng ever does this, but just in case: */
	if (size > 0)
	{
		g_string_append_len(wpnginfo->out, (const char *)data, size);
	}
}

/*** ---------------------------------------------------------------------- ***/

static GString *get_png(GRECT *gr, GString *out)
{
	int width = gr->g_w;
	int height = gr->g_h;
	unsigned char *volatile dst;
	size_t dststride;
	_WORD pxy[4];
	writepng_info *wpnginfo;
	png_structp png_ptr;				/* note:  temporary variables! */
	png_infop info_ptr;
	unsigned char *row;
	int y;
	
	dststride = width * 3;
	dst = g_new(unsigned char, dststride * height);
	if (dst == NULL)
	{
		return NULL;
	}
	
	pxy[0] = gr->g_x;
	pxy[1] = gr->g_y;
	pxy[2] = gr->g_x + gr->g_w - 1;
	pxy[3] = gr->g_y + gr->g_h - 1;

	v_hardcopy_ex(vdi_handle, pxy, PX_RGB, dststride, dst);
	
	wpnginfo = g_new0(writepng_info, 1);
	if (wpnginfo == NULL)
	{
		g_free(dst);
		return NULL;
	}
	
	wpnginfo->out = out;

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, wpnginfo, writepng_error_handler, writepng_warning_handler);
	if (!png_ptr)
	{
		g_free(wpnginfo);
		g_free(dst);
		return NULL;						/* out of memory */
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_write_struct(&png_ptr, NULL);
		g_free(wpnginfo);
		g_free(dst);
		return NULL;						/* out of memory */
	}

	wpnginfo->png_ptr = png_ptr;
	wpnginfo->info_ptr = info_ptr;

	if (setjmp(wpnginfo->jmpbuf))
	{
		png_destroy_write_struct(&wpnginfo->png_ptr, &wpnginfo->info_ptr);
		g_free(wpnginfo);
		g_free(dst);
		return NULL;
	}

	png_set_write_fn(png_ptr, wpnginfo, image_memory_write, image_memory_flush);
	
	wpnginfo->width = gr->g_w;
	wpnginfo->height = gr->g_h;

	png_set_compression_level(wpnginfo->png_ptr, 9 /* Z_BEST_COMPRESSION */);

	png_set_IHDR(wpnginfo->png_ptr, wpnginfo->info_ptr, wpnginfo->width, wpnginfo->height,
				 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
				 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	png_write_info(wpnginfo->png_ptr, wpnginfo->info_ptr);

	row = dst;
	for (y = gr->g_h, row = dst; --y >= 0; row += dststride)
	{
		png_write_row(wpnginfo->png_ptr, row);
	}
	png_write_end(wpnginfo->png_ptr, NULL);
	
	if (wpnginfo->png_ptr && wpnginfo->info_ptr)
	{
		png_destroy_write_struct(&wpnginfo->png_ptr, &wpnginfo->info_ptr);
		wpnginfo->png_ptr = NULL;
		wpnginfo->info_ptr = NULL;
	}
	
	g_free(wpnginfo);
	g_free(dst);
	return out;
}

/* ------------------------------------------------------------------------- */

static gboolean show_image(RSCFILE *file, RSCTREE *tree, _UWORD treenr, GRECT *gr, GString *out)
{
	(void)file;
	(void)tree;
	(void)treenr;
	if (get_png(gr, out) == NULL)
		return FALSE;
	
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean draw_dialog(RSCFILE *file, RSCTREE *tree, _UWORD treenr, GString *out)
{
	OBJECT *ob;
	GRECT gr, draw_gr;
	gboolean ret;
	
	ob = tree->rt_objects.dial.di_tree;
	if (ob == NULL)
		return FALSE;
	form_center_grect(ob, &gr);

	wind_update(BEG_UPDATE);
	form_dial_grect(FMD_START, &gr, &gr);
	
	clear_screen(tree->rt_name);
	
	objc_draw_grect(ob, ROOT, MAX_DEPTH, &gr);
	
	/*
	 * now move it to the top-left corner,
	 * to simplify objc_find() later
	 */
	ob[ROOT].ob_x -= gr.g_x;
	ob[ROOT].ob_y -= gr.g_y;

	draw_gr = gr;
	ret = show_image(file, tree, treenr, &draw_gr, out);

	form_dial_grect(FMD_FINISH, &gr, &gr);
	wind_update(END_UPDATE);
	
	return ret;
}

/* ------------------------------------------------------------------------- */

static gboolean draw_menu(RSCFILE *file, RSCTREE *tree, _UWORD treenr, GString *out)
{
	OBJECT *ob;
	_WORD thebar;
	_WORD theactive;
	_WORD themenus;
	_WORD title, menubox;
	_WORD x;
	GRECT gr, draw_gr;
	_WORD maxx, maxy;
	gboolean ret;
	
	ob = tree->rt_objects.menu.mn_tree;
	if (ob == NULL)
		return FALSE;
	/*
	 * just in case, put the menu at top-left corner
	 */
	ob[ROOT].ob_x = 0;
	ob[ROOT].ob_y = 0;

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
	
	draw_gr.g_x = 0;
	draw_gr.g_y = 0;
	draw_gr.g_w = maxx;
	draw_gr.g_h = maxy;
	ret = show_image(file, tree, treenr, &draw_gr, out);

	menu_bar(ob, FALSE);
	form_dial_grect(FMD_FINISH, &gr, &gr);
	wind_update(END_UPDATE);
	
	return ret;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

gboolean show_resource(const char *filename, hcp_opts *opts, GString *out, _UWORD treenr, hyp_pic_format *pic_format)
{
	_UWORD load_flags = XRSC_SAFETY_CHECKS;
	gboolean ret = FALSE;
	RSCFILE *file;
	
	err_opts = opts;
	err_out = out;
	goterr = FALSE;

	appl_init();
	
	menu_register(-1, gl_program_name);
	phys_handle = graf_handle(&gl_wchar, &gl_hchar, &gl_wbox, &gl_hbox);
	wind_get(DESK, WF_WORKXYWH, &desk.g_x, &desk.g_y, &desk.g_w, &desk.g_h);
	wind_get(DESK, WF_CURRXYWH, &screen.g_x, &screen.g_y, &screen.g_w, &screen.g_h);

	file = load_all(filename, gl_wchar, gl_hchar, load_flags);
	if (file != NULL)
	{
		RSCTREE *tree;
		
		tree = rsc_tree_index(file, treenr, RT_DIALOG);
		if (tree == NULL)
		{
			if (err_out->len == 0)
				html_out_header(NULL, err_opts, err_out, _("404 Not Found"), HYP_NOINDEX, FALSE, NULL, NULL, TRUE, NULL);
			hyp_utf8_sprintf_charset(err_out, err_opts->output_charset, NULL, _("Tree #%u not found!\n"), treenr);
		
			rsc_file_delete(file, FALSE);
			xrsrc_free(file);
		} else
		{
			open_screen();
			vst_font(vdi_handle, file->fontset);
			vst_font(phys_handle, file->fontset);
	
			if (tree->rt_type == RT_MENU)
				ret = draw_menu(file, tree, treenr, out);
			else
				ret = draw_dialog(file, tree, treenr, out);
			
			vst_font(phys_handle, 1);
			close_screen();
		
			rsc_file_delete(file, FALSE);
			xrsrc_free(file);
		}
	}
	
	appl_exit();

	if (goterr)
		ret = FALSE;
	if (ret == FALSE)
	{
		html_out_trailer(NULL, err_opts, err_out, HYP_NOINDEX, TRUE, FALSE, NULL);
	} else
	{
		*pic_format = HYP_PIC_PNG;
	}
	return ret;
}
