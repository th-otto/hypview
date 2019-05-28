/*
 * Tool to test a PDF file
 *
 * based on a python script by Didier Stevens:
 * Source code put in public domain by Didier Stevens, no Copyright
 * https://DidierStevens.com
 * Use at your own risk
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>
#include <sys/stat.h>
#include <getopt.h>
#include <errno.h>
#include <math.h>
#include <assert.h>

#undef FALSE
#undef TRUE
#define FALSE 0
#define TRUE 1

#define _(x) x

static char const gl_program_name[] = "pdfid";
static char const gl_program_version[] = "0.2.5";

static int scan;
static int allnames;
static int extra;
static int force;
static int disarm;
static int csv;
static int verbose;
static double minimumscore;
static char *selection;
static int nozero;
static char *output;
static int recursedir;

static int calc_entropy;
static int calc_date;
static int calc_eof;

enum kwtype {
	KW_NONE = -1,
	KW_OBJ = 0,
	KW_ENDOBJ,
	KW_STREAM,
	KW_ENDSTREAM,
	KW_XREF,
	KW_TRAILER,
	KW_STARTXREF,
	KW_PAGE,
	KW_ENCRYPT,
	KW_OBJSTM,
	KW_JS,
	KW_JAVASCRIPT,
	KW_AA,
	KW_OPENACTION,
	KW_ACROFORM,
	KW_JBIG2DECODE,
	KW_RICHMEDIA,
	KW_LAUNCH,
	KW_EMBEDDEDFILE,
	KW_XFA,
	KW_URI,
	KW_COLORS_GT_2_24,
	
	KW_MAX
};

#define MAX_WORDLEN 1024
 
static const char *const keywords[KW_MAX] = {
	"obj",
	"endobj",
	"stream",
	"endstream",
	"xref",
	"trailer",
	"startxref",
	"/Page",
	"/Encrypt",
	"/ObjStm",
	"/JS",
	"/JavaScript",
	"/AA",
	"/OpenAction",
	"/AcroForm",
	"/JBIG2Decode",
	"/RichMedia",
	"/Launch",
	"/EmbeddedFile",
	"/XFA",
	"/URI",
	"/Colors > 2^24"
};

struct ccount {
	long count;
	long hexcode_count;
};

struct word {
	struct word *next;
	char *name;
	enum kwtype id;
	struct ccount count;
};

#define g_new(t, n) (t *)malloc((n) * sizeof(t))
#define g_new0(t, n) (t *)calloc((n), sizeof(t))
#define g_free(p) free(p)


struct pdf {
	char *filename;
	int errorOccured;
	char *errorMessage;
	int isPDF;
	char *header;
	struct word *words;
	struct ccount counts[KW_MAX];
	long cntEOFs;
	long cntCharsAfterLastEOF;
	int eof_state;

	struct {
		long allBucket[256];
		long streamBucket[256];
		long nonStreamBucket[256];
		long allCount;
		long streamCount;
		long nonStreamCount;
		double entropyAll;
		double entropyStream;
		double entropyNonStream;
	} entropy;
	
	struct {
		int state;
		int digitlen1;
		char digits1[100];
		int digitlen2;
		char digits2[100];
		char date[100];
		char TZ[2];
	} date;

	FILE *infile;
	FILE *fout;

#define MAX_UNGETC 1024
	unsigned char ungetcbuf[MAX_UNGETC];
	unsigned int ungetc_count;
};


struct plugin {
	const char *name;
	int onlyValidPDF;
	double (*score)(struct pdf *pdf);
	const char *(*Instructions)(struct pdf *pdf);
};
static const struct plugin *plugins[10];
static int num_plugins;


static inline int is_pdf_whitespace(unsigned int c)
{
	return c == 0 ||
		c == 0x09 ||
		c == 0x0a ||
		c == 0x0c ||
		c == 0x0d ||
		c == 0x20;
}


static inline int is_pdf_delimiter(unsigned int c)
{
	return c == '(' ||
		c == ')' ||
		c == '<' ||
		c == '>' ||
		c == '[' ||
		c == ']' ||
		c == '{' ||
		c == '}' ||
		c == '/' ||
		c == '%';
}


static inline int is_pdf_regular(unsigned int c)
{
	return !is_pdf_whitespace(c) && !is_pdf_delimiter(c);
}


static int ascii_toupper(int c)
{
	if (c >= 'a' && c <= 'z')
		return c - 'a' + 'A';
	return c;
}


static int ascii_swap(int c)
{
	if (c >= 'a' && c <= 'z')
		return c - 'a' + 'A';
	if (c >= 'A' && c <= 'Z')
		return c - 'A' + 'a';
	return c;
}


static int is_hex(int c)
{
	c = ascii_toupper(c);
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}


static char *g_strdup(const char *str)
{
	char *newstr;
	
	if (str == NULL)
		return NULL;
	newstr = g_new(char, strlen(str) + 1);
	assert(newstr);
	strcpy(newstr, str);
	return newstr;
}


static struct word *find_word(struct word *words, char slash, const char *name)
{
	struct word *w;
	
	for (w = words; w != NULL; w = w->next)
	{
		if ((slash == 0 && strcmp(w->name, name) == 0) ||
			(slash != 0 && w->name[0] == slash && strcmp(w->name + 1, name) == 0))
		{
			return w;
		}
	}
	return NULL;
}


static struct word *add_word(struct word **words, char slash, const char *name)
{
	struct word *w;
	struct word **last;
	
	w = find_word(*words, slash, name);
	if (w == NULL)
	{
		w = g_new(struct word, 1);
		assert(w);
		last = words;
		while (*last != NULL)
			last = &(*last)->next;
		*last = w;
		w->next = NULL;
		w->name = g_new(char, strlen(name) + 2);
		assert(w->name);
		if (slash)
		{
			w->name[0] = slash;
			strcpy(w->name + 1, name);
		} else
		{
			strcpy(w->name, name);
		}
		w->id = KW_NONE;
		w->count.count = 0;
		w->count.hexcode_count = 0;
	}
	return w;
}


static struct pdf *pdf_new(const char *filename)
{
	struct pdf *pdf;
	
	pdf = g_new0(struct pdf, 1);
	assert(pdf);
	pdf->filename = g_strdup(filename);
	pdf->ungetc_count = 0;

	return pdf;
}


static inline int pdf_getc(struct pdf *pdf)
{
	if (pdf->ungetc_count != 0)
	{
		return pdf->ungetcbuf[--pdf->ungetc_count];
	}
	return fgetc(pdf->infile);
}


static inline void pdf_ungetc(struct pdf *pdf, int c)
{
	assert(pdf->ungetc_count < MAX_UNGETC);
	pdf->ungetcbuf[pdf->ungetc_count++] = c;
}


static void pdf_parse_eof(struct pdf *pdf, unsigned int ch)
{
	if (pdf->cntEOFs > 0)
	{
		pdf->cntCharsAfterLastEOF += 1;
	}
	if (pdf->eof_state == 0 && ch == '%')
	{
		pdf->eof_state = 1;
		return;
	}
	if (pdf->eof_state == 1 && ch == '%')
	{
		pdf->eof_state = 2;
		return;
	}
	if (pdf->eof_state == 2 && ch == 'E')
	{
		pdf->eof_state = 3;
		return;
	}
	if (pdf->eof_state == 3 && ch == 'O')
	{
		pdf->eof_state = 4;
		return;
	}
	if (pdf->eof_state == 4 && ch == 'F')
	{
		pdf->eof_state = 5;
		return;
	}
	if (pdf->eof_state >= 5 && (ch == 0x0d || ch == 0x0a || ch == ' ' || ch == '\t'))
	{
		pdf->cntEOFs += 1;
		pdf->cntCharsAfterLastEOF = 0;
		if (ch == '\n')
			pdf->eof_state = 0;
		else if (pdf->eof_state == 5 && ch == 0x0d)
			pdf->eof_state = 6;
		return;
	}
	if (pdf->eof_state == 6)
	{
		if (ch == 0x0a)
			pdf->cntCharsAfterLastEOF = 0;
		pdf->eof_state = 0;
	} else
	{
		pdf->eof_state = 0;
	}
}


static __attribute__((format(printf, 1, 2))) void LogPrint(const char *format, ...)
{
	va_list args;
	const char *filename = NULL;

	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);
	if (scan)
		filename = "pdfid.log";
	if (output)
		filename = output;
	if (filename)
	{
		FILE *logfile = fopen(filename, "a");
		
		if (logfile)
		{
			va_start(args, format);
			vfprintf(logfile, format, args);
			va_end(args);
			fclose(logfile);
		}
	}
}


static const char *xml_bool(int val)
{
	return val ? "\"True\"" : "\"False\"";
}


static char *xml_quote(const char *str)
{
	char *newstr;
	char *dst;
	
	if (str == NULL)
		return g_strdup("\"\"");
	newstr = g_new(char, 2 + 2 * strlen(str) + 1);
	assert(newstr);
	dst = newstr;
	*dst++ = '"';
	while (*str)
	{
		if (*str == '"' || *str == '\\')
			*dst++ = '\\';
		*dst++ = *str++;
	}
	*dst++ = '"';
	*dst = '\0';
	return newstr;
}


static void print_xml(struct pdf *pdf, FILE *out)
{
	int i;
	char *errorMessage;
	char *filename;
	char *header;
	
	errorMessage = xml_quote(pdf->errorMessage);
	filename = xml_quote(pdf->filename);
	header = xml_quote(pdf->header);
	fprintf(out, "<PDFiD ErrorOccured=%s ErrorMessage=%s Filename=%s Header=%s IsPDF=%s Version=\"%s\">\n",
		xml_bool(pdf->errorOccured),
		errorMessage,
		filename,
		header,
		xml_bool(pdf->isPDF),
		gl_program_version);
	g_free(header);
	g_free(filename);
	g_free(errorMessage);
	for (i = 0; i < KW_MAX; i++)
	{
		fprintf(out, "        <Keyword Count=\"%ld\" HexcodeCount=\"%ld\" Name=\"%s\"/>\n",
			pdf->counts[i].count,
			pdf->counts[i].hexcode_count,
			keywords[i]);
	}
	fprintf(out, "    <Keywords>\n");
	fprintf(out, "    </Keywords>\n");
	fprintf(out, "    <Dates>\n");
	fprintf(out, "    </Dates>\n");
	fprintf(out, "</PDFiD>\n");
}


static char *find_pdf_header(struct pdf *pdf)
{
	char *header;
	int max_count = 1020;
	int i;
	int c;
	
	header = g_new(char, max_count + 1);
	assert(header);
	for (i = 0; i < max_count; i++)
	{
		c = pdf_getc(pdf);
		if (c == '%')
		{
			i++;
			c = pdf_getc(pdf);
			if (c == 'P')
			{
				i++;
				c = pdf_getc(pdf);
				if (c == 'D')
				{
					i++;
					c = pdf_getc(pdf);
					if (c == 'F')
					{
						i = 0;
						header[i++] = '%';
						header[i++] = 'P';
						header[i++] = 'D';
						header[i++] = 'F';
						while (i < 24)
						{
							c = pdf_getc(pdf);
							if (c == 0x0d)
							{
								pdf_ungetc(pdf, c);
								break;
							} else if (c == 0x0a)
							{
								pdf_ungetc(pdf, c);
								break;
							}
							header[i++] = c;
						}
						header[i] = '\0';
						return header;
					} else
					{
						header[i] = c;
					}
				} else
				{
					header[i] = c;
				}
			} else
			{
				header[i] = c;
			}
		} else
		{
			header[i] = c;
		}
	}
	while (i > 0)
	{
		pdf_ungetc(pdf, (unsigned char)header[--i]);
	}
	g_free(header);
	return NULL;
}


static void entropy_add(struct pdf *pdf, unsigned char byte, int insideStream)
{
	pdf->entropy.allBucket[byte]++;
	if (insideStream)
		pdf->entropy.streamBucket[byte]++;
}


static double fEntropy(long countByte, long countTotal)
{
	double x;
	
	x = (double)countByte / countTotal;
	if (x > 0)
		return -x * log2(x);
	return 0.0;
}


static void entropy_calc(struct pdf *pdf)
{
	int i;
	
	for (i = 0; i < 256; i++)
	{
		pdf->entropy.nonStreamBucket[i] = pdf->entropy.allBucket[i] - pdf->entropy.streamBucket[i];
		pdf->entropy.allCount += pdf->entropy.allBucket[i];
		pdf->entropy.streamCount += pdf->entropy.streamBucket[i];
		pdf->entropy.nonStreamCount += pdf->entropy.nonStreamBucket[i];
	}
	for (i = 0; i < 256; i++)
	{
		if (pdf->entropy.allCount != 0)
			pdf->entropy.entropyAll += fEntropy(pdf->entropy.allBucket[i], pdf->entropy.allCount);
 		if (pdf->entropy.streamCount != 0)
			pdf->entropy.entropyStream += fEntropy(pdf->entropy.streamBucket[i], pdf->entropy.streamCount);
		if (pdf->entropy.nonStreamCount != 0)
			pdf->entropy.entropyNonStream += fEntropy(pdf->entropy.nonStreamBucket[i], pdf->entropy.nonStreamCount);
	}
}


static int date_parse(struct pdf *pdf, int byte)
{
	if (byte == 'D')
	{
		pdf->date.state = 1;
		return FALSE;
	}
	if (pdf->date.state == 1)
	{
		if (byte == ':')
		{
			pdf->date.state = 2;
			pdf->date.digitlen1 = 0;
		} else
		{
			pdf->date.state = 0;
		}
		return FALSE;
	}
	if (pdf->date.state == 2)
	{
		if (pdf->date.digitlen1 < 14)
		{
			if (byte >= '0' && byte <= '9')
				pdf->date.digits1[pdf->date.digitlen1++] = byte;
			else
				pdf->date.state = 0;
			return FALSE;
		}
		if (byte == '+' || byte == '-' || byte == 'Z')
		{
			pdf->date.state = 3;
			pdf->date.digitlen2 = 0;
			pdf->date.TZ[0] = byte;
			return FALSE;
		}
		if (byte == '"')
		{
			pdf->date.state = 0;
			pdf->date.digits1[pdf->date.digitlen1] = '\0';
			strcpy(pdf->date.date, "D:");
			strcat(pdf->date.date, pdf->date.digits1);
			return TRUE;
		}
		if (byte < '0' || byte > '9')
		{
			pdf->date.state = 0;
			pdf->date.digits1[pdf->date.digitlen1] = '\0';
			strcpy(pdf->date.date, "D:");
			strcat(pdf->date.date, pdf->date.digits1);
			return TRUE;
		}
		pdf->date.state = 0;
		return FALSE;
	}
	if (pdf->date.state == 3)
	{
		if (pdf->date.digitlen2 < 2)
		{
			if (byte >= '0' && byte <= '9')
				pdf->date.digits2[pdf->date.digitlen2++] = byte;
			else
				pdf->date.state = 0;
			return FALSE;
		}
		if (pdf->date.digitlen2 == 2)
		{
			if (byte == '\'')
				pdf->date.digits2[pdf->date.digitlen2++] = byte;
			else
				pdf->date.state = 0;
			return FALSE;
		}
		if (pdf->date.digitlen2 < 5)
		{
			if (byte >= '0' && byte <= '9')
			{
				pdf->date.digits2[pdf->date.digitlen2++] = byte;
				if (pdf->date.digitlen2 == 5)
				{
					pdf->date.state = 0;
					pdf->date.digits1[pdf->date.digitlen1] = '\0';
					pdf->date.digits2[pdf->date.digitlen2] = '\0';
					strcpy(pdf->date.date, "D:");
					strcat(pdf->date.date, pdf->date.digits1);
					strcat(pdf->date.date, pdf->date.TZ);
					strcat(pdf->date.date, pdf->date.digits2);
					return TRUE;
				} else
				{
					return FALSE;
				}
			}
			pdf->date.state = 0;
			return FALSE;
		}
	}	
	return FALSE;
}


static void append_date(struct pdf *pdf, const char *date, const char *name)
{
	(void)pdf;
	(void)date;
	(void)name;
}


static void removeInsideStream(struct pdf *pdf, unsigned char byte)
{
	if (pdf->entropy.streamBucket[byte] > 0)
		pdf->entropy.streamBucket[byte]--;
}


static void HexCodeToString(struct pdf *pdf, unsigned char byte)
{
	static char const hexchars[] = "0123456789abcdef";

	if (!is_pdf_regular(byte) || byte == '#')
	{
		fputc('#', pdf->fout);
		fputc(hexchars[(byte >> 4) & 0x0f], pdf->fout);
		fputc(hexchars[(byte     ) & 0x0f], pdf->fout);
	} else
	{
		fputc(byte, pdf->fout);
	}
}


static void UpdateWords(struct pdf *pdf, const char *name, char slash, int hexcode, char *lastname, int *insideStream)
{
	struct word *word;
	
	if (*name == '\0')
		return;
	word = find_word(pdf->words, slash, name);
	if (word == NULL)
	{
		if (*insideStream)
			return;
		if (!allnames || slash == 0)
			return;
		word = add_word(&pdf->words, slash, name);
	}
	word->count.count++;
	if (hexcode)
		word->count.hexcode_count++;
	if (slash != '\0')
	{
		lastname[0] = slash;
		strcpy(lastname + 1, word->name);
	}
	if (slash == 0)
	{
		if (word->id == KW_STREAM)
		{
			*insideStream = TRUE;
		} else if (word->id == KW_ENDSTREAM)
		{
			if (*insideStream && calc_entropy)
			{
				const char *s = "endstream";
				while (*s)
				{
					removeInsideStream(pdf, *s);
					s++;
				}
			}
			*insideStream = FALSE;
		}
	}
	if (pdf->fout)
	{
		const char *s = word->name;
		
		if (slash != 0 && (
			word->id == KW_JS ||
			word->id == KW_JAVASCRIPT ||
			word->id == KW_AA ||
			word->id == KW_OPENACTION ||
			word->id == KW_JBIG2DECODE ||
			word->id == KW_RICHMEDIA ||
			word->id == KW_LAUNCH))
		{
			while (*s)
			{
				HexCodeToString(pdf, ascii_swap(*s));
				s++;
			}
		} else
		{
			while (*s)
			{
				HexCodeToString(pdf, *s);
				s++;
			}
		}
	}
}


static int pdfid(struct pdf *pdf)
{
	char wordbuf[MAX_WORDLEN + 1];
	char lastname[MAX_WORDLEN + 1];
	int wordlen;
	char slash;
	int insideStream;
	struct word *word;
	int i;
	int byte, d1, d2;
	int charUpper;
	int hexcode;

	pdf->header = find_pdf_header(pdf);
	pdf->isPDF = pdf->header != NULL;
	if (!pdf->isPDF && !force)
	{
		return TRUE;
	}

	if (disarm)
	{
		char *p;
		
		char *filename = g_new(char, strlen(pdf->filename) + sizeof(".disarmed"));
		assert(filename);
		strcpy(filename, pdf->filename);
		p = strrchr(filename, '.');
		if (p && (strcmp(p, ".pdf") == 0 || strcmp(p, ".PDF") == 0))
		{
			strcpy(p, ".disarmed");
			strcat(p, ".pdf");
		} else
		{
			strcat(filename, ".disarmed");
		}
		pdf->fout = fopen(filename, "wb");
		if (pdf->fout == NULL)
		{
			fprintf(stderr, "%s: %s\n", filename, strerror(errno));
			return FALSE;
		}
		if (pdf->header)
		{
			fputs(pdf->header, pdf->fout);
		}
	}
	
	wordlen = 0;
	slash = 0;
	insideStream = FALSE;
	pdf->words = NULL;
	hexcode = FALSE;
	for (i = 0; i < KW_MAX; i++)
		add_word(&pdf->words, 0, keywords[i])->id = (enum kwtype)i;

	if (calc_entropy && pdf->header)
	{
		for (i = 0; pdf->header[i] != 0; i++)
			entropy_add(pdf, pdf->header[i], insideStream);
	}
	
	byte = pdf_getc(pdf);
	while (byte != EOF)
	{
		charUpper = ascii_toupper(byte);
		if ((charUpper >= 'A' && charUpper <= 'Z') || (charUpper >= '0' && charUpper <= '9'))
		{
			if (wordlen < MAX_WORDLEN)
				wordbuf[wordlen++] = byte;
		} else if (slash != 0 && byte == '#')
		{
			d1 = pdf_getc(pdf);
			if (d1 != EOF)
			{
				d2 = pdf_getc(pdf);
				if (d2 != EOF && (d1 = is_hex(d1)) >= 0 && (d2 = is_hex(d2)) >= 0)
				{
					if ((wordlen + 3) <= MAX_WORDLEN)
					{
						wordbuf[wordlen++] = (d1 << 4) | d2;
					}
					hexcode = TRUE;
					if (calc_entropy)
					{
						entropy_add(pdf, d1, insideStream);
						entropy_add(pdf, d2, insideStream);
					}
					if (calc_eof)
					{
						pdf_parse_eof(pdf, d1);
						pdf_parse_eof(pdf, d2);
					}
				} else
				{
					if (d2 != EOF)
						pdf_ungetc(pdf, d2);
					pdf_ungetc(pdf, d1);
					wordbuf[wordlen] = '\0';
					UpdateWords(pdf, wordbuf, slash, hexcode, lastname, &insideStream);
					wordlen = 0;
					hexcode = FALSE;
					if (pdf->fout)
						fputc(byte, pdf->fout);
				}
			} else
			{
				wordbuf[wordlen] = '\0';
				UpdateWords(pdf, wordbuf, slash, hexcode, lastname, &insideStream);
				wordlen = 0;
				hexcode = FALSE;
				if (pdf->fout)
					fputc(byte, pdf->fout);
			}
		} else
		{
			wordbuf[wordlen] = '\0';
			if (wordlen != 0)
			{
				if (strcmp(lastname, "/Colors") == 0 && strtoul(wordbuf, NULL, 10) > 0x1000000UL)
					find_word(pdf->words, 0, keywords[KW_COLORS_GT_2_24])->count.count++;
				UpdateWords(pdf, wordbuf, slash, hexcode, lastname, &insideStream);
			}
			wordlen = 0;
			hexcode = FALSE;
			if (pdf->fout)
				fputc(byte, pdf->fout);
			if (byte == '/')
				slash = byte;
			else
				slash = 0;
		}
		
		if (calc_date)
			if (date_parse(pdf, byte))
				append_date(pdf, pdf->date.date, lastname);

		if (calc_entropy)
			entropy_add(pdf, byte, insideStream);
		if (calc_eof)
			pdf_parse_eof(pdf, byte);
		
		byte = pdf_getc(pdf);
	}

	wordbuf[wordlen] = '\0';
	UpdateWords(pdf, wordbuf, slash, hexcode, lastname, &insideStream);
	wordlen = 0;

	/*
	 * check to see if file ended with %%EOF.  If so, we can reset charsAfterLastEOF and add one to EOF count.  This is never performed in
	 * the parse function because it never gets called due to hitting the end of file.
	 */
	if (calc_eof)
	{
		if (pdf->eof_state == 5)
		{
			pdf->cntEOFs += 1;
			pdf->cntCharsAfterLastEOF = 0;
			pdf->eof_state = 0;
		}
	}
	if (calc_entropy)
		entropy_calc(pdf);

	for (word = pdf->words; word != NULL; word = word->next)
	{
		if (word->id >= 0)
			pdf->counts[word->id] = word->count;
	}

	return TRUE;
}


static void print_normal(struct pdf *pdf)
{
	struct word *word;

	LogPrint("PDFiD %s %s\n", gl_program_version, pdf->filename);
	if (pdf->errorOccured)
	{
		LogPrint("***Error occured***\n%s\n", pdf->errorMessage ? pdf->errorMessage : "(null)");
		return;
	}
	if (!force && !pdf->isPDF)
	{
		LogPrint(" Not a PDF document\n");
		return;
	}
	LogPrint(" PDF Header: %s\n", pdf->header ? pdf->header : "");
	for (word = pdf->words; word != NULL; word = word->next)
	{
		if (allnames || word->id >= 0)
		{
			if (!nozero || word->count.count > 0 || word->count.hexcode_count > 0)
			{
				LogPrint(" %-16s %7ld", word->name, word->count.count);
				if (word->count.hexcode_count > 0)
					LogPrint(" (%ld)", word->count.hexcode_count);
				LogPrint("\n");
			}
		}
	}
	if (calc_eof)
	{
		LogPrint(" %-16s %7ld\n", "%%EOF", pdf->cntEOFs);
		if (pdf->cntEOFs > 0)
			LogPrint("  %-16s %7ld\n", "After last %%EOF", pdf->cntCharsAfterLastEOF);
	}
	if (calc_date)
	{
	}
	if (calc_entropy)
	{
		LogPrint(" Total entropy:           %f (%10ld bytes)\n", pdf->entropy.entropyAll, pdf->entropy.allCount);
		LogPrint(" Entropy inside streams:  %f (%10ld bytes)\n", pdf->entropy.entropyStream, pdf->entropy.streamCount);
		LogPrint(" Entropy outside streams: %f (%10ld bytes)\n", pdf->entropy.entropyNonStream, pdf->entropy.nonStreamCount);
	}
}



