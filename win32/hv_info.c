#include "hv_defs.h"
#include "resource.rh"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static INT_PTR CALLBACK info_dialog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (DWORD_PTR)win);
		CenterWindow(hwnd);
		{
			DOCUMENT *doc = win->data;
			HYP_HOSTNAME *h;
			char buf[30];
			
			DlgSetText(hwnd, IDC_INFO_FILE, hyp_basename(doc->path));
			if (doc->type == HYP_FT_HYP)
			{
				HYP_DOCUMENT *hyp = (HYP_DOCUMENT *)doc->data;
				
				DlgSetText(hwnd, IDC_INFO_DATABASE, fixnull(hyp->database));
				DlgSetText(hwnd, IDC_INFO_AUTHOR, fixnull(hyp->author));
				DlgSetText(hwnd, IDC_INFO_VERSION, fixnull(hyp->version));
				DlgSetText(hwnd, IDC_INFO_SUBJECT, fixnull(hyp->subject));
				sprintf(buf, "%7u", hyp->num_index);
				DlgSetText(hwnd, IDC_INFO_NODES, buf);
				sprintf(buf, "%7ld", hyp->itable_size);
				DlgSetText(hwnd, IDC_INFO_INDEXSIZE, buf);
				sprintf(buf, "%3u", hyp->comp_vers);
				DlgSetText(hwnd, IDC_INFO_HCPVERSION, buf);
				DlgSetText(hwnd, IDC_INFO_OS, hyp_osname(hyp->comp_os));
				DlgSetText(hwnd, IDC_INFO_CHARSET, hyp_charset_name(hyp->comp_charset));
				DlgSetText(hwnd, IDC_INFO_DEFAULT, fixnull(hyp->default_name));
				DlgSetText(hwnd, IDC_INFO_HELP, fixnull(hyp->help_name));
				DlgSetText(hwnd, IDC_INFO_OPTIONS, fixnull(hyp->hcp_options));
				for (h = hyp->hostname; h != NULL; h = h->next)
				{
				}
			}
		}
		return TRUE;
	case WM_CLOSE:
		EndDialog(hwnd, IDCANCEL);
		DestroyWindow(hwnd);
		return TRUE;
	case WM_COMMAND:
		notifyCode = HIWORD(wParam);
		win = (WINDOW_DATA *)(DWORD_PTR)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			DestroyWindow(hwnd);
			return TRUE;
		case IDOK:
			EndDialog(hwnd, IDOK);
			DestroyWindow(hwnd);
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

/*** ---------------------------------------------------------------------- ***/

void DocumentInfos(WINDOW_DATA *win)
{
	DialogBoxExW(NULL, MAKEINTRESOURCEW(IDD_FILEINFO), win->hwnd, info_dialog, (LPARAM)win);
}
