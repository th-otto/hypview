#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#undef HAVE_GLIB
#undef HAVE_GTK

#include "hypdefs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct flist
{
	char *file;
	char *name;
	char *title;
	int flags;
	struct flist *next;
} FLIST;

typedef struct
{
	unsigned version;
	unsigned headlen;
	unsigned nplanes;
	unsigned patlen;
	unsigned pixelw;
	unsigned pixelh;
	unsigned linew;
	unsigned lines;
	unsigned palette[16];
} IMGHEADER;

static FLIST *FBase = NULL;					/* Base of the file-list */
static FILE *inhandle;						/* Handle of the current input-file */
static char inbuf[4096];					/* File-buffer for input */
static char *inptr;
static long inlen;
static char inpath[128];					/* Path of the input-file */
static char mainpath[128];					/* Path of the main file */
static FILE *outhandle;						/* Handle of the output-file */
static char outbuf[4096];					/* File-buffer for output */
static char *outptr;
static unsigned int maxlines;
static unsigned int Lineno;
static char output[128];					/* Name of the output-file */
static int ExtCnt;
static int NeedNL;							/* Output NL before warning */
static int use_filenames;					/* Use file-name as node-name */
static int code;
static int quiet;
static int NoAuto;
static char *CurrFile;

#define suffix(s) (strrchr(s, '.'))

/*
 * Report errors and terminate
 */
static void Message(const char *str1, const char *str2)
{
	fputs(str1, stderr);
	if (str2)
		fputs(str2, stderr);
	fputs("\n", stderr);
}


static void error(const char *s1, const char *s2)
{
	Message(s1, s2);
	if (outhandle)
		fclose(outhandle);
	if (inhandle)
		fclose(inhandle);
	exit(EXIT_FAILURE);
}


/*
 * Open output-file
 */
static void Wopen(const char *outfile)
{
	if ((outhandle = fopen(outfile, "w")) == NULL)
	{
		fprintf(stderr, _("can't open %s"), outfile);
		error("", NULL);
	}
	outptr = outbuf;
	Lineno = 0;
}


static void outputs(const char *line)
{
	char *p = outptr;

	while (*line)
	{
		if (*line == '\n')
			++Lineno;
		if ((p - outbuf) == sizeof(outbuf))
		{
			if (fwrite(outbuf, sizeof(outbuf), 1, outhandle) != 1)
				error(_("write error"), NULL);
			p = outptr = outbuf;
		}
		*p++ = *line++;
	}
	outptr = p;
}


/*
 * Convert visible cross-reference
 */
static int ConvKey(char *key)
{
	char buf[100];
	const char *src = key;
	char *dst = buf;
	int rv = 1;

	if (*src == '@' || NoAuto)
		rv = 0;							/* Force link */

	while (*src)
	{
		if (*src == '"' || *src == '\\')
		{
			*dst++ = '\\';
			rv = 0;
		}
		*dst++ = *src++;
	}
	*dst = 0;
	strcpy(key, buf);
	return rv;
}


/*
 * Close file, and if necessary flush buffer
 */
static void xclose(FILE *handle)
{
	size_t len;

	if (handle == outhandle)
	{
		len = outptr - outbuf;
		if (len)
		{
			if (fwrite(outbuf, len, 1, handle) != 1)
				error(_("write error"), NULL);
		}
	}

	fclose(handle);
}


static void replaceext(char *buf, const char *ext)
{
	char *p = strrchr(buf, '.');

	if (p == NULL)
		p = buf + strlen(buf);
	strcpy(p, ext);
}


/*
 * Create page end
 * If file is too long, create a new one
 */
static void EndNode(void)
{
	char file[128];

	outputs("@endnode\n\n\n");
	xclose(inhandle);					/* Close input */

	if (maxlines && Lineno > maxlines)
	{
		strcpy(file, output);
		++ExtCnt;
		sprintf(suffix(file) + 1, "%d", ExtCnt);
		xclose(outhandle);
		Wopen(file);
	}
}