static int eval(const char *expression)
{
	/* TODO: */
	(void)expression;
	return FALSE;
}


static int process_file(const char *filename)
{
	struct pdf *pdf;
	int retvalue;

	pdf = pdf_new(filename);
	if (pdf == NULL)
		return FALSE;
	if (filename == NULL || strcmp(filename, "-") == 0)
	{
		pdf->infile = fdopen(0, "rb");
	} else
	{
		pdf->infile = fopen(filename, "rb");
	}
	if (pdf->infile == NULL)
	{
		fprintf(stderr, "%s: %s\n", filename, strerror(errno));
		return FALSE;
	}
	retvalue = pdfid(pdf);
	fclose(pdf->infile);
	if (pdf->fout)
	{
		fclose(pdf->fout);
		pdf->fout = 0;
	}

	if (selection)
	{
		if (force || (!pdf->errorOccured && pdf->isPDF))
		{
			/* TODO: */
			int selected = eval(selection);
			if (selected)
			{
				if (csv)
					LogPrint("%s\n", pdf->filename);
				else
					print_normal(pdf);
			}
		}
	} else if (num_plugins)
	{
		int i;
		
		for (i = 0; i < num_plugins; i++)
		{
			const struct plugin *plugin = plugins[i];
			double score;
			
			if (!plugin->onlyValidPDF || (!pdf->errorOccured && pdf->isPDF))
			{
				score = plugin->score(pdf);
				if (score >= minimumscore)
				{
					if (csv)
					{
						LogPrint("%s;%s;%.2f\n", pdf->filename, plugin->name, score);
					} else
					{
						print_normal(pdf);
						LogPrint("%s score:        %.2f\n", plugin->name, score);
						if (plugin->Instructions)
							LogPrint("%s instructions: %s\n", plugin->name, plugin->Instructions(pdf));
					}
				}
			} else
			{
				if (csv)
				{
					if (pdf->errorOccured)
						LogPrint("%s;%s;%s\n", pdf->filename, plugin->name, "Error occured");
					if (!pdf->isPDF)
						LogPrint("%s;%s;%s\n", pdf->filename, plugin->name, "Not a PDF document");
				} else
				{
					print_normal(pdf);
				}
			}
		}
	} else
	{
		print_normal(pdf);
	}
	
	return retvalue;
}


