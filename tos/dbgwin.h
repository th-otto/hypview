#ifndef __DBGWIN_H__
#define __DBGWIN_H__

#define DBG_WIND 0

#if DBG_WIND

#define wind_open(hndl, x, y, w, h) dbg_wind_open(hndl, x, y, w, h, __FILE__, __LINE__)
#define wind_open_grect(hndl, gr) dbg_wind_open(hndl, (gr)->g_x, (gr)->g_y, (gr)->g_w, (gr)->g_h, __FILE__, __LINE__)
#define wind_create(kind, x, y, w, h) dbg_wind_create(kind, x, y, w, h, __FILE__, __LINE__)
#define wind_create_grect(kind, gr) dbg_wind_create(kind, (gr)->g_x, (gr)->g_y, (gr)->g_w, (gr)->g_h, __FILE__, __LINE__)
#define wind_close(hndl) dbg_wind_close(hndl, __FILE__, __LINE__)
#define wind_delete(hndl) dbg_wind_delete(hndl, __FILE__, __LINE__)
#define wind_set(hndl, field, a, b, c, d) dbg_wind_set(hndl, field, a, b, c, d, __FILE__, __LINE__)
#define wind_set_grect(hndl, field, gr) dbg_wind_set(hndl, field, (gr)->g_x, (gr)->g_y, (gr)->g_w, (gr)->g_h, __FILE__, __LINE__)
#define wind_set_str(hndl, field, p1) dbg_wind_set_ptr(hndl, field, NO_CONST(p1), NULL, __FILE__, __LINE__)
#define wind_set_ptr(hndl, field, p1, p2) dbg_wind_set_ptr(hndl, field, p1, p2, __FILE__, __LINE__)
#define wind_set_int(hndl, field, a) dbg_wind_set(hndl, field, a, 0, 0, 0, __FILE__, __LINE__)
#define wind_get(hndl, field, a, b, c, d) dbg_wind_get(hndl, field, a, b, c, d, __FILE__, __LINE__)
#define wind_get_int(hndl, field, a) dbg_wind_get(hndl, field, a, NULL, NULL, NULL, __FILE__, __LINE__)
#define wind_get_grect(hndl, field, gr) dbg_wind_get(hndl, field, &(gr)->g_x, &(gr)->g_y, &(gr)->g_w, &(gr)->g_h, __FILE__, __LINE__)

_WORD dbg_wind_open(_WORD hndl, _WORD x, _WORD y, _WORD w, _WORD h, const char *file, int line);
_WORD dbg_wind_create(_WORD kind, _WORD x, _WORD y, _WORD w, _WORD h, const char *file, int line);
_WORD dbg_wind_close(_WORD hndl, const char *file, int line);
_WORD dbg_wind_delete(_WORD hndl, const char *file, int line);
_WORD dbg_wind_set(_WORD hndl, _WORD field, _WORD a, _WORD b, _WORD c, _WORD d, const char *file, int line);
_WORD dbg_wind_set_ptr(_WORD hndl, _WORD field, void *p1, void *p2, const char *file, int line);
_WORD dbg_wind_get(_WORD hndl, _WORD field, _WORD *a, _WORD *b, _WORD *c, _WORD *d, const char *file, int line);

#endif

#endif /* __DBGWIN_H__ */