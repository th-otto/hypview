/* Provide fallback values for macros that ought to be defined in <inttypes.h>.
   Note that our fallback values need not be literal strings, because we don't
   use them with preprocessor string concatenation.  */
#if !defined PRId8 || defined PRI_MACROS_BROKEN
# undef PRId8
# define PRId8 "d"
#endif
#if !defined PRIi8 || defined PRI_MACROS_BROKEN
# undef PRIi8
# define PRIi8 "i"
#endif
#if !defined PRIo8 || defined PRI_MACROS_BROKEN
# undef PRIo8
# define PRIo8 "o"
#endif
#if !defined PRIu8 || defined PRI_MACROS_BROKEN
# undef PRIu8
# define PRIu8 "u"
#endif
#if !defined PRIx8 || defined PRI_MACROS_BROKEN
# undef PRIx8
# define PRIx8 "x"
#endif
#if !defined PRIX8 || defined PRI_MACROS_BROKEN
# undef PRIX8
# define PRIX8 "X"
#endif
#if !defined PRId16 || defined PRI_MACROS_BROKEN
# undef PRId16
# define PRId16 "d"
#endif
#if !defined PRIi16 || defined PRI_MACROS_BROKEN
# undef PRIi16
# define PRIi16 "i"
#endif
#if !defined PRIo16 || defined PRI_MACROS_BROKEN
# undef PRIo16
# define PRIo16 "o"
#endif
#if !defined PRIu16 || defined PRI_MACROS_BROKEN
# undef PRIu16
# define PRIu16 "u"
#endif
#if !defined PRIx16 || defined PRI_MACROS_BROKEN
# undef PRIx16
# define PRIx16 "x"
#endif
#if !defined PRIX16 || defined PRI_MACROS_BROKEN
# undef PRIX16
# define PRIX16 "X"
#endif
#if !defined PRId32 || defined PRI_MACROS_BROKEN
# undef PRId32
# define PRId32 "d"
#endif
#if !defined PRIi32 || defined PRI_MACROS_BROKEN
# undef PRIi32
# define PRIi32 "i"
#endif
#if !defined PRIo32 || defined PRI_MACROS_BROKEN
# undef PRIo32
# define PRIo32 "o"
#endif
#if !defined PRIu32 || defined PRI_MACROS_BROKEN
# undef PRIu32
# define PRIu32 "u"
#endif
#if !defined PRIx32 || defined PRI_MACROS_BROKEN
# undef PRIx32
# define PRIx32 "x"
#endif
#if !defined PRIX32 || defined PRI_MACROS_BROKEN
# undef PRIX32
# define PRIX32 "X"
#endif
#if !defined PRId64 || defined PRI_MACROS_BROKEN
# undef PRId64
# define PRId64 (sizeof (long) == 8 ? "ld" : "lld")
#endif
#if !defined PRIi64 || defined PRI_MACROS_BROKEN
# undef PRIi64
# define PRIi64 (sizeof (long) == 8 ? "li" : "lli")
#endif
#if !defined PRIo64 || defined PRI_MACROS_BROKEN
# undef PRIo64
# define PRIo64 (sizeof (long) == 8 ? "lo" : "llo")
#endif
#if !defined PRIu64 || defined PRI_MACROS_BROKEN
# undef PRIu64
# define PRIu64 (sizeof (long) == 8 ? "lu" : "llu")
#endif
#if !defined PRIx64 || defined PRI_MACROS_BROKEN
# undef PRIx64
# define PRIx64 (sizeof (long) == 8 ? "lx" : "llx")
#endif
#if !defined PRIX64 || defined PRI_MACROS_BROKEN
# undef PRIX64
# define PRIX64 (sizeof (long) == 8 ? "lX" : "llX")
#endif
#if !defined PRIdLEAST8 || defined PRI_MACROS_BROKEN
# undef PRIdLEAST8
# define PRIdLEAST8 "d"
#endif
#if !defined PRIiLEAST8 || defined PRI_MACROS_BROKEN
# undef PRIiLEAST8
# define PRIiLEAST8 "i"
#endif
#if !defined PRIoLEAST8 || defined PRI_MACROS_BROKEN
# undef PRIoLEAST8
# define PRIoLEAST8 "o"
#endif
#if !defined PRIuLEAST8 || defined PRI_MACROS_BROKEN
# undef PRIuLEAST8
# define PRIuLEAST8 "u"
#endif
#if !defined PRIxLEAST8 || defined PRI_MACROS_BROKEN
# undef PRIxLEAST8
# define PRIxLEAST8 "x"
#endif
#if !defined PRIXLEAST8 || defined PRI_MACROS_BROKEN
# undef PRIXLEAST8
# define PRIXLEAST8 "X"
#endif
#if !defined PRIdLEAST16 || defined PRI_MACROS_BROKEN
# undef PRIdLEAST16
# define PRIdLEAST16 "d"
#endif
#if !defined PRIiLEAST16 || defined PRI_MACROS_BROKEN
# undef PRIiLEAST16
# define PRIiLEAST16 "i"
#endif
#if !defined PRIoLEAST16 || defined PRI_MACROS_BROKEN
# undef PRIoLEAST16
# define PRIoLEAST16 "o"
#endif
#if !defined PRIuLEAST16 || defined PRI_MACROS_BROKEN
# undef PRIuLEAST16
# define PRIuLEAST16 "u"
#endif
#if !defined PRIxLEAST16 || defined PRI_MACROS_BROKEN
# undef PRIxLEAST16
# define PRIxLEAST16 "x"
#endif
#if !defined PRIXLEAST16 || defined PRI_MACROS_BROKEN
# undef PRIXLEAST16
# define PRIXLEAST16 "X"
#endif
#if !defined PRIdLEAST32 || defined PRI_MACROS_BROKEN
# undef PRIdLEAST32
# define PRIdLEAST32 "d"
#endif
#if !defined PRIiLEAST32 || defined PRI_MACROS_BROKEN
# undef PRIiLEAST32
# define PRIiLEAST32 "i"
#endif
#if !defined PRIoLEAST32 || defined PRI_MACROS_BROKEN
# undef PRIoLEAST32
# define PRIoLEAST32 "o"
#endif
#if !defined PRIuLEAST32 || defined PRI_MACROS_BROKEN
# undef PRIuLEAST32
# define PRIuLEAST32 "u"
#endif
#if !defined PRIxLEAST32 || defined PRI_MACROS_BROKEN
# undef PRIxLEAST32
# define PRIxLEAST32 "x"
#endif
#if !defined PRIXLEAST32 || defined PRI_MACROS_BROKEN
# undef PRIXLEAST32
# define PRIXLEAST32 "X"
#endif
#if !defined PRIdLEAST64 || defined PRI_MACROS_BROKEN
# undef PRIdLEAST64
# define PRIdLEAST64 PRId64
#endif
#if !defined PRIiLEAST64 || defined PRI_MACROS_BROKEN
# undef PRIiLEAST64
# define PRIiLEAST64 PRIi64
#endif
#if !defined PRIoLEAST64 || defined PRI_MACROS_BROKEN
# undef PRIoLEAST64
# define PRIoLEAST64 PRIo64
#endif
#if !defined PRIuLEAST64 || defined PRI_MACROS_BROKEN
# undef PRIuLEAST64
# define PRIuLEAST64 PRIu64
#endif
#if !defined PRIxLEAST64 || defined PRI_MACROS_BROKEN
# undef PRIxLEAST64
# define PRIxLEAST64 PRIx64
#endif
#if !defined PRIXLEAST64 || defined PRI_MACROS_BROKEN
# undef PRIXLEAST64
# define PRIXLEAST64 PRIX64
#endif
#if !defined PRIdFAST8 || defined PRI_MACROS_BROKEN
# undef PRIdFAST8
# define PRIdFAST8 "d"
#endif
#if !defined PRIiFAST8 || defined PRI_MACROS_BROKEN
# undef PRIiFAST8
# define PRIiFAST8 "i"
#endif
#if !defined PRIoFAST8 || defined PRI_MACROS_BROKEN
# undef PRIoFAST8
# define PRIoFAST8 "o"
#endif
#if !defined PRIuFAST8 || defined PRI_MACROS_BROKEN
# undef PRIuFAST8
# define PRIuFAST8 "u"
#endif
#if !defined PRIxFAST8 || defined PRI_MACROS_BROKEN
# undef PRIxFAST8
# define PRIxFAST8 "x"
#endif
#if !defined PRIXFAST8 || defined PRI_MACROS_BROKEN
# undef PRIXFAST8
# define PRIXFAST8 "X"
#endif
#if !defined PRIdFAST16 || defined PRI_MACROS_BROKEN
# undef PRIdFAST16
# define PRIdFAST16 "d"
#endif
#if !defined PRIiFAST16 || defined PRI_MACROS_BROKEN
# undef PRIiFAST16
# define PRIiFAST16 "i"
#endif
#if !defined PRIoFAST16 || defined PRI_MACROS_BROKEN
# undef PRIoFAST16
# define PRIoFAST16 "o"
#endif
#if !defined PRIuFAST16 || defined PRI_MACROS_BROKEN
# undef PRIuFAST16
# define PRIuFAST16 "u"
#endif
#if !defined PRIxFAST16 || defined PRI_MACROS_BROKEN
# undef PRIxFAST16
# define PRIxFAST16 "x"
#endif
#if !defined PRIXFAST16 || defined PRI_MACROS_BROKEN
# undef PRIXFAST16
# define PRIXFAST16 "X"
#endif
#if !defined PRIdFAST32 || defined PRI_MACROS_BROKEN
# undef PRIdFAST32
# define PRIdFAST32 "d"
#endif
#if !defined PRIiFAST32 || defined PRI_MACROS_BROKEN
# undef PRIiFAST32
# define PRIiFAST32 "i"
#endif
#if !defined PRIoFAST32 || defined PRI_MACROS_BROKEN
# undef PRIoFAST32
# define PRIoFAST32 "o"
#endif
#if !defined PRIuFAST32 || defined PRI_MACROS_BROKEN
# undef PRIuFAST32
# define PRIuFAST32 "u"
#endif
#if !defined PRIxFAST32 || defined PRI_MACROS_BROKEN
# undef PRIxFAST32
# define PRIxFAST32 "x"
#endif
#if !defined PRIXFAST32 || defined PRI_MACROS_BROKEN
# undef PRIXFAST32
# define PRIXFAST32 "X"
#endif
#if !defined PRIdFAST64 || defined PRI_MACROS_BROKEN
# undef PRIdFAST64
# define PRIdFAST64 PRId64
#endif
#if !defined PRIiFAST64 || defined PRI_MACROS_BROKEN
# undef PRIiFAST64
# define PRIiFAST64 PRIi64
#endif
#if !defined PRIoFAST64 || defined PRI_MACROS_BROKEN
# undef PRIoFAST64
# define PRIoFAST64 PRIo64
#endif
#if !defined PRIuFAST64 || defined PRI_MACROS_BROKEN
# undef PRIuFAST64
# define PRIuFAST64 PRIu64
#endif
#if !defined PRIxFAST64 || defined PRI_MACROS_BROKEN
# undef PRIxFAST64
# define PRIxFAST64 PRIx64
#endif
#if !defined PRIXFAST64 || defined PRI_MACROS_BROKEN
# undef PRIXFAST64
# define PRIXFAST64 PRIX64
#endif
#if !defined PRIdMAX || defined PRI_MACROS_BROKEN
# undef PRIdMAX
# define PRIdMAX (sizeof (uintmax_t) == sizeof (long) ? "ld" : "lld")
#endif
#if !defined PRIiMAX || defined PRI_MACROS_BROKEN
# undef PRIiMAX
# define PRIiMAX (sizeof (uintmax_t) == sizeof (long) ? "li" : "lli")
#endif
#if !defined PRIoMAX || defined PRI_MACROS_BROKEN
# undef PRIoMAX
# define PRIoMAX (sizeof (uintmax_t) == sizeof (long) ? "lo" : "llo")
#endif
#if !defined PRIuMAX || defined PRI_MACROS_BROKEN
# undef PRIuMAX
# define PRIuMAX (sizeof (uintmax_t) == sizeof (long) ? "lu" : "llu")
#endif
#if !defined PRIxMAX || defined PRI_MACROS_BROKEN
# undef PRIxMAX
# define PRIxMAX (sizeof (uintmax_t) == sizeof (long) ? "lx" : "llx")
#endif
#if !defined PRIXMAX || defined PRI_MACROS_BROKEN
# undef PRIXMAX
# define PRIXMAX (sizeof (uintmax_t) == sizeof (long) ? "lX" : "llX")
#endif
#if !defined PRIdPTR || defined PRI_MACROS_BROKEN
# undef PRIdPTR
# define PRIdPTR \
  (sizeof (void *) == sizeof (long) ? "ld" : \
   sizeof (void *) == sizeof (int) ? "d" : \
   "lld")