static int scan_dir(const char *dirname)
{
	DIR *dir;
	struct dirent *ent;
	struct stat st;
	char *filename;
	int retval = TRUE;

	dir = opendir(dirname);
	if (dir == NULL)
	{
		if (errno == ENOTDIR)
			return process_file(dirname);
		fprintf(stderr, "%s: %s\n", dirname, strerror(errno));
		return FALSE;
	}
	while ((ent = readdir(dir)) != NULL)
	{
		if (strcmp(ent->d_name, ".") == 0)
			continue;
		if (strcmp(ent->d_name, "..") == 0)
			continue;
		filename = g_new(char, strlen(dirname) + strlen(ent->d_name) + 2);
		assert(filename);
		strcat(strcat(strcpy(filename, dirname), "/"), ent->d_name);
		retval = TRUE;
		if (stat(filename, &st) == 0)
		{
			if (S_ISDIR(st.st_mode))
			{
				if (recursedir)
					if (scan_dir(filename) == FALSE)
						retval = FALSE;
			} else
			{
				if (process_file(filename) == FALSE)
					retval = FALSE;
			}
		}
		g_free(filename);
		if (retval == FALSE)
			break;
	}
	closedir(dir);

	return retval;
}


enum opt {
	OPT_VERSION = 'V',
	OPT_HELP = 'h',
	OPT_SCAN = 's',
	OPT_ALL = 'a',
	OPT_EXTRA = 'e',
	OPT_FORCE = 'f',
	OPT_DISARM = 'd',
	OPT_CSV = 'c',
	OPT_MINIMUMSCORE = 'm',
	OPT_SELECT = 'S',
	OPT_NOZERO = 'n',
	OPT_OUTPUT = 'o',
	OPT_VERBOSE = 'v',
	OPT_RECURSE = 'r',
};


