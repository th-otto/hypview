/*****************************************************************************
 * win32/windebug.h
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#define WIN32_DEBUG_MSGS 0

const char *win32debug_msg_name(UINT message);

#if WIN32_DEBUG_MSGS
void win32debug_msg_print(FILE *out, const TCHAR *prefix, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

#define win32debug_msg_end(prefix, message, ret) \
	fprintf(stderr, "%s %s return %s\n", prefix, win32debug_msg_name(message), ret)

#else

#define win32debug_msg_print(out, prefix, hwnd, message, wparam, lparam)
#define win32debug_msg_end(prefix, message, ret)

#endif

#ifdef __cplusplus
}
#endif
