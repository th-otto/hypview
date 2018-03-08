/*
 * HypView - (c)      - 2006 Philipp Donze
 *               2006 -      Philipp Donze & Odd Skancke
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
 * along with HypView; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "hv_defs.h"
#include "hypview.h"


FONTSEL_DATA *CreateFontselector(HNDL_FONT proc, short font_flag, const char *sample_text, const char *opt_button)
{
	FONTSEL_DATA *ptr = NULL;

	if (has_fonts_dialog())
	{
		short i;
		_WORD tmp_workin[16];
		_WORD tmp_workout[57];

		ptr = g_new0(FONTSEL_DATA, 1);
		if (ptr == NULL)
		{
			form_alert(1, rs_string(DI_MEMORY_ERROR));
			return NULL;
		}

		for (i = 0; i < 10; tmp_workin[i++] = 1)
			;
		tmp_workin[10] = 2;
		for (i = 11; i < (_WORD)(sizeof(tmp_workin) / sizeof(tmp_workin[0])); i++)
			tmp_workin[i] = 0;
		ptr->vdi_handle = aes_handle;
		v_opnvwk(tmp_workin, &ptr->vdi_handle, tmp_workout);
		if (!ptr->vdi_handle)
		{
			form_alert(1, rs_string(DI_VDI_WKS_ERROR));
			g_free(ptr);
			return NULL;
		}
		
		ptr->dialog = fnts_create(ptr->vdi_handle, 0, font_flag, FNTS_3D, sample_text, opt_button);
		if (ptr->dialog)
		{
			ptr->type = WIN_FONTSEL;
			ptr->proc = proc;
			ptr->font_flag = font_flag;
			ptr->opt_button = opt_button;
			ptr->last.g_x = -1;
			ptr->last.g_y = -1;
			ptr->status = 0;
			ptr->owner = gl_apid;
			
			add_item((CHAIN_DATA *)ptr);
		} else
		{
			v_clsvwk(ptr->vdi_handle);
			g_free(ptr);
			ptr = NULL;
			form_alert(1, rs_string(DI_WDIALOG_FONTSEL_ERROR));
		}
	} else
	{
		form_alert(1, rs_string(DI_WDIALOG_ERROR));
	}
	return ptr;
}


short OpenFontselector(FONTSEL_DATA *ptr, short button_flag, long id, long pt, long ratio)
{
	if (ptr == NULL || ptr->dialog == NULL)
		return 0;

	ptr->whandle = fnts_open(ptr->dialog, button_flag, ptr->last.g_x, ptr->last.g_y, id, pt, ratio);

	if (ptr->whandle > 0)
	{
		ptr->type = WIN_FONTSEL;
		ptr->id = id;
		ptr->pt = pt;
		ptr->ratio = ratio;
		ptr->status |= WIS_OPEN;
		ModalItem();
	}
	return ptr->whandle;
}


void CloseFontselector(FONTSEL_DATA *ptr)
{
	if (ptr == NULL || ptr->dialog == NULL)
		return;
	
	if (ptr->whandle > 0)
	{
		fnts_close(ptr->dialog, &ptr->last.g_x, &ptr->last.g_y);
		ptr->status &= ~WIS_OPEN;
		ptr->whandle = 0;
		if (modal_items >= 0)
			modal_items--;
	}
}

void RemoveFontselector(FONTSEL_DATA *ptr)
{
	if (ptr == NULL || ptr->dialog == NULL)
		return;

	CloseFontselector(ptr);
	fnts_delete(ptr->dialog, ptr->vdi_handle);
	remove_item((CHAIN_DATA *)ptr);
	v_clsvwk(ptr->vdi_handle);
	g_free(ptr);
}

void FontselectorEvents(FONTSEL_DATA *ptr, EVNT *event)
{
	if (!fnts_evnt(ptr->dialog, event, &ptr->button, &ptr->check_boxes, &ptr->id, &ptr->pt, &ptr->ratio))
	{
		if (!ptr->proc(ptr))
			RemoveFontselector(ptr);
	}
}