static struct option const long_options[] = {
	{ "scan", no_argument, NULL, OPT_SCAN },
	{ "all", no_argument, NULL, OPT_ALL },
	{ "extra", no_argument, NULL, OPT_EXTRA },
	{ "force", no_argument, NULL, OPT_FORCE },
	{ "disarm", no_argument, NULL, OPT_DISARM },
	{ "csv", no_argument, NULL, OPT_CSV },
	{ "minimumscore", required_argument, NULL, OPT_MINIMUMSCORE },
	{ "verbose", no_argument, NULL, OPT_VERBOSE },
	{ "select", required_argument, NULL, OPT_SELECT },
	{ "nozero", no_argument, NULL, OPT_NOZERO },
	{ "output", required_argument, NULL, OPT_OUTPUT },
	{ "recursedir", no_argument, NULL, OPT_RECURSE },
	{ "version", no_argument, NULL, OPT_VERSION },
	{ "help", no_argument, NULL, OPT_HELP },
	{ NULL, no_argument, NULL, 0 }
};


static void usage(FILE *fp)
{
	fprintf(fp, _("usage: %s [options] [pdf-file]...\n"), gl_program_name);
	fputs(_("Tool to test a PDF file"), fp);
	fputs("\n", fp);
	fputs(_("Arguments:\n"), fp);
	fputs(_("pdf-file can be a single file, or several files\n"), fp);
	fputs("\n", fp);
	fputs(_("Source code put in the public domain by Didier Stevens, no Copyright\n"), fp);
	fputs(_("Use at your own risk\n"), fp);
	fputs("\n", fp);
	fputs(_("  -s, --scan            scan the given directory\n"), fp);
	fputs(_("  -a, --all             display all the names\n"), fp);
	fputs(_("  -e, --extra           display extra data, like dates\n"), fp);
	fputs(_("  -f, --force           force the scan of the file, even without proper %PDF header\n"), fp);
	fputs(_("  -d, --disarm          disable JavaScript and auto launch\n"), fp);
	fputs(_("  -c, --csv             output csv data\n"), fp);
	fputs(_("  -m, --minimumscore    minimum score for plugin results output\n"), fp);
	fputs(_("  -v, --verbose         verbose\n"), fp);
	fputs(_("  -S, --select          selection expression\n"), fp);
	fputs(_("  -n, --nozero          suppress output for counts equal to zero\n"), fp);
	fputs(_("  -r, --recursedir      Recurse directories\n"), fp);
}


