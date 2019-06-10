#ifndef __HYPBM_H__
#define __HYPBM_H__ 1

/*
 * Jump table structures.
 */
typedef struct {
	size_t last_jump;
	size_t delta[256];
	size_t patlen;
	size_t patlen_minus_1;
	const char *pattern;
	gboolean casesensitive;
	gboolean slowcase;
	size_t *skip_table[256];
} BM_TABLE;

gboolean bm_init(BM_TABLE *tbl, const char *pstring, gboolean casesensitive);
void bm_exit(BM_TABLE *tbl);

const char *bm_scanner(BM_TABLE *tbl, const char *buf, size_t len);
const char *bm_casescanner(BM_TABLE *tbl, const char *buf, size_t len);

#endif /* __HYPBM_H__ */