static void ImagePage(FLIST *p)
{
	IMGHEADER *img = (IMGHEADER *) inbuf;
	char *str = inbuf + 100;
	int cnt;

	fread(img, sizeof(IMGHEADER), 1, inhandle);
	cnt = (img->lines + 15) / HYP_PIC_FONTH;
	outputs("@image ");
	outputs(p->file);
	outputs(" 1 ");
	sprintf(str, "%ld", (long) ((img->linew + 7) / HYP_PIC_FONTW));	/* Width in characters */
	outputs(str);
	outputs(" ");
	sprintf(str, "%d", cnt);			/* Height in characters */
	outputs(str);
	outputs("\n");

	EndNode();
}


/*
 * Open file for reading
 */
static int Ropen(void)
{
	FLIST *p;
	const char *suf;
	char tmp[150];

  again:
	p = FBase;
	while (p)
	{
		if (p->flags == 0)
			break;						/* Find first, that has */
		p = p->next;					/* not been read yet */
	}
	if (p == NULL)
		return 0;						/* End of list reached */
	p->flags = 1;						/* Mark as read */

	strcpy(inpath, p->file);			/* Save input path */
	suf = hyp_basename(inpath);
	inpath[suf - inpath] = '\0';
	if (*inpath == 0)
		strcpy(inpath, mainpath);

	if ((inhandle = fopen(p->file, "rb")) == NULL)	/* Open file */
	{
		fprintf(stderr, _("can't open %s"), p->file);
		error("", NULL);
	}		

	CurrFile = p->file;
	if (quiet == 0)
	{
		fputs(_("reading "), stderr);
		fputs(p->file, stderr);
		fflush(stderr);
		NeedNL = 1;
	}

	if (p == FBase)
	{									/* First file? */
		outputs("@options \"+x -s");
		if (NoAuto)
			outputs("a");
		outputs("\"\n\n");
	}

	outputs("@node \"");				/* Name of the page */
	strcpy(tmp, p->name);
	ConvKey(tmp);
	outputs(tmp);
	outputs("\"");
	if (p->title)
	{									/* Possible title */
		outputs(" \"");
		strcpy(tmp, p->title);
		ConvKey(tmp);
		outputs(tmp);
		outputs("\"");
	}
	outputs("\n");

	inlen = -1;
	inptr = inbuf;						/* Initialise read-pointer */

	/*
	 * If it is an image that HCP can read,
	 * then we will incorporate it.
	 */
	suf = suffix(p->file);
	if (suf != NULL && g_ascii_strcasecmp(suf, ".img") == 0)
	{
		ImagePage(p);
		goto again;
	}

	return 1;							/* OK */
}


static int ingets(char *line)
{
	char *p = inptr;
	int rv = 1;
	int cnt = 0;

	do
	{
		if (inlen <= 0)
		{
			inlen = fread(inbuf, 1, sizeof(inbuf), inhandle);
			if (inlen < 0)				/* Error? */
				error(_("read error"), NULL);

			if (inlen == 0)
			{							/* Nothing read? */
				rv = 0;					/* File-end */
				break;
			}
			inptr = p = inbuf;
		}
		if (*p == 10)
		{								/* NL */
			--inlen;
			++p;
			break;
		}

		if (*p != 13)					/* Don't copy CR */
		{
			*line++ = *p;
			++cnt;
		}

		--inlen;						/* One less */
		++p;
	} while (cnt < 255);

	*line = 0;							/* Close line */
	inptr = p;							/* Position of next character */
	return rv || cnt;					/* and finished */
}


/*
 > Read INF-file for current 1stGuide-text
 */
static void OpenINF(const char *path)
{
	char line[128];

	strcpy(line, path);
	replaceext(line, ".prj");
	if ((inhandle = fopen(line, "rb")) != NULL)
	{
		inlen = -1;
		inptr = inbuf;					/* Initialise read-pointer */
		while (ingets(line))
		{
			outputs(line);
			outputs("\n");
		}
		xclose(inhandle);
	}
}


/*******************************************************************/

static int Find(const char *key)
{
	FLIST *p;

	p = FBase;
	while (p)
	{
		if (strcmp(p->name, key) == 0)
			return 0;
		p = p->next;
	}
	return 1;
}


