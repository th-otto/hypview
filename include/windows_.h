#ifndef __WINDOWS__H__
#define __WINDOWS__H__ 1

/*
 * define __WIN32__ if using any compiler for Windoze
 */
#if defined(_WIN32) && !defined(__WIN32__)
#define __WIN32__ 1
#endif
#if defined(_WIN64) && !defined(__WIN64__)
# define __WIN64__ 1
#endif

#ifdef __WIN32__

#define WIN32_LEAN_AND_MEAN
#define STRICT
#define WINVER 0x0500
#define _WIN32_IE 0x0600
#define __SIMPLE_LOCALES__
#include <windows.h>
#include <shellapi.h>
#include <commdlg.h>
#include <cderr.h>


#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif
#ifndef LOCAL
#define LOCAL static
#endif
#ifndef GLOBAL
#define GLOBAL
#endif
#define GetInstance()  ((HINSTANCE)GetModuleHandleW(NULL))

#undef MAKEWPARAM
#define MAKEWPARAM(l, h) ((WPARAM)(((WORD)((DWORD_PTR)(l) & 0xffff)) | (((DWORD)((WORD)((DWORD_PTR)(h) & 0xffff)) << 16))))

#undef IS_INTRESOURCE
#define IS_INTRESOURCE(_r) ((((ULONG_PTR)(_r)) >> 16) == 0)

#ifndef CDSIZEOF_STRUCT
#define CDSIZEOF_STRUCT(structname,member) (((int)((LPBYTE)(&((structname*)0)->member) - ((LPBYTE)((structname*)0)))) + sizeof(((structname*)0)->member))
#endif

#undef OPENFILENAME_SIZE_VERSION_400 /* Mingw32 gets this wrong */
#define OPENFILENAME_SIZE_VERSION_400 CDSIZEOF_STRUCT(OPENFILENAME,lpTemplateName)

#ifndef VK_XBUTTON1
#  define VK_XBUTTON1 0x05
#endif
#ifndef VK_XBUTTON2
#  define VK_XBUTTON2 0x06
#endif
#ifndef VK_PACKET
#  define VK_PACKET 0xe7
#endif

#ifndef VK_NAVIGATION_VIEW
#define VK_NAVIGATION_VIEW     0x88
#endif
#ifndef VK_NAVIGATION_MENU
#define VK_NAVIGATION_MENU     0x89
#endif
#ifndef VK_NAVIGATION_UP
#define VK_NAVIGATION_UP       0x8A
#endif
#ifndef VK_NAVIGATION_DOWN
#define VK_NAVIGATION_DOWN     0x8B
#endif
#ifndef VK_NAVIGATION_LEFT
#define VK_NAVIGATION_LEFT     0x8C
#endif
#ifndef VK_NAVIGATION_RIGHT
#define VK_NAVIGATION_RIGHT    0x8D
#endif
#ifndef VK_NAVIGATION_ACCEPT
#define VK_NAVIGATION_ACCEPT   0x8E
#endif
#ifndef VK_NAVIGATION_CANCEL
#define VK_NAVIGATION_CANCEL   0x8F
#endif
#ifndef VK_OEM_FJ_JISHO
#  define VK_OEM_FJ_JISHO 0x92
#endif
#ifndef VK_OEM_FJ_MASSHOU
#  define VK_OEM_FJ_MASSHOU 0x93
#endif
#ifndef VK_OEM_FJ_TOUROKU
#  define VK_OEM_FJ_TOUROKU 0x94
#endif
#ifndef VK_OEM_FJ_LOYA
#  define VK_OEM_FJ_LOYA 0x95
#endif
#ifndef VK_OEM_FJ_ROYA
#  define VK_OEM_FJ_ROYA 0x96
#endif
#ifndef VK_BROWSER_BACK
#  define VK_BROWSER_BACK 0xa6
#endif
#ifndef VK_BROWSER_FORWARD
#  define VK_BROWSER_FORWARD 0xa7
#endif
#ifndef VK_BROWSER_REFRESH
#  define VK_BROWSER_REFRESH 0xa8
#endif
#ifndef VK_BROWSER_STOP
#  define VK_BROWSER_STOP 0xa9
#endif
#ifndef VK_BROWSER_SEARCH
#  define VK_BROWSER_SEARCH 0xaa
#endif
#ifndef VK_BROWSER_FAVORITES
#  define VK_BROWSER_FAVORITES 0xab
#endif
#ifndef VK_BROWSER_HOME
#  define VK_BROWSER_HOME 0xac
#endif
#ifndef VK_VOLUME_MUTE
#  define VK_VOLUME_MUTE 0xad
#endif
#ifndef VK_VOLUME_DOWN
#  define VK_VOLUME_DOWN 0xae
#endif
#ifndef VK_VOLUME_UP
#  define VK_VOLUME_UP 0xaf
#endif
#ifndef VK_MEDIA_NEXT_TRACK
#  define VK_MEDIA_NEXT_TRACK 0xb0
#endif
#ifndef VK_MEDIA_PREV_TRACK
#  define VK_MEDIA_PREV_TRACK 0xb1
#endif
#ifndef VK_MEDIA_STOP
#  define VK_MEDIA_STOP 0xb2
#endif
#ifndef VK_MEDIA_PLAY_PAUSE
#  define VK_MEDIA_PLAY_PAUSE 0xb3
#endif
#ifndef VK_LAUNCH_MAIL
#  define VK_LAUNCH_MAIL 0xb4
#endif
#ifndef VK_LAUNCH_MEDIA_SELECT
#  define VK_LAUNCH_MEDIA_SELECT 0xb5
#endif
#ifndef VK_LAUNCH_APP1
#  define VK_LAUNCH_APP1 0xb6
#endif
#ifndef VK_LAUNCH_APP2
#  define VK_LAUNCH_APP2 0xb7
#endif

#ifndef WM_MOUSEWHEEL
#  define WM_MOUSEWHEEL 522
#endif
#ifndef WM_MOUSEHWHEEL
#  define WM_MOUSEHWHEEL 526
#endif

#ifndef WM_UNICHAR
#define WM_UNICHAR 0x0109
#endif

#ifndef TPM_HORPOSANIMATION
#define TPM_HORPOSANIMATION 0x0400L
#endif
#ifndef TPM_HORNEGANIMATION
#define TPM_HORNEGANIMATION 0x0800L
#endif
#ifndef TPM_VERPOSANIMATION
#define TPM_VERPOSANIMATION 0x1000L
#endif
#ifndef TPM_VERNEGANIMATION
#define TPM_VERNEGANIMATION 0x2000L
#endif
#ifndef TPM_NOANIMATION
#define TPM_NOANIMATION 0x4000L
#endif
#ifndef TPM_LAYOUTRTL
#define TPM_LAYOUTRTL 0x8000L
#endif

#ifndef MAPVK_VSC_TO_VK_EX
#define MAPVK_VSC_TO_VK_EX 3
#endif

#else
typedef unsigned short LANGID;
#endif /* __WIN32__ */

#endif /* __WINDOWS__H__ */
