#define _GNU_SOURCE

#include "config.h"
#include <stdint.h>
#include <gem.h>
#include <object.h>
#include <ctype.h>
#include <ro_mem.h>
#include "fileio.h"
#include "rsc.h"
#include <time.h>
#include "debug.h"
#include "aesutils.h"
#include "rso.h"
#include "nls.h"
#include "rsc_lang.h"

FILE *ffp = NULL;
const char *fname;

static _BOOL fopen_mode;

#define g_box			"G_BOX"
#define g_text			"G_TEXT"
#define g_boxtext		"G_BOXTEXT"
#define g_image         "G_IMAGE"
#define g_userdef		"G_USERDEF"
#define g_ibox			"G_IBOX"
#define g_button		"G_BUTTON"
#define g_boxchar		"G_BOXCHAR"
#define g_string		"G_STRING"
#define g_ftext         "G_FTEXT"
#define g_fboxtext		"G_FBOXTEXT"
#define g_icon			"G_ICON"
#define g_cicon         "G_CICON"
#define g_title         "G_TITLE"
#define g_swbutton		"G_SWBUTTON"
#define g_popup			"G_POPUP"
#define g_edit			"G_EDIT"
#define g_shortcut		"G_SHORTCUT"
#define g_slist			"G_SLIST"
#define g_extbox		"G_EXTBOX"
#define g_oblink		"G_OBLINK"
#define g_unknown		"  ERROR: Unknown type"

#define FREAD(buf, size) if (test_read(buf, size) == FALSE) return FALSE
#define INPC(x) if (inpc(&(x)) == FALSE) return FALSE
#define INPW(x) if (inpw(&(x)) == FALSE) return FALSE
#define INPWC(x) if (inpwc(&(x)) == FALSE) return FALSE
#define INPL(x) if (inpl(&(x)) == FALSE) return FALSE

/* special flags for EmuTOS */
#define CENTRE_ALIGNED  0x8000
#define RIGHT_ALIGNED   0x4000