static char *GetName(const char *name, const char *key)
{
	char buf[20];
	const char *rv;

	/*
	 * If desired, then always use file-name as
	 * the page-name
	 */
	if (use_filenames)
	{
		rv = name;
		goto end;
	}

	if (Find(key))
	{									/* Search for title */
		rv = key;
	} else
	{
		strcpy(buf, hyp_basename(name));
		replaceext(buf, "");
		if (Find(buf))					/* File without extension */
		{
			rv = buf;
		} else
		{
			strcpy(buf, hyp_basename(name));
			if (Find(buf))				/* File with extension */
				rv = buf;
			else
			{
				rv = name;
			}
		}
	}
  end:
	return g_strdup(rv);
}


#if 0
static char *GetName(const char *name, const char *key, int flg)
{
	FLIST *p;
	char buf[20];
	char *new;
	int flag;

	flag = 0;
	new = key;							/* First we test the keyword */

  again:
	p = FBase;
	while (p)
	{
		if (strcmp(p->name, new) == 0)
		{
			if (flag == 2)				/* Test for file-name? */
				return NULL;			/* Yes: It all won't work */
		  chk:
			++flag;						/* Was test for keyword */
			new = buf;					/* Now we try the file-name */
			strcpy(new, hyp_basename(name));
			if (flag == 1)
				replaceext(new, "");
			goto again;
		}
		p = p->next;
	}
	if (flg == 0)
	{
		flg = 1;
		goto chk;
	}
	return g_strdup(new);				/* Return found node-name */
}
#endif


/*
 * Append new file-name to the list of
 * files to be read. if it doesn't exist
 * already
 */
static FLIST *AddFile(const char *name, const char *key)
{
	FLIST *p = FBase;
	FLIST *last = p;

	while (p)
	{
		if (strcmp(p->file, name) == 0)
			return p;
		last = p;
		p = p->next;
	}
	p = g_new(FLIST, 1);
	if (p == NULL)						/* Error */
		error(_("out of memory"), NULL);
	p->name = GetName(name, key);
	p->file = g_strdup(name);			/* Copy name */
	if (strcmp(p->name, key))
	{									/* When nodename != key */
		p->title = g_strdup(key);		/* dthen use key as the title */
	} else
	{
		p->title = NULL;
	}
	p->flags = 0;						/* Not yet read */
	p->next = NULL;						/* Always the last */

	if (last)
	{
		last->next = p;
	} else
	{									/* First element */
		FBase = p;
	}
	return p;
}

/**********************************************************/

#define IS_ASCII	1
#define NO_TYPE 	2

static int FileType(char *file, char *key)
{
	char path[128];
	char *p;
	char *save;
	FILE *h;
	FLIST *q;

	/*
	 * Convert file-name to capitals
	 * and complete the path
	 */
	strcpy(path, inpath);
	strcat(path, file);

	/*
	 * The path could contain '...' portions;
	 * for comparing names we will delete
	 * these portions
	 */
  again:
	p = path;
	save = p;
	while (*p)
	{
		if (G_IS_DIR_SEPARATOR(*p))
		{
			if (p[1] == '.' && p[2] == '.')
			{
				int skip = G_IS_DIR_SEPARATOR(*save) ? 3 : 4;

				strcpy(save, p + skip);
				goto again;
			}
			save = p;					/* Last slash */
		}
		++p;
	}

	/*
	 * So that the file doesn't become too large, we
	 * will now shorten the path until it is relative
	 * to the main file
	 */
	save = mainpath;
	p = path;
	while (*p == *save)
	{
		++p;
		++save;
	}

	strcpy(file, p);
	p = suffix(file);

	/*
	 * Let's see if we are dealing with one of
	 * the file-types supported by 1stView...
	 * If so, then the compiler can do nothing
	 * with it (not an ASCII-file) and we return
	 * NO_TYPE
	 */
	if (p != NULL)
	{
		if (g_ascii_strcasecmp(p, ".rsc") == 0 ||
			g_ascii_strcasecmp(p, ".gem") == 0 ||
			g_ascii_strcasecmp(p, ".sam") == 0 ||
			g_ascii_strcasecmp(p, ".snd") == 0 ||
			g_ascii_strcasecmp(p, ".out") == 0 ||
			g_ascii_strcasecmp(p, ".dok") == 0 ||
			g_ascii_strcasecmp(p, ".img") == 0 ||
			g_ascii_strcasecmp(p, ".iff") == 0)
			return NO_TYPE;
	}
	
	/*
	 * ...then it's no doubt an ASCII-file;
	 * in that case we will want to read it later,
	 * so it will be placed in the list, if it
	 * can be opened
	 */
	if ((h = fopen(file, "rb")) != NULL)
	{
		fclose(h);
	} else
	{
		if (NeedNL)
		{
			fputs("\n", stderr);
			NeedNL = 0;
		}
		fprintf(stderr, _("*** warning in '%s': missing file %s\n"), CurrFile, file);
		code = 2;
		return NO_TYPE;
	}

	q = AddFile(file, key);
	strcpy(file, q->name);
	return IS_ASCII;
}


