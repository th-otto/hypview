#include "hypdefs.h"
#include "hypdebug.h"
#include "hcp.h"
#include "bm.h"

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static void bm_init_block(BM_TABLE *tbl, size_t *delta)
{
	size_t j;

	for (j = 0; j < 256; j++)
		delta[j] = tbl->patlen;
}

/* ------------------------------------------------------------------------- */

static gboolean bm_add_delta(BM_TABLE *tbl, h_unichar_t ch, size_t delta)
{
	unsigned int idx;
	
	if (ch >= 0x10000UL)
	{
		tbl->slowcase = TRUE;
		return TRUE;
	}
	idx = (unsigned int)(ch >> 8);
	if (tbl->skip_table[idx] == tbl->delta)
	{
		tbl->skip_table[idx] = g_new(size_t, 256);
		if (tbl->skip_table[idx] == NULL)
			return FALSE;
		bm_init_block(tbl, tbl->skip_table[idx]);
	}
	tbl->skip_table[idx][ch & 0xff] = delta;
	return TRUE;
}

/* ------------------------------------------------------------------------- */

/*
 * bm_init -- Create the delta tables.
 */
gboolean bm_init(BM_TABLE *tbl, const char *pstring, gboolean casesensitive)
{
	size_t j;
	size_t jump_by;
	
	for (j = 0; j < 256; j++)
		tbl->skip_table[j] = tbl->delta;
		
	tbl->pattern = pstring;

	if (pstring == NULL)
	{
		tbl->patlen = 0;
		bm_init_block(tbl, tbl->delta);
		return TRUE;
	}
	
	/*
	 * we cannot scan for an empty pattern
	 */
	if (*pstring == '\0')
		return FALSE;
	
	tbl->casesensitive = casesensitive;
	tbl->slowcase = FALSE;
	
	jump_by = strlen(pstring);
	tbl->patlen = jump_by;

	bm_init_block(tbl, tbl->delta);

	if (casesensitive)
	{
		const unsigned char *sp;
		unsigned char ch;
		
		/*
		 * we don't have to care for any mapping,
		 * and can just search for the bytes.
		 */
		jump_by -= 1;
		tbl->patlen_minus_1 = jump_by;
		
		/* Now put in the characters contained
		 * in the pattern.
		 */
		sp = (const unsigned char *)tbl->pattern;
		for (j = 0; j < jump_by; j++)
		{
			ch = *sp++;
			tbl->delta[ch] = jump_by - j;
		}
		
		/* The last character (left over from the loop above) will
		 * have the pattern length, unless there are duplicates of
		 * it.  Get the number to jump from the delta array, and
		 * overwrite with zeroes in delta.
		 */
		ch = *sp;
		tbl->last_jump = tbl->delta[ch];
		
		tbl->delta[ch] = 0;
	} else
	{
		h_unichar_t ch;
		const char *sp;
		
		/*
		 * get byte-length of pattern without the last character
		 */
		sp = tbl->pattern;
		do {
			jump_by = sp - tbl->pattern;
			sp = g_utf8_skipchar(sp);
		} while (*sp != '\0');
		tbl->patlen_minus_1 = jump_by;
		
		/* Now put in the characters contained
		 * in the pattern, duplicating the CASE.
		 */
		sp = tbl->pattern;
		for (;;)
		{
			j = jump_by - (sp - tbl->pattern);
			ch = hyp_utf8_get_char(sp);
			sp = g_utf8_skipchar(sp);
			if (*sp == '\0')
				break;
			if (!bm_add_delta(tbl, ch, j))
				return FALSE;
			if (!bm_add_delta(tbl, g_unichar_tolower(ch), j))
				return FALSE;
			if (!bm_add_delta(tbl, g_unichar_toupper(ch), j))
				return FALSE;
		}
		
		/* The last character (left over from the loop above) will
		 * have the pattern length, unless there are duplicates of
		 * it.  Get the number to jump from the delta array, and
		 * overwrite with zeroes in delta duplicating the CASE.
		 */
		if (ch >= 0x10000UL)
		{
			tbl->slowcase = TRUE;
		} else
		{
			unsigned int idx = (unsigned int)(ch >> 8);
			tbl->last_jump = tbl->skip_table[idx][ch & 0xff];
			if (!bm_add_delta(tbl, ch, 0))
				return FALSE;
			if (!bm_add_delta(tbl, g_unichar_tolower(ch), 0))
				return FALSE;
			if (!bm_add_delta(tbl, g_unichar_toupper(ch), 0))
				return FALSE;
		}
	}
	
	return TRUE;
}

