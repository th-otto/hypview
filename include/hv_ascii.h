typedef struct
{
	long length;
	unsigned char **line_ptr;
	long line_height;			/* copy of win->y_raster */
	long char_width;			/* copy of win->x_raster */
	unsigned char start[1];
} FMT_ASCII;

typedef FMT_ASCII FMT_BINARY;


/*
 *		Ascii.c
 */
void AsciiDisplayPage(DOCUMENT *doc);
void AsciiGetCursorPosition(DOCUMENT *doc, int x, int y, TEXT_POS *pos);
gboolean AsciiBlockOperations(DOCUMENT *doc, hyp_blockop op, BLOCK *block, void *param);
long AsciiAutolocator(DOCUMENT *doc, long line, const char *search);
unsigned char *AsciiGetTextLine(const unsigned char *src, const unsigned char *end);

/*
 *		Binary.c
 */
void BinaryDisplayPage(DOCUMENT *doc);
void BinaryGetCursorPosition(DOCUMENT *doc, int x, int y, TEXT_POS *pos);
gboolean BinaryBlockOperations(DOCUMENT *doc, hyp_blockop op, BLOCK *block, void *param);
long BinaryAutolocator(DOCUMENT *doc, long line, const char *search);
