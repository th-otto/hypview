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

static FILE *inhandle;					/* Handle of the current input-file */
static char inbuf[8192];				/* File-buffer for input */
static char *inptr;
static long inlen;
static FILE *outhandle;					/* Handle of the output-file */
static char outbuf[4096];				/* File-buffer for output */
static char *outptr;
static int Autoref = 1;					/* Let autoreferencer do it */
static int NewFormat;					/* 1: Recompiled with HELPDISC */
static unsigned int maxlines = 0;
static int HasExternals;				/* Number of external references */
static unsigned int Line;				/* Line number of output */
static unsigned int total;				/* Number of output lines */
static int ExtCnt;						/* Counter for extensions */
static long Syms;						/* Number of nodes/alabels */
static char outfile[128];				/* Name of original output-file */
static const char *ExtPage;

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
	fputs("\n", stderr);
	Message(s1, s2);
	if (outhandle != NULL)
		fclose(outhandle);
	if (inhandle != NULL)
		fclose(inhandle);
	exit(EXIT_FAILURE);
}


/*
 * Open output file
 */
static void Wopen(const char *outfile)
{
	if ((outhandle = fopen(outfile, "w")) == NULL)
		error(_("can't open outfile"), NULL);
	outptr = outbuf;
	Line = 0;
}


static void outputs(const char *line)
{
	char *p = outptr;

	while (*line)
	{
		if (*line == '\n')
			++Line;
		if ((p - outbuf) == sizeof(outbuf))
		{
			if (fwrite(outbuf, sizeof(outbuf), 1, outhandle) != 1)
				error(_("Write error"), NULL);
			p = outptr = outbuf;
		}
		*p++ = *line++;
	}
	outptr = p;
}


/*
 * Open file for reading
 */
static void Ropen(const char *file)
{
	if ((inhandle = fopen(file, "r")) == NULL)	/* Open file */
		error(_("can't open "), file);
}


static void outheader(void)
{
	outputs("@options -s");				/* No line-wrap */
	if (!Autoref)
		outputs("a");					/* No autoreferencer */
	if (NewFormat)
		outputs("n");					/* "PageN" not in Index */
	outputs("\n\n");
	outputs("@node Main\n"
			" This text has been converted automatically into the\n"
			" ST-Guide format. This page should actually contain\n" " an Index... \n" "@endnode\n");
#if 0
	outputs("@node Main\n"
			" Dieser Text wurde automatisch in das ST-Guide Format\n"
			" konvertiert. Auf dieser Seite sollte eigentlich ein\n" " Inhaltsverzeichnis stehen...\n" "@endnode\n");
#endif

	inlen = -1;
	inptr = inbuf;						/* Initialise read pointer */
}


/*
 * Close file and flush buffer if necessary
 */
static void xclose(FILE *handle)
{
	size_t len;

	if (handle == outhandle)
	{
		if (outptr != outbuf)
		{
			len = outptr - outbuf;
			if (fwrite(outbuf, len, 1, handle) != 1)
				error(_("write error"), NULL);
		}
	}

	fclose(handle);
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
			fputc('.', stderr);
			if (inlen < 0)				/* Error? */
				error(_("read error"), NULL);

			if (inlen == 0)
			{							/* Nothing read? */
				rv = 0;					/* End of file */
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

		if (*p != 13)
		{								/* Don't copy CR */
			*line++ = *p;
			++cnt;
		}

		--inlen;						/* One less */
		++p;
	} while (cnt < 255);

	*line = 0;							/* Close line */
	inptr = p;							/* Position of next character */
	return (rv || cnt);					/* and finish */
}


static void KonvKey(char *buf)
{
	if (strcmp(buf, "Copyright") == 0)
		strcpy(buf, "Help");
	else if (strcmp(buf, "Help") == 0)
		strcpy(buf, "Help 2");
	else if (strcmp(buf, "Main") == 0)
		strcpy(buf, "main 2");
}