#endif
#if !defined PRIiPTR || defined PRI_MACROS_BROKEN
# undef PRIiPTR
# define PRIiPTR \
  (sizeof (void *) == sizeof (long) ? "li" : \
   sizeof (void *) == sizeof (int) ? "i" : \
   "lli")
#endif
#if !defined PRIoPTR || defined PRI_MACROS_BROKEN
# undef PRIoPTR
# define PRIoPTR \
  (sizeof (void *) == sizeof (long) ? "lo" : \
   sizeof (void *) == sizeof (int) ? "o" : \
   "llo")
#endif
#if !defined PRIuPTR || defined PRI_MACROS_BROKEN
# undef PRIuPTR
# define PRIuPTR \
  (sizeof (void *) == sizeof (long) ? "lu" : \
   sizeof (void *) == sizeof (int) ? "u" : \
   "llu")
#endif
#if !defined PRIxPTR || defined PRI_MACROS_BROKEN
# undef PRIxPTR
# define PRIxPTR \
  (sizeof (void *) == sizeof (long) ? "lx" : \
   sizeof (void *) == sizeof (int) ? "x" : \
   "llx")
#endif
#if !defined PRIXPTR || defined PRI_MACROS_BROKEN
# undef PRIXPTR
# define PRIXPTR \
  (sizeof (void *) == sizeof (long) ? "lX" : \
   sizeof (void *) == sizeof (int) ? "X" : \
   "llX")
