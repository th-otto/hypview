#ifndef __W_DRAW_H__
#define __W_DRAW_H__

typedef struct {
	_ULONG bm_magic;
#define BITMAP_MAGIC 0x4afcdeadl
	_UBYTE *bm_orig_data;
	_UBYTE *bm_data;
	void   *bm_info;
} IBITMAP;

typedef struct _font_attr {
	int size;						/* size of font, in 1/10 points */
	CGFloat font_size;
	unsigned int textstyle;			/* bitmask of text effects */
	char name[FONT_NAME_LEN];		/* font Family */
	CGFontRef cgFont;
	CGFloat font_mult;
	NSFont *font;
	int ascent, descent;
} FONT_ATTR;

#endif /* __W_DRAW_H__ */