static const char *GetName(const char *src, char *buf)
{
	char *dst = buf;

	while (*src && *src != '"')
		++src;							/* Start <node> */
	++src;
	do
	{
		*dst++ = *src++;
		if (dst[-1] == '\\')			/* Masked character */
			*dst++ = *src++;
	} while (*src && *src != '"');		/* Ende <node> */
	*dst = 0;
	KonvKey(buf);
	return ++src;
}


static void NewPage(char *line)
{
	char buf[255];
	const char *src = line;
	int cnt;

	src = GetName(src + 7, buf);

	if (!NewFormat && strcmp(buf, "Index") == 0)
	{									/* This index is */
		while (ingets(line))
		{								/* nonsense */
			if (strncmp(line, "\\end", 4) == 0)
				break;
		}
		return;
	}
	outputs("@node \"");
	outputs(buf);
	outputs("\"");
	if (NewFormat)
		cnt = 0;
	else
	{
		outputs("\n");
		cnt = 1;
	}

  again:
	++Syms;
	while (*src)
	{
		if (*src == ',')
		{
			if (NewFormat == 0)
			{
				ingets(line);
				src = line;
			}
			src = GetName(src, buf);
			if (cnt++ == 0)
			{
				outputs(" \"");
				outputs(buf);
				outputs("\"\n");
			}
			outputs("@symbol \"");
			outputs(buf);
			outputs("\"\n");
			goto again;
		}
		++src;
	}
	if (cnt == 0)
		outputs("\n");
}


static char *MakeLink(char *dst, const char *node, const char *key)
{
	strcpy(dst, "@{\"");
	strcat(dst, node);
	strcat(dst, "\" link \"");
	strcat(dst, key);
	strcat(dst, "\"}");
	while (*dst++)
		;
	return --dst;
}


static void EndNode(void)
{
	char buf[128];

	outputs("@endnode\n\n");
	if (maxlines && Line > maxlines)
	{
		strcpy(buf, outfile);
		ExtCnt++;
		sprintf(suffix(buf) + 1, "%d", ExtCnt);
		xclose(outhandle);
		total += Line;
		Wopen(buf);
		fputs("\n", stderr);
		fputs(" --> ", stderr);
		fputs(buf, stderr);
		fputs("\n", stderr);
	}
}


static void ConvLine(char *buf)
{
	char line[255], alias[80];
	char *src = buf;
	char *dst = line;
	const char *key;

	int EndFlag = 0;

	while (*src)
	{
		if (*src == '\\')
		{
			if (src[1] == '\\')
			{
				*dst++ = *src++;
				goto copy;
			}
			if (src[1] == '#')
			{
				src += 2;
				key = src;
				while (*src)
				{
					if (*src == '\\' && src[1] == '#')
						break;
					++src;
				}
				if (*src == 0)
				{
					Message("\n-> ", buf);
					error(_("syntax error."), NULL);
				}
				*src = 0;
				strcpy(alias, key);
				KonvKey(alias);
				if (strcmp(key, alias) == 0 && Autoref)
				{
					/*
					 * We let the auto-referencer make the link
					 */
					strcpy(dst, alias);
					while (*dst++)
						;
					--dst;
				} else
				{
					dst = MakeLink(dst, key, alias);
				}
				*src = '\\';
				src += 2;
			} else if (strncmp(src + 1, "link", 4) == 0)
			{
				/*
				 * \link ("<node>")<text>\#
				 */
				src = buf + (GetName(src + 4, alias) - buf);	/* fetch <node> */
				while (*src++ != ')')	/* Start <text> */
					;
				key = src;				/* Memorise */
				while (*src)
				{
					if (*src == '\\' && src[1] == '#')
					{
						*src = 0;
						if (strcmp(alias, ExtPage) == 0)
							++HasExternals;
						dst = MakeLink(dst, key, alias);
						*src = '\\';
						src += 2;
						break;
					}
					++src;
				}
			} else if (strncmp(src + 1, "end", 3) == 0)
			{
				if (NewFormat)
					EndFlag = 1;
				else
					EndNode();
				goto ende;
			} else
			{
				*dst++ = *src++;		/* Copy backslash */
			}
		} else if (*src == '@')
		{								/* Mask it now */
			*dst++ = '@';
			goto copy;
		} else
		  copy:
			*dst++ = *src++;
	}
  ende:
	*dst++ = '\n';
	*dst = 0;
	outputs(line);
	if (EndFlag)
		EndNode();
}


