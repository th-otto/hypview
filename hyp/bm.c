/*
 * Jump table structures.
 */
typedef struct {
	size_t last_jump;
	size_t delta[256];
	size_t patlen;
	const unsigned char *pattern;
} BM_TABLE;

/* ------------------------------------------------------------------------- */

/*
 * make_delta -- Create the delta tables.
 */
static void make_delta(const char *pstring, BM_TABLE *tbl)
{
	size_t j;
	size_t jump_by;
	unsigned char ch;
	const unsigned char *sp;
	
	tbl->pattern = (const unsigned char *)pstring;

	jump_by = strlen(pstring);

	for (j = 0; j < 256; j++)
		tbl->delta[j] = jump_by;

	jump_by -= 1;

	/* Now put in the characters contained
	 * in the pattern, duplicating the CASE.
	 */
	sp = tbl->pattern;
	for (j = 0; j < jump_by; j++)
	{
		ch = *sp++;
		tbl->delta[ch] = jump_by - j;
	}

	/* The last character (left over from the loop above) will
	 * have the pattern length, unless there are duplicates of
	 * it.  Get the number to jump from the delta array, and
	 * overwrite with zeroes in delta duplicating the CASE.
	 */
	ch = *sp;
	tbl->patlen = jump_by;
	tbl->last_jump = tbl->delta[ch];

	tbl->delta[ch] = 0;
}

/* ------------------------------------------------------------------------- */

/*
 * liteq -- compare the string versus the current characters in the line.
 *  Returns 0 (no match) or the number of characters matched.
 */
static gboolean liteq(const unsigned char *text, const unsigned char *pattern)
{
	const unsigned char *strptr = pattern;
	
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

static gboolean fbound(const unsigned char **ptext, size_t jump, const unsigned char *end, BM_TABLE *tbl)
{
	const unsigned char *text = *ptext;
	
	while (jump != 0)
	{
		text += jump;
		if (text >= end)
			return TRUE;			/* hit end of buffer */

		jump = tbl->BM_TABLE[*text];
	}
	*ptext = text;
	return FALSE;
}

/* ------------------------------------------------------------------------- */

/*
 * scanner -- Search for a pattern in either direction.  If found,
 *	reset the "." to be at the start or just after the match string,
 *	and (perhaps) repaint the display.
 *	Fast version using simplified version of Boyer and Moore
 *	Software-Practice and Experience, vol 10, 501-506 (1980)
 */
static const char *scanner(struct hypfind_opts *opts, const char *buf, size_t len)
{
	BM_TABLE *tbl = &opts->deltapat;
	register size_t patlenadd;
	const unsigned char *text = (const unsigned char *)buf;
	const unsigned char *end = text + len;
	const char *match;
	
	patlenadd = tbl->patlen;
	
	/* Scan each character until we hit the head link record.
	 * Get the character resolving newlines, offset
	 * by the pattern length, i.e. the last character of the
	 * potential match.
	 */
	if (!fbound(&text, patlenadd, end, tbl))
	{
		do
		{
			/* Save the current position in case we match
			 * the search string at this point.
			 */
			text -= patlenadd;
			match = (const char *)text;
			
			if (liteq(text, opts->deltapat.pattern))
			{
				return match;
			}
			
			text += patlenadd;
		} while (!fbound(&text, tbl->last_jump, end, tbl));
	}

	return NULL;	/* We could not find a match */
}

