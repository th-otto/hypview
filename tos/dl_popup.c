#include "hv_defs.h"
#include "hypdebug.h"


typedef struct {
	_WORD x, y;
	_WORD width;
	_WORD height;
	_WORD nplanes;
	_UBYTE data[1];
} SCRBLOCK;


SCRBLOCK *screen_get(GRECT *gr);
void screen_put(SCRBLOCK *block, _WORD mode);

typedef struct _fd_draw FD_DRAW;
struct _fd_draw {
	OBJECT *tree;
	_WORD start;
	GRECT clip;
	SCRBLOCK *buf;
};

typedef struct {
	OBJECT *obj;
	_WORD old_h;
	_WORD index, new_index;
	GRECT clip;
	FD_DRAW *fdDr;
	_WORD pos_x, pos_y;
} POPUP;

void ClipToDesktop(_WORD *x, _WORD *y, _WORD w, _WORD h);
void ClipObjToDesktop(OBJECT *tree, _WORD start);
void form_calc(OBJECT *obj, GRECT *gr);
void form_size(OBJECT *tree, _WORD idx, GRECT *gr);
FD_DRAW *draw_popup(OBJECT *tree, _WORD start);
void undraw_popup(FD_DRAW *dial);


/*****************************************************************************
 *	handle popup dialogs
 *****************************************************************************/


/*
 * save part of screen into block
 */
