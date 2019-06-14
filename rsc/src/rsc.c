/*****************************************************************************
 * RSC.C
 * rsc tree functions
 *****************************************************************************/

#include "config.h"
#include "windows_.h"
#include <stdint.h>
#include <gem.h>
#include <mobject.h>
#include <rsc.h>
#include <xrsrc.h>
#include <ro_mem.h>
#include <fileio.h>
#include <time.h>
#include "aesutils.h"
#include "nls.h"

rsc_options op_rsc_opts = {
	'@',	/* ted_fillchar */
	2,		/* menu_leftmargin */
	1,		/* menu_rightmargin */
	2,		/* menu_minspace */
	FALSE,	/* menu_fillspace */
	500,	/* menu_maxchars */
	FALSE,	/* alert_limits */
	3,		/* alert_max_icon */
	30,		/* alert_max_linesize */
	FALSE,	/* crc_string */
	FALSE,	/* dummy_icons */
	TRUE,	/* ted_small_valid */
	0,		/* layer_save_mode */
	TRUE,	/* objc_size_check */
	FALSE,	/* magic_buttons */
	TRUE,	/* menu_keycheck */
};

#define FOR_FILE(a, b) ((a) == (b) ? (a) : (for_file ? (a) : (b)))

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void rsc_tree_count(RSCFILE *file)
{
	RSCTREE *tree;
	_LONG ntrees, nimages, nstrings;
	_LONG bghmore, bghuser;
	_LONG number;
	
	ntrees = nimages = nstrings = bghmore = bghuser = number = 0;
	FOR_ALL_RSC(file, tree)
	{
		tree->rt_number = number++;
		switch (tree->rt_type)
		{
		case RT_DIALOG:
		case RT_FREE:
		case RT_UNKNOWN:
		case RT_MENU:
			tree->rt_index = ntrees++;
			break;
		case RT_FRSTR:
		case RT_ALERT:
			tree->rt_index = nstrings++;
			break;
		case RT_FRIMG:
		case RT_MOUSE:
			tree->rt_index = nimages++;
			break;
		case RT_BUBBLEMORE:
			tree->rt_index = bghmore++;
			break;
		case RT_BUBBLEUSER:
			tree->rt_index = bghuser++;
			break;
		default: /* this should not happen */
			tree->rt_index = 0;
			break;
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

static _WORD obj_count(OBJECT *tree, _WORD parent, _WORD n)
{
	_WORD ob = parent == NIL ? ROOT : tree[parent].ob_head;

	do
	{
		/* OB_INDEX(ob) = n; */
		n++;
		if (tree[ob].ob_head != NIL)
			n = obj_count(tree, ob, n);
		ob = tree[ob].ob_next;
	} while (ob != parent);
	return n;
}

/*** ---------------------------------------------------------------------- ***/

void rsc_count_all(RSCFILE *file)
{
	RSCTREE *tree;
	
	FOR_ALL_RSC(file, tree)
	{
		switch (tree->rt_type)
		{
		case RT_DIALOG:
		case RT_FREE:
		case RT_UNKNOWN:
			if (tree->rt_objects.dial.di_tree)
				obj_count(tree->rt_objects.dial.di_tree, NIL, 0);
			break;
		case RT_MENU:
			if (tree->rt_objects.menu.mn_tree)
				obj_count(tree->rt_objects.menu.mn_tree, NIL, 0);
			break;
		case RT_FRSTR:
		case RT_ALERT:
			break;
		case RT_FRIMG:
		case RT_MOUSE:
			break;
		case RT_BUBBLEMORE:
		case RT_BUBBLEUSER:
			break;
		default: /* this should not happen */
			break;
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL rsc_insert_tree(RSCFILE *file, RSCTREE *tree, void *object)
{
	RSCTREE *next;
	_BOOL duplicates = FALSE;
	
	tree->rt_file = file;
	switch (tree->rt_type)
	{
	case RT_DIALOG:
	case RT_FREE:
	case RT_UNKNOWN:
		tree->rt_objects.dial.di_tree = (OBJECT *)object;
		tree->rt_objects.dial.di_popup = NULL;
		tree->rt_objects.dial.di_popup_tree = NULL;
		tree->rt_objects.dial.di_x = 0;
		tree->rt_objects.dial.di_y = 0;
		FOR_ALL_RSC(file, next)
		{
			if (next->rt_type == RT_FRIMG ||
				next->rt_type == RT_MOUSE ||
				next->rt_type == RT_FRSTR ||
				next->rt_type == RT_ALERT)
				break;
		}
		break;
	case RT_MENU:
		tree->rt_objects.menu.mn_tree = (OBJECT *)object;
		tree->rt_objects.menu.mn_title = NULL;
		tree->rt_objects.menu.mn_menu = NULL;
		tree->rt_objects.menu.mn_submenu = NULL;
		FOR_ALL_RSC(file, next)
		{
			if (next->rt_type == RT_FRIMG ||
				next->rt_type == RT_MOUSE ||
				next->rt_type == RT_FRSTR ||
				next->rt_type == RT_ALERT)
				break;
		}
		break;
	case RT_FRSTR:
		tree->rt_objects.str.fr_str = (char *)object;
		FOR_ALL_RSC(file, next)
		{
			if (next->rt_type == RT_FRIMG ||
				next->rt_type == RT_MOUSE)
				break;
		}
		break;
	case RT_ALERT:
		tree->rt_objects.alert.al_str = (char *)object;
		FOR_ALL_RSC(file, next)
		{
			if (next->rt_type == RT_FRIMG ||
				next->rt_type == RT_MOUSE)
				break;
		}
		break;
	case RT_FRIMG:
	case RT_MOUSE:
		tree->rt_objects.bit = (BITBLK *)object;
		FOR_ALL_RSC(file, next)
		{
			if (next->rt_type == RT_BUBBLEMORE ||
				next->rt_type == RT_BUBBLEUSER)
				break;
		}
		break;
	case RT_BUBBLEMORE:
	case RT_BUBBLEUSER:
		tree->rt_objects.bgh = (BGHINFO *)object;
		next = &file->rsc_treehead;
		break;
	default: /* this should not happen */
		return FALSE;
	}
	tree->rt_next = next;
	tree->rt_prev = next->rt_prev;
	tree->rt_prev->rt_next = tree;
	tree->rt_next->rt_prev = tree;
	file->rsc_ntrees++;
	rsc_tree_count(file);
	return duplicates;
}

/*** ---------------------------------------------------------------------- ***/

RSCTREE *rsc_add_tree(RSCFILE *file, _WORD type, const char *name, void *object)
{
	RSCTREE *tree;
	
	if ((tree = g_new0(RSCTREE, 1)) == NULL)
		return NULL;
	tree->rt_type = type;
	strcpy(tree->rt_name, name);
	tree->rt_cmnt = NULL;
	rsc_insert_tree(file, tree, object);
	return tree;
}

/*** ---------------------------------------------------------------------- ***/

static void rsc_tree_dispose(RSCTREE *tree)
{
	switch (tree->rt_type)
	{
	case RT_DIALOG:
	case RT_FREE:
	case RT_UNKNOWN:
		tree->rt_objects.dial.di_tree = NULL;
		break;
	case RT_MENU:
		tree->rt_objects.menu.mn_tree = NULL;
		break;
	case RT_FRSTR:
#if 0
		g_free(tree->rt_objects.str.fr_str);
#endif
		bgh_delete(tree->rt_objects.str.fr_bgh);
		break;
	case RT_ALERT:
#if 0
		g_free(tree->rt_objects.alert.al_str);
#endif
		bgh_delete(tree->rt_objects.alert.al_bgh);
		break;
	case RT_FRIMG:
	case RT_MOUSE:
		/* bit_free(tree->rt_objects.bit); */
		break;
	case RT_BUBBLEUSER:
	case RT_BUBBLEMORE:
		bgh_delete(tree->rt_objects.bgh);
		break;
	}
	g_free(tree->rt_cmnt);
	g_free(tree->rt_obnames);
	g_free(tree);
}

/*** ---------------------------------------------------------------------- ***/

void rsc_tree_delete(RSCTREE *tree)
{
	RSCFILE *file = tree->rt_file;
	
	tree->rt_prev->rt_next = tree->rt_next;
	tree->rt_next->rt_prev = tree->rt_prev;
	file->rsc_ntrees--;
	rsc_tree_count(file);
	rsc_tree_dispose(tree);
}

/*** ---------------------------------------------------------------------- ***/

static NAMERULE const op_rule1 = { TRUE, FALSE, TRUE, FALSE, { 0 }, { 0 }, { 0 } };
static NAMERULE const op_rule2 = { TRUE, FALSE, FALSE, TRUE, "_", { 0 }, { 0 } };

/*** ---------------------------------------------------------------------- ***/

static void rule_bitset(CSET cset, _WORD ch)
{
	_WORD i, bit;

	i = ch / 32;
	bit = ch % 32;
	cset[i] |= 1l << bit;
}

/*** ---------------------------------------------------------------------- ***/

static void rule_bitclr(CSET cset, _WORD ch)
{
	_WORD i, bit;

	i = ch / 32;
	bit = ch % 32;
	cset[i] &= ~(1l << bit);
}

void rule_calc(NAMERULE *rule)
{
	_WORD i;
	_UBYTE *p;

	for (i = 0; i < (_WORD)(sizeof(rule->charset) / sizeof(rule->charset[0])); i++)
		rule->charset[i] = 0;
	if (rule->alpha || rule->alnum)
	{
		for (i = 'A'; i <= 'Z'; i++)
			rule_bitset(rule->charset, i);
		for (i = 'a'; i <= 'z'; i++)
			rule_bitset(rule->charset, i);
	}
	if (rule->alnum)
	{
		for (i = '0'; i <= '9'; i++)
			rule_bitset(rule->charset, i);
	}
	p = rule->add;
	while (*p)
		rule_bitset(rule->charset, *p++ & 0xff);
	p = rule->sub;
	while (*p)
		rule_bitclr(rule->charset, *p++ & 0xff);
}

void rsc_init_file(RSCFILE *file)
{
	file->rsc_ntrees = 0;
	file->rsc_treehead.rt_next =
	file->rsc_treehead.rt_prev = &file->rsc_treehead;
	file->rsc_flags = RF_C | RF_RSO | RF_RSC;
	file->rsc_flags2 = 0;
	file->rsc_extob.mode = EXTOB_NONE;
	file->rsc_extob.lang = RSC_LANG_DEFAULT;
	file->rsc_namelen = MAXNAMELEN;
	file->rsc_exact = TRUE;
	file->rsc_rule1 = op_rule1;
	rule_calc(&file->rsc_rule1);
	file->rsc_rule2 = op_rule2;
	rule_calc(&file->rsc_rule2);
	file->rsc_cmnt = NULL;
	file->rsc_emutos = EMUTOS_NONE;
	file->rsc_emutos_frstrcond_name = NULL;
	file->rsc_emutos_frstrcond_string = NULL;
	file->rsc_emutos_othercond_name = NULL;
	file->rsc_emutos_othercond_string = NULL;
	file->rsc_use_oldlang = FALSE;
	file->rsc_output_prefix = NULL;
	file->rsc_output_basename = NULL;
	file->rsc_date_created = time(NULL);
	file->rsc_date_changed = 0;
	file->rsc_edition = 0;
	file->rsc_swap_flag = FALSE;
	file->rsc_xrsc_flag = FALSE;
	file->rsc_opts = op_rsc_opts;
	file->rsc_rsm_crc = RSC_CRC_NONE;
	file->rsc_opts.crc_string = FALSE;
	file->rsc_crc_for_string = RSC_CRC_NONE;
	file->had_rsm_hdr = FALSE;
	file->need_rsm_hdr = FALSE;
	file->fontset = CHARSET_ST;
}

/*** ---------------------------------------------------------------------- ***/

RSC_RSM_CRC rsc_rsm_calc_crc(const void *buf, _ULONG size)
{
	RSC_RSM_CRC crc = 0x5555;
	const signed char *p = (const signed char *) buf;
	_ULONG i;
	
	for (i = 0; i < size; i++)
		crc += *p++;
	return crc & 0x7fff;
}

/*** ---------------------------------------------------------------------- ***/

RSCFILE *rsc_new_file(const char *filename, const char *basename)
{
	RSCFILE *file;
	
	if ((file = g_new0(RSCFILE, 1)) == NULL)
		return NULL;
	if (filename != NULL)
	{
		strcpy(file->rsc_rsxfilename, filename);
	} else
	{
		strcpy(file->rsc_rsxfilename, "");
	}
	strcpy(file->rsc_rsxname, basename);
	rsc_init_file(file);
	return file;
}

/*** ---------------------------------------------------------------------- ***/

static void rsc_lang_deletelist(rsc_lang **list)
{
	rsc_lang *lang, *next;
	
	for (lang = *list; lang; lang = next)
	{
		next = lang->next;
		g_free(lang->id);
		g_free(lang->charset);
		g_free(lang->filename);
		g_free(lang);
	}
	*list = NULL;
}

/*** ---------------------------------------------------------------------- ***/

_BOOL rsc_lang_add(rsc_lang **list, const char *id, const char *charset, const char *filename)
{
	rsc_lang *lang;
	
	while (*list)
		list = &(*list)->next;
	lang = g_new(rsc_lang, 1);
	if (lang == NULL)
		return FALSE;
	lang->id = g_strdup(id);
	lang->charset = g_strdup(charset);
	lang->filename = g_strdup(filename);
	lang->next = NULL;
	*list = lang;
	return TRUE;
}
	
/*** ---------------------------------------------------------------------- ***/

void rsc_file_delete(RSCFILE *file, _BOOL all)
{
	while (file->rsc_ntrees != 0)
		rsc_tree_delete(file->rsc_treehead.rt_next);
#if 0 /* NYI here */
	for (i = 0; i < file->rsc_num_extensions; i++)
	{
		if (file->rsc_extensions[i].ext_size != 0)
		{
			_UBYTE *cp = (_UBYTE *)(file->rsc_extensions[i].ext_ptr);
			g_free(cp);
		}
	}
	g_free(file->rsc_extob.overlay_id);
	file->rsc_extob.overlay_id = NULL;
#endif
	if (all)
	{
#if 0 /* NYI here */
		rsc_module_deletelist(&file->rsc_modules);
		g_free(file->rsc_extensions);
#endif
		rsc_lang_deletelist(&file->rsc_langs);
		g_free(file->rsc_cmnt);
		g_free(file->rsc_emutos_frstrcond_name);
		g_free(file->rsc_emutos_frstrcond_string);
		g_free(file->rsc_emutos_othercond_name);
		g_free(file->rsc_emutos_othercond_string);
		g_free(file->rsc_output_prefix);
		g_free(file->rsc_output_basename);
		g_free(file);
	} else
	{
		rsc_lang_deletelist(&file->rsc_langs);
		g_free(file->rsc_cmnt);
		g_free(file->rsc_emutos_frstrcond_name);
		g_free(file->rsc_emutos_frstrcond_string);
		g_free(file->rsc_emutos_othercond_name);
		g_free(file->rsc_emutos_othercond_string);
		g_free(file->rsc_output_prefix);
		g_free(file->rsc_output_basename);
		rsc_init_file(file);
	}
}

/*** ---------------------------------------------------------------------- ***/

RSC_RSM_CRC rsc_get_crc_string(const char *str)
{
	RSC_RSM_CRC crc;
	
	if (str == NULL)
		return RSC_CRC_NONE;
	if (strlen(str) != (RSM_CRC_STRLEN - 1))
		return RSC_CRC_NONE;
	if (strncmp(str, RSM_CRC_STRING, 11) != 0)
		return RSC_CRC_NONE;
	if (strcmp(str + 15, RSM_CRC_STRING + 15) != 0)
		return RSC_CRC_NONE;
	crc = (RSC_RSM_CRC)strtoul(str + 11, NULL, 16);
	return crc;
}

/*** ---------------------------------------------------------------------- ***/

_BOOL rsc_is_crc_string(const char *str)
{
	return rsc_get_crc_string(str) != RSC_CRC_NONE;
}

/*** ---------------------------------------------------------------------- ***/

void rsc_remove_crc_string(RSCFILE *file)
{
	RSCTREE *tree;
	
	FOR_ALL_RSC(file, tree)
	{
		if (tree->rt_type == RT_FRSTR &&
			rsc_is_crc_string(tree->rt_objects.str.fr_str))
		{
			/* like rsc_tree_delete(), but dont mark the just loaded file as changed */
			tree->rt_prev->rt_next = tree->rt_next;
			tree->rt_next->rt_prev = tree->rt_prev;
			file->rsc_ntrees--;
			rsc_tree_count(file);
			rsc_tree_dispose(tree);
			break;
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

const char *ob_name(RSCFILE *file, RSCTREE *tree, _WORD ob)
{
	const char *p;
	
	if (file == NULL || tree == NULL || ob < 0 || ob >= tree->rt_nobs)
		return NULL;
	p = tree->rt_obnames + ob * (MAXNAMELEN + 1);
	if (*p == '\0')
		return NULL;
	return p;
}

/*** ---------------------------------------------------------------------- ***/

const char *ob_name_or_index(RSCFILE *file, RSCTREE *tree, _WORD ob)
{
	static char namebuf[4][MAXNAMELEN + 1];
	static int namebufidx = 0;
	const char *name;
	
	if ((name = ob_name(file, tree, ob)) == NULL)
	{
		sprintf(namebuf[namebufidx], "#%u", ob);
		name = namebuf[namebufidx++];
		namebufidx %= 4;
	}
	return name;
}

/*** ---------------------------------------------------------------------- ***/

_BOOL ob_setname(RSCFILE *file, RSCTREE *tree, _WORD ob, const char *name, size_t maxlen)
{
	char *p;
	
	if (file == NULL || tree == NULL || ob < 0 || ob >= tree->rt_nobs)
		return FALSE;
	p = tree->rt_obnames + ob * (MAXNAMELEN + 1);
	strmaxcpy(p, maxlen, name);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

const char *ob_cmnt(RSCFILE *file, RSCTREE *tree, _WORD ob)
{
	UNUSED(file);
	UNUSED(tree);
	UNUSED(ob);
	return NULL;
}

/*** ---------------------------------------------------------------------- ***/

static _LONG count_objs(RSCTREE *rsctree, _WORD parent, _LONG idx, XRS_HEADER *xrsc_header, RSCFILE *file, rsc_counter *counter, _BOOL for_file)
{
	_WORD mob;
	OBJECT *tree = rsctree->rt_objects.dial.di_tree;
	
	mob = parent == NIL ? ROOT : tree[parent].ob_head;
	do
	{
		/* OB_INDEX(mob) = (_WORD)idx; */
		idx++;
		if (ob_name(file, rsctree, mob))
			counter->nnames++;
		switch (tree[mob].ob_type & OBTYPEMASK)
		{
		case G_STRING:
		case G_TITLE:
		case G_BUTTON:
		case G_SHORTCUT:
			counter->string_space_objects += strlen(tree[mob].ob_spec.free_string) + 1;
			file->rsc_nstrings += 1;
			break;
		
		case G_BOX:
		case G_IBOX:
		case G_BOXCHAR:
		case G_EXTBOX:
			break;
		
		case G_TEXT:
		case G_BOXTEXT:
			if (xrsc_header != NULL)
			{
				xrsc_header->rsh_nted++;
			}
			counter->string_space_objects += strlen(tree[mob].ob_spec.tedinfo->te_ptext) + 1;
			counter->string_space_objects += strlen(tree[mob].ob_spec.tedinfo->te_pvalid) + 1;
			counter->string_space_objects += strlen(tree[mob].ob_spec.tedinfo->te_ptmplt) + 1;
			counter->total_size += FOR_FILE(RSC_SIZEOF_TEDINFO, sizeof(TEDINFO));
			file->rsc_nstrings += 3;
			break;
		
		case G_FTEXT:
		case G_FBOXTEXT:
			if (xrsc_header != NULL)
			{
				xrsc_header->rsh_nted++;
			}
			counter->string_space_objects += tree[mob].ob_spec.tedinfo->te_txtlen;
			counter->string_space_objects += strlen(tree[mob].ob_spec.tedinfo->te_pvalid) + 1;
			counter->string_space_objects += strlen(tree[mob].ob_spec.tedinfo->te_ptmplt) + 1;
			counter->total_size += FOR_FILE(RSC_SIZEOF_TEDINFO, sizeof(TEDINFO));
			file->rsc_nstrings += 3;
			break;
		
		case G_IMAGE:
			if (xrsc_header != NULL)
			{
				xrsc_header->rsh_nbb++;
			}
			counter->imdata_size += bitblk_datasize(tree[mob].ob_spec.bitblk);
			if (counter->imdata_size & 1)
				counter->imdata_size++;
			counter->total_size += FOR_FILE(RSC_SIZEOF_BITBLK, sizeof(BITBLK));
			file->rsc_nimages += 1;
			break;
		
		case G_ICON:
			if (xrsc_header != NULL)
			{
				xrsc_header->rsh_nib++;
			}
			counter->string_space_objects += strlen(tree[mob].ob_spec.iconblk->ib_ptext) + 1;
			counter->imdata_size += iconblk_masksize(tree[mob].ob_spec.iconblk) * 2;
			counter->total_size += FOR_FILE(RSC_SIZEOF_ICONBLK, sizeof(ICONBLK));
			file->rsc_nstrings += 1;
			file->rsc_nimages += 2;
			break;
		
		case G_CICON:
			{
				CICON *cicon;
				_LONG size;
				_WORD len;

				file->rsc_nciconblks++;
				counter->cstring_space += CICON_STR_SIZE;
				len = (_WORD)strlen(tree[mob].ob_spec.ciconblk->monoblk.ib_ptext);
				if (len > CICON_STR_SIZE)
					counter->string_space_objects += len + 1;
				size = iconblk_masksize(&tree[mob].ob_spec.ciconblk->monoblk);
				counter->cimdata_size += size * 2;
				counter->ctotal_size += FOR_FILE(RSC_SIZEOF_CICONBLK, sizeof(CICONBLK));
				cicon = tree[mob].ob_spec.ciconblk->mainlist;
				while (cicon != NULL)
				{
					counter->cimdata_size += size * (cicon->num_planes + 1); /* col_data * num_planes + col_mask */
					if (cicon->sel_data != 0)
						counter->cimdata_size += size * (cicon->num_planes + 1);
					counter->ctotal_size += FOR_FILE(RSC_SIZEOF_CICON, sizeof(CICON));
					file->rsc_ncicons++;
					cicon = cicon->next_res;
				}
			}
			file->rsc_nstrings += 1;
			break;
		
		case G_USERDEF:
			file->rsc_nuserblks++;
			counter->total_size += FOR_FILE(RSC_SIZEOF_USERBLK, sizeof(USERBLK));
			break;
		}
		if (tree[mob].ob_head != NIL)
			idx = count_objs(rsctree, mob, idx, xrsc_header, file, counter, for_file);
		mob = tree[mob].ob_next;
		counter->total_size += FOR_FILE(RSC_SIZEOF_OBJECT, sizeof(OBJECT));
	} while (mob != parent);
	return idx;
}

/*** ---------------------------------------------------------------------- ***/

void count_init(XRS_HEADER *xrsc_header, RSCFILE *file, rsc_counter *counter)
{
	counter->types.menus = 0;
	counter->types.dialogs = 0;
	counter->types.alerts = 0;
	counter->types.strings = 0;
	counter->types.images = 0;
	counter->types.bgh = 0;
	counter->types.bgh_more = 0;
	counter->types.bgh_user = 0;
	xrsc_header->rsh_nobs = 0;
	xrsc_header->rsh_ntree = 0;
	xrsc_header->rsh_nted = 0;
	xrsc_header->rsh_nib = 0;
	xrsc_header->rsh_nbb = 0;
	xrsc_header->rsh_nstring = 0;
	xrsc_header->rsh_nimages = 0;
	file->rsc_nciconblks = 0;
	file->rsc_ncicons = 0;
	file->rsc_nuserblks = 0;
	file->rsc_nstrings = 0;
	file->rsc_nimages = 0;
	counter->total_size = 0;
	counter->ctotal_size = 0;
	counter->ext_size = 0;
	counter->string_space_objects = 0;
	counter->string_space_free = 0;
	counter->string_space_total = 0;
	counter->userblks = 0;
	counter->imdata_size = 0;
	counter->cimdata_size = 0;
	counter->cstring_space = 0;
	counter->nnames = 0;
	counter->crc_for_string = 0x5555;
	counter->conditional.trees = -1;
	counter->conditional.objects = -1;
	counter->conditional.tedinfos = -1;
	counter->conditional.iconblks = -1;
	counter->conditional.ciconblks = -1;
	counter->conditional.cicons = -1;
	counter->conditional.bitblks = -1;
	counter->conditional.frstr = -1;
	counter->conditional.strings = -1;
	counter->conditional.images = -1;
}

/*** ---------------------------------------------------------------------- ***/

void calc_offsets(XRS_HEADER *xrsc_header, RSCFILE *file, rsc_counter *counter, _BOOL for_file)
{
	size_t offset;
	
	counter->string_space_total = counter->string_space_objects + counter->string_space_free;
	counter->total_size += counter->string_space_total;
	if (counter->total_size & 1)
		counter->total_size++;
	counter->total_size += counter->imdata_size;
	counter->total_size += FOR_FILE(RSC_SIZEOF_XRS_HEADER, sizeof(XRS_HEADER));
	counter->total_size += xrsc_header->rsh_ntree * FOR_FILE(RSC_SIZEOF_PTR, sizeof(OBJECT *));
	counter->total_size += xrsc_header->rsh_nimages * FOR_FILE(RSC_SIZEOF_PTR, sizeof(BITBLK *));
	counter->total_size += xrsc_header->rsh_nstring * FOR_FILE(RSC_SIZEOF_PTR, sizeof(char *));
	
	/*
	 * start after header
	 */
	offset = FOR_FILE(RSC_SIZEOF_XRS_HEADER, sizeof(XRS_HEADER));
	xrsc_header->rsh_vrsn = 1;
	if (counter->total_size > (RS_THRESHOLD + XRS_DIFF_SIZE))
		xrsc_header->rsh_vrsn = 3;
	xrsc_header->rsh_extvrsn = XRSC_VRSN_ORCS;
	/*
	 * reserve space for G_USERDEFs. Something that DRI forgot.
	 */
	offset += (size_t)(file->rsc_nuserblks * FOR_FILE(RSC_SIZEOF_USERBLK, sizeof(USERBLK)));
	if (file->rsc_flags2 & RF_ALTORDER)
	{
		/*
		 * alternate output order: only needed to write TOS ROM resources
		 * in their original order
		 */
		xrsc_header->rsh_trindex = offset;
		offset += (size_t)(xrsc_header->rsh_ntree * FOR_FILE(RSC_SIZEOF_PTR, sizeof(OBJECT *)));
		xrsc_header->rsh_object = offset;
		offset += (size_t)(xrsc_header->rsh_nobs * FOR_FILE(RSC_SIZEOF_OBJECT, sizeof(OBJECT)));
		xrsc_header->rsh_tedinfo = offset;
		offset += (size_t)(xrsc_header->rsh_nted * FOR_FILE(RSC_SIZEOF_TEDINFO, sizeof(TEDINFO)));
		xrsc_header->rsh_iconblk = offset;
		offset += (size_t)(xrsc_header->rsh_nib * FOR_FILE(RSC_SIZEOF_ICONBLK, sizeof(ICONBLK)));
		xrsc_header->rsh_bitblk = offset;
		offset += (size_t)(xrsc_header->rsh_nbb * FOR_FILE(RSC_SIZEOF_BITBLK, sizeof(BITBLK)));
		xrsc_header->rsh_frstr = offset;
		offset += (size_t)(xrsc_header->rsh_nstring * FOR_FILE(RSC_SIZEOF_PTR, sizeof(char *)));
		xrsc_header->rsh_string = offset;
		offset += (size_t)counter->string_space_total;
		if (offset & 1)
			offset++;
		xrsc_header->rsh_frimg = offset;
		offset += (size_t)(xrsc_header->rsh_nimages * FOR_FILE(RSC_SIZEOF_PTR, sizeof(BITBLK *)));
		xrsc_header->rsh_imdata = offset;
		offset += (size_t)counter->imdata_size;
	} else
	{
		xrsc_header->rsh_string = offset;
		offset += (size_t)counter->string_space_total;
		if (offset & 1)
			offset++;
		xrsc_header->rsh_imdata = offset;
		offset += (size_t)counter->imdata_size;
		xrsc_header->rsh_frstr = offset;
		offset += (size_t)(xrsc_header->rsh_nstring * FOR_FILE(RSC_SIZEOF_PTR, sizeof(char *)));
		xrsc_header->rsh_bitblk = offset;
		offset += (size_t)(xrsc_header->rsh_nbb * FOR_FILE(RSC_SIZEOF_BITBLK, sizeof(BITBLK)));
		xrsc_header->rsh_frimg = offset;
		offset += (size_t)(xrsc_header->rsh_nimages * FOR_FILE(RSC_SIZEOF_PTR, sizeof(BITBLK *)));
		xrsc_header->rsh_tedinfo = offset;
		offset += (size_t)(xrsc_header->rsh_nted * FOR_FILE(RSC_SIZEOF_TEDINFO, sizeof(TEDINFO)));
		xrsc_header->rsh_iconblk = offset;
		offset += (size_t)(xrsc_header->rsh_nib * FOR_FILE(RSC_SIZEOF_ICONBLK, sizeof(ICONBLK)));
		xrsc_header->rsh_object = offset;
		offset += (size_t)(xrsc_header->rsh_nobs * FOR_FILE(RSC_SIZEOF_OBJECT, sizeof(OBJECT)));
		xrsc_header->rsh_trindex = offset;
		offset += (size_t)(xrsc_header->rsh_ntree * FOR_FILE(RSC_SIZEOF_PTR, sizeof(OBJECT *)));
	}
	xrsc_header->rsh_rssize = counter->total_size;
	UNUSED(offset);

	counter->ctotal_size += counter->cstring_space;
	counter->ctotal_size += counter->cimdata_size;
	if (file->rsc_nciconblks != 0)
	{
		xrsc_header->rsh_vrsn |= RSC_EXT_FLAG;
		counter->ctotal_size += (file->rsc_nciconblks + 1) * FOR_FILE(RSC_SIZEOF_LONG, sizeof(_LONG));
	}

	(void) for_file;
}

/*** ---------------------------------------------------------------------- ***/

void count_trees(RSCFILE *file, XRS_HEADER *xrsc_header, rsc_counter *counter, _BOOL for_file)
{
	RSCTREE *tree;
	OBJECT *ob;

	count_init(xrsc_header, file, counter);
	FOR_ALL_RSC(file, tree)
	{
		switch (tree->rt_type)
		{
		case RT_DIALOG:
		case RT_FREE:
		case RT_UNKNOWN:
		case RT_MENU:
			if (tree->rt_type == RT_MENU)
			{
				if ((ob = tree->rt_objects.menu.mn_tree) != NULL)
					counter->types.menus += 1;
			} else
			{
				if ((ob = tree->rt_objects.dial.di_tree) != NULL)
					counter->types.dialogs += 1;
			}
			if (file->rsc_emutos_othercond_name &&
				file->rsc_emutos_othercond_string &&
				counter->conditional.trees < 0 &&
				strcmp(tree->rt_name, file->rsc_emutos_othercond_name) == 0)
			{
				counter->conditional.trees = xrsc_header->rsh_ntree;
				counter->conditional.objects = xrsc_header->rsh_nobs;
				counter->conditional.tedinfos = xrsc_header->rsh_nted;
				counter->conditional.iconblks = xrsc_header->rsh_nib;
				counter->conditional.ciconblks = file->rsc_nciconblks;
				counter->conditional.cicons = file->rsc_ncicons;
				counter->conditional.strings = file->rsc_nstrings;
				counter->conditional.bitblks = xrsc_header->rsh_nbb;
				counter->conditional.images = file->rsc_nimages;
			}
			if (ob != NULL)
			{
				xrsc_header->rsh_nobs += count_objs(tree, NIL, 0l, xrsc_header, file, counter, for_file);
				tree->rt_index = xrsc_header->rsh_ntree++;
				counter->nnames += 1;
			}
			break;
		case RT_FRSTR:
			if (file->rsc_emutos_frstrcond_name &&
				file->rsc_emutos_frstrcond_string &&
				counter->conditional.frstr < 0 &&
				strcmp(tree->rt_name, file->rsc_emutos_frstrcond_name) == 0)
			{
				counter->conditional.frstr = xrsc_header->rsh_nstring;
			}
			counter->types.strings += 1;
			counter->string_space_free += strlen(tree->rt_objects.str.fr_str) + 1;
			tree->rt_index = xrsc_header->rsh_nstring;
			xrsc_header->rsh_nstring += 1;
			counter->nnames += 1;
			file->rsc_nstrings += 1;
			break;
		case RT_ALERT:
			if (file->rsc_emutos_frstrcond_name &&
				file->rsc_emutos_frstrcond_string &&
				counter->conditional.frstr < 0 &&
				strcmp(tree->rt_name, file->rsc_emutos_frstrcond_name) == 0)
			{
				counter->conditional.frstr = file->rsc_nstrings;
			}
			counter->types.alerts += 1;
			counter->string_space_free += strlen(tree->rt_objects.alert.al_str) + 1;
			tree->rt_index = xrsc_header->rsh_nstring;
			xrsc_header->rsh_nstring += 1;
			counter->nnames++;
			file->rsc_nstrings += 1;
			break;
		case RT_FRIMG:
		case RT_MOUSE:
			counter->types.images += 1;
			xrsc_header->rsh_nbb += 1;
			counter->imdata_size += bitblk_datasize(tree->rt_objects.bit);
			if (counter->imdata_size & 1)
				counter->imdata_size++;
			tree->rt_index = xrsc_header->rsh_nimages;
			xrsc_header->rsh_nimages += 1;
			counter->total_size += FOR_FILE(RSC_SIZEOF_BITBLK, sizeof(BITBLK));
			counter->nnames += 1;
			file->rsc_nimages += 1;
			break;
		case RT_BUBBLEUSER:
			counter->types.bgh++;
			counter->types.bgh_user += bgh_count(tree->rt_objects.bgh);
			break;
		case RT_BUBBLEMORE:
			counter->types.bgh++;
			counter->types.bgh_more += bgh_count(tree->rt_objects.bgh);
			break;
		}
	}

	if (file->rsc_opts.crc_string)
	{
		counter->types.strings += 1;
		counter->string_space_free += RSM_CRC_STRLEN;
		xrsc_header->rsh_nstring += 1;
		counter->nnames += 1;
		file->rsc_nstrings += 1;
	}
	
	calc_offsets(xrsc_header, file, counter, for_file);
}

/*** ---------------------------------------------------------------------- ***/

_BOOL is_mouseform(BITBLK *bit)
{
	MFORM *mform;
	
	if (bit == NULL ||
		bit->bi_wb != 2 ||
		bit->bi_hl != 37 ||
		bit->bi_color < 0 ||
		bit->bi_color >= 16 ||
		(mform = (MFORM *)bit->bi_pdata) == NULL ||
		mform->mf_fg < 0 ||
		mform->mf_fg >= 16 ||
		mform->mf_bg < 0 ||
		mform->mf_bg >= 16 ||
		mform->mf_nplanes != 1 ||
		mform->mf_xhot < 0 ||
		mform->mf_xhot >= 16 ||
		mform->mf_yhot < 0 ||
		mform->mf_yhot >= 16)
		return FALSE;
	return TRUE;
}