static int HasBlanks(char *p)
{
	while (*p)
	{
		if (*p == ' ')
			return 1;
		++p;
	}
	return 0;
}


static void DoList(void)
{
	char line[255];
	char buf[512];
	char file[128];
	char key[100];
	char *src;
	char *dst;
	const char *p;
	char *pkey;
	char *save;

	int cnt;

	while (Ropen())
	{									/* Over all files */
	  again:
		while (ingets(line))
		{								/* All lines */
			src = line;
			dst = buf;

			/*
			 * If the line is an Info-block, then we
			 * simply ignore it
			 */
			if (*src == 0x1F)
				goto again;

			/*
			 * If there is a comment character at the start,
			 * we will simply indent it. so that the compiler
			 * will ignore it
			 */
			if (*src == '#' && src[1] == '#')
				*dst++ = ' ';

			while (*src)
			{							/* All characters */
				switch ((unsigned char)*src)
				{
				case 0x1C:				/* Stretch-space character */
				case 0x1D:				/* Indent-space character */
				case 0x1E:				/* Variable-space character */
					*dst++ = ' ';
					++src;
					break;
				case 27:
					++src;
					if ((unsigned char)*src >= 0x80)
					{
						*dst++ = '@';
						*dst++ = '{';
						*src -= 0x80;
						if (*src == 0)
							*dst++ = '0';	/* All off */
						if (*src & 1)
							*dst++ = 'B';	/* Bold */
						if (*src & 2)
							*dst++ = 'G';	/* Light/grey */
						if (*src & 4)
							*dst++ = 'I';	/* Italic */
						if (*src & 8)
							*dst++ = 'U';	/* Underlined */
						*dst++ = '}';
					} else
					{
						*dst++ = 27;
						*dst++ = *src;
					}
					++src;
					break;
				case '@':
					*dst++ = '@';
					/* fall thru */
				default:
					*dst++ = *src++;
					break;
				case 0xdd:
					pkey = key;
					cnt = 0;
					save = dst;
					do
					{
						*dst++ = *src++;
						if ((unsigned char)*src == 0xdd)
						{
							*pkey = 0;
							if (cnt == 1)
							{
								++src;
								break;
							} else
							{
								pkey = file;
								*dst++ = *src++;
							}
							++cnt;
						}
						if (*src == 0)
							goto more;
						*pkey++ = *src;
					} while (1);

					dst = save;
					if (FileType(file, key) != IS_ASCII)
					{
						p = "\\Main";
					} else
					{
						p = NULL;
						cnt = ConvKey(key);
						/*
						 * If the auto-referencer finds this
						 * place on its own, we let it do so
						 */
						if (cnt && *src < '0' && dst[-1] < '0' && (HasBlanks(file) == 0) && strcmp(key, file) == 0)
						{
							strcpy(dst, key);
							while (*dst++)
								;
							--dst;
							break;
						}
					}
					*dst++ = '@';
					*dst++ = '{';
					*dst++ = '"';
					strcpy(dst, key);
					while (*dst++)
						;
					--dst;
					strcpy(dst, "\" link ");
					dst += 7;
					*dst++ = '"';
					strcpy(dst, file);
					while (*dst++)
						;
					--dst;
					if (p)
					{
						strcpy(dst, p);
						while (*dst++)
							;
						--dst;
					}
					*dst++ = '"';
					*dst++ = '}';
					break;
				}
			}
		  more:
			*dst = 0;
			outputs(buf);
			outputs("\n");
		}
		EndNode();
	}
}


