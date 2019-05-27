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

#include "hv_defs.h"
#include "hv_vers.h"
#include "resource.rh"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static INT_PTR CALLBACK about_dialog(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HBITMAP hBitmap;
	HICON hIcon;
	WORD notifyCode;
	
	UNUSED(lParam);
	switch (message)
	{
	case WM_CREATE:
		break;
	case WM_INITDIALOG:
		{
			char *hyp_version = hyp_lib_version();
			char *compile_date = g_strdup_printf(_("(Compiled %s)"), gl_compile_date);
			char *compiler_version = hyp_compiler_version();
			
			CenterWindow(hwnd);
			hBitmap = LoadBitmap(GetInstance(), MAKEINTRESOURCE(IDB_WEBLINK));
			SendDlgItemMessage(hwnd, IDC_WEBLINK, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
			hBitmap = LoadBitmap(GetInstance(), MAKEINTRESOURCE(IDB_EMAIL));
			SendDlgItemMessage(hwnd, IDC_EMAILLINK, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
			hIcon = LoadIcon(GetInstance(), MAKEINTRESOURCE(IDI_MAINFRAME));
			SendDlgItemMessage(hwnd, IDC_HV_ICON, STM_SETICON, (WPARAM)hIcon, 0);
			DlgSetText(hwnd, IDC_HCP_VERSION, hyp_version);
			DlgSetText(hwnd, IDC_COMPILE_DATE, compile_date);
			DlgSetText(hwnd, IDC_COMPILER_VERSION, compiler_version);
			g_free(compiler_version);
			g_free(compile_date);
			g_free(hyp_version);
		}
		return TRUE;
	case WM_CLOSE:
		EndDialog(hwnd, IDCANCEL);
		DestroyWindow(hwnd);
		return TRUE;
	case WM_COMMAND:
		notifyCode = HIWORD(wParam);
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
		case IDC_EMAILLINK:
		case IDC_EMAIL:
			if (notifyCode == BN_CLICKED)
			{
				char *buf;
				
				buf = g_strdup_printf("mailto:%s?subject=HypView%%20for%%20Windows", HYP_URL);
				ShellExecuteA(NULL, "open", buf, NULL, NULL, SW_SHOWNORMAL);
				g_free(buf);
			}
			break;
		case IDC_WEBLINK:
		case IDC_URL:
			if (notifyCode == BN_CLICKED)
			{
				wchar_t *url = DlgGetTextW(hwnd, IDC_URL);
				ShellExecuteW(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL);
				g_free(url);
			}
			break;
		}
		break;
	case WM_DESTROY:
		hBitmap = (HBITMAP)(DWORD_PTR)SendDlgItemMessage(hwnd, IDC_WEBLINK, BM_GETIMAGE, IMAGE_BITMAP, 0);
		DeleteObject(hBitmap);
		hBitmap = (HBITMAP)(DWORD_PTR)SendDlgItemMessage(hwnd, IDC_EMAILLINK, STM_GETIMAGE, IMAGE_BITMAP, 0);
		DeleteObject(hBitmap);
		hIcon = (HICON)(DWORD_PTR)SendDlgItemMessage(hwnd, IDC_HV_ICON, STM_GETIMAGE, IMAGE_ICON, 0);
		DeleteObject(hIcon);
		break;
	}
	return FALSE;
}

/*** ---------------------------------------------------------------------- ***/

void About(HWND parent)
{
	DialogBoxExW(NULL, MAKEINTRESOURCEW(IDD_ABOUT), parent, about_dialog, 0);
}
