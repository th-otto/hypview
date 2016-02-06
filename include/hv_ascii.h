#ifndef __HYP_ASCII_H__
#define __HYP_ASCII_H__

typedef struct
{
	long length;
	unsigned char **line_ptr;
	long lines;
	long columns;
	HYP_CHARSET charset;
	unsigned char start[1];
} FMT_ASCII;

typedef FMT_ASCII FMT_BINARY;


/*
 * hv_asc.c
 */
void AsciiDisplayPage(WINDOW_DATA *win);
void AsciiGetCursorPosition(WINDOW_DATA *win, int x, int y, TEXT_POS *pos);
gboolean AsciiBlockOperations(WINDOW_DATA *win, hyp_blockop op, BLOCK *block, void *param);
long AsciiAutolocator(WINDOW_DATA *win, long line, const char *search, gboolean casesensitive, gboolean wordonly);
unsigned char *AsciiGetTextLine(const unsigned char *src, const unsigned char *end);
void AsciiPrep(WINDOW_DATA *win, HYP_NODE *node);

/*
 * hv_bin.c
 */
void BinaryDisplayPage(WINDOW_DATA *win);
void BinaryGetCursorPosition(WINDOW_DATA *win, int x, int y, TEXT_POS *pos);
gboolean BinaryBlockOperations(WINDOW_DATA *win, hyp_blockop op, BLOCK *block, void *param);
long BinaryAutolocator(WINDOW_DATA *win, long line, const char *search, gboolean casesensitive, gboolean wordonly);

#endif /* __HYP_ASCII_H__ */
