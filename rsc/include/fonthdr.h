#ifndef FONTHDR_H
#define FONTHDR_H

#define VDI_FONTNAMESIZE 32

typedef struct FONT_HDR
{
	short font_id;
	short point;
	char name[VDI_FONTNAMESIZE];
	unsigned short first_ade;
	unsigned short last_ade;
	unsigned short top;
	unsigned short ascent;
	unsigned short half;
	unsigned short descent;
	unsigned short bottom;
	unsigned short max_char_width;
	unsigned short max_cell_width;
	unsigned short left_offset;          /* amount character slants left when skewed */
	unsigned short right_offset;         /* amount character slants right */
	unsigned short thicken;              /* number of pixels to smear when bolding */
	unsigned short ul_size;              /* height of the underline */
	unsigned short lighten;              /* mask for lightening  */
	unsigned short skew;                 /* mask for skewing */
	unsigned short flags;                /* see bwlow */
	const uint16_t *hor_table;           /* horizontal offsets */
	const uint16_t *off_table;           /* character offsets  */
	const uint8_t *dat_table;            /* character definitions (raster data) */
	unsigned short form_width;           /* width of raster in bytes */
	unsigned short form_height;          /* height of raster in lines */
	const struct FONT_HDR *next_font;    /* pointer to next font */
} FONT_HDR;

#define SIZEOF_FONT_HDR 90

/* definitions for flags */
#define FONTF_SYSTEM     0x0001            /* Default system font */
#define FONTF_HORTABLE   0x0002            /* Use horizontal offsets table */
#define FONTF_BIGENDIAN  0x0004            /* Font image is in byteswapped format */
#define FONTF_MONOSPACED 0x0008            /* Font is monospaced */
#define FONTF_EXTENDED   0x0020            /* Extended font header */
#define FONTF_FULLID     0x2000            /* Use 'full font ID' */

#endif /* FONTHDR_H */