/* ------------------------------------------------------------------------- */

void bm_exit(BM_TABLE *tbl)
{
	size_t j;
	
	if (tbl->pattern == NULL || tbl->casesensitive)
		return;
	for (j = 0; j < 256; j++)
		if (tbl->skip_table[j] != tbl->delta)
		{
			g_free(tbl->skip_table[j]);
			tbl->skip_table[j] = NULL;
		}
}

/* ------------------------------------------------------------------------- */

/*
 * bm_streq -- compare the string versus the current characters in the line.
 *  Returns 0 (no match) or the number of characters matched.
 */
static gboolean bm_streq(BM_TABLE *tbl, const unsigned char *text)
{
	const unsigned char *strptr = (const unsigned char *)tbl->pattern;
	
	while (*strptr)
	{
		if (*strptr != *text)
			return FALSE;
		strptr++;
		text++;
	}
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean bm_strcaseeq(BM_TABLE *tbl, const unsigned char *text)
{
	const char *strptr = tbl->pattern;
	const char *textptr = (const char *)text;
	h_unichar_t ch1, ch2;
	
	while (*strptr)
	{
		ch1 = g_unichar_tolower(hyp_utf8_get_char(textptr));
		if (ch1 >= 0x10000UL)
			return hyp_utf8_strncasecmp((const char *)text, tbl->pattern, tbl->patlen) == 0;
		ch2 = g_unichar_tolower(hyp_utf8_get_char(strptr));
		if (ch1 != ch2)
			return FALSE;
		strptr = g_utf8_skipchar(strptr);
		textptr = g_utf8_skipchar(textptr);
	}
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static const unsigned char *bm_boundary(BM_TABLE *tbl, const unsigned char *text, size_t jump, const unsigned char *end)
{
	while (jump != 0)
	{
		text += jump;
		if (text >= end)
			return NULL;

		jump = tbl->delta[*text];
	}
	return text;
}

/* ------------------------------------------------------------------------- */

static const unsigned char *bm_caseboundary(BM_TABLE *tbl, const unsigned char *text, size_t jump, const unsigned char *end)
{
	h_unichar_t ch;
	unsigned int idx;
	
	while (jump != 0)
	{
		text += jump;
		if (text >= end)
			return NULL;
		ch = hyp_utf8_get_char((const char *)text);
		if (ch >= 0x10000UL)
		{
			tbl->slowcase = TRUE;
			return text;
		}
		idx = (unsigned int)(ch >> 8);
		jump = tbl->skip_table[idx][ch & 0xff];
	}
	return text;
}

/* ------------------------------------------------------------------------- */

const char *bm_scanner(BM_TABLE *tbl, const char *buf, size_t len)
{
	size_t patlenadd;
	const unsigned char *text = (const unsigned char *)buf;
	const unsigned char *end = text + len;
	
	patlenadd = tbl->patlen_minus_1;
	
	/* Scan each character until we hit the head link record.
	 * Get the character resolving newlines, offset
	 * by the pattern length, i.e. the last character of the
	 * potential match.
	 */
	if ((text = bm_boundary(tbl, text, patlenadd, end)) != NULL)
	{
		do
		{
			/* Save the current position in case we match
			 * the search string at this point.
			 */
			text -= patlenadd;
			
			if (bm_streq(tbl, text))
			{
				return (const char *)text;
			}
			
			text += patlenadd;
		} while ((text = bm_boundary(tbl, text, tbl->last_jump, end)) != NULL);
	}

	return NULL;	/* We could not find a match */
}

/* ------------------------------------------------------------------------- */

const char *bm_casescanner(BM_TABLE *tbl, const char *buf, size_t len)
{
	size_t patlenadd;
	const unsigned char *text = (const unsigned char *)buf;
	const unsigned char *end = text + len;
	
	patlenadd = tbl->patlen_minus_1;
	
	/* Scan each character until we hit the head link record.
	 * Get the character resolving newlines, offset
	 * by the pattern length, i.e. the last character of the
	 * potential match.
	 */
	if ((text = bm_caseboundary(tbl, text, patlenadd, end)) != NULL)
	{
		do
		{
			if (tbl->slowcase)
				return hyp_utf8_strcasestr(buf, tbl->pattern);
			
			/* Save the current position in case we match
			 * the search string at this point.
			 */
			text -= patlenadd;
			
			if (bm_strcaseeq(tbl, text))
			{
				return (const char *)text;
			}
			
			text += patlenadd;
		} while ((text = bm_caseboundary(tbl, text, tbl->last_jump, end)) != NULL);
	}

	return NULL;	/* We could not find a match */
}
