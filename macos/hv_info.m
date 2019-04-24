#include "hv_defs.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

#if 0
static INT_PTR CALLBACK info_dialog(WINDOW_DATA *win, UINT message, WPARAM wParam, LPARAM lParam)
{
	WORD notifyCode;
	WINDOW_DATA *win;
	
	UNUSED(lParam);
	switch (message)
	{
	case WM_CREATE:
		break;
	case WM_INITDIALOG:
		win = (WINDOW_DATA *)lParam;
		SetWindowLongPtr(win, GWLP_USERDATA, (DWORD_PTR)win);
		CenterWindow(win);
		{
			DOCUMENT *doc = win->data;
			HYP_HOSTNAME *h;
			char buf[30];
			
			DlgSetText(win, IDC_INFO_FILE, hyp_basename(doc->path));
			if (doc->type == HYP_FT_HYP)
			{
				HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
				
				DlgSetText(win, IDC_INFO_DATABASE, fixnull(hyp->database));
				DlgSetText(win, IDC_INFO_LANGUAGE, fixnull(hyp->language));
				DlgSetText(win, IDC_INFO_LANGUAGE_GUESSED, hyp->language_guessed ? _(" (guessed)") : "");
				DlgSetText(win, IDC_INFO_AUTHOR, fixnull(hyp->author));
				DlgSetText(win, IDC_INFO_VERSION, fixnull(hyp->version));
				DlgSetText(win, IDC_INFO_SUBJECT, fixnull(hyp->subject));
				sprintf(buf, "%7u", hyp->num_index);
				DlgSetText(win, IDC_INFO_NODES, buf);
				sprintf(buf, "%7ld", hyp->itable_size);
				DlgSetText(win, IDC_INFO_INDEXSIZE, buf);
				sprintf(buf, "%3u", hyp->comp_vers);
				DlgSetText(win, IDC_INFO_HCPVERSION, buf);
				DlgSetText(win, IDC_INFO_OS, hyp_osname(hyp->comp_os));
				DlgSetText(win, IDC_INFO_CHARSET, hyp_charset_name(hyp->comp_charset));
				DlgSetText(win, IDC_INFO_DEFAULT, fixnull(hyp->default_name));
				DlgSetText(win, IDC_INFO_HELP, fixnull(hyp->help_name));
				DlgSetText(win, IDC_INFO_OPTIONS, fixnull(hyp->hcp_options));
				for (h = hyp->hostname; h != NULL; h = h->next)
				{
				}
			}
		}
		return TRUE;
	case WM_CLOSE:
		EndDialog(win, IDCANCEL);
		DestroyWindow(win);
		return TRUE;
	case WM_COMMAND:
		notifyCode = HIWORD(wParam);
		win = (WINDOW_DATA *)(DWORD_PTR)GetWindowLongPtr(win, GWLP_USERDATA);
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(win, IDCANCEL);
			DestroyWindow(win);
			return TRUE;
		case IDOK:
			EndDialog(win, IDOK);
			DestroyWindow(win);
			break;
		case IDHELP:
			if (notifyCode == BN_CLICKED)
			{
				Help_Contents(win);
			}
			break;
		}
		break;
	case WM_DESTROY:
		break;
	}
	return FALSE;
}
#endif

/*** ---------------------------------------------------------------------- ***/

void DocumentInfos(WINDOW_DATA *win)
{
}
