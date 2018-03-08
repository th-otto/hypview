#include "hv_defs.h"


void HypGetCursorPosition(DOCUMENT *doc, short x, short y, TEXT_POS *pos)
{
	WINDOW_DATA *win = doc->window;
	HYP_NODE *node;
	LINEPTR *line_ptr;
	short line = 0;
	long i = 0;
	long sy;
	short curr_txt_effect = 0;
	short x_pos = win->x_raster;
	short width = 0;
	_WORD ext[8];
	const unsigned char *src;
	unsigned char *dst;
	unsigned char temp[LINE_BUF];
	HYP_DOCUMENT *hyp;

	if (doc->type != HYP_FT_HYP)
	{
		return;
	}

	hyp = doc->data;
	node = doc->displayed_node;

	sy = -(win->docsize.y * win->y_raster);
	line_ptr = node->line_ptr;

	while (line < node->lines)
	{
		long y2;

		y2 = sy + line_ptr->y + line_ptr->h;
		if ((sy + line_ptr->y) >= y || (y2 > y))
		{
			sy += line_ptr->y;
			break;
		}
		sy = y2;
		line_ptr++;
		line++;
	}

	if (line >= node->lines)
		return;

	src = line_ptr->txt;

	if (x < win->x_raster || !src)
	{
		pos->line = line;
		pos->y = sy + (win->docsize.y * win->y_raster);
		pos->offset = 0;
		pos->x = 0;
		return;
	}

	/*  Standard Text-Effekt    */
	vst_effects(vdi_handle, 0);
	dst = temp;

	while (*src)
	{
		if (*src == HYP_ESC)					/*  ESC-Sequenz ??  */
		{
			*dst = 0;
			/*  ungeschriebene Daten?   */
			if (*temp)
			{
				/*  noch fehlende Daten ausgeben    */
				vqt_extent(vdi_handle, (const char *)temp, ext);
				width = (ext[2] + ext[4]) >> 1;
				if (x_pos + width >= x)
					goto go_back;

				x_pos += width;
			}
			dst = temp;
			src++;

			switch (*src)
			{
			case HYP_ESC_ESC:					/*  ESC */
				*dst++ = *src++;
				i++;
				break;
			case HYP_ESC_LINK:
			case HYP_ESC_LINK_LINE:
			case HYP_ESC_ALINK:
			case HYP_ESC_ALINK_LINE:
				{
					hyp_nodenr idx;	/*  Index auf die Zielseite */

					if (*src == HYP_ESC_LINK_LINE || *src == HYP_ESC_ALINK_LINE)		/*  Zeilennummer ueberspringen  */
						src += 2;

					idx = DEC_255(&src[1]);
					src += 3;

					/*  Verknuepfungstext mit entsprechendem Texteffekt ausgeben    */
					vst_color(vdi_handle, gl_profile.viewer.link_color);
					vst_effects(vdi_handle, gl_profile.viewer.link_effect | curr_txt_effect);

					/*  Verknuepfungstext ermitteln und ausgeben    */
					if (*src <= HYP_STRLEN_OFFSET)		/*  Kein Text angegeben */
					{
						strcpy((char *)temp, (const char *)hyp->indextable[idx]->name);
						src++;
					} else
					{
						_UWORD num = *src - HYP_STRLEN_OFFSET;

						memcpy(temp, src + 1, num);
						temp[num] = 0;
						src += num + 1;
					}

					i += ustrlen(temp);
					vqt_extent(vdi_handle, (const char *)temp, ext);
					width = (ext[2] + ext[4]) >> 1;
					if (x_pos + width >= x)
						goto go_back;

					x_pos += width;

					vst_color(vdi_handle, gl_profile.viewer.text_color);
					vst_effects(vdi_handle, curr_txt_effect);
				}
				break;
			
			case HYP_ESC_CASE_TEXTATTR:
				curr_txt_effect = *src - HYP_ESC_TEXTATTR_FIRST;
				vst_effects(vdi_handle, curr_txt_effect);
				src++;
				break;
			
			default:
				src = hyp_skip_esc(src - 1);
				break;
			}
		} else
		{
			*dst++ = *src++;
			i++;
		}
	}
	*dst = 0;

	if (*temp)							/*  Noch ungeschriebene Daten?  */
	{
		vqt_extent(vdi_handle, (const char *)temp, ext);
		width = (ext[2] + ext[4]) >> 1;
		if (x_pos + width >= x)
			goto go_back;
		x_pos += width;
		*temp = 0;
	}
	width = 0;

  go_back:
	if (*temp && x_pos + width > x)
	{
		dst = temp;
		while (*dst)
			dst++;
		dst--;

		while (dst >= temp)
		{
			i--;
			*dst = 0;
			vqt_extent(vdi_handle, (const char *)temp, ext);
			if (x_pos + ext[2] < x)
			{
				if (x - (x_pos + ext[2]) > (x_pos + width) - x)
				{
					i++;
					break;
				}
				width = (ext[2] + ext[4]) >> 1;
				break;
			}
			width = (ext[2] + ext[4]) >> 1;
			dst--;
		}

		if (dst >= temp)
			pos->x = x_pos + width;
		else
			pos->x = x_pos;
	} else
		pos->x = x_pos + width;

	if (i == 0)
		pos->x = 0;
	pos->offset = i;
	pos->line = line;
	pos->y = sy + (win->docsize.y * win->y_raster);
}