static void replaceext(char *buf, const char *ext)
{
	char *p = strrchr(buf, '.');

	if (p == NULL)
		p = buf + strlen(buf);
	strcpy(p, ext);
}


static void AddIncludes(const char *file, int cnt)
{
	char buf[128];
	int i;

	for (i = 0; i < cnt; i++)
	{
		strcpy(buf, file);
		sprintf(suffix(buf) + 1, "%d", i);
		outputs("\n@include ");
		outputs(buf);
	}
	outputs("\n");
}


#include "hypmain.h"

int main(int argc, const char **argv)
{
	char buf[255];
	const char *p;
	int val;

	Message("PC-Conv V(" __DATE__ "): PureC-Help --> ST-Guide sources\n"
			"        Written by Holger Weets using SOZOBON-C V2.00x10\n", NULL);

	if (argc < 2)
	{
	  error:
		fputs("usage: PC-Conv [+-anmN] file1 [file2 ...]\n"
			  "       a: explicit (-) or automatic (+) links\n"
			  "       n: HELP_RC (-) or new HELPDISC (+) format\n"
			  "       mN: max N lines per output file\n"
			  "       <fileN> must be ASCII\n", stderr);
		exit(EXIT_FAILURE);
	}

	/*
	 * Determine output file
	 */
	do
	{
		++argv;
		p = *argv;
		if (*p == '-' || *p == '+')
		{
			val = 0;
			do
			{
				switch (*p)
				{
				case '+':
					val = 1;
					break;
				case '-':
					val = 0;
					break;
				case 'A':
					Autoref = val;
					break;
				case 'M':
					maxlines = atoi(p + 1);
					goto next;
				case 'N':
					NewFormat = val;
					break;
				default:
					goto error;
				}
				++p;
			} while (*p);
		  next:
		  	;
		} else
		{
			Ropen(*argv);

			strcpy(buf, p);
			replaceext(buf, ".stg");

			fputs("\n", stderr);
			fputs(p, stderr);
			fputs(" --> ", stderr);
			fputs(buf, stderr);
			fputs("\n", stderr);

			Wopen(buf);
			strcpy(outfile, buf);
			if (NewFormat)
				ExtPage = "page-1";
			else
				ExtPage = "%%GLOBAL%%";
			ExtCnt = 0;
			total = 0;
			Syms = 0;
			HasExternals = 0;
			outheader();

			while (ingets(buf))
			{
				if (strncmp(buf, "screen", 6) == 0)
					NewPage(buf);
				else
					ConvLine(buf);
			}
			fprintf(stderr, _("\ntotal symbols      : %ld\n"), Syms);
			if (HasExternals)
			{
				fprintf(stderr, _("external references: %d\n"), HasExternals);
				outputs("@node \"");
				outputs(ExtPage);
				outputs("\"\n"
				        " Reference to external file.\n"
				        " Please complete by hand.\n"
				        "@endnode\n\n");
#if 0
				outputs("\"\n"
				        " Referenz zu externer Datei.\n"
				        " Bitte von Hand vervollst„ndigen.\n"
				        "@endnode\n\n");
#endif
			}
			total += Line;
			fprintf(stderr, _("total lines        : %d\n"), total);
			xclose(inhandle);
			if (ExtCnt)
			{
				AddIncludes(*argv, ExtCnt);
			}
			xclose(outhandle);
		}
	} while (--argc > 1);
	return EXIT_SUCCESS;
}

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