/* minimum width for the "about" entry in the desktop */
#define MIN_DESKMENU_WIDTH  20  /* in characters, compatible with Atari TOS */

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static _BOOL __attribute__((__noinline__)) inpc(_UBYTE *x)
{
	int c;
	
	if ((c = getc(ffp)) == EOF)
		return FALSE;
	*x = (_UBYTE)c;
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL __attribute__((__noinline__)) inpw(_UWORD *x)
{
	_UWORD c1;
	int c;
	
	if ((c = getc(ffp)) == EOF)
		return FALSE;
	c1 = c << 8;
	if ((c = getc(ffp)) == EOF)
		return FALSE;
	c1 = (_UWORD)(c1 + (c & 0xff));
	*x = (_UWORD)c1;
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

_BOOL test_read(void *buf, size_t size)
{
	if (fread(buf, 1, size, ffp) != size || ferror(ffp))
	{
		return file_close(FALSE);
	}
	return TRUE;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

_BOOL file_close(_BOOL status)
{
	if (ffp != NULL)
	{
		fflush(ffp);
		if (ferror(ffp))
			status = FALSE;
		if (ffp != stdout)
			fclose(ffp);
		if (fname != NULL && status == FALSE)
			(*(fopen_mode ? err_fwrite : err_fread))(fname);
		ffp = NULL;
	}
	fname = NULL;
	return status;
}

/*** ---------------------------------------------------------------------- ***/

_BOOL file_create(const char *filename, const char *mode)
{
	if ((ffp = fopen(filename, mode)) == NULL)
	{
		err_fcreate(filename);
		return FALSE;
	}
	fopen_mode = TRUE;
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

_BOOL file_open(const char *filename, const char *mode)
{
	if ((ffp = fopen(filename, mode)) == NULL)
		return FALSE;
	fopen_mode = FALSE;
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

RSCTREE *rsc_tree_index(RSCFILE *file, _UWORD idx, _UWORD type)
{
	RSCTREE *tree;

	FOR_ALL_RSC(file, tree)
	{
		switch (type)
		{
		case RT_UNKNOWN:
		case RT_FREE:
		case RT_DIALOG:
		case RT_MENU:
			switch (tree->rt_type)
			{
			case RT_UNKNOWN:
			case RT_FREE:
			case RT_DIALOG:
			case RT_MENU:
				if (idx == 0)
					return tree;
				idx--;
				break;
			}
			break;
		case RT_ALERT:
		case RT_FRSTR:
			switch (tree->rt_type)
			{
			case RT_ALERT:
			case RT_FRSTR:
				if (idx == 0)
					return tree;
				idx--;
				break;
			}
			break;
		case RT_FRIMG:
		case RT_MOUSE:
			switch (tree->rt_type)
			{
			case RT_FRIMG:
			case RT_MOUSE:
				if (idx == 0)
					return tree;
				idx--;
				break;
			}
			break;
		case RT_BUBBLEMORE:
			switch (tree->rt_type)
			{
			case RT_BUBBLEMORE:
				if (idx == 0)
					return tree;
				idx--;
				break;
			}
			break;
		case RT_BUBBLEUSER:
			switch (tree->rt_type)
			{
			case RT_BUBBLEUSER:
				if (idx == 0)
					return tree;
				idx--;
				break;
			}
			break;
		case RT_ANY:
			if (idx == 0)
				return tree;
			idx--;
			break;
		default:
			return NULL;
		}
	}
	return NULL;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

#define f_none			"NONE"
#define f_selectable	"SELECTABLE"
#define f_default		"DEFAULT"
#define f_exit			"EXIT"
#define f_editable		"EDITABLE"
#define f_rbutton		"RBUTTON"
#define f_lastob		"LASTOB"
#define f_touchexit     "TOUCHEXIT"
#define f_hidetree		"HIDETREE"
#define f_indirect		"INDIRECT"
#define f_3dind         "FL3DIND"
#define f_3dbak         "FL3DBAK"
#define f_3dact         "FL3DACT"

const char *flags_name(char *sbuf, _UWORD flags, enum emutos rsctype)
{
	char sname[20];

	if (flags == OF_NONE)
		return f_none;
	sbuf[0] = '\0';
	do
	{
		sname[0] = '\0';
		if (flags & OF_SELECTABLE)
		{
			strcpy(sname, f_selectable);
			flags &= ~OF_SELECTABLE;
		} else if (flags & OF_DEFAULT)
		{
			strcpy(sname, f_default);
			flags &= ~OF_DEFAULT;
		} else if (flags & OF_EXIT)
		{
			strcat(sname, f_exit);
			flags &= ~OF_EXIT;
		} else if (flags & OF_EDITABLE)
		{
			strcpy(sname, f_editable);
			flags &= ~OF_EDITABLE;
		} else if (flags & OF_RBUTTON)
		{
			strcpy(sname, f_rbutton);
			flags &= ~OF_RBUTTON;
		} else if (flags & OF_LASTOB)
		{
			strcpy(sname, f_lastob);
			flags &= ~OF_LASTOB;
		} else if (flags & OF_TOUCHEXIT)
		{
			strcpy(sname, f_touchexit);
			flags &= ~OF_TOUCHEXIT;
		} else if (flags & OF_HIDETREE)
		{
			strcpy(sname, f_hidetree);
			flags &= ~OF_HIDETREE;
		} else if (flags & OF_INDIRECT)
		{
			strcpy(sname, f_indirect);
			flags &= ~OF_INDIRECT;
		} else if (rsctype == EMUTOS_DESK && (flags & CENTRE_ALIGNED))
		{
			strcpy(sname, "CENTRE_ALIGNED");
			flags &= ~CENTRE_ALIGNED;
		} else if (rsctype == EMUTOS_DESK && (flags & RIGHT_ALIGNED))
		{
			strcpy(sname, "RIGHT_ALIGNED");
			flags &= ~RIGHT_ALIGNED;
		} else if ((flags & OF_FL3DMASK) == OF_FL3DIND)
		{
			strcpy(sname, f_3dind);
			flags &= ~OF_FL3DMASK;
		} else if ((flags & OF_FL3DMASK) == OF_FL3DBAK)
		{
			strcpy(sname, f_3dbak);
			flags &= ~OF_FL3DMASK;
		} else if ((flags & OF_FL3DMASK) == OF_FL3DACT)
		{
			strcpy(sname, f_3dact);
			flags &= ~OF_FL3DMASK;
		}
		if (sname[0] == '\0')
		{
			sprintf(sname, "0x%0X",
				(unsigned int)(flags));
			flags = 0;
		}
		strcat(sbuf, sname);
		if (flags != 0)
			strcat(sbuf, "|");
	} while (flags != 0);
	return sbuf;
}

/*** ---------------------------------------------------------------------- ***/

#define s_normal		"NORMAL"
#define s_selected		"SELECTED"
#define s_crossed		"CROSSED"
#define s_checked		"CHECKED"
#define s_disabled		"DISABLED"
#define s_outlined		"OUTLINED"
#define s_shadowed		"SHADOWED"
#define s_whitebak		"WHITEBAK"
#define s_draw3d		"DRAW3D"

const char *state_name(char *sbuf, _UWORD state)
{
	char sname[12];

	if (state == OS_NORMAL)
		return s_normal;
	sbuf[0] = '\0';
	do
	{
		sname[0] = '\0';
		if (state & OS_SELECTED)
		{
			strcpy(sname, s_selected);
			state &= ~OS_SELECTED;
		} else if (state & OS_CROSSED)
		{
			strcpy(sname, s_crossed);
			state &= ~OS_CROSSED;
		} else if (state & OS_CHECKED)
		{
			strcpy(sname, s_checked);
			state &= ~OS_CHECKED;
		} else if (state & OS_DISABLED)
		{
			strcpy(sname, s_disabled);
			state &= ~OS_DISABLED;
		} else if (state & OS_OUTLINED)
		{
			strcpy(sname, s_outlined);
			state &= ~OS_OUTLINED;
		} else if (state & OS_SHADOWED)
		{
			strcpy(sname, s_shadowed);
			state &= ~OS_SHADOWED;
		} else if (state & OS_WHITEBAK)
		{
			strcpy(sname, s_whitebak);
			state &= ~OS_WHITEBAK;
		} else if (state & OS_DRAW3D)
		{
			strcpy(sname, s_draw3d);
			state &= ~OS_DRAW3D;
		}
		if (sname[0] == '\0')
		{
			sprintf(sname, "0x%0X",
				(unsigned int)(state));
			state = 0;
		}
		strcat(sbuf, sname);
		if (state != 0)
			strcat(sbuf, "|");
	} while (state != 0);
	return sbuf;
}

/*** ---------------------------------------------------------------------- ***/

const char *rtype_name(_WORD type)
{
	switch (type)
	{
		case RT_DIALOG : return "form/dialog";
		case RT_MENU   : return "menu";
		case RT_FREE   : return "free form";
		case RT_ALERT  : return "Alert string";
		case RT_FRSTR  : return "Free string";
		case RT_FRIMG  : return "Free image";
		case RT_MOUSE  : return "Mouse cursor";
		case RT_BUBBLEMORE : return "BubbleMore";
		case RT_BUBBLEUSER : return "BubbleUser";
		default:
		case RT_UNKNOWN: return "unknown form";
	}
}

/*** ---------------------------------------------------------------------- ***/

const char *rtype_name_short(_WORD type)
{
	switch (type)
	{
		case RT_DIALOG : return "dialog";
		case RT_MENU   : return "menu";
		case RT_FREE   : return "free";
		case RT_ALERT  : return "alert";
		case RT_FRSTR  : return "string";
		case RT_FRIMG  : return "image";
		case RT_MOUSE  : return "cursor";
		case RT_BUBBLEMORE : return "more";
		case RT_BUBBLEUSER : return "user";
		default:
		case RT_UNKNOWN: return "unknown";
	}
}

/*** ---------------------------------------------------------------------- ***/

const char *type_name(_WORD type)
{
	switch (type)
	{
		case G_BOX: return g_box;
		case G_IBOX: return g_ibox;
		case G_BOXCHAR: return g_boxchar;
		case G_STRING: return g_string;
		case G_TITLE: return g_title;
		case G_BUTTON: return g_button;
		case G_TEXT: return g_text;
		case G_FTEXT: return g_ftext;
		case G_BOXTEXT: return g_boxtext;
		case G_FBOXTEXT: return g_fboxtext;
		case G_IMAGE: return g_image;
		case G_ICON: return g_icon;
		case G_CICON: return g_cicon;
		case G_USERDEF: return g_userdef;
		case G_SWBUTTON: return g_swbutton;
		case G_POPUP: return g_popup;
		case G_EDIT: return g_edit;
		case G_SHORTCUT: return g_shortcut;
		case G_SLIST: return g_slist;
		case G_EXTBOX: return g_extbox;
		case G_OBLINK: return g_oblink;
		default: return g_unknown;
	}
}

/*** ---------------------------------------------------------------------- ***/

char *rsx_basename(const char *name)
{
	static char namebuf[FNAMELEN+1];
	char *dotp;

	strcpy(namebuf, name);
	dotp = strrchr(namebuf, '.');
	if (dotp != NULL && strcasecmp(dotp + 1, "rsc") == 0)
		*dotp = '\0';
	return namebuf;
}

/*** ---------------------------------------------------------------------- ***/

char *rsc_path_get_dirname(const char *path)
{
	const char *base;
	char *dir;
	char *ptr;
	
	if (path == NULL)
		return NULL;
	base = rsc_basename(path);
	dir = g_strndup(path, base - path);
	ptr = dir + strlen(dir);
	while (ptr > dir && G_IS_DIR_SEPARATOR(ptr[-1]))
		*--ptr = '\0';
	return dir;
}

/*** ---------------------------------------------------------------------- ***/

const char *rsc_basename(const char *path)
{
	const char *p;
	const char *base = NULL;
	
	if (path == NULL)
		return path;
	p = path;
	while (*p != '\0')
	{
		if (G_IS_DIR_SEPARATOR(*p))
			base = p + 1;
		++p;
	}
	if (base != NULL)
		return base;
	
	if (isalpha(path[0]) && path[1] == ':')
	{
    	/* can only be X:name, without slash */
    	path += 2;
	}
	
	return path;
}

/*** ---------------------------------------------------------------------- ***/

char *rsc_path_get_basename(const char *path)
{
	return g_strdup(rsc_basename(path));
}

/*** ---------------------------------------------------------------------- ***/

void set_extension(char *filename, const char *ext)
{
	const char *p;
	char *p2;

	p = rsc_basename(filename);
	if ((p2 = strrchr(p, '.')) == NULL)
	{
		strncat(filename, ".", PATH_MAX);
		p2 = strchr(p, '.');
	}
	if (*ext != '\0')
		++p2;
	*p2 = '\0';
	strncat(filename, ext, PATH_MAX);
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL is_menu(OBJECT *tree)
{
	_WORD titleline;
	_WORD titlebox;
	_WORD workscreen;
	_WORD title, menubox;

	if (tree == NULL ||
		tree[ROOT].ob_type != G_IBOX ||
		(titleline = tree[ROOT].ob_head) == NIL ||
		tree[titleline].ob_type != G_BOX ||
		(workscreen = tree[titleline].ob_next) == titleline ||
		tree[workscreen].ob_type != G_IBOX ||
		tree[workscreen].ob_next != ROOT ||
		(titlebox = tree[titleline].ob_head) == NIL ||
		tree[titlebox].ob_type != G_IBOX ||
		tree[titlebox].ob_next != titleline ||
		(title = tree[titlebox].ob_head) == NIL ||
		(menubox = tree[workscreen].ob_head) == NIL)
		return FALSE;
	do
	{
		if (tree[title].ob_type != G_TITLE ||
			tree[title].ob_head != NIL ||
			tree[menubox].ob_type != G_BOX ||
			tree[menubox].ob_head == NIL)
			return FALSE;
		title = tree[title].ob_next;
		menubox = tree[menubox].ob_next;
	} while (title != titlebox && menubox != workscreen);
	if (title != titlebox || menubox != workscreen)
		return FALSE;
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL any_exit(OBJECT *tree)
{
	_WORD i = 0;
	_UWORD flags;
	
	for (;;)
	{
		flags = tree[i].ob_flags;
		if (!(flags & OF_HIDETREE))
		{
			if (!(tree[i].ob_state & OS_DISABLED) &&
				((flags & OF_TOUCHEXIT) || ((flags & OF_EXIT) && (flags & (OF_SELECTABLE|OF_DEFAULT)))))
					return TRUE;
		}
		if (tree[i].ob_flags & OF_LASTOB)
			return FALSE;
		++i;
	}		
}

static _BOOL Form_Al_is_Str_Ok(const char *str)
{
	if (str != NULL &&
		str[0] == '[' &&
		str[1] >= '0' &&
#if 0
		str[1] <= '0' + MAX_ALERT_SYM &&
#endif
		str[2] == ']' &&
		str[3] == '[' &&
#if 0
		strlen(str) < FO_BUFSIZE &&
#endif
		(str = strchr(str + 4, ']')) != NULL &&
		str[1] == '[' &&
		(str = strchr(str + 2, ']')) != NULL &&
		str[1] == '\0')
		return TRUE;
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL uses_oldlang(RSCFILE *file)
{
	const char *str;
	
	if (file->header.rsh_nstring == 0)
		return FALSE;
	str = file->rs_frstr[0];
	if (str != NULL && strchr(str, RSC_LANG_DELIM) != NULL)
		return TRUE;
	
	return FALSE;
}
	
/*** ---------------------------------------------------------------------- ***/

static _BOOL rsc_load_trees(RSCFILE *file)
{
	_ULONG i;
	_WORD type;
	RSCTREE *tree;
	char name[MAXNAMELEN+1];
	const char *prefix;
	
	if (uses_oldlang(file))
	{
		file->rsc_use_oldlang = TRUE;

		for (i = 0; i < file->header.rsh_nobs; i++)
		{
			OBJECT *ob = &file->rs_object[i];
			_WORD type = ob->ob_type & OBTYPEMASK;
			switch (type)
			{
			case G_STRING:
			case G_TITLE:
			case G_BUTTON:
			case G_SHORTCUT:
				ob->ob_spec.free_string = rsc_language_str(ob->ob_spec.free_string, file->rsc_extob.lang);
				break;
			case G_TEXT:
			case G_BOXTEXT:
				ob->ob_spec.tedinfo->te_ptext = rsc_language_str(ob->ob_spec.tedinfo->te_ptext, file->rsc_extob.lang);
				break;
			case G_FTEXT:
			case G_FBOXTEXT:
				ob->ob_spec.tedinfo->te_ptmplt = rsc_language_str(ob->ob_spec.tedinfo->te_ptmplt, file->rsc_extob.lang);
				break;
			case G_ICON:
			case G_CICON:
				ob->ob_spec.iconblk->ib_ptext = rsc_language_str(ob->ob_spec.iconblk->ib_ptext, file->rsc_extob.lang);
				break;
			}
		}

		for (i = 0; i < file->header.rsh_nstring; i++)
		{
			file->rs_frstr[i] = rsc_language_str(file->rs_frstr[i], file->rsc_extob.lang);
		}
	}
	
	for (i = 0; i < file->header.rsh_ntree; i++)
	{
		OBJECT *ob = file->rs_trindex[i];
		
		if (is_menu(ob))
		{
			type = RT_MENU;
			prefix = "MENU";
		} else if (any_exit(ob))
		{
			type = RT_DIALOG;
			prefix = "DIALOG";
		} else
		{
			type = RT_UNKNOWN;
			prefix = "TREE";
		}
		sprintf(name, "%s%03ld", prefix, i + 1);
		if ((tree = rsc_add_tree(file, type, name, ob)) == NULL)
		{
			return FALSE;
		}
		tree->rt_nobs = Objc_Count(ob, ROOT);
		tree->rt_obnames = g_new0(char, tree->rt_nobs * (MAXNAMELEN + 1));
	}

	for (i = 0; i < file->header.rsh_nstring; i++)
	{
		char *str = file->rs_frstr[i];
		if (Form_Al_is_Str_Ok(str))
		{
			type = RT_ALERT;
			prefix = "ALERT";
		} else
		{
			type = RT_FRSTR;
			prefix = "STR";
		}
		sprintf(name, "%s%03ld", prefix, i + 1);
		if (rsc_add_tree(file, type, name, str) == NULL)
		{
			g_free(str);
			return FALSE;
		}
	}

	for (i = 0; i < file->header.rsh_nimages; i++)
	{
		BITBLK *bitblk = file->rs_frimg[i];
		if (is_mouseform(bitblk))
		{
			type = RT_MOUSE;
			prefix = "MOUSE";
		} else
		{
			type = RT_FRIMG;
			prefix = "IMAGE";
		}
		sprintf(name, "%s%03ld", prefix, i + 1);
		if ((tree = rsc_add_tree(file, type, name, bitblk)) == NULL)
		{
			return FALSE;
		}
	}

	return TRUE;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*
 *  load a .HRD definition file
 */
static _BOOL hrd_entry(HRD_ENTRY *entry, _BOOL *too_long)
{
	_UBYTE c;
	_WORD i;

	INPC(entry->entry_type);
	INPC(entry->filler);
	/*
	 * HRD has all fields in big-endian format
	 */
	INPW(entry->tree_index);
	INPW(entry->ob_index);
	i = 0;
	for (;;)
	{
		INPC(c);
		if (c == 0)
			break;
		if (i == HRD_NAMELEN)
		{
			/*
			 * RSM seems to write entries with more than
			 * 16 characters. Interface errors out on them,
			 * and truncates them. So do we.
			 */
			*too_long = TRUE;
		} else if (i < HRD_NAMELEN)
		{
			entry->hrd_name[i] = c;
		}
		i++;
	}
	if (i > HRD_NAMELEN)
		i = HRD_NAMELEN;
	entry->hrd_name[i] = 0;
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL hrd_load(RSCFILE *file, const char *filename, _BOOL *def_found)
{
	HRD_HEADER hrd_header;
	RSCTREE *tree;
	OBJECT *ob;
	_UWORD lasttype;
	HRD_ENTRY entry;
	_BOOL too_long = FALSE;
	
	fname = filename;
	if (file_open(filename, "rb") == FALSE)
		return FALSE;
	*def_found = TRUE;
	/*
	 * HRD has all fields in big-endian format
	 */
	INPW(hrd_header.hrd_version);
	INPW(hrd_header.hrd_flags);
	INPC(hrd_header.filler1);
	INPC(hrd_header.hrd_nametype);
	INPC(hrd_header.filler2);
	INPC(hrd_header.filler3);
	if (hrd_header.hrd_version != 1)
		return FALSE;
	file->rsc_namelen = min(MAXNAMELEN, HRD_NAMELEN);
	file->rsc_flags |= RF_HRD;
	if (hrd_header.hrd_flags & HRD_C)
		file->rsc_flags |= RF_C;
	if (hrd_header.hrd_flags & HRD_PASCAL)
		file->rsc_flags |= RF_PASCAL;
	if (hrd_header.hrd_flags & HRD_MODULA)
		file->rsc_flags |= RF_MODULA;
	/*
	 * if HRD_VERBOSE or HRD_CSOURCE are set,
	 * file was definitly written by INTRFACE, KUMA
	 * does not use these flags.
	 * Otherwise, if HRD_BASIC, HRD_GFA or HRD_STATIC are set,
	 * warn later, because those flags are ambigious,
	 * but interpret them as if being written by Interface.
	 */
	if (!(hrd_header.hrd_flags & (HRD_VERBOSE | HRD_CSOURCE)) &&
		(hrd_header.hrd_flags & (HRD_BASIC | HRD_GFA | HRD_STATIC)))
	{
		warn_interface_flags(filename);
	}
#define INTERFACE 1
#if INTERFACE
	if (hrd_header.hrd_flags & HRD_BASIC)
		file->rsc_flags |= RF_BASIC;
	if (hrd_header.hrd_flags & HRD_GFA)
		file->rsc_flags |= RF_GFA;
	if (hrd_header.hrd_flags & HRD_STATIC)
		file->rsc_flags2 |= RF_STATIC;
#else
	if (hrd_header.hrd_flags & HRD_WFORTRAN)
		file->rsc_flags |= RF_FORTRAN;
	if (hrd_header.hrd_flags & HRD_ASSEMBLER)
		file->rsc_flags |= RF_ASS;
	if (hrd_header.hrd_flags & HRD_WBASIC)
		file->rsc_flags |= RF_BASIC;
#endif
#undef INTERFACE
	if (hrd_header.hrd_flags & HRD_CSOURCE)
		file->rsc_flags |= RF_CSOURCE2;
	if (hrd_header.hrd_flags & HRD_VERBOSE)
		file->rsc_flags |= RF_VERBOSE;
	switch (hrd_header.hrd_nametype)
	{
	case HRD_NAME_UPCASE:
		file->rsc_rule1.upper = TRUE;
		file->rsc_rule1.lower = FALSE;
		file->rsc_rule2.upper = FALSE;
		file->rsc_rule2.lower = TRUE;
		break;
	case HRD_NAME_LOWER:
		file->rsc_rule1.upper = FALSE;
		file->rsc_rule1.lower = TRUE;
		file->rsc_rule2.upper = FALSE;
		file->rsc_rule2.lower = TRUE;
		break;
	case HRD_NAME_UPPER:
	default:
		file->rsc_rule1.upper = TRUE;
		file->rsc_rule1.lower = FALSE;
		file->rsc_rule2.upper = TRUE;
		file->rsc_rule2.lower = FALSE;
		break;
	}
	lasttype = (_UWORD)-1;
	while (hrd_entry(&entry, &too_long) != FALSE)
	{
		switch (entry.entry_type)
		{
		case HRD_DIALOG:
			lasttype = RT_DIALOG;
			goto common;
		case HRD_MENU:
			lasttype = RT_MENU;
			goto common;
		case HRD_ALERT:
			lasttype = RT_ALERT;
			goto common;
		case HRD_FRSTR:
			lasttype = RT_FRSTR;
			goto common;
		case HRD_FRIMG:
			lasttype = RT_FRIMG;
		common:
			if (entry.ob_index != 0)
				return FALSE;
			tree = rsc_tree_index(file, entry.tree_index, lasttype);
			if (tree != NULL)
			{
				tree->rt_type = lasttype;
				strmaxcpy(tree->rt_name, min(HRD_NAMELEN,MAXNAMELEN) + 1, (const char *)entry.hrd_name);
			}
			break;
		case HRD_OBJECT:
			tree = rsc_tree_index(file, entry.tree_index, lasttype);
			if (tree != NULL)
			{
				ob = NULL;
				switch (tree->rt_type)
				{
					case RT_DIALOG:
					case RT_FREE:
					case RT_UNKNOWN: ob = tree->rt_objects.dial.di_tree; break;
					case RT_MENU: ob = tree->rt_objects.menu.mn_tree; break;
					case RT_FRSTR:
					case RT_ALERT:
						break;
					case RT_FRIMG:
					case RT_MOUSE:
						if (is_mouseform(tree->rt_objects.bit))
							tree->rt_type = lasttype = RT_MOUSE;
						break;
					default:
						warn_def_damaged(filename);
						file_close(TRUE);
						return FALSE;
				}
				if (ob != NULL)
				{
					ob_setname(file, tree, entry.ob_index, (const char *)entry.hrd_name, min(HRD_NAMELEN,MAXNAMELEN) + 1);
				}
			}
			break;
		case HRD_EOF:
			goto eof;
		case HRD_PREFIX:
			/* from erd; can't convince interface to produce such an entry */
			break;
		default:
			KINFO(("unknown hrd entry type %u: %-*.*s\n", entry.entry_type, HRD_NAMELEN, HRD_NAMELEN, entry.hrd_name));
			break;
		}
	}
eof:
	if (too_long)
		warn_names_truncated(HRD_NAMELEN);
	return file_close(TRUE);
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*
 *  load a .DFN definition file
 */
static _BOOL dfn_nameinfo_read(NAMEINFO *nameinfo)
{
	_WORD c, c2;
	_WORD i;

	/*
	 * DFN has count in little-endian format
	 */
	if ((c = fgetc(ffp)) == EOF)
		return FALSE;
	if ((c2 = fgetc(ffp)) == EOF)
		return FALSE;
	nameinfo->na_count = (c2 << 8) + c;
	INPC(nameinfo->na_obidx);
	INPC(nameinfo->na_treeidx);
	INPC(nameinfo->na_treetype);
	INPC(nameinfo->na_nametype);
	for (i = 0; i < RSDNAMELEN; i++)
	{
		INPC(*((unsigned char *)(&nameinfo->na_name[i])));
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL dfn_load(RSCFILE *file, const char *filename, _BOOL *def_found)
{
	NAMEINFO nameinfo;
	RSCTREE *tree;
	OBJECT *ob;
	_WORD i, count;
	_BOOL treename;
	_UWORD type;
	
	fname = filename;
	if (file_open(filename, "rb") == FALSE)
		return FALSE;
	*def_found = TRUE;
	if (dfn_nameinfo_read(&nameinfo) == FALSE)
		return file_close(FALSE);
	file->rsc_flags |= RF_DFN;
	file->rsc_namelen = min(MAXNAMELEN, RSDNAMELEN);
	count = nameinfo.na_count;
	for (i = count; i--; )
	{
		tree = NULL;
		treename = TRUE;
		type = RT_UNKNOWN;
		switch ((nameinfo.na_nametype << 8) | nameinfo.na_treetype)
		{
		case 0x0000:
			type = RT_UNKNOWN;
			tree = rsc_tree_index(file, nameinfo.na_obidx, type);
			break;
		case 0x0001:
			type = RT_FREE; /* RT_PANEL in RCS */
			tree = rsc_tree_index(file, nameinfo.na_obidx, type);
			break;
		case 0x0002:
			type = RT_MENU;
			tree = rsc_tree_index(file, nameinfo.na_obidx, type);
			break;
		case 0x0003:
			type = RT_DIALOG;
			tree = rsc_tree_index(file, nameinfo.na_obidx, type);
			break;
		case 0x0004:
			type = RT_ALERT;
			tree = rsc_tree_index(file, nameinfo.na_obidx, type);
			break;
		case 0x0005:
			type = RT_FRSTR;
			tree = rsc_tree_index(file, nameinfo.na_obidx, type);
			break;
		case 0x0101:
			type = RT_FRSTR;
			tree = rsc_tree_index(file, nameinfo.na_obidx, type);
			break;
		case 0x0102:
		case 0x0006:
			type = RT_FRIMG;
			tree = rsc_tree_index(file, nameinfo.na_obidx, type);
			if (tree != NULL && is_mouseform(tree->rt_objects.bit))
				type = RT_MOUSE;
			break;
		case 0x0100:
			tree = rsc_tree_index(file, nameinfo.na_treeidx, RT_UNKNOWN);
			treename = FALSE;
			break;
		default:
			type = RT_UNKNOWN;
			tree = rsc_tree_index(file, nameinfo.na_obidx, type);
			break;
		}
		if (tree != NULL)
		{
			if (treename)
			{
				tree->rt_type = type;
				strmaxcpy(tree->rt_name, min(RSDNAMELEN,MAXNAMELEN) + 1, nameinfo.na_name);
			} else
			{
				ob = NULL;
				switch (tree->rt_type)
				{
					case RT_DIALOG:
					case RT_FREE:
					case RT_UNKNOWN: ob = tree->rt_objects.dial.di_tree; break;
					case RT_MENU: ob = tree->rt_objects.menu.mn_tree; break;
					case RT_FRSTR:
					case RT_ALERT:
					case RT_FRIMG:
					case RT_MOUSE:
						break;
					default:
						warn_def_damaged(filename);
						file_close(TRUE);
						return FALSE;
				}
				if (ob == NULL ||
					ob_setname(file, tree, nameinfo.na_obidx, nameinfo.na_name, min(RSDNAMELEN,MAXNAMELEN) + 1) == FALSE)
				{
					KINFO(("object not found: %3d: nametype=%d treetype=%d treeidx=%d obidx=%d %-8.8s\n",
						count - i - 1,
						nameinfo.na_nametype,
						nameinfo.na_treetype,
						nameinfo.na_treeidx,
						nameinfo.na_obidx,
						nameinfo.na_name));
				}
			}
		} else
		{
			KINFO(("tree not found: %3d: nametype=%d treetype=%d treeidx=%d obidx=%d %-8.8s\n",
				count - i - 1,
				nameinfo.na_nametype,
				nameinfo.na_treetype,
				nameinfo.na_treeidx,
				nameinfo.na_obidx,
				nameinfo.na_name));
		}
		if (dfn_nameinfo_read(&nameinfo) == FALSE &&
			(ferror(ffp) || i != 0))
		{
			return file_close(FALSE);
		}
	}
	return file_close(TRUE);
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*
 *  load a .DEF/.RSD definition file
 */
static _BOOL rsd_nameinfo_read(NAMEINFO *nameinfo)
{
	/*
	 * RSD has all fields in big-endian format
	 */
	INPW(nameinfo->na_count);
	INPW(nameinfo->na_flags);
	INPC(nameinfo->na_treeidx);
	INPC(nameinfo->na_obidx);
	INPC(nameinfo->na_nametype);
	INPC(nameinfo->na_treetype);
	FREAD(nameinfo->na_name, RSDNAMELEN);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL rsd_load(RSCFILE *file, const char *filename, _BOOL *def_found)
{
	NAMEINFO nameinfo;
	RSCTREE *tree;
	OBJECT *ob;
	_WORD i;
	_UWORD lasttype;

	fname = filename;
	if (file_open(filename, "rb") == FALSE)
		return FALSE;
	*def_found = TRUE;

	if (rsd_nameinfo_read(&nameinfo) == FALSE)
		return FALSE;
	file->rsc_namelen = min(MAXNAMELEN, RSDNAMELEN);
	lasttype = (_UWORD)-1;
	for (i = nameinfo.na_count; i--; )
	{
		if (nameinfo.na_nametype == 0)
		{
			if (nameinfo.na_treeidx != 0)
				break;
			tree = rsc_tree_index(file, nameinfo.na_obidx, nameinfo.na_treetype);
			if (tree != NULL)
			{
				tree->rt_type = nameinfo.na_treetype;
				strmaxcpy(tree->rt_name, min(RSDNAMELEN,MAXNAMELEN) + 1, nameinfo.na_name);
			}
			lasttype = nameinfo.na_treetype;
		} else if (nameinfo.na_nametype == 1)
		{
			if (nameinfo.na_treetype != 0)
				break;
			tree = rsc_tree_index(file, nameinfo.na_treeidx, lasttype);
			if (tree != NULL)
			{
				ob = NULL;
				switch (tree->rt_type)
				{
					case RT_DIALOG:
					case RT_FREE:
					case RT_UNKNOWN: ob = tree->rt_objects.dial.di_tree; break;
					case RT_MENU: ob = tree->rt_objects.menu.mn_tree; break;
					case RT_FRSTR:
					case RT_ALERT:
					case RT_FRIMG:
					case RT_MOUSE:
						break;
					default:
						warn_def_damaged(filename);
						file_close(TRUE);
						return FALSE;
				}
				if (ob != NULL)
				{
					ob_setname(file, tree, nameinfo.na_obidx, nameinfo.na_name, min(RSDNAMELEN,MAXNAMELEN) + 1);
				}
			}
		}
		if (rsd_nameinfo_read(&nameinfo) == FALSE &&
			(ferror(ffp) || i != 0))
		{
			return file_close(FALSE);
		}
	}
	return file_close(TRUE);
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*
 *  load a .RSO definition file
 */
/*
 * format of RSO file is:
 *
 * - header
 * - file comments
 * {
 *	 - tree index
 *	 - tree type
 *	 - tree name
 *	 - tree comments
 *	 {
 *	   - object index
 *	   - object name
 *	   - object comments
 *	 }
 *	 - MARK
 * }
 * - MARK
 *
 * where
 *	 strings (names & comments) are stored with len byte & text
 *	 comments continue until zero length
 */

static _UWORD checksum;

#define reset_checksum() checksum = 0
#define add_checksum(c) checksum = (((checksum << 1) | (checksum & 0x8000 ? 1 : 0)) + (c)) & 0xffff

static _BOOL rso_cmntread(stringarray *cmnt)
{
	char cmntbuf[COMMENTLINES][COMMENTLEN+1];
	_WORD ncmnt;
	_UBYTE len, rlen;
	size_t total;
	_WORD i;
	char *p;
	
	ncmnt = 0;
	INPC(len);
	total = 0;
	while (len != 0)
	{
		if (ncmnt < COMMENTLINES)
		{
			rlen = len;
			if (rlen > COMMENTLEN)
			{
				/* !!! issue warning comment too long */
				rlen = COMMENTLEN;
			}
			FREAD(cmntbuf[ncmnt], rlen);
			cmntbuf[ncmnt][rlen] = '\0';
			ncmnt++;
			total += rlen + 1;
		} else
		{
			/* !!! issue warning too many lines */
			rlen = 0;
		}
		if (rlen < len)
			fseek(ffp, len - rlen, SEEK_CUR);
		INPC(len);
	}
	if (total == 0)
	{
		*cmnt = NULL;
		return TRUE;
	}
	*cmnt = p = g_new(char, total + 1);
	if (p == NULL)
		return FALSE;
	
	for (i = 0; i < ncmnt; i++)
	{
		strcpy(p, cmntbuf[i]);
		p += strlen(cmntbuf[i]) + 1;
	}
	*p = '\0';
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL rso_nameread(char *name, _WORD maxlen)
{
	_UBYTE len, rlen;

	INPC(len);
	rlen = len;
	if (rlen > maxlen)
	{
		/* !!! issue warning, ask to truncate */
	}
	if (rlen > MAXNAMELEN)
	{
		/* !!! issue warning */
		rlen = MAXNAMELEN;
	}
	FREAD(name, rlen);
	name[rlen] = '\0';
	if (rlen < len)
		fseek(ffp, len - rlen, SEEK_CUR);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static void update_checksum(const unsigned char *buf, _WORD bytes)
{
	_WORD i;
	_UWORD c;

	for (i = bytes; i > 0; )
	{
		c = (((_UWORD)buf[0]) << 8) | buf[1];
		i -= 2;
		if (i >= 0)
			c |= buf[1];
		add_checksum(c);
		buf += 2;
	}
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL __attribute__((__noinline__)) inpwc(_UWORD *x)
{
	if (inpw(x) == FALSE)
		return FALSE;
	add_checksum(*x);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL __attribute__((__noinline__)) inpl(_ULONG *x)
{
	_UWORD l1, l2;
	
	INPWC(l1);
	INPWC(l2);
	*x = ((_ULONG)l1 << 16) + (_ULONG)l2;
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static char *rso_strread(void)
{
	_UWORD len;
	char *str;
	
	INPWC(len);
	if (len == 0)
		return NULL;
	str = g_new(char, len + 1);
	if (str == NULL)
		return NULL;
	if (test_read(str, len) == FALSE)
	{
		g_free(str);
		return NULL;
	}
	str[len] = '\0';
	update_checksum((const unsigned char *)str, len);
	return str;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL rso_read_header(RSO_HEADER *rso_header)
{
	memset(rso_header, 0, sizeof(*rso_header));
	reset_checksum();
	INPL(rso_header->rso_magic);
	if (rso_header->rso_magic != RSO_MAGIC)
	{
		warn_def_damaged(fname);
		return FALSE;
	}
	INPL(rso_header->rso_hsize);
	INPWC(rso_header->rso_version);
	if (rso_header->rso_version == 1)
		rso_header->rso_hsize = RSO_VER1_SIZE;
	if ((rso_header->rso_version == 1 && rso_header->rso_hsize != RSO_VER1_SIZE) ||
		(rso_header->rso_version == 2 && rso_header->rso_hsize != RSO_VER2_SIZE) ||
		(rso_header->rso_version == 3 && rso_header->rso_hsize != RSO_VER3_SIZE) ||
		(rso_header->rso_version == 4 && rso_header->rso_hsize != RSO_VER4_SIZE) ||
		(rso_header->rso_version == 5 && rso_header->rso_hsize != RSO_VER5_SIZE) ||
		(rso_header->rso_version == 6 && rso_header->rso_hsize != RSO_VER6_SIZE) ||
		(rso_header->rso_version == 7 && rso_header->rso_hsize != RSO_VER7_SIZE) ||
		(rso_header->rso_version == 8 && rso_header->rso_hsize != RSO_VER8_SIZE) ||
		(rso_header->rso_version == 9 && rso_header->rso_hsize != RSO_VER9_SIZE) ||
		(rso_header->rso_version >= 10 && rso_header->rso_hsize < RSO_VER10_MIN_SIZE)
		)
	{
		warn_def_damaged(fname);
		return FALSE;
	}
	INPL(rso_header->rso_size);
	INPL(rso_header->rso_flags);
	INPWC(rso_header->rso_namelen);
	if (rso_header->rso_namelen > MAXNAMELEN)
	{
		/* !!! issue warning */
		rso_header->rso_namelen = MAXNAMELEN;
	}
	INPWC(rso_header->rso_rule1.upper);
	INPWC(rso_header->rso_rule1.lower);
	INPWC(rso_header->rso_rule1.alpha);
	INPWC(rso_header->rso_rule1.alnum);
	FREAD(rso_header->rso_rule1.add, 40); update_checksum(rso_header->rso_rule1.add, 40);
	FREAD(rso_header->rso_rule1.sub, 40); update_checksum(rso_header->rso_rule1.sub, 40);
	INPL(rso_header->rso_rule1.charset[0]);
	INPL(rso_header->rso_rule1.charset[1]);
	INPL(rso_header->rso_rule1.charset[2]);
	INPL(rso_header->rso_rule1.charset[3]);
	INPL(rso_header->rso_rule1.charset[4]);
	INPL(rso_header->rso_rule1.charset[5]);
	INPL(rso_header->rso_rule1.charset[6]);
	INPL(rso_header->rso_rule1.charset[7]);

	INPWC(rso_header->rso_rule2.upper);
	INPWC(rso_header->rso_rule2.lower);
	INPWC(rso_header->rso_rule2.alpha);
	INPWC(rso_header->rso_rule2.alnum);
	FREAD(rso_header->rso_rule2.add, 40); update_checksum(rso_header->rso_rule2.add, 40);
	FREAD(rso_header->rso_rule2.sub, 40); update_checksum(rso_header->rso_rule2.sub, 40);
	INPL(rso_header->rso_rule2.charset[0]);
	INPL(rso_header->rso_rule2.charset[1]);
	INPL(rso_header->rso_rule2.charset[2]);
	INPL(rso_header->rso_rule2.charset[3]);
	INPL(rso_header->rso_rule2.charset[4]);
	INPL(rso_header->rso_rule2.charset[5]);
	INPL(rso_header->rso_rule2.charset[6]);
	INPL(rso_header->rso_rule2.charset[7]);

	rso_header->rso_ted_fillchar = op_rsc_opts.ted_fillchar;
	rso_header->rso_edition = 1;
	rso_header->rso_date_created = time(NULL);
	rso_header->rso_date_changed = time(NULL);
	rso_header->rso_menu_leftmargin = op_rsc_opts.menu_leftmargin;
	rso_header->rso_menu_rightmargin = op_rsc_opts.menu_rightmargin;
	rso_header->rso_menu_minspace = op_rsc_opts.menu_minspace;
	rso_header->rso_menu_fillspace = op_rsc_opts.menu_fillspace;
	rso_header->rso_flags2 = 0;
	rso_header->rso_languages = 0;
	rso_header->rso_rsm_crc = RSC_CRC_NONE;
	rso_header->rso_modules = NULL;
	rso_header->rso_langs = NULL;
	
	if (rso_header->rso_version >= 2)
	{
		if (rso_header->rso_version < 5)
		{
			_UWORD date, time;
			
			/* old format with dos date/time; should not happen anymore */
			INPWC(date);
			INPWC(time);
			INPWC(date);
			INPWC(time);
		} else
		{
			INPL(rso_header->rso_date_created);
			INPL(rso_header->rso_date_changed);
		}
		if (rso_header->rso_version >= 6)
		{
			INPWC(rso_header->rso_ted_fillchar);
		}
		if (rso_header->rso_version >= 7)
		{
			INPW(rso_header->rso_menu_leftmargin);
			INPW(rso_header->rso_menu_rightmargin);
			INPW(rso_header->rso_menu_minspace);
			INPW(rso_header->rso_menu_fillspace);
		}
		if (rso_header->rso_version >= 8)
		{
			INPL(rso_header->rso_flags2);
			INPL(rso_header->rso_languages);
			/*
			 * fix 2.11 bug which saved the unitialized memory pattern
			 */
			if (rso_header->rso_flags2 == 0xbbbbbbbbUL)
				rso_header->rso_flags2 = 0;
			if (rso_header->rso_languages == 0xbbbbbbbbUL)
				rso_header->rso_languages = 0;
		}
		if (rso_header->rso_version >= 9)
		{
			FREAD(rso_header->rso_overlay, 32);
			update_checksum(rso_header->rso_overlay, 32);
		}
		if (rso_header->rso_version >= 10)
		{
			_UWORD type;
			_UWORD active;
			rsc_module *mod;
			char *id, *filename, *charset;
			
			INPWC(rso_header->rso_rsm_crc);
			for (;;)
			{
				INPWC(type);
				if (type == MOD_UNKNOWN)
					break;
				INPWC(active);
				mod = g_new0(rsc_module, 1);
				if (mod == NULL)
					return FALSE;
				mod->type = (module_type)type;
				mod->id = rso_strread();
				mod->name = rso_strread();
				mod->param1 = rso_strread();
				mod->param2 = rso_strread();
				mod->param3 = rso_strread();
				mod->param4 = rso_strread();
				mod->param5 = rso_strread();
				mod->active = active;
				mod->for_all_layers = FALSE;
				mod->next = rso_header->rso_modules;
				rso_header->rso_modules = mod;
			}
			while ((id = rso_strread()) != NULL)
			{
				_BOOL res;
				
				charset = rso_strread();
				filename = rso_strread();
				res = rsc_lang_add(&rso_header->rso_langs, id, charset, filename);
				g_free(filename);
				g_free(charset);
				g_free(id);
				if (res == FALSE)
					return FALSE;
			}
		}
		
		INPL(rso_header->rso_edition);
	}

	INPW(rso_header->rso_checksum);
	if (rso_header->rso_version > RSO_VERSION)
	{
		warn_rso_toonew();
		fseek(ffp, rso_header->rso_hsize, 0);
	} else
	{
		if (checksum != rso_header->rso_checksum)
		{
			warn_def_damaged(fname);
			return FALSE;
		}
	}
	
	if (rso_header->rso_version == 1)
		rso_header->rso_hsize = 0;
	
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL rso_obtree(RSCFILE *file, const char *filename, RSCTREE *tree)
{
	char namebuf[MAXNAMELEN+1];
	stringarray cmntbuf;
	_UWORD obindex;

	INPW(obindex);
	while (obindex != RSO_ENDMARK)
	{
		if (rso_nameread(namebuf, file->rsc_namelen) == FALSE ||
			rso_cmntread(&cmntbuf) == FALSE)
		{
			warn_def_damaged(filename);
			file_close(TRUE);
			return FALSE;
		}
		if (tree == NULL || ob_setname(file, tree, obindex, namebuf, MAXNAMELEN+1) == FALSE)
		{
			g_free(cmntbuf);
			if (tree != NULL && ask_object_notfound(obindex, tree->rt_name) == FALSE)
			{
				file_close(TRUE);
				return FALSE;
			}
		}
		INPW(obindex);
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL rso_load(RSCFILE *file, const char *filename, _BOOL *def_found)
{
	RSCTREE *tree;
	RSO_HEADER rso_header;
	_UWORD trindex;
	_UWORD trtype;
	_ULONG mask;
	char namebuf[MAXNAMELEN + 1];
	stringarray cmntbuf;
	
	fname = filename;
	if (file_open(filename, "rb") == FALSE)
		return FALSE;
	*def_found = TRUE;

	if (rso_read_header(&rso_header) == FALSE)
		return FALSE;

	if (rso_header.rso_version < 4)
		rso_header.rso_flags |= EXTOB_TO_RF(EXTOB_ORCS);
	if (rso_header.rso_version >= 2)
	{
		file->rsc_date_created = rso_header.rso_date_created;
		file->rsc_date_changed = rso_header.rso_date_changed;
		file->rsc_edition = rso_header.rso_edition;
	}
	
	/*
	 * convert old-style RSO file which had neither RSC nor RSX flag set
	 */
	mask = RF_RSC | RF_RSX;
	if ((rso_header.rso_flags & mask) != 0)
		mask = 0;
	file->rsc_flags = (file->rsc_flags & mask) | (rso_header.rso_flags & ~RF_EXTOB_MASK & ~mask);
	file->rsc_flags2 = rso_header.rso_flags2;
	
	file->rsc_extob.mode = RF_TO_EXTOB(rso_header.rso_flags);
	file->rsc_namelen = min(MAXNAMELEN, rso_header.rso_namelen);
	file->rsc_rule1 = rso_header.rso_rule1;
	file->rsc_rule2 = rso_header.rso_rule2;
	file->rsc_opts.ted_fillchar = rso_header.rso_ted_fillchar;
	file->rsc_opts.menu_leftmargin = rso_header.rso_menu_leftmargin;
	file->rsc_opts.menu_rightmargin = rso_header.rso_menu_rightmargin;
	file->rsc_opts.menu_minspace = rso_header.rso_menu_minspace;
	file->rsc_opts.menu_fillspace = rso_header.rso_menu_fillspace;
#if 0 /* not implemented here */
	file->rsc_languages = rso_header.rso_languages;
	g_free(file->rsc_extob.overlay_id);
	file->rsc_extob.overlay_id = NULL;
	if (!empty(rso_header.rso_overlay))
		file->rsc_extob.overlay_id = g_strdup(rso_header.rso_overlay);

	activate_modules(file, filename, &rso_header.rso_modules, FALSE);
	file->rsc_langs = rso_header.rso_langs;
#endif
	
	if (rso_cmntread(&file->rsc_cmnt) == FALSE)
	{
		warn_def_damaged(filename);
		return FALSE;
	}
	
	INPW(trindex);
	while (trindex != RSO_ENDMARK)
	{
		INPW(trtype);
		if (rso_nameread(namebuf, file->rsc_namelen) == FALSE)
		{
			warn_def_damaged(filename);
			file_close(TRUE);
			return FALSE;
		}
		if (rso_cmntread(&cmntbuf) == FALSE)
		{
			warn_def_damaged(filename);
			file_close(TRUE);
			return FALSE;
		}
		tree = rsc_tree_index(file, trindex, trtype);
		if (tree == NULL)
		{
			switch (trtype)
			{
			case RT_DIALOG:
			case RT_FREE:
			case RT_UNKNOWN:
			case RT_MENU:
				g_free(cmntbuf);
				cmntbuf = NULL;
				if (ask_tree_notfound(trindex) == FALSE)
				{
					return FALSE;
				}
				if (rso_obtree(file, filename, NULL) == FALSE)
					return FALSE;
				break;
			case RT_BUBBLEMORE:
			case RT_BUBBLEUSER:
#if 0 /* not implemented here */
				if ((tree = bubbledef_add(file, filename, trtype, trindex, namebuf, NULL)) == NULL)
				{
					g_free(cmntbuf);
					return FALSE;
				}
#endif
				tree->rt_cmnt = cmntbuf;
				break;
			default:
				g_free(cmntbuf);
				cmntbuf = NULL;
				if (ask_tree_notfound(trindex) == FALSE)
				{
					return FALSE;
				}
				break;
			}
		}
		if (tree != NULL)
		{
			tree->rt_type = trtype;
			strcpy(tree->rt_name, namebuf);
			tree->rt_cmnt = cmntbuf;
			cmntbuf = NULL;
			switch (trtype)
			{
			case RT_DIALOG:
			case RT_FREE:
			case RT_UNKNOWN:
				if (rso_obtree(file, filename, tree) == FALSE)
					return FALSE;
				break;
			case RT_MENU:
				if (rso_obtree(file, filename, tree) == FALSE)
					return FALSE;
				break;
			case RT_BUBBLEMORE:
			case RT_BUBBLEUSER:
#if 0 /* not implemented here */
				if (rso_bghread(file, filename, tree) == FALSE)
					return FALSE;
#endif
				break;
			default:
				break;
			}
		} else
		{
			if (rso_obtree(file, filename, NULL) == FALSE)
				return FALSE;
		}
		INPW(trindex);
	}

	/*
	 * walk object trees once again for BubbleGem help strings
	 */
	if (inpw(&trindex))
	{
#if 0 /* not implemented here */
		while (trindex != RSO_ENDMARK)
		{
			INPW(trtype);
			tree = rsc_tree_index(file, trindex, trtype);
			if (tree != NULL)
			{
				switch (trtype)
				{
				case RT_DIALOG:
				case RT_FREE:
				case RT_UNKNOWN:
					if (rso_bghtree(file, filename, tree, tree->rt_objects.dial.di_tree) == FALSE)
						return FALSE;
					break;
				case RT_MENU:
					if (rso_bghtree(file, filename, tree, tree->rt_objects.menu.mn_tree) == FALSE)
						return FALSE;
					break;
				case RT_FRSTR:
				case RT_ALERT:
					if (rso_bgh_alert(file, filename, tree) == FALSE)
						return FALSE;
					break;
				default:
					if (rso_bghtree(file, filename, NULL, NULL) == FALSE)
						return FALSE;
					break;
				}
			} else
			{
				if (rso_bghtree(file, filename, NULL, NULL) == FALSE)
					return FALSE;
			}
			INPW(trindex);
		}
#endif
	}
	
	if (rso_header.rso_rsm_crc != RSC_CRC_NONE && rso_header.rso_rsm_crc != file->rsc_rsm_crc)
	{
		warn_crc_mismatch(filename, rso_header.rso_rsm_crc, file->rsc_rsm_crc);
	}

	return file_close(TRUE);
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static _BOOL is_emutos_aes(RSCFILE *file)
{
	RSCTREE *tree;
	
	if (strcasecmp(rsx_basename(file->rsc_rsxname), "gem") != 0 &&
		strcasecmp(rsx_basename(file->rsc_rsxname), "gem_rsc") != 0)
		return FALSE;
	/*
	 * check for exactly 3 dialogs (fileselector, alert template & desktop)
	 */
	if ((tree = rsc_tree_index(file, 2, RT_DIALOG)) == NULL)
		return FALSE;
	if (Objc_Count(tree->rt_objects.dial.di_tree, ROOT) != 3)
		return FALSE;
	if (rsc_tree_index(file, 3, RT_DIALOG) != NULL)
		return FALSE;
	/*
	 * check for at least 11 images (3 icons & 8 mouse types)
	 */
	if (rsc_tree_index(file, 2, RT_FRIMG) == NULL)
		return FALSE;
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL is_emutos_icon(RSCFILE *file)
{
	if (strcasecmp(rsx_basename(file->rsc_rsxname), "icon") != 0)
		return FALSE;
	if (rsc_tree_index(file, 1, RT_DIALOG) == NULL)
		return FALSE;
	if (rsc_tree_index(file, 2, RT_DIALOG) != NULL)
		return FALSE;
	if (rsc_tree_index(file, 0, RT_FRSTR) != NULL)
		return FALSE;
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL is_emutos_desktop(RSCFILE *file)
{
	RSCTREE *rsctree;
	OBJECT *ob;

#define ADMENU 0
#define ADFFINFO 1

#define ADDINFO 3
#define DELABEL 4
#define DEVERSN 5
#define DECOPYRT 7

#define ADCPYDEL 7

	if (strcasecmp(rsx_basename(file->rsc_rsxname), "desktop") != 0)
		return FALSE;
	if ((rsctree = rsc_tree_index(file, ADDINFO, RT_UNKNOWN)) == NULL || rsctree->rt_type != RT_DIALOG)
		return FALSE;
	if (Objc_Count(rsctree->rt_objects.dial.di_tree, ROOT) < 10)
		return FALSE;
	ob = rsctree->rt_objects.dial.di_tree + 2;
	if (ob->ob_type != G_STRING)
		return FALSE;
	if (strcmp(ob->ob_spec.free_string, "- EmuTOS -") != 0)
		return FALSE;
	ob = rsctree->rt_objects.dial.di_tree + DELABEL;
	if (ob->ob_type != G_STRING)
		return FALSE;
	
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

/*
 *  trims trailing spaces from string
 */
static void trim_spaces(char *string)
{
	char *p;

	for (p = string + strlen(string) - 1; p >= string; p--)
		if (*p != ' ')
			break;
	p[1] = '\0';
}

/*** ---------------------------------------------------------------------- ***/

/*
 *  Align text objects according to special values in ob_flags
 *
 *  Translations typically have a length different from the original
 *  English text.  In order to keep dialogs looking tidy in all
 *  languages, it is often useful to centre- or right-align text
 *  objects.  The AES does not provide an easy way of doing this
 *  (alignment in TEDINFO objects affects the text within the object,
 *  as well as object positioning).
 *
 *  To allow centre- or right-alignment alignment of text objects,
 *  we steal unused bits in ob_flags to indicate the required
 *  alignment.  Note that this does not cause any incompatibilities
 *  because this extra function is performed outside the AES, and
 *  only for the internal desktop resource.  Furthermore, we zero
 *  out the stolen bits after performing the alignment.
 *
 *  Also note that this aligns the *object*, not the text within
 *  the object.  It is perfectly reasonable (and common) to have
 *  left-aligned text within a right-aligned TEDINFO object.
 */
static void align_objects(OBJECT *obj_array, int nobj)
{
	OBJECT *obj;
	char *p;
	_WORD len;		 /* string length in pixels */
	_WORD wchar, hchar;
	_WORD type;
	
	GetTextSize(&wchar, &hchar);
	for (obj = obj_array; --nobj >= 0; obj++)
	{
		type = obj->ob_type & OBTYPEMASK;
		switch (type)
		{
		case G_STRING:
		case G_TEXT:
		case G_FTEXT:
		case G_BOXTEXT:
		case G_FBOXTEXT:
			if (type == G_STRING)
				p = obj->ob_spec.free_string;
			else if (type == G_BOXTEXT || type == G_TEXT)
				p = obj->ob_spec.tedinfo->te_ptext;
			else
				p = obj->ob_spec.tedinfo->te_ptmplt;
			len = strlen(p) * wchar;
			if (obj->ob_flags & CENTRE_ALIGNED)
			{
				obj->ob_x += (obj->ob_width - len) / 2;
				if (obj->ob_x < 0)
					obj->ob_x = 0;
				obj->ob_width = len;
			} else if (obj->ob_flags & RIGHT_ALIGNED)
			{
				obj->ob_x += obj->ob_width - len;
				if (obj->ob_x < 0)
					obj->ob_x = 0;
				obj->ob_width = len;
			}
			/* obj->ob_flags &= ~(CENTRE_ALIGNED | RIGHT_ALIGNED); */
			break;
		default:
			break;
		}
	}
}

/*
 *  Horizontally centre dialog title: this is done dynamically to
 *  handle translated titles.
 *
 *  If object 1 of a tree is a G_STRING and its y position equals
 *  one character height, we assume it's the dialog title.
 */
static void centre_title(OBJECT *root)
{
	OBJECT *title;
	_WORD len;
	_WORD wchar, hchar;
	
	GetTextSize(&wchar, &hchar);

	title = root + 1;

	/* if object #1 is a dialog title, center it */
	if (title->ob_type == G_STRING && title->ob_y == hchar)
	{
		len = strlen(title->ob_spec.free_string) * wchar;
		if (len > root->ob_width)
			len = root->ob_width;
		title->ob_x = (root->ob_width - len) / 2;
	}
}

/*** ---------------------------------------------------------------------- ***/

static void centre_titles(RSCFILE *file)
{
	_ULONG i;
	
	for (i = 0; i < file->header.rsh_ntree; i++)
		centre_title(file->rs_trindex[i]);
}

/*** ---------------------------------------------------------------------- ***/

/*
 *  Change the sizes of the menus after translation
 */
static void adjust_menu(RSCFILE *file, _WORD treeindex)
{
	OBJECT *menu = file->rs_trindex[treeindex];
	_WORD i, j;	/* index in the menu bar */
	int n, x;
	_WORD mbar = menu_the_active(menu);
	_WORD the_menus;
	OBJECT *title;
	_WORD wchar, hchar;
	RSCTREE *tree = rsc_tree_index(file, treeindex, RT_UNKNOWN);
	
	GetTextSize(&wchar, &hchar);

	/*
	 * first, set ob_x & ob_width for all the menu headings, and
	 * determine the required width of the (translated) menu bar.
	 */
	for (i = menu[mbar].ob_head, x = 0; i != mbar; i = menu[i].ob_next, x += n)
	{
		title = &menu[i];
		n = strlen(title->ob_spec.free_string) * wchar;
		title->ob_x = x;
		title->ob_width = n;
	}
	menu[mbar].ob_width = x;

	/*
	 * finally we can set ob_x and ob_width for the pulldown objects within the menu
	 */
	the_menus = menu_the_menus(menu);
	j = menu[the_menus].ob_head;
	for (i = menu[mbar].ob_head; i != mbar; i = menu[i].ob_next)
	{
		int k, m;
		OBJECT *dropbox = &menu[j];
		_WORD separator_width;
		
		title = &menu[i];
		/* find widest object under this menu heading */
		separator_width = 0;
		for (k = dropbox->ob_head, m = 0; k != j; k = menu[k].ob_next)
		{
			OBJECT *item = &menu[k];
			int l;
			int is_separator = FALSE;
			char *str;
			
			if (item->ob_type == G_STRING)
			{
				/*
				 * english strings where not translated,
				 * trim their spaces now.
				 */
				trim_spaces(item->ob_spec.free_string);
				str = item->ob_spec.free_string;
				l = strlen(str);
				
				if ((item->ob_state & OS_DISABLED) && *str == '-')
				{
					if (separator_width != 0 && l != separator_width)
					{
						KINFO(("tree %s, object %s: mismatch in separator length (was %d, now %d)\n",
							tree->rt_name, ob_name_or_index(file, tree, k), separator_width, l));
					}
					separator_width = l;
					is_separator = TRUE;
				} else
				{
					/*
					 * correct for at least 1 space after last char
					 */
					if (l > 0 && str[l - 1] != ' ')
						l++;
				}
				if (!is_separator || file->rsc_emutos != EMUTOS_DESK)
				{
					if (m < l)
						m = l;
				}
			}
		}
		
		if (m == 0)
		{
			KINFO(("tree %s, object %s: empty menu\n",
				tree->rt_name, ob_name_or_index(file, tree, i)));
		}
		m *= wchar;
		
		dropbox->ob_x = menu[mbar].ob_x + title->ob_x;

		/* make sure the menu is not too far on the right of the screen */
#if 0
		if (dropbox->ob_x + m >= width)
		{
			dropbox->ob_x = width - m - 1;
		}
#endif

		for (k = dropbox->ob_head; k != j; k = menu[k].ob_next)
		{
			OBJECT *item = &menu[k];
			const char *str = item->ob_spec.free_string;
			int l = strlen(str);
			
			if (item->ob_type == G_STRING)
			{
				/*
				 * if there is a separator, the string should not exceeds its length
				 */
				if (!((item->ob_state & OS_DISABLED) && *item->ob_spec.free_string == '-'))
				{
					/*
					 * correct for at least 1 space after last char
					 */
					if (l > 0 && str[l - 1] != ' ')
						l++;
				} else if (file->rsc_emutos == EMUTOS_DESK)
				{
					int l = m / wchar;
					char *str = g_new(char, l + 1);
					memset(str, '-', l);
					str[l] = '\0';
					item->ob_spec.free_string = str; /* leaked */
				}
			}
			item->ob_width = m;
		}
		dropbox->ob_width = m;

		j = dropbox->ob_next;
	}
	KDEBUG(("desktop menu bar: x=0x%04x, w=0x%04x\n", mbar->ob_x, mbar->ob_width));
}


static void emutos_desktop_fix(RSCFILE *file)
{
	OBJECT *tree = file->rs_trindex[ADDINFO];
	static char version[80];
	static char copyright_year[12];
	time_t now;
	struct tm *tm;
	_WORD wchar, hchar;
	
	GetTextSize(&wchar, &hchar);
	
	now = time(NULL);
	tm = localtime(&now);
	sprintf(version, "%04d%02d%02d-%02d%02d%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	sprintf(copyright_year, "%04d", tm->tm_year + 1900);
	
	/* insert the version number */
	tree[DEVERSN].ob_spec.free_string = version;

	/* slightly adjust the about box for a timestamp build */
	if (version[1] != '.')
	{
		tree[DELABEL].ob_flags |= OF_HIDETREE;	/* hide the word "Version" */
		tree[DEVERSN].ob_x = 0;					/* and enlarge the version object  */
		tree[DEVERSN].ob_width = tree[ROOT].ob_width;
		tree[DEVERSN].ob_flags |= CENTRE_ALIGNED;
	}

	/* insert the version number */
	tree[DECOPYRT].ob_spec.free_string = copyright_year;

	/* adjust the size and coordinates of menu items */
	adjust_menu(file, ADMENU);

	/*
	 * perform special object alignment - this must be done after
	 * translation and coordinate fixing
	 */
	align_objects(file->rs_object, file->header.rsh_nobs);
	
	centre_titles(file);
}


#undef ADMENU
#undef ADFFINFO
#undef ADDINFO
#undef DELABEL
#undef DEVERSN
#undef DECOPYRT

static void emutos_aes_fix(RSCFILE *file)
{
	centre_titles(file);
}

/*** ---------------------------------------------------------------------- ***/

RSCFILE *load_all(const char *file_name, _UWORD flags)
{
	RSCFILE *file;
	char filename[PATH_MAX];
	_BOOL status;
	_BOOL def_found;
	_WORD state;
	
	static struct {
		const char *extension;
		_BOOL (*func)(RSCFILE *_file, const char *_filename, _BOOL *def_found);
		_LONG mask;
	} const def_tab[] = {
		{ "rso", rso_load, RF_RSO },
		{ "hrd", hrd_load, RF_HRD },
		{ "dfn", dfn_load, RF_DFN },
		{ "rsd", rsd_load, RF_NRSC },
		{ "def", rsd_load, RF_DEF },
	};
	
	file = xrsrc_load(file_name, flags);
	if (file == NULL)
		return NULL;
	
	file->rsc_extob.lang = RSC_LANG_DEFAULT;
	
	rsc_load_trees(file);
	
	if (is_emutos_desktop(file))
	{
		file->rsc_emutos = EMUTOS_DESK;
		file->rsc_output_prefix = g_strdup("desktop");
		file->rsc_output_basename = g_strdup("desk_rsc");
		file->rsc_flags2 |= RF_ROMABLE | RF_IMAGEWORDS;
		file->rsc_flags |= RF_CSOURCE2;
		file->rsc_emutos_frstrcond_name = g_strdup("STICNTYP");
		file->rsc_emutos_frstrcond_string = g_strdup("#ifndef TARGET_192");
		file->rsc_emutos_othercond_name = g_strdup("ADTTREZ");
		file->rsc_emutos_othercond_string = g_strdup("#ifndef TARGET_192");
		KDEBUG(("EmuTOS desktop resource loaded\n"));
	} else if (is_emutos_icon(file))
	{
		file->rsc_emutos = EMUTOS_ICONS;
		file->rsc_output_prefix = g_strdup("icons");
		file->rsc_output_basename = g_strdup("icons");
		file->rsc_flags2 |= RF_ROMABLE | RF_IMAGEWORDS;
		file->rsc_flags |= RF_CSOURCE2;
		KDEBUG(("EmuTOS icons resource loaded\n"));
	} else if (is_emutos_aes(file))
	{
		file->rsc_emutos = EMUTOS_AES;
		file->rsc_output_prefix = g_strdup("gem");
		file->rsc_output_basename = g_strdup("gem_rsc");
		file->rsc_flags2 |= RF_ROMABLE | RF_IMAGEWORDS;
		file->rsc_flags |= RF_CSOURCE2;
		file->rsc_emutos_othercond_name = g_strdup("APPS");
		file->rsc_emutos_othercond_string = g_strdup("#if 0");
		KDEBUG(("EmuTOS gem resource loaded\n"));
	}
	
	status = FALSE;
	strcpy(filename, file_name);
	def_found = FALSE;
	state = 0;
	while (def_found == FALSE && status == FALSE && state < (_WORD)(sizeof(def_tab) / sizeof(def_tab[0])))
	{
		set_extension(filename, def_tab[state].extension);
		status = (*(def_tab[state].func))(file, filename, &def_found);
		status = file_close(status);
		if (status)
			file->rsc_flags |= def_tab[state].mask;
		if (def_found)
			break;
		state++;
	}

	switch (file->rsc_emutos)
	{
	case EMUTOS_AES:
		emutos_aes_fix(file);
		break;
	case EMUTOS_DESK:
		emutos_desktop_fix(file);
		break;
	case EMUTOS_ICONS:
	case EMUTOS_NONE:
		break;
	}
	
	if (!(file->rsc_flags & RF_RSO))
	{
		rule_calc(&file->rsc_rule1);
		rule_calc(&file->rsc_rule2);
	}

	/*
	 * Silently remove the CRC string.
	 * It will be added again when the file is saved.
	 */
	if (file->rsc_opts.crc_string)
		rsc_remove_crc_string(file);
	
	return file;
}