static void AddIncludes(void)
{
	char buf[128];
	int i;

	outptr = outbuf;

	for (i = 1; i <= ExtCnt; i++)
	{
		strcpy(buf, output);
		sprintf(suffix(buf) + 1, "%d", i);
		outputs("\n@include ");
		outputs(buf);
	}
	outputs("\n");
}



/**************************************************************/

#include "hypmain.h"

int main(int argc, const char **argv)
{
	char outfile[128];
	const char *p;

	maxlines = 0;

	if (argc < 2)
	{
		fputs(_("usage: 1stConv [-NNN -f -q -a] file\n"
			    "       -NNN: max lines per file\n"
			    "       -f  : use filenames as nodenames\n"
			    "       -q  : be quiet\n"
			    "       -a  : explicit links only\n"), stderr);
		exit(EXIT_FAILURE);
	}

	use_filenames = 0;
	do
	{
		++argv;
		--argc;
		if (**argv == '-')
		{
			p = *argv + 1;
			if (*p == 'f')
			{
				use_filenames = 1;
			} else if (*p == 'q')
			{
				++quiet;
			} else if (*p == 'a')
			{
				++NoAuto;
			} else
			{
				maxlines = atoi(p);
			}
		} else
		{
			if (quiet == 0)
			{
				/* FIXME: compiler version is a lie */
				Message("1stConv: V(" __DATE__ ") 1stGuide --> ST-Guide sources\n"
						"         Written by Holger Weets using SOZOBON-C V2.00x10\n", NULL);
			}

			/*
			 * Determine output-file
			 */
			strcpy(outfile, *argv);
			AddFile(hyp_basename(outfile), "Main");	/* Enter file-name only */
			strcpy(mainpath, outfile);		/* Save path */
			p = hyp_basename(mainpath);
			mainpath[p - mainpath] = '\0';
			
			replaceext(outfile, ".stg");
			Wopen(outfile);
			OpenINF(outfile);
			strcpy(output, outfile);
			ExtCnt = 0;
			DoList();

			if (ExtCnt)
				AddIncludes();

			xclose(outhandle);

			if (NeedNL)
				fputs("\n", stderr);
			break;
		}
	} while (argc > 1);
	return code;
}

/*** ---------------------------------------------------------------------- ***/

const char *hyp_basename(const char *path)
{
	const char *p;
	const char *base = NULL;
	
	if (path == NULL)
		return path;
	p = path;
	while (*p != '\0')
	{
		if (G_IS_DIR_SEPARATOR(*p))
			base = p + 1;
		++p;
	}
	if (base != NULL)
		return base;
	
	if (isalpha(path[0]) && path[1] == ':')
	{
    	/* can only be X:name, without slash */
    	path += 2;
	}
	
	return path;
}

/*** ---------------------------------------------------------------------- ***/

#define ISSPACE(c)              ((c) == ' ' || (c) == '\f' || (c) == '\n' || \
                                 (c) == '\r' || (c) == '\t' || (c) == '\v')
#define ISUPPER(c)              ((c) >= 'A' && (c) <= 'Z')
#define ISLOWER(c)              ((c) >= 'a' && (c) <= 'z')
#define ISALPHA(c)              (ISUPPER (c) || ISLOWER (c))
#define TOUPPER(c)              (ISLOWER (c) ? (c) - 'a' + 'A' : (c))
#define TOLOWER(c)              (ISUPPER (c) ? (c) - 'A' + 'a' : (c))

int g_ascii_strcasecmp(const char *s1, const char *s2)
{
	int c1, c2;

	while (*s1 && *s2)
    {
		c1 = (int)(unsigned char) TOLOWER (*s1);
		c2 = (int)(unsigned char) TOLOWER (*s2);
		if (c1 != c2)
			return c1 - c2;
		s1++; s2++;
	}

	return (((int)(unsigned char) *s1) - ((int)(unsigned char) *s2));
}

/*** ---------------------------------------------------------------------- ***/

#ifndef HAVE_GLIB
#ifdef ENABLE_NLS
#ifndef g_strdup
char *g_strdup(const char *str)
{
	char *dst;
	
	if (str == NULL)
		return NULL;
	dst = g_new(char, strlen(str) + 1);
	if (dst == NULL)
		return NULL;
	return strcpy(dst, str);
}
#endif
#endif
#endif

#include "../../hyp/hyp_intl.c"