#endif

/* Expand a system dependent string segment.  Return NULL if unsupported.  */
static const char *get_sysdep_segment_value(const char *name)
{
	/* Test for an ISO C 99 section 7.8.1 format string directive.
	   Syntax:
	   P R I { d | i | o | u | x | X }
	   { { | LEAST | FAST } { 8 | 16 | 32 | 64 } | MAX | PTR }  */
	/* We don't use a table of 14 times 6 'const char *' strings here, because
	   data relocations cost startup time.  */
	if (name[0] == 'P' && name[1] == 'R' && name[2] == 'I')
	{
		if (name[3] == 'd' || name[3] == 'i' || name[3] == 'o' || name[3] == 'u' || name[3] == 'x' || name[3] == 'X')
		{
			if (name[4] == '8' && name[5] == '\0')
			{
				if (name[3] == 'd')
					return PRId8;
				if (name[3] == 'i')
					return PRIi8;
				if (name[3] == 'o')
					return PRIo8;
				if (name[3] == 'u')
					return PRIu8;
				if (name[3] == 'x')
					return PRIx8;
				if (name[3] == 'X')
					return PRIX8;
				abort();
			}
			if (name[4] == '1' && name[5] == '6' && name[6] == '\0')
			{
				if (name[3] == 'd')
					return PRId16;
				if (name[3] == 'i')
					return PRIi16;
				if (name[3] == 'o')
					return PRIo16;
				if (name[3] == 'u')
					return PRIu16;
				if (name[3] == 'x')
					return PRIx16;
				if (name[3] == 'X')
					return PRIX16;
				abort();
			}
			if (name[4] == '3' && name[5] == '2' && name[6] == '\0')
			{
				if (name[3] == 'd')
					return PRId32;
				if (name[3] == 'i')
					return PRIi32;
				if (name[3] == 'o')
					return PRIo32;
				if (name[3] == 'u')
					return PRIu32;
				if (name[3] == 'x')
					return PRIx32;
				if (name[3] == 'X')
					return PRIX32;
				abort();
			}
			if (name[4] == '6' && name[5] == '4' && name[6] == '\0')
			{
				if (name[3] == 'd')
					return PRId64;
				if (name[3] == 'i')
					return PRIi64;
				if (name[3] == 'o')
					return PRIo64;
				if (name[3] == 'u')
					return PRIu64;
				if (name[3] == 'x')
					return PRIx64;
				if (name[3] == 'X')
					return PRIX64;
				abort();
			}
			if (name[4] == 'L' && name[5] == 'E' && name[6] == 'A' && name[7] == 'S' && name[8] == 'T')
			{
				if (name[9] == '8' && name[10] == '\0')
				{
					if (name[3] == 'd')
						return PRIdLEAST8;
					if (name[3] == 'i')
						return PRIiLEAST8;
					if (name[3] == 'o')
						return PRIoLEAST8;
					if (name[3] == 'u')
						return PRIuLEAST8;
					if (name[3] == 'x')
						return PRIxLEAST8;
					if (name[3] == 'X')
						return PRIXLEAST8;
					abort();
				}
				if (name[9] == '1' && name[10] == '6' && name[11] == '\0')
				{
					if (name[3] == 'd')
						return PRIdLEAST16;
					if (name[3] == 'i')
						return PRIiLEAST16;
					if (name[3] == 'o')
						return PRIoLEAST16;
					if (name[3] == 'u')
						return PRIuLEAST16;
					if (name[3] == 'x')
						return PRIxLEAST16;
					if (name[3] == 'X')
						return PRIXLEAST16;
					abort();
				}
				if (name[9] == '3' && name[10] == '2' && name[11] == '\0')
				{
					if (name[3] == 'd')
						return PRIdLEAST32;
					if (name[3] == 'i')
						return PRIiLEAST32;
					if (name[3] == 'o')
						return PRIoLEAST32;
					if (name[3] == 'u')
						return PRIuLEAST32;
					if (name[3] == 'x')
						return PRIxLEAST32;
					if (name[3] == 'X')
						return PRIXLEAST32;
					abort();
				}
				if (name[9] == '6' && name[10] == '4' && name[11] == '\0')
				{
					if (name[3] == 'd')
						return PRIdLEAST64;
					if (name[3] == 'i')
						return PRIiLEAST64;
					if (name[3] == 'o')
						return PRIoLEAST64;
					if (name[3] == 'u')
						return PRIuLEAST64;
					if (name[3] == 'x')
						return PRIxLEAST64;
					if (name[3] == 'X')
						return PRIXLEAST64;
					abort();
				}
			}
			if (name[4] == 'F' && name[5] == 'A' && name[6] == 'S' && name[7] == 'T')
			{
				if (name[8] == '8' && name[9] == '\0')
				{
					if (name[3] == 'd')
						return PRIdFAST8;
					if (name[3] == 'i')
						return PRIiFAST8;
					if (name[3] == 'o')
						return PRIoFAST8;
					if (name[3] == 'u')
						return PRIuFAST8;
					if (name[3] == 'x')
						return PRIxFAST8;
					if (name[3] == 'X')
						return PRIXFAST8;
					abort();
				}
				if (name[8] == '1' && name[9] == '6' && name[10] == '\0')
				{
					if (name[3] == 'd')
						return PRIdFAST16;
					if (name[3] == 'i')
						return PRIiFAST16;
					if (name[3] == 'o')
						return PRIoFAST16;
					if (name[3] == 'u')
						return PRIuFAST16;
					if (name[3] == 'x')
						return PRIxFAST16;
					if (name[3] == 'X')
						return PRIXFAST16;
					abort();
				}
				if (name[8] == '3' && name[9] == '2' && name[10] == '\0')
				{
					if (name[3] == 'd')
						return PRIdFAST32;
					if (name[3] == 'i')
						return PRIiFAST32;
					if (name[3] == 'o')
						return PRIoFAST32;
					if (name[3] == 'u')
						return PRIuFAST32;
					if (name[3] == 'x')
						return PRIxFAST32;
					if (name[3] == 'X')
						return PRIXFAST32;
					abort();
				}
				if (name[8] == '6' && name[9] == '4' && name[10] == '\0')
				{
					if (name[3] == 'd')
						return PRIdFAST64;
					if (name[3] == 'i')
						return PRIiFAST64;
					if (name[3] == 'o')
						return PRIoFAST64;
					if (name[3] == 'u')
						return PRIuFAST64;
					if (name[3] == 'x')
						return PRIxFAST64;
					if (name[3] == 'X')
						return PRIXFAST64;
					abort();
				}
			}
			if (name[4] == 'M' && name[5] == 'A' && name[6] == 'X' && name[7] == '\0')
			{
				if (name[3] == 'd')
					return PRIdMAX;
				if (name[3] == 'i')
					return PRIiMAX;
				if (name[3] == 'o')
					return PRIoMAX;
				if (name[3] == 'u')
					return PRIuMAX;
				if (name[3] == 'x')
					return PRIxMAX;
				if (name[3] == 'X')
					return PRIXMAX;
				abort();
			}
			if (name[4] == 'P' && name[5] == 'T' && name[6] == 'R' && name[7] == '\0')
			{
				if (name[3] == 'd')
					return PRIdPTR;
				if (name[3] == 'i')
					return PRIiPTR;
				if (name[3] == 'o')
					return PRIoPTR;
				if (name[3] == 'u')
					return PRIuPTR;
				if (name[3] == 'x')
					return PRIxPTR;
				if (name[3] == 'X')
					return PRIXPTR;
				abort();
			}
		}
	}
	/* Test for a glibc specific printf() format directive flag.  */
	if (name[0] == 'I' && name[1] == '\0')
	{
#if defined _LIBC || defined _MINT_SOURCE || ((defined(__GLIBC__) && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 2))) && !defined __UCLIBC__)
		/* The 'I' flag, in numeric format directives, replaces ASCII digits
		   with the 'outdigits' defined in the LC_CTYPE locale facet.  This is
		   used for Farsi (Persian), some Indic languages, and maybe Arabic.  */
		return "I";
#else
		return "";
#endif
	}
	/* Other system dependent strings are not valid.  */
	return NULL;
}