static void print_version(FILE *fp)
{
	fprintf(fp, "%s %s\n", gl_program_name, gl_program_version);
}


int main(int argc, char **argv)
{
	int retvalue = EXIT_SUCCESS;
	int c;
	const char *filename;

	num_plugins = 0;
	while ((c = getopt_long(argc, argv, "acdefm:no:svhS:V", long_options, NULL)) >= 0)
	{
		switch ((enum opt) c)
		{
		case OPT_SCAN:
			scan = TRUE;
			break;
		
		case OPT_ALL:
			allnames = TRUE;
			break;
		
		case OPT_EXTRA:
			extra = TRUE;
			break;
		
		case OPT_FORCE:
			force = TRUE;
			break;
		
		case OPT_DISARM:
			disarm = TRUE;
			break;
		
		case OPT_CSV:
			csv = TRUE;
			break;
		
		case OPT_MINIMUMSCORE:
			minimumscore = strtod(optarg, NULL);
			break;
		
		case OPT_VERBOSE:
			verbose = TRUE;
			break;
		
		case OPT_SELECT:
			selection = optarg;
			break;
		
		case OPT_NOZERO:
			nozero = TRUE;
			break;
		
		case OPT_OUTPUT:
			output = optarg;
			break;
		
		case OPT_RECURSE:
			recursedir = TRUE;
			break;
		
		case OPT_HELP:
			usage(stdout);
			return EXIT_SUCCESS;
		
		case OPT_VERSION:
			print_version(stdout);
			return EXIT_SUCCESS;
		
		default:
			return EXIT_FAILURE;
		}
	}

	calc_entropy = extra;
	calc_date = extra;
	calc_eof = extra;
	
	if (optind >= argc)
	{
		if (disarm)
		{
			fprintf(stderr, _("Option disarm not supported with stdin\n"));
			disarm = FALSE;
			retvalue = EXIT_FAILURE;
		}
		if (scan)
		{
			fprintf(stderr, _("Option scan not supported with stdin\n"));
			scan = FALSE;
			retvalue = EXIT_FAILURE;
		}
	}
	
	if (csv && retvalue == EXIT_SUCCESS)
	{
		if (num_plugins != 0)
			LogPrint("Filename;Plugin-name;Score\n");
		else if (selection)
			LogPrint("Filename\n");
	}
	
	if (optind >= argc)
	{
		process_file("-");
	} else
	{
		while (optind < argc)
		{
			filename = argv[optind++];
			
			if (scan)
			{
				if (scan_dir(filename) == FALSE)
					retvalue = EXIT_FAILURE;
			} else
			{
				if (process_file(filename) == FALSE)
					retvalue = EXIT_FAILURE;
			}
		}
	}
			
	(void)print_xml;
	return retvalue;
}
