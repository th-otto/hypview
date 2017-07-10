/**********************************************************************8*******
 * ICN.C
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <picture.h>

#define toword(pixels) (((pixels) + 15) >> 4)

#define C_CR 0x0d
#define C_NL 0x0a

#define C_VALLEN 128

#undef isalpha
#define isalpha(c) (((c) >= 'A' && (c) <= 'Z') || ((c) >= 'a' && (c) <= 'z'))
#undef isdigit
#define isdigit(c) ((c) >= '0' && (c) <= '9')
#undef isalnum
#define isalnum(c) (isalpha(c) || isdigit(c))

/*** ---------------------------------------------------------------------- ***/

typedef enum {
	T_EOF = 0,
	T_EXCLAMATION = '!',
	T_DOLLAR = '$',
	T_CPP = '#',
	T_MOD = '%',
	T_AND = '&',
	T_LPAREN = '(',
	T_RPAREN = ')',
	T_MUL = '*',
	T_PLUS = '+',
	T_COMMA = ',',
	T_MINUS = '-',
	T_PERIOD = '.',
	T_DIV = '/',
	T_COLON = ':',
	T_SEMI = ';',
	T_LESS = '<',
	T_EQUAL = '=',
	T_GREATER = '>',
	T_QUEST = '?',
	T_AT = '@',
	T_LBRACKET = '[',
	T_BACKSLASH = '\\',
	T_RBRACKET = ']',
	T_LBRACE = '{',
	T_OR = '|',
	T_RBRACE = '}',
	T_TILDE = '~',
	T_NUMCONST = 256,
	T_CHARCONST,
	T_STRCONST,
	T_ID,
	T_INVALID = T_EOF
} TOKEN;


static int c_escchar(int c)
{
	switch (c)
	{
		case 'n': return C_NL;
		case 'r': return C_CR;
		case 'f': return '\f';
		case 'v': return '\v';
		case 't': return '\t';
		case 'a': return '\007';
		case 'e': return '\033';
		case '"': return '"';
		case '\'': return '\'';
		case '\\': return '\\';
	}		
	return c;
}


#define put_numconst(val) \
	buf[0] = (val >> 24) & 0xff; \
	buf[1] = (val >> 16) & 0xff; \
	buf[2] = (val >>  8) & 0xff; \
	buf[3] = (val      ) & 0xff; \
	buf += 4
#define get_numconst(buf) \
	((((unsigned long)(buf[0])) << 24) | \
	 (((unsigned long)(buf[1])) << 16) | \
	 (((unsigned long)(buf[2])) <<  8) | \
	 (((unsigned long)(buf[3]))      ))

/* ------------------------------------------------------------------------- */

static int c_parse_getc(PICTURE *pic, const unsigned char *inbuf)
{
	int c;
	
	if (pic->pi_dataoffset >= pic->pi_filesize)
		return EOF;
	c = inbuf[pic->pi_dataoffset++];
	if (c == C_NL)
	{
		c = '\n';
	} else if (c == C_CR)
	{
		c = '\n';
		if (pic->pi_dataoffset < pic->pi_filesize && inbuf[pic->pi_dataoffset] == C_NL)
			pic->pi_dataoffset++;
	}
	return c;
}


static void c_parse_ungetc(PICTURE *pic)
{
	if (pic->pi_dataoffset > 0)
		--pic->pi_dataoffset;
}