SCRBLOCK *screen_get(GRECT *gr)
{
	_WORD pxyarray[8];
	SCRBLOCK *buf;
	size_t bufsize;
	MFDB screen_fdb;
	MFDB buf_fdb;
	_WORD clip[4];
	
	/*
	 * init memory form definition block for screen
	 */
	GetScreenSize(&screen_fdb.fd_w, &screen_fdb.fd_h);
	screen_fdb.fd_wdwidth = screen_fdb.fd_w >> 4;
	screen_fdb.fd_stand = 0;
	screen_fdb.fd_nplanes = GetNumPlanes();
	screen_fdb.fd_addr = NULL;			/* source is screen */
	screen_fdb.fd_r1 = 0;
	screen_fdb.fd_r2 = 0;
	screen_fdb.fd_r3 = 0;
	/*
	 * check bounds
	 */
	if (gr->g_x < 0)
	{
		gr->g_w += gr->g_x;
		gr->g_x = 0;
	}
	if (gr->g_y < 0)
	{
		gr->g_h += gr->g_y;
		gr->g_y = 0;
	}
	if ((gr->g_x + gr->g_w) > screen_fdb.fd_w)
	{
		gr->g_w = screen_fdb.fd_w - gr->g_x;
	}
	if ((gr->g_y + gr->g_h) > screen_fdb.fd_h)
	{
		gr->g_h = screen_fdb.fd_h - gr->g_y;
	}
	/*
	 * init memory form definition block for buffer
	 */
	buf_fdb.fd_w = gr->g_w;
	buf_fdb.fd_h = gr->g_h;
	buf_fdb.fd_wdwidth = (gr->g_w + 15) >> 4;
	buf_fdb.fd_stand = 0;
	buf_fdb.fd_nplanes = GetNumPlanes();
	buf_fdb.fd_r1 = 0;
	buf_fdb.fd_r2 = 0;
	buf_fdb.fd_r3 = 0;
	/*
	 * allocate data buffer
	 */
	bufsize = (size_t)buf_fdb.fd_wdwidth * 2 * gr->g_h * buf_fdb.fd_nplanes + sizeof(SCRBLOCK);
	if ((buf = (SCRBLOCK *)g_malloc(bufsize)) == NULL)
		return NULL;
	buf_fdb.fd_addr = buf->data;		/* destination is buffer */
	/*
	 * save width & height
	 */
	buf->x = gr->g_x;
	buf->y = gr->g_y;
	buf->width = gr->g_w;
	buf->height = gr->g_h;
	buf->nplanes = buf_fdb.fd_nplanes;
	/*
	 * set coordinates
	 */
	pxyarray[0] = gr->g_x; pxyarray[1] = gr->g_y;
	pxyarray[2] = gr->g_x + gr->g_w - 1; pxyarray[3] = gr->g_y + gr->g_h - 1;
	pxyarray[4] = 0; pxyarray[5] = 0;
	pxyarray[6] = gr->g_w - 1; pxyarray[7] = gr->g_h - 1;
	/*
	 * copy rectangle
	 */
	v_hide_c(vdi_handle);
	clip[0] = 0;
	clip[1] = 0;
	clip[2] = screen_fdb.fd_w - 1;
	clip[3] = screen_fdb.fd_h - 1;
	vs_clip(vdi_handle, TRUE, clip);
	vro_cpyfm(vdi_handle, S_ONLY, pxyarray, &screen_fdb, &buf_fdb);
	vs_clip(vdi_handle, FALSE, clip);
	v_show_c(vdi_handle, FALSE);
	return buf;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * restore screen block
 */
void screen_put(SCRBLOCK *buf, _WORD mode)
{
	_WORD pxy[8];
	MFDB screen_fdb;
	MFDB buf_fdb;
	_WORD clip[4];
	
	/*
	 * set coordinates using width & height in buffer
	 */
	pxy[0] = 0;
	pxy[1] = 0;
	pxy[2] = buf->width - 1;
	pxy[3] = buf->height - 1;
	pxy[4] = buf->x;
	pxy[5] = buf->y;
	pxy[6] = buf->x + buf->width - 1;
	pxy[7] = buf->y + buf->height - 1;
	/*
	 * init memory form definition block for screen
	 */
	GetScreenSize(&screen_fdb.fd_w, &screen_fdb.fd_h);
	screen_fdb.fd_wdwidth = screen_fdb.fd_w >> 4;
	screen_fdb.fd_stand = 0;
	screen_fdb.fd_nplanes = GetNumPlanes();
	screen_fdb.fd_addr = NULL;		/* destination is screen */
	screen_fdb.fd_r1 = 0;
	screen_fdb.fd_r2 = 0;
	screen_fdb.fd_r3 = 0;
	/*
	 * init memory form definition block of buffer
	 */
	buf_fdb.fd_w = buf->width;
	buf_fdb.fd_h = buf->height;
	buf_fdb.fd_wdwidth = (buf->width + 15 ) >> 4;
	buf_fdb.fd_stand = 0;
	buf_fdb.fd_nplanes = buf->nplanes;
	buf_fdb.fd_addr = buf->data;	/* source is buffer */
	buf_fdb.fd_r1 = 0;
	buf_fdb.fd_r2 = 0;
	buf_fdb.fd_r3 = 0;
	/*
	 * copy rectangle
	 */
	v_hide_c(vdi_handle);
	clip[0] = 0;
	clip[1] = 0;
	clip[2] = screen_fdb.fd_w - 1;
	clip[3] = screen_fdb.fd_h - 1;
	vs_clip(vdi_handle, TRUE, clip);
	vro_cpyfm(vdi_handle, mode, pxy, &buf_fdb, &screen_fdb);
	vs_clip(vdi_handle, FALSE, clip);
	v_show_c(vdi_handle, FALSE);
	g_free(buf);
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void ClipToDesktop(_WORD *x, _WORD *y, _WORD w, _WORD h)
{
	GRECT desk;

	wind_get_grect(DESK, WF_WORKXYWH, &desk);
	if (*x < desk.g_x)
		*x = desk.g_x;
	if (*y < desk.g_y)
		*y = desk.g_y;
	if (*x + w > desk.g_x + desk.g_w)
		*x = desk.g_x + desk.g_w - w;
	if (*y + h > desk.g_y + desk.g_h)
		*y = desk.g_y + desk.g_h - h;
	if (*x < desk.g_x)
		*x = desk.g_x;
	if (*y < desk.g_y)
		*y = desk.g_y;
}

/*** ---------------------------------------------------------------------- ***/

void ClipObjToDesktop(OBJECT *tree, _WORD start)
{
	_WORD ox, oy;
	GRECT gr;

	form_size(tree, start, &gr);
	ox = tree[start].ob_x - gr.g_x;
	oy = tree[start].ob_y - gr.g_y;
	ClipToDesktop(&gr.g_x, &gr.g_y, gr.g_w, gr.g_h);
	tree[start].ob_x = gr.g_x + ox;
	tree[start].ob_y = gr.g_y + oy;
}

/*** ---------------------------------------------------------------------- ***/

/* Return the object's GRECT including attributes */

void form_calc(OBJECT *obj, GRECT *gr)
{
	_WORD x, y, w, h;
	_WORD frame, frame2;
	_BOOL outline;
	OBSPEC obspec;
	
	x = gr->g_x;
	y = gr->g_y;
	w = gr->g_w;
	h = gr->g_h;
	obspec = obj->ob_spec;
	if (obj->ob_flags & OF_INDIRECT)
		obspec = *(obspec.indirect);
	frame = 0;
	outline = TRUE;
	switch (obj->ob_type & OBTYPEMASK)
	{
	case G_BOX:
	case G_IBOX:
	case G_EXTBOX:
		frame = OBSPEC_GET_FRAMESIZE(obspec);
		break;
	case G_BOXCHAR:
		frame = OBSPEC_GET_FRAMESIZE(obspec);
		if (obj->ob_flags & OF_FL3DMASK)
		{
			x -= 3;
			y -= 3;
			w += 6;
			h += 6;
		}
		break;
	case G_BOXTEXT:
	case G_FBOXTEXT:
		frame = obspec.tedinfo->te_thickness;
		break;
	case G_BUTTON:
		frame = -1;
		if (obj->ob_flags & OF_EXIT)
			frame--;
		if (obj->ob_flags & OF_DEFAULT)
			frame--;
		if (obj->ob_flags & OF_FL3DMASK)
		{
			x -= 3;
			y -= 3;
			w += 6;
			h += 6;
		}
		break;
	case G_USERDEF:
		frame = -1;
		break;
	default:
		outline = FALSE;
		break;
	}
	frame2 = frame;
	if (outline && (obj->ob_state & OS_OUTLINED) && frame > -3)
		frame = -3;
	if (frame < 0)
	{
		frame = -frame;
		x -= frame;
		y -= frame;
		w += frame;
		w += frame;
		h += frame;
		h += frame;
	}
	if (frame2 != 0 && (obj->ob_state & OS_SHADOWED))
	{
		if (frame2 < 0)
			frame2 = -frame2;
		w += frame2;
		w += frame2;
		h += frame2;
		h += frame2;
	}
	if ((obj->ob_type & OBTYPEMASK) == G_ICON)
	{
		ICONBLK *icon;

		icon = obspec.iconblk;
		if (gr->g_x + icon->ib_xicon < x)
		{
			w += x - gr->g_x - icon->ib_xicon;
			x = gr->g_x + icon->ib_xicon;
		}
		if (gr->g_y + icon->ib_yicon < y)
		{
			h += y - gr->g_y - icon->ib_yicon;
			y = gr->g_y + icon->ib_yicon;
		}
		if (gr->g_x + icon->ib_xtext < x)
		{
			w += x - gr->g_x - icon->ib_xtext;
			x = gr->g_x + icon->ib_xtext;
		}
		if (gr->g_y + icon->ib_ytext < y)
		{
			h += y - gr->g_y - icon->ib_ytext;
			y = gr->g_y + icon->ib_ytext;
		}
		if (x + w < gr->g_x + icon->ib_xicon + icon->ib_wicon)
			w = gr->g_x + icon->ib_xicon + icon->ib_wicon - x;
		if (y + h < gr->g_y + icon->ib_yicon + icon->ib_hicon)
			h = gr->g_y + icon->ib_yicon + icon->ib_hicon - y;
		if (x + w < gr->g_x + icon->ib_xtext + icon->ib_wtext)
			w = gr->g_x + icon->ib_xtext + icon->ib_wtext - x;
		if (y + h < gr->g_y + icon->ib_ytext + icon->ib_htext)
			h = gr->g_y + icon->ib_ytext + icon->ib_htext - y;
	}
	gr->g_x = x;
	gr->g_y = y;
	gr->g_w = w;
	gr->g_h = h;
}

/*** ---------------------------------------------------------------------- ***/

void form_size(OBJECT *tree, _WORD idx, GRECT *gr)
{
	objc_offset(tree, idx, &gr->g_x, &gr->g_y);
	tree += idx;
	gr->g_w = tree->ob_width;
	gr->g_h = tree->ob_height;
	form_calc(tree, gr);
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static FD_DRAW dial_buf;

FD_DRAW *draw_popup(OBJECT *tree, _WORD start)
{
	FD_DRAW *dial;
	
	dial = g_new(FD_DRAW, 1);
	if (dial == NULL)
		dial = &dial_buf;
	ClipObjToDesktop(tree, start);

	dial->tree = tree;
	dial->start = start;
	form_size(tree, start, &dial->clip);
	wind_update(BEG_UPDATE);
	if ((dial->buf = screen_get(&dial->clip)) == NULL)
    	form_dial_grect(FMD_START, &dial->clip, &dial->clip);
	objc_draw_grect(tree, start, MAX_DEPTH, &dial->clip);
    return dial;
}

/*** ---------------------------------------------------------------------- ***/

void undraw_popup(FD_DRAW *dial)
{
	if (dial == NULL)
		return;
	if (dial->buf != NULL)
		screen_put(dial->buf, S_ONLY);
	else
		form_dial_grect(FMD_FINISH, &dial->clip, &dial->clip);
	if (dial != &dial_buf)
		g_free(dial);
	wind_update(END_UPDATE);
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL popup_setup(POPUP *popup, OBJECT *obj, _WORD x, _WORD y)
{
	_WORD i;
	OBJECT *p;
	_WORD sy;
	_WORD h;
	_BOOL found;

	if (obj == NULL || obj[ROOT].ob_head < 0)
		return -1;
	p = &obj[ROOT];
	popup->obj = obj;
	popup->index = -1;
	popup->old_h = p->ob_height;
	h = 0;
	sy = 0;
	for (i = obj[ROOT].ob_head; i != ROOT; i = obj[i].ob_next)
	{
		p = &obj[i];
		p->ob_state &= ~OS_SELECTED;
		if (!(p->ob_flags & OF_HIDETREE))
		{
			p->ob_y = sy;
			sy += p->ob_height;
			h = max(h, sy);
		}
	}
	obj[ROOT].ob_height = h;
	if (x != 0)
		obj[ROOT].ob_x = x - obj[ROOT].ob_width / 2;
	if (y != 0)
		obj[ROOT].ob_y = y - obj[ROOT].ob_height / 2;
	ClipObjToDesktop(obj, ROOT);
	popup->pos_x = obj[ROOT].ob_x;
	popup->pos_y = obj[ROOT].ob_y;
	i = obj[ROOT].ob_head;
	found = FALSE;
	do
	{
		if (!(obj[i].ob_state & OS_DISABLED) &&
			!(obj[i].ob_flags & OF_HIDETREE))
		{
			found = TRUE;
			break;
		}
		i = obj[i].ob_next;
	} while (i != ROOT);
	if (!found)
	{
		/* no entry that could be selected */
		i = obj[ROOT].ob_head;
	}
	popup->new_index = i;
	return found;
}

/*** ---------------------------------------------------------------------- ***/

static void popup_drawobj(POPUP *popup)
{
	if (popup->index != popup->new_index && popup->new_index != -1)
	{
		if (popup->index > 0)
			objc_change_grect(popup->obj, popup->index, MAX_DEPTH, &popup->clip, OS_NORMAL, TRUE);
		if (popup->new_index >= 0 && popup->obj[popup->new_index].ob_state & OS_DISABLED)
			popup->new_index = -1;
		if (popup->new_index > 0)
			objc_change_grect(popup->obj, popup->new_index, MAX_DEPTH, &popup->clip, OS_SELECTED, TRUE);
		else
			popup->new_index = -1;
		popup->index = popup->new_index;
	}
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL popup_mouse(EVNT *event, POPUP *popup)
{
	popup->new_index = objc_find(popup->obj, ROOT, 2, event->mx, event->my);
	if (popup->new_index == -1)
		popup->new_index = -2;
	popup_drawobj(popup);
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL popup_key(EVNT *event, POPUP *popup)
{
	_WORD start, i;
	_WORD ascii = event->key, scan;
	_WORD kstate = event->kstate;
	OBJECT *obj = popup->obj;
	
	ConvertKeypress(&ascii, &kstate);
	scan = (ascii >> 8) & 0xff;

	popup->new_index = popup->index;
	if (kstate == 0)
	{
		switch (scan)
		{
		case KbUP:
			start = popup->new_index;
			i = obj[ROOT].ob_head;
			do
			{
				if (!(obj[i].ob_state & OS_DISABLED) &&
					!(obj[i].ob_flags & OF_HIDETREE))
				{
					popup->new_index = i;
				}
				i = obj[i].ob_next;
			} while (i != ROOT && i != start);
			break;
		case KbDOWN: /* Cursor down */
			i = popup->new_index;
			if (i < 0)
				i = obj[ROOT].ob_tail;
			start = i;
			do
			{
				i = obj[i].ob_next;
				if (i == ROOT)
					i = obj[ROOT].ob_head;
				if (!(obj[i].ob_state & OS_DISABLED) &&
					!(obj[i].ob_flags & OF_HIDETREE))
				{
					popup->new_index = i;
					break;
				}
			} while (i != start);
			break;
		case KbESC: /* Esc */
		case KbUNDO: /* Undo */
			popup->index = -2;
			return TRUE;
		case KbRETURN: /* Return */
		case KbENTER: /* Enter */
			return TRUE;
		}
	}
	popup_drawobj(popup);
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL popup_button(EVNT *event, POPUP *popup)
{
	popup->new_index = objc_find(popup->obj, ROOT, 2, event->mx, event->my);
	if (popup->new_index == -1)
		popup->new_index = -2;
	if (popup->new_index == ROOT)
		popup->new_index = -1;
	popup_drawobj(popup);
	if (popup->index > 0)
	{
		if (event->mclicks == 2)
			popup->index |= 0x8000;
		if (event->mbutton & MOB_RIGHT)
			popup->index |= 0x4000;
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * popup_select(obj, x, y): select item from popup-menu
 *
 * Parameters:
 *		obj: (OBJECT *) of popup-menu
 *		x:	 x-koordinate on desktop
 *		y:	 y-koordinate on desktop
 *			 if both = 0, old position is used
 *
 * returns:
 *		0 if click on desktop
 *		index (1-n) of item otherwise
 *		bit 15 set if double-click
 *		bit 14 set if right-button pressed
 */

_WORD popup_select(OBJECT *tree, _WORD mx, _WORD my)
{
	POPUP my_popup;
	POPUP *popup = &my_popup;
	_BOOL done;
	_WORD mask = 256 + 2;
	_WORD bmask = 3;
	EVNT event;
	_BOOL in_rect = FALSE;
	
#if 0
	if (has_wlffp != 0 || has_form_popup)
		return form_popup(tree, mx, my);
#endif
	
	if (!popup_setup(popup, tree, mx, my))
		return -1;
	popup->obj[ROOT].ob_x = popup->pos_x;
	popup->obj[ROOT].ob_y = popup->pos_y;
	popup->fdDr = draw_popup(popup->obj, ROOT);
	if (popup->fdDr == NULL)
		return -1;
	form_size(popup->obj, ROOT, &popup->clip);
	popup_drawobj(popup);
	
	wind_update(BEG_MCTRL);
	memset(&event, 0, sizeof(event));
	graf_mkstate(&event.mx, &event.my, &event.mbutton, &event.kstate);
	if (event.mbutton & (MOB_LEFT|MOB_RIGHT))
	{
		mask = 1;
		bmask = event.mbutton;
	}
	do {
		event.mwhich = evnt_multi_gemlib(MU_KEYBD|MU_BUTTON|MU_M1, mask, bmask, 0,
			in_rect, event.mx, event.my, 1, 1,
			0, 0, 0, 0, 0,
			event.msg, 0,
			&event.mx, &event.my, &event.mbutton, &event.kstate, &event.key, &event.mclicks);
		done = FALSE;
		if (event.mwhich & MU_BUTTON)
			done |= popup_button(&event, popup);
		if (event.mwhich & MU_M1)
		{
			in_rect = TRUE;
			done |= popup_mouse(&event, popup);
		}
		if (event.mwhich & MU_KEYBD)
			done |= popup_key(&event, popup);
	} while (!done);
	wind_update(END_MCTRL);
	
	undraw_popup(popup->fdDr);
	popup->obj[ROOT].ob_height = popup->old_h;
	return popup->index;
}
