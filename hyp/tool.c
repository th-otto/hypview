#include "hypdefs.h"
#include "hypdebug.h"
#include "picture.h"


/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/*
 * FIXME: this can actually give 2 results, e.g. "index" and "Index"
 */
hyp_nodenr find_nr_by_title(HYP_DOCUMENT *hyp, const char *title, gboolean last)
{
	hyp_nodenr i;
	int res;
	hyp_nodenr found = HYP_NOINDEX;
	
	for (i = 0; i < hyp->num_index; i++)
	{
		if (HYP_NODE_IS_TEXT(hyp->indextable[i]->type))
		{
			INDEX_ENTRY *entry = hyp->indextable[i];
#ifdef NO_UTF8
			res = strncasecmp(title, (const char *)entry->name, entry->length - SIZEOF_INDEX_ENTRY);
#else
			char *name = hyp_conv_to_utf8(hyp->comp_charset, entry->name, entry->length - SIZEOF_INDEX_ENTRY);
			res = hyp_utf8_strcasecmp(name, title);
			g_free(name);
#endif
			if (res == 0)
			{
				found = i;
				if (!last)
					break;
			}
		}
	}
	return found;
}

/* ------------------------------------------------------------------------- */

gboolean hyp_node_find_windowtitle(HYP_NODE *nodeptr)
{
	const unsigned char *src;
	const unsigned char *end;

	src = nodeptr->start;
	end = nodeptr->end;

	while (src < end && *src == HYP_ESC)
	{
		if (src[1] == HYP_ESC_WINDOWTITLE)
		{
			nodeptr->window_title = src + 2;
			return TRUE;
		}
		src = hyp_skip_esc(src);
	}
	return FALSE;
}

/* ------------------------------------------------------------------------- */

hyp_nodenr hyp_node_find_objref(HYP_NODE *nodeptr, _WORD tree, _WORD obj, hyp_lineno *line)
{
	const unsigned char *src;
	const unsigned char *end;

	src = nodeptr->start;
	end = nodeptr->end;

	while (src < end && *src == HYP_ESC)
	{
		if (src[1] == HYP_ESC_OBJTABLE)
		{
			if (DEC_255(&src[4]) == tree && DEC_255(&src[6]) == obj)
			{
				*line = DEC_255(&src[2]);
				return DEC_255(&src[8]);
			}
		}
		src = hyp_skip_esc(src);
	}
	return HYP_NOINDEX;
}

/* ------------------------------------------------------------------------- */

/*
 * bad design. why does not every esc sequence just contain a length byte?
 */
const unsigned char *hyp_skip_esc(const unsigned char *pos)
{
	if (*pos != HYP_ESC)				/* no more escapes */
		return pos;

	pos++;								/* skip ESC marker */
	switch (*pos)						/* what code? */
	{
	case HYP_ESC_ESC:					/* ESC character */
		pos++;
		break;
	case HYP_ESC_PIC:					/* image */
		pos += 8;
		break;
	case HYP_ESC_LINE:					/* other gfx */
	case HYP_ESC_BOX:
	case HYP_ESC_RBOX:
		pos += 7;
		break;
	case HYP_ESC_LINK_LINE:				/* link */
	case HYP_ESC_ALINK_LINE:
		pos += 2;
		/* fall through */
	case HYP_ESC_LINK:
	case HYP_ESC_ALINK:
		pos += (pos[3] - HYP_STRLEN_OFFSET + 1) + 3;
		break;
	case HYP_ESC_EXTERNAL_REFS:			/* up to 12 xref entries */
		if (pos[1] < 5u)
			pos += 4;
		else
			pos += pos[1] - 1;			/* skip data */
		break;
	case HYP_ESC_CASE_DATA:				/* data blocks */
		if (pos[1] < 3u)
			pos += 2;
		else
			pos += pos[1] - 1;			/* skip data */
		break;
	case HYP_ESC_WINDOWTITLE:
		pos += ustrlen(pos) + 1;		/* @title, skip data */
		break;
	case HYP_ESC_OBJTABLE:				/* @tree */
		pos += 9;						/* skip data */
		break;
	case HYP_ESC_CASE_TEXTATTR:			/* @{UBISGO} attribute */
		pos++;
		break;
	case HYP_ESC_ATTR_TYPEWRITER:		/* from hyp2gdos.hyp */
		/* actually same as text attributes in range 0xa4-0xe3,
		   but using typewriter font for subsequent output */
		pos++;
		break;
	case HYP_ESC_FG_COLOR:				/* foreground color */
	case HYP_ESC_BG_COLOR:				/* background color */
		pos += 2;
		break;
	default:
		HYP_DBG(("unknown Tag: %u", *pos));
		break;
	}
	return pos;
}

/* ------------------------------------------------------------------------- */

hyp_nodenr hyp_first_text_page(HYP_DOCUMENT *hyp)
{
	hyp_nodenr node_num;
	
	node_num = 0;
	while (node_num < hyp->num_index)
	{
		if (HYP_NODE_IS_TEXT(hyp->indextable[node_num]->type))
			return node_num;
		node_num++;
	}
	return HYP_NOINDEX;
}

/* ------------------------------------------------------------------------- */

hyp_nodenr hyp_last_text_page(HYP_DOCUMENT *hyp)
{
	hyp_nodenr node_num;
	
	node_num = hyp->num_index;
	while (node_num > 0)
	{
		--node_num;
		if (HYP_NODE_IS_TEXT(hyp->indextable[node_num]->type))
			return node_num;
	}
	return HYP_NOINDEX;
}

/* ------------------------------------------------------------------------- */

int __my_assert(const char *expr, const char *file, int line)
{
	fflush(stdout);
	fprintf(stderr, "\nassertion failed: file %s, line %d:\n%s\n", file, line, expr);
	fflush(stderr);
	abort();
	return 1;
}

/* ------------------------------------------------------------------------- */

gboolean is_weblink(const char *str)
{
	return strncmp(str, "http://", 7) == 0 ||
	       strncmp(str, "https://", 8) == 0 ||
	       strncmp(str, "ftp://", 6) == 0 ||
	       strncmp(str, "mailto:", 7) == 0;
}