static TOKEN gettok(PICTURE *pic, const unsigned char *inbuf, unsigned char *buf)
{
	int c;
	TOKEN toktype;
	int len;
	
again:;
	while ((c = c_parse_getc(pic, inbuf)) == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f')
		;
	
	if (c == '/')
	{
		if ((c = c_parse_getc(pic, inbuf)) == '*')
		{
			for (;;)
			{
				while ((c = c_parse_getc(pic, inbuf)) != EOF && c != '*')
					;
				if (c == EOF)
					return T_EOF;
				if ((c = c_parse_getc(pic, inbuf)) == '/')
					goto again;
				if (c == EOF)
					return T_EOF;
				c_parse_ungetc(pic);
			}
		} else
		{
			*buf++ = '/';
			toktype = T_DIV;
		}
	} else if (isalpha(c) || c == '_')
	{
		*buf++ = (unsigned char)c;
		len = 1;
		c = c_parse_getc(pic, inbuf);
		while (isalnum(c) || c == '_')
		{
			if (len < C_VALLEN)
			{
				*buf++ = (unsigned char)c;
				len++;
			}
			c = c_parse_getc(pic, inbuf);
		}
		toktype = T_ID;
	} else if (isdigit(c))
	{
		unsigned long base;
		unsigned long val;
		
		base = 10;
		val = 0;
		if (c == '0')
		{
			c = c_parse_getc(pic, inbuf);
			if (c == 'x' || c == 'X')
			{
				base = 16;
			} else
			{
				base = 8;
				if (c != EOF)
					c_parse_ungetc(pic);
			}
		} else
		{
			val = c - '0';
		}
		while ((c = c_parse_getc(pic, inbuf)) != EOF)
		{
			if (isdigit(c))
				c = c - '0';
			else if (c >= 'a' && c <= 'f')
				c = c - 'a' + 10;
			else if (c >= 'A' && c <= 'F')
				c = c - 'A' + 10;
			else
				break;
			if ((unsigned int)c >= base)
				break;
			val = val * base + (unsigned int)(c);
		}
		put_numconst(val);
		toktype = T_NUMCONST;
	} else if (c == '\'')
	{
		if ((c = (c_parse_getc(pic, inbuf))) == '\\')
			c = c_escchar(c_parse_getc(pic, inbuf));
		*buf++ = (unsigned char)c;
		while ((c = c_parse_getc(pic, inbuf)) != EOF && c != '\'')
			;
		c = EOF;
		toktype = T_CHARCONST;
	} else if (c == '"')
	{
		len = 0;
		while ((c = c_parse_getc(pic, inbuf)) != EOF)
		{
			if (c == '\\')
				c = c_escchar(c_parse_getc(pic, inbuf));
			else if (c == '"')
				break;
			if (len < C_VALLEN)
			{
				*buf++ = (unsigned char)c;
				len++;
			}
		}
		c = EOF;
		toktype = T_STRCONST;
	} else
	{
		*buf++ = (unsigned char)c;
		switch (c)
		{
			case '!': toktype = T_EXCLAMATION; break;
			case '$': toktype = T_DOLLAR; break;
			case '#': toktype = T_CPP; break;
			case '%': toktype = T_MOD; break;
			case '&': toktype = T_AND; break;
			case '(': toktype = T_LPAREN; break;
			case ')': toktype = T_RPAREN; break;
			case '*': toktype = T_MUL; break;
			case '+': toktype = T_PLUS; break;
			case ',': toktype = T_COMMA; break;
			case '-': toktype = T_MINUS; break;
			case '.': toktype = T_PERIOD; break;
			case '/': toktype = T_DIV; break;
			case ':': toktype = T_COLON; break;
			case ';': toktype = T_SEMI; break;
			case '<': toktype = T_LESS; break;
			case '=': toktype = T_EQUAL; break;
			case '>': toktype = T_GREATER; break;
			case '?': toktype = T_QUEST; break;
			case '@': toktype = T_AT; break;
			case '[': toktype = T_LBRACKET; break;
			case '\\': toktype = T_BACKSLASH; break;
			case ']': toktype = T_RBRACKET; break;
			case '{': toktype = T_LBRACE; break;
			case '|': toktype = T_OR; break;
			case '}': toktype = T_RBRACE; break;
			case '~': toktype = T_TILDE; break;
			case EOF: toktype = T_EOF; break;
			default: toktype = T_INVALID; break;
		}
		c = EOF;
	}
	*buf = '\0';
	if (c != EOF)
		c_parse_ungetc(pic);
	return toktype;
}

/* ------------------------------------------------------------------------- */

static TOKEN icn_data_read(PICTURE *pic, const unsigned char *inbuf, unsigned char *data, long width, long height, _WORD planes, TOKEN tok)
{
	unsigned char buf[C_VALLEN + 1];
	long offset;
	long h;
	
	offset = 0;
	while (--planes >= 0)
	{
		h = height;
		while (h)
		{
			if (tok == T_EOF)
				tok = gettok(pic, inbuf, buf);
			if (tok != T_NUMCONST)
				break;
			*data++ = buf[2];
			*data++ = buf[3];
			if (++offset == width)
			{
				offset = 0;
				h--;
			}
			if ((tok = gettok(pic, inbuf, buf)) == T_COMMA)
				tok = gettok(pic, inbuf, buf);
		}
	}
	return tok;
}


static TOKEN icn_data_skip(PICTURE *pic, const unsigned char *inbuf, long width, long height, _WORD planes, TOKEN tok)
{
	unsigned char buf[C_VALLEN + 1];
	long offset;
	long h;
	
	offset = 0;
	while (--planes >= 0)
	{
		h = height;
		while (h)
		{
			if (tok == T_EOF)
				tok = gettok(pic, inbuf, buf);
			if (tok != T_NUMCONST)
				break;
			if (++offset == width)
			{
				offset = 0;
				h--;
			}
			if ((tok = gettok(pic, inbuf, buf)) == T_COMMA)
				tok = gettok(pic, inbuf, buf);
		}
	}
	return tok;
}


static gboolean icn_scan(PICTURE *pic, const unsigned char *inbuf, unsigned char *datap, unsigned char *maskp)
{
	TOKEN tok;
	unsigned char buf[C_VALLEN + 1];
	unsigned char buf2[C_VALLEN + 1];
	unsigned long iconw, iconh, iconsize;
	unsigned long planes;
	unsigned long val;
	gboolean has_mask = FALSE;
	
	iconw = iconh = iconsize = 0;
	while ((tok = gettok(pic, inbuf, buf)) != T_EOF && tok != T_LBRACKET)
	{
		switch ((int)tok)
		{
		case T_CPP:
			if (gettok(pic, inbuf, buf) != T_ID || strcmp((char *) buf, "define") != 0)
				return FALSE;
			if (gettok(pic, inbuf, buf) != T_ID ||
				gettok(pic, inbuf, buf2) != T_NUMCONST)
				return FALSE;
			val = get_numconst(buf2);
			if (strcmp((char *) buf, "ICON_W") == 0 || strcmp((char *) buf, "SHAP_W") == 0)
				iconw = val;
			else if (strcmp((char *) buf, "ICON_H") == 0 || strcmp((char *) buf, "SHAP_H") == 0)
				iconh = val;
			else if (strcmp((char *) buf, "ICONSIZE") == 0 || strcmp((char *) buf, "DATASIZE") == 0)
				iconsize = val;
			else
				return FALSE;
			break;
		case T_LBRACE:
		case T_RBRACKET:
			return FALSE;
		default:
			break;
		}
	}
	if (iconw == 0 || iconh == 0 || tok == T_EOF)
		return FALSE;
	pic->pi_width = (_WORD) iconw;
	pic->pi_height = (_WORD) iconh;
	iconw = toword(iconw);
	if (iconsize == 0)
	{
		planes = 1;
	} else if (iconsize == iconw * iconh)
	{
		planes = 1;
	} else if (iconsize == iconw * iconh * 2)
	{
		planes = 2;
	} else if (iconsize == iconw * iconh * 4)
	{
		planes = 4;
	} else if (iconsize == iconw * iconh * 8)
	{
		planes = 8;
	} else
	{
		return FALSE;
	}
	pic->pi_planes = (_WORD) planes;
	while ((tok = gettok(pic, inbuf, buf)) != T_RBRACKET && tok != T_EOF)
	{
		if (tok == T_MUL)
		{
			if ((tok = gettok(pic, inbuf, buf)) == T_NUMCONST)
			{
				planes = get_numconst(buf);
				if ((planes == 2 && pic->pi_planes == 1) ||
					(planes & 1))
				{
					planes--;
					has_mask = TRUE;
				}
			}
		}
	}
	pic->pi_planes = (_WORD) planes;
	if (tok == T_EOF)
		return FALSE;	
	if ((tok = gettok(pic, inbuf, buf)) != T_EQUAL)
		return FALSE;
	while ((tok = gettok(pic, inbuf, buf)) != T_LBRACE && tok != T_EOF)
		;
	if (tok == T_EOF)
		return FALSE;
	tok = T_EOF;
	if (datap != NULL)
		tok = icn_data_read(pic, inbuf, datap, iconw, iconh, (_WORD) planes, tok);
	else
		tok = icn_data_skip(pic, inbuf, iconw, iconh, (_WORD) planes, tok);
	if (has_mask)
	{
		if (maskp != NULL)
			tok = icn_data_read(pic, inbuf, maskp, iconw, iconh, 1, tok);
		else
			tok = icn_data_skip(pic, inbuf, iconw, iconh, 1, tok);
	} else
	{
		if (maskp != NULL)
			memset(maskp, 0, iconw * 2 * iconh);
	}
	if (datap != NULL && tok != T_NUMCONST)
	{
		if (tok != T_RBRACE ||
			gettok(pic, inbuf, buf) != T_SEMI ||
			gettok(pic, inbuf, buf) != T_EOF)
			return FALSE;
	}

	pic_calcsize(pic);
	pic_stdpalette(pic->pi_palette, pic->pi_planes);
 	return TRUE;
}


gboolean pic_type_icn(PICTURE *pic, const _UBYTE *buf, _LONG size)
{
	pic->pi_filesize = size;
	pic->pi_dataoffset = 0;
	if (icn_scan(pic, buf, NULL, NULL))
	{
		pic->pi_dataoffset = 0;
		pic->pi_type = FT_ICN;
		return TRUE;
	}
	return FALSE;
}


gboolean icn_unpack(_UBYTE *dest, const _UBYTE *src, PICTURE *pic, gboolean mask)
{
	pic->pi_dataoffset = 0;
	return icn_scan(pic, src, dest, mask ? dest + pic->pi_picsize : NULL);
}


/*** ---------------------------------------------------------------------- ***/

static gboolean icn_data_write(FILE *fp, const unsigned char *data, _LONG width, _WORD height, _WORD planes, _WORD *offp)
{
	_WORD offset;
	_LONG i;
	_WORD j;
	_UWORD c;
	
	offset = *offp;
	while (--planes >= 0)
	{
		for (j = height; --j >= 0; )
		{
			for (i = width; (i -= 1) >= 0; )
			{
				if (offset == 0)
					fputs("  ", fp);
				c = (data[0] << 8) + data[1];
				fprintf(fp, "0x%04x", c);
				if (i != 0 || j != 0 || planes != 0)
					fputs(",", fp);
				data += 2;
				if (++offset == 8)
				{
					fputs("\n", fp);
					offset = 0;
				}
			}
		}
		if (planes != 0)
		{
			fputs("\n", fp);
			offset = 0;
		}
	}
	*offp = offset;
	return TRUE;
}


gboolean icn_fwrite(FILE *fp, const _UBYTE *src, PICTURE *pic, gboolean mask)
{
	_LONG iconsize;
	_WORD offset;
	_LONG width;
	const _UBYTE *datap = src;
	const _UBYTE *maskp = NULL;
	
	fputs("/* GEM icon definition */\n", fp);
	fprintf(fp, "#define ICON_W 0x%04x\n", pic->pi_width);
	fprintf(fp, "#define ICON_H 0x%04x\n", pic->pi_height);
	width = toword(pic->pi_width);
	iconsize = width * pic->pi_height;
	if (mask)
		maskp = src + iconsize * 2;
	fprintf(fp, "#define ICONSIZE 0x%04lx\n", iconsize);
	if (pic->pi_planes == 1)
	{
		fprintf(fp, "WORD image[ICONSIZE%s] = {\n", (datap != NULL && maskp != NULL) ? "*2" : "");
	} else
	{
		fprintf(fp, "WORD image[ICONSIZE*%d] = {\n", pic->pi_planes + (maskp != NULL ? 1 : 0));
	}
	offset = 0;
	if (datap != NULL)
		if (icn_data_write(fp, datap, width, pic->pi_height, pic->pi_planes, &offset) == FALSE)
			return FALSE;
	if (maskp != NULL)
	{
		if (datap != NULL)
			fputs(",\n", fp);
		offset = 0;
		if (icn_data_write(fp, maskp, width, pic->pi_height, 1, &offset) == FALSE)
			return FALSE;
	}
	if (offset != 0)
		fputs("\n", fp);
	fputs("};\n", fp);
	return !ferror(fp);
}
