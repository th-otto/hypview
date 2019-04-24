#include "hv_defs.h"
#include "hypdebug.h"
#include "w_draw.h"

#define DEFAULT_FONT "Courier New 12"
#define FALLBACK_FONT "Courier 12"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void ApplyFont(gboolean clearcache)
{
	WINDOW_DATA *win;
	DOCUMENT *doc;
	GSList *l;
	
	/* adjust all open documents and windows */
	for (l = all_list; l; l = l->next)
	{
		win = (WINDOW_DATA *)l->data;
		hv_set_font(win);
		/* if (win->type == WIN_WINDOW) */
		{
			gboolean ret;
			long topline;
			hyp_nodenr node_num;
			
			doc = win->data;
			/* reload page or file */

			topline = hv_win_topline(win);
			node_num = doc->getNodeProc(win);
			if (clearcache && doc->type == HYP_FT_HYP)
			{
				HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
				RemovePictures(hyp, TRUE);
			}
			ret = doc->gotoNodeProc(win, NULL, node_num);
			
			if (ret)
			{
				doc->start_line = topline;

				ReInitWindow(win, TRUE);
			}
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

void SwitchFont(WINDOW_DATA *win, gboolean clearcache)
{
	UNUSED(win);
	gl_profile.viewer.use_xfont = gl_profile.viewer.use_xfont && gl_profile.viewer.xfont_name != NULL;
	ApplyFont(clearcache);
	HypProfile_SetChanged();
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

char *W_Fontdesc(const FONT_ATTR *attr)
{
	char namebuf[FONT_NAME_LEN + 80];
	char sizebuf[30];
	char attrbuf[FONT_NAME_LEN];
	
	strcpy(namebuf, attr->name);
	strcat(namebuf, ",");
	*attrbuf = '\0';
	if (attr->textstyle & HYP_TXT_BOLD)
		strcat(attrbuf, " Bold");
	if (attr->textstyle & HYP_TXT_ITALIC)
		strcat(attrbuf, " Italic");
	if (attr->textstyle & HYP_TXT_UNDERLINED)
		strcat(attrbuf, " Underline");
	if (attrbuf[0] != '\0')
		strcat(namebuf, attrbuf + 1);
	sprintf(sizebuf, " , %d", attr->size);
	strcat(namebuf, sizebuf);
	return g_strdup(namebuf);
}

/*** ---------------------------------------------------------------------- ***/

gboolean W_Fontname(const char *name, FONT_ATTR *attr)
{
	char namebuf[FONT_NAME_LEN];
	char *p;
	char *stylename;
	
#define font_attr(name, len, mask) \
	while (*stylename == ' ') \
		stylename++; \
	if (strncasecmp(stylename, name, len) == 0) \
	{ \
		attr->textstyle |= mask; \
		stylename += len; \
		while (*stylename == ' ') \
			stylename++; \
	}

	attr->size = 0;
	attr->textstyle = HYP_TXT_NORMAL;
	attr->name[0] = '\0';
	if (name == NULL)
		return FALSE;
	strncpy(namebuf, name, sizeof(namebuf));
	p = strchr(namebuf, ',');
	if (p != NULL)
	{
		*p++ = '\0';
		stylename = p;
		p = strchr(p, ',');
		if (p != NULL)
		{
			*p++ = '\0';
			attr->size = (int)strtol(p, NULL, 10);
			p = strchr(p, ',');
			if (p != NULL)
			{
				*p++ = '\0';
			}
		}
		font_attr("Bold", 4, HYP_TXT_BOLD);
		font_attr("Italic", 6, HYP_TXT_ITALIC);
		font_attr("Underline", 9, HYP_TXT_UNDERLINED);
	}
	strcpy(attr->name, namebuf);
	if (*attr->name == '\0')
		return FALSE;
	return TRUE;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void SelectFont(WINDOW_DATA *win)
{
	/* TODO */
}
