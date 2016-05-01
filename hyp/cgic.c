/*
 * This is the CGIC CGI development library for C programmers.
 * Copyright 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003,
 * 2004, Thomas Boutell and Boutell.Com, Inc.
 */

#include "hypdefs.h"
#include <time.h>
#include "hypdebug.h"
#include "cgic.h"

/* cgicTempDir is the only setting you are likely to need
	to change in this file. */

/* Used only in Unix environments, in conjunction with mkstemp(). 
	Elsewhere (Windows), temporary files go where the tmpnam() 
	function suggests. If this behavior does not work for you, 
	modify the getTempFileName() function to suit your needs. */

#define cgicTempDir "/tmp"

#define cgiStrEq(a, b) (strcmp((a), (b)) == 0)

const char *cgiServerSoftware;
const char *cgiServerName;
const char *cgiGatewayInterface;
const char *cgiServerProtocol;
const char *cgiServerPort;
const char *cgiRequestMethod;
const char *cgiPathInfo;
const char *cgiPathTranslated;
const char *cgiScriptName;
const char *cgiScriptFilename;
const char *cgiQueryString;
const char *cgiRemoteHost;
const char *cgiRemoteAddr;
const char *cgiAuthType;
const char *cgiRemoteUser;
const char *cgiRemoteIdent;
const char *cgiContentType;
const char *cgiMultipartBoundary;
const char *cgiCookie;
const char *cgiAccept;
const char *cgiUserAgent;
const char *cgiReferrer;
const char *cgiDocumentRoot;
int cgiContentLength;

/* Pointer to CGI output. The cgiHeader functions should be used
	first to output the mime headers; the output HTML
	page, GIF image or other web document should then be written
	to cgiOut by the programmer. In the standard CGIC library,
	cgiOut is always equivalent to stdout. */

static GString *cgiOut;

static char cgiContentTypeData[1024];


typedef enum
{
	cgiParseSuccess,
	cgiParseMemory,
	cgiParseIO
} cgiParseResultType;

/* One form entry, consisting of an attribute-value pair,
	and an optional filename and content type. All of
	these are guaranteed to be valid null-terminated strings,
	which will be of length zero in the event that the
	field is not present, with the exception of tfileName
	which will be null when 'in' is null. DO NOT MODIFY THESE 
	VALUES. Make local copies if modifications are desired. */

typedef struct cgiFormEntryStruct
{
	char *attr;
	/* value is populated for regular form fields only.
	   For file uploads, it points to an empty string, and file
	   upload data should be read from the file tfileName. */
	char *value;
	/* Valid for both files and regular fields; does not include
	   terminating null of regular fields. */
	int valueLength;
	char *fileName;
	char *contentType;
	struct cgiFormEntryStruct *next;
} cgiFormEntry;


/* The first form entry. */
static cgiFormEntry *cgiFormEntryFirst;

static const char *const days[] = {
	"Sun",
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri",
	"Sat"
};

static const char *const months[] = {
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec"
};

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static void cgiGetenv(const char **s, const char *var)
{
	*s = getenv(var);
}

/* ------------------------------------------------------------------------- */

typedef enum
{
	cgiEscapeRest,
	cgiEscapeFirst,
	cgiEscapeSecond
} cgiEscapeState;

typedef enum
{
	cgiUnescapeSuccess,
	cgiUnescapeMemory
} cgiUnescapeResultType;

static int cgiHexValue[256];

static cgiUnescapeResultType cgiUnescapeChars(char **sp, const char *cp, int len)
{
	char *s;
	cgiEscapeState escapeState = cgiEscapeRest;
	int escapedValue = 0;
	int srcPos = 0;
	int dstPos = 0;

	s = g_new(char, len + 1);
	if (!s)
	{
		return cgiUnescapeMemory;
	}
	while (srcPos < len)
	{
		int ch = cp[srcPos];

		switch (escapeState)
		{
		case cgiEscapeRest:
			if (ch == '%')
			{
				escapeState = cgiEscapeFirst;
			} else if (ch == '+')
			{
				s[dstPos++] = ' ';
			} else
			{
				s[dstPos++] = ch;
			}
			break;
		case cgiEscapeFirst:
			escapedValue = cgiHexValue[ch] << 4;
			escapeState = cgiEscapeSecond;
			break;
		case cgiEscapeSecond:
			escapedValue += cgiHexValue[ch];
			s[dstPos++] = escapedValue;
			escapeState = cgiEscapeRest;
			break;
		}
		srcPos++;
	}
	s[dstPos] = '\0';
	*sp = s;
	return cgiUnescapeSuccess;
}


/* ------------------------------------------------------------------------- */

static cgiParseResultType cgiParseFormInput(const char *data, int length)
{
	/* Scan for pairs, unescaping and storing them as they are found. */
	int pos = 0;
	cgiFormEntry *n;
	cgiFormEntry *l = 0;

	while (pos != length)
	{
		int foundEq = 0;
		int foundAmp = 0;
		int start = pos;
		int len = 0;
		char *attr;
		char *value;

		while (pos != length)
		{
			if (data[pos] == '=')
			{
				foundEq = 1;
				pos++;
				break;
			}
			pos++;
			len++;
		}
		if (!foundEq)
		{
			break;
		}
		if (cgiUnescapeChars(&attr, data + start, len) != cgiUnescapeSuccess)
		{
			return cgiParseMemory;
		}
		start = pos;
		len = 0;
		while (pos != length)
		{
			if (data[pos] == '&')
			{
				foundAmp = 1;
				pos++;
				break;
			}
			pos++;
			len++;
		}
		/* The last pair probably won't be followed by a &, but
		   that's fine, so check for that after accepting it */
		if (cgiUnescapeChars(&value, data + start, len) != cgiUnescapeSuccess)
		{
			g_free(attr);
			return cgiParseMemory;
		}
		/* OK, we have a new pair, add it to the list. */
		n = g_new(cgiFormEntry, 1);
		if (!n)
		{
			g_free(attr);
			g_free(value);
			return cgiParseMemory;
		}
		n->attr = attr;
		n->value = value;
		n->valueLength = strlen(n->value);
		n->contentType = NULL;
		n->fileName = NULL;
		n->next = 0;
		if (!l)
		{
			cgiFormEntryFirst = n;
		} else
		{
			l->next = n;
		}
		l = n;
		if (!foundAmp)
		{
			break;
		}
	}
	return cgiParseSuccess;
}

/* ------------------------------------------------------------------------- */

static cgiParseResultType cgiParsePostFormInput(void)
{
	char *input;
	cgiParseResultType result;

	if (!cgiContentLength)
	{
		return cgiParseSuccess;
	}
	input = g_new(char, cgiContentLength);
	if (!input)
	{
		return cgiParseMemory;
	}
	if (((int) fread(input, 1, cgiContentLength, stdin)) != cgiContentLength)
	{
		return cgiParseIO;
	}
	result = cgiParseFormInput(input, cgiContentLength);
	g_free(input);
	return result;
}

/* ------------------------------------------------------------------------- */

/* 2.0: A virtual datastream supporting putback of 
	enough characters to handle multipart boundaries easily.
	A simple memset(&mp, 0, sizeof(mp)) is suitable initialization. */

typedef struct
{
	/* Buffer for putting characters back */
	char putback[1024];
	/* Position in putback from which next character will be read.
	   If readPos == writePos, then next character should
	   come from cgiIn. */
	int readPos;
	/* Position in putback to which next character will be put back.
	   If writePos catches up to readPos, as opposed to the other
	   way around, the stream no longer functions properly.
	   Calling code must guarantee that no more than 
	   sizeof(putback) bytes are put back at any given time. */
	int writePos;
	/* Offset in the virtual datastream; can be compared
	   to cgiContentLength */
	int offset;
} mpStream, *mpStreamPtr;

static int mpRead(mpStreamPtr mpp, char *buffer, int len)
{
	int ilen = len;
	int got = 0;

	while (len)
	{
		if (mpp->readPos != mpp->writePos)
		{
			*buffer++ = mpp->putback[mpp->readPos++];
			mpp->readPos %= sizeof(mpp->putback);
			got++;
			len--;
		} else
		{
			break;
		}
	}
	/* Refuse to read past the declared length in order to
	   avoid deadlock */
	if (len > (cgiContentLength - mpp->offset))
	{
		len = cgiContentLength - mpp->offset;
	}
	if (len)
	{
		int fgot = fread(buffer, 1, len, stdin);

		if (fgot >= 0)
		{
			mpp->offset += (got + fgot);
			return got + fgot;
		} else if (got > 0)
		{
			mpp->offset += got;
			return got;
		} else
		{
			/* EOF or error */
			return fgot;
		}
	} else if (got)
	{
		return got;
	} else if (ilen)
	{
		return EOF;
	} else
	{
		/* 2.01 */
		return 0;
	}
}

/* ------------------------------------------------------------------------- */

static void mpPutBack(mpStreamPtr mpp, char *data, int len)
{
	mpp->offset -= len;
	while (len)
	{
		mpp->putback[mpp->writePos++] = *data++;
		mpp->writePos %= sizeof(mpp->putback);
		len--;
	}
}


#define APPEND(string, char) \
	{ \
		if ((string##Len + 1) < string##Space) { \
			string[string##Len++] = (char); \
		} \
	}

#define RAPPEND(string, ch) \
	{ \
		if ((string##Len + 1) == string##Space)  { \
			char *sold = string; \
			string##Space *= 2; \
			string = g_renew(char, string, string##Space); \
			if (!string) { \
				string = sold; \
				goto outOfMemory; \
			} \
		} \
		string[string##Len++] = (ch); \
	}

#define BAPPEND(ch) \
	{ \
		if (out) { \
			RAPPEND(out, ch); \
		} \
	}

/* ------------------------------------------------------------------------- */

static void decomposeValue(char *value, char *mvalue, int mvalueSpace, const char *const *argNames, char **argValues, int argValueSpace)
{
	char argName[1024];
	int argNameSpace = sizeof(argName);
	int argNameLen = 0;
	int mvalueLen = 0;
	char *argValue;
	int argNum = 0;

	while (argNames[argNum])
	{
		if (argValueSpace)
		{
			argValues[argNum][0] = '\0';
		}
		argNum++;
	}
	while (g_ascii_isspace(*value))
	{
		value++;
	}
	/* Quoted mvalue */
	if (*value == '\"')
	{
		value++;
		while ((*value) && (*value != '\"'))
		{
			APPEND(mvalue, *value);
			value++;
		}
		while ((*value) && (*value != ';'))
		{
			value++;
		}
	} else
	{
		/* Unquoted mvalue */
		while ((*value) && (*value != ';'))
		{
			APPEND(mvalue, *value);
			value++;
		}
	}
	if (mvalueSpace)
	{
		mvalue[mvalueLen] = '\0';
	}
	while (*value == ';')
	{
		int argNum;
		int argValueLen = 0;

		/* Skip the ; between parameters */
		value++;
		/* Now skip leading whitespace */
		while ((*value) && (g_ascii_isspace(*value)))
		{
			value++;
		}
		/* Now read the parameter name */
		argNameLen = 0;
		while ((*value) && (isalnum(*value)))
		{
			APPEND(argName, *value);
			value++;
		}
		if (argNameSpace)
		{
			argName[argNameLen] = '\0';
		}
		while ((*value) && g_ascii_isspace(*value))
		{
			value++;
		}
		if (*value != '=')
		{
			/* Malformed line */
			return;
		}
		value++;
		while ((*value) && g_ascii_isspace(*value))
		{
			value++;
		}
		/* Find the parameter in the argument list, if present */
		argNum = 0;
		argValue = 0;
		while (argNames[argNum])
		{
			if (g_ascii_strcasecmp(argName, argNames[argNum]) == 0)
			{
				argValue = argValues[argNum];
				break;
			}
			argNum++;
		}
		/* Finally, read the parameter value */
		if (*value == '\"')
		{
			value++;
			while ((*value) && (*value != '\"'))
			{
				if (argValue)
				{
					APPEND(argValue, *value);
				}
				value++;
			}
			while ((*value) && (*value != ';'))
			{
				value++;
			}
		} else
		{
			/* Unquoted value */
			while ((*value) && (*value != ';'))
			{
				if (argNames[argNum])
				{
					APPEND(argValue, *value);
				}
				value++;
			}
		}
		if (argValueSpace)
		{
			argValue[argValueLen] = '\0';
		}
	}
}

/* ------------------------------------------------------------------------- */

static cgiParseResultType afterNextBoundary(mpStreamPtr mpp, char **outP, int *bodyLengthP, int first)
{
	int outLen = 0;
	int outSpace = 256;
	char *out = 0;
	cgiParseResultType result;
	int boffset;
	int got;
	char d[2];

	/* This is large enough, because the buffer into which the
	   original boundary string is fetched is shorter by more
	   than four characters due to the space required for
	   the attribute name */
	char workingBoundaryData[1024];
	char *workingBoundary = workingBoundaryData;
	int workingBoundaryLength;

	if (outP)
	{
		out = g_new(char, outSpace);
		if (!out)
		{
			goto outOfMemory;
		}
	}
	boffset = 0;
	sprintf(workingBoundaryData, "\r\n--%s", cgiMultipartBoundary);
	if (first)
	{
		workingBoundary = workingBoundaryData + 2;
	}
	workingBoundaryLength = strlen(workingBoundary);
	for (;;)
	{
		got = mpRead(mpp, d, 1);
		if (got != 1)
		{
			/* 2.01: cgiParseIO, not cgiFormIO */
			result = cgiParseIO;
			goto error;
		}
		if (d[0] == workingBoundary[boffset])
		{
			/* We matched the next byte of the boundary.
			   Keep track of our progress into the
			   boundary and don't emit anything. */
			boffset++;
			if (boffset == workingBoundaryLength)
			{
				break;
			}
		} else if (boffset > 0)
		{
			/* We matched part, but not all, of the
			   boundary. Now we have to be careful:
			   put back all except the first
			   character and try again. The 
			   real boundary could begin in the
			   middle of a false match. We can
			   emit the first character only so far. */
			BAPPEND(workingBoundary[0]);
			mpPutBack(mpp, workingBoundary + 1, boffset - 1);
			mpPutBack(mpp, d, 1);
			boffset = 0;
		} else
		{
			/* Not presently in the middle of a boundary
			   match; just emit the character. */
			BAPPEND(d[0]);
		}
	}
	/* Read trailing newline or -- EOF marker. A literal EOF here
	   would be an error in the input stream. */
	got = mpRead(mpp, d, 2);
	if (got != 2)
	{
		result = cgiParseIO;
		goto error;
	}
	if ((d[0] == '\r') && (d[1] == '\n'))
	{
		/* OK, EOL */
	} else if (d[0] == '-')
	{
		/* Probably EOF, but we check for
		   that later */
		mpPutBack(mpp, d, 2);
	}
	if (out && outSpace)
	{
		char *oout = out;

		out[outLen] = '\0';
		out = g_renew(char, out, outLen + 1);
		if (!out)
		{
			/* Surprising if it happens; and not fatal! We were
			   just trying to give some space back. We can
			   keep it if we have to. */
			out = oout;
		}
		*outP = out;
	}
	if (bodyLengthP)
	{
		*bodyLengthP = outLen;
	}
	return cgiParseSuccess;
  outOfMemory:
	result = cgiParseMemory;
	if (outP)
	{
		if (out)
		{
			g_free(out);
		}
		*outP = '\0';
	}
  error:
	if (bodyLengthP)
	{
		*bodyLengthP = 0;
	}
	if (out)
	{
		g_free(out);
	}
	if (outP)
	{
		*outP = 0;
	}
	return result;
}

/* ------------------------------------------------------------------------- */

static int readHeaderLine(mpStreamPtr mpp, char *attr, int attrSpace, char *value, int valueSpace)
{
	int attrLen = 0;
	int valueLen = 0;
	int valueFound = 0;

	for (;;)
	{
		char d[1];

		int got = mpRead(mpp, d, 1);

		if (got != 1)
		{
			return 0;
		}
		if (d[0] == '\r')
		{
			got = mpRead(mpp, d, 1);
			if (got == 1)
			{
				if (d[0] == '\n')
				{
					/* OK */
				} else
				{
					mpPutBack(mpp, d, 1);
				}
			}
			break;
		} else if (d[0] == '\n')
		{
			break;
		} else if ((d[0] == ':') && attrLen)
		{
			valueFound = 1;
			while (mpRead(mpp, d, 1) == 1)
			{
				if (!g_ascii_isspace(d[0]))
				{
					mpPutBack(mpp, d, 1);
					break;
				}
			}
		} else if (!valueFound)
		{
			if (!g_ascii_isspace(*d))
			{
				if (attrLen < (attrSpace - 1))
				{
					attr[attrLen++] = *d;
				}
			}
		} else if (valueFound)
		{
			if (valueLen < (valueSpace - 1))
			{
				value[valueLen++] = *d;
			}
		}
	}
	if (attrSpace)
	{
		attr[attrLen] = '\0';
	}
	if (valueSpace)
	{
		value[valueLen] = '\0';
	}
	if (attrLen && valueLen)
	{
		return 1;
	} else
	{
		return 0;
	}
}

/* ------------------------------------------------------------------------- */

/* This function copies the body to outf if it is not null, otherwise to
	a newly allocated character buffer at *outP, which will be null
	terminated; if both outf and outP are null the body is not stored.
	If bodyLengthP is not null, the size of the body in bytes is stored
	to *bodyLengthP, not including any terminating null added to *outP. 
	If 'first' is nonzero, a preceding newline is not expected before
	the boundary. If 'first' is zero, a preceding newline is expected.
	Upon return mpp is positioned after the boundary and its trailing 
	newline, if any; if the boundary is followed by -- the next two 
	characters read after this function returns will be --. Upon error, 
	if outP is not null, *outP is a null pointer; *bodyLengthP 
	is set to zero. Returns cgiParseSuccess, cgiParseMemory 
	or cgiParseIO. */

static cgiParseResultType cgiParsePostMultipartInput(void)
{
	cgiParseResultType result;
	cgiFormEntry *n = 0, *l = 0;
	int got;
	char *out = 0;
	mpStream mp;
	mpStreamPtr mpp = &mp;

	memset(&mp, 0, sizeof(mp));
	if (!cgiContentLength)
	{
		return cgiParseSuccess;
	}
	/* Read first boundary, including trailing newline */
	result = afterNextBoundary(mpp, NULL, NULL, 1);
	if (result == cgiParseIO)
	{
		/* An empty submission is not necessarily an error */
		return cgiParseSuccess;
	} else if (result != cgiParseSuccess)
	{
		return result;
	}
	for (;;)
	{
		char d[1024];
		char fvalue[1024];
		char fname[1024];
		int bodyLength = 0;
		char ffileName[1024];
		char fcontentType[1024];
		char attr[1024];
		char value[1024];

		fvalue[0] = 0;
		fname[0] = 0;
		ffileName[0] = 0;
		fcontentType[0] = 0;
		out = 0;
		/* Check for EOF */
		got = mpRead(mpp, d, 2);
		if (got < 2)
		{
			/* Crude EOF */
			break;
		}
		if (d[0] == '-' && d[1] == '-')
		{
			/* Graceful EOF */
			break;
		}
		mpPutBack(mpp, d, 2);
		/* Read header lines until end of header */
		while (readHeaderLine(mpp, attr, sizeof(attr), value, sizeof(value)))
		{
			const char *argNames[3];
			char *argValues[2];

			g_string_append_printf(cgiOut, "<!-- %s %s -->\n", attr, value);
			/* Content-Disposition: form-data; 
			   name="test"; filename="googley.gif" */
			if (g_ascii_strcasecmp(attr, "Content-Disposition") == 0)
			{
				argNames[0] = "name";
				argNames[1] = "filename";
				argNames[2] = 0;
				argValues[0] = fname;
				argValues[1] = ffileName;
				decomposeValue(value, fvalue, sizeof(fvalue), argNames, argValues, 1024);
			} else if (g_ascii_strcasecmp(attr, "Content-Type") == 0)
			{
				argNames[0] = 0;
				decomposeValue(value, fcontentType, sizeof(fcontentType), argNames, 0, 0);
			}
		}
		if (g_ascii_strcasecmp(fvalue, "form-data") != 0)
		{
			/* Not form data */
			continue;
		}
		/* Body is everything from here until the next 
		   boundary. So, set it aside and move past boundary. 
		   If a filename was submitted as part of the
		   disposition header, store to a temporary file.
		   Otherwise, store to a memory buffer (it is
		   presumably a regular form field). */
		result = afterNextBoundary(mpp, &out, &bodyLength, 0);
		if (result != cgiParseSuccess)
		{
			/* Lack of a boundary here is an error. */
			g_free(out);
			return result;
		}
		/* OK, we have a new pair, add it to the list. */
		n = g_new0(cgiFormEntry, 1);
		if (!n)
		{
			goto outOfMemory;
		}
		n->attr = g_strdup(fname);
		if (!n->attr)
		{
			goto outOfMemory;
		}
		if (out)
		{
			n->value = out;
			out = 0;
		}
		n->valueLength = bodyLength;
		n->next = 0;
		if (!l)
		{
			cgiFormEntryFirst = n;
		} else
		{
			l->next = n;
		}
		n->fileName = g_strdup(ffileName);
		if (!n->fileName)
		{
			goto outOfMemory;
		}
		n->contentType = g_strdup(fcontentType);
		if (!n->contentType)
		{
			goto outOfMemory;
		}

		l = n;
	}
	return cgiParseSuccess;
  outOfMemory:
	if (n)
	{
		g_free(n->attr);
		g_free(n->value);
		g_free(n->fileName);
		g_free(n->contentType);
		g_free(n);
	}
	g_free(out);
	return cgiParseMemory;
}

/* ------------------------------------------------------------------------- */

static const char *cgiFindTarget = 0;

static cgiFormEntry *cgiFindPos = 0;

static cgiFormEntry *cgiFormEntryFindNext(void)
{
	while (cgiFindPos)
	{
		cgiFormEntry *c = cgiFindPos;

		cgiFindPos = c->next;
		if (strcmp(c->attr, cgiFindTarget) == 0)
		{
			return c;
		}
	}
	return 0;
}

/* ------------------------------------------------------------------------- */

static cgiFormEntry *cgiFormEntryFindFirst(const char *name)
{
	cgiFindTarget = name;
	cgiFindPos = cgiFormEntryFirst;
	return cgiFormEntryFindNext();
}

/* ------------------------------------------------------------------------- */

static cgiFormResultType cgiFormEntryString(cgiFormEntry *e, char *result, int max)
{
	char *dp, *sp;
	int truncated = 0;
	int len = 0;
	int avail = max - 1;

	dp = result;
	sp = e->value;
	for (;;)
	{
		int ch;

		ch = *sp;
		if (ch == '\0')
		{
			/* The end of the source string */
			break;
		}
		if (len >= avail)
		{
			truncated = 1;
			break;
		}
		*dp = ch;
		dp++;
		len++;
		sp++;
	}
	*dp = '\0';
	if (truncated)
	{
		return cgiFormTruncated;
	} else if (!len)
	{
		return cgiFormEmpty;
	} else
	{
		return cgiFormSuccess;
	}
}

/* ------------------------------------------------------------------------- */

char *cgiFormString(const char *name)
{
	cgiFormEntry *e;
	char *res;

	e = cgiFormEntryFindFirst(name);
	if (e == NULL || e->value == NULL)
		return NULL;
	res = g_new(char, e->valueLength + 1);
	cgiFormEntryString(e, res, e->valueLength + 1);
	return res;
}

/* ------------------------------------------------------------------------- */

char *cgiFormFileName(const char *name, int *bodyLength)
{
	cgiFormEntry *e;

	e = cgiFormEntryFindFirst(name);
	if (e == NULL)
	{
		if (bodyLength)
			*bodyLength = 0;
		return NULL;
	}
	if (bodyLength)
		*bodyLength = e->valueLength;
	if (e->fileName == NULL)
		return g_strdup("");
	return g_strdup(e->fileName);
}

/* ------------------------------------------------------------------------- */

const char *cgiFormFileData(const char *name, int *bodyLength)
{
	cgiFormEntry *e;

	e = cgiFormEntryFindFirst(name);
	if (e == NULL)
	{
		if (bodyLength)
			*bodyLength = 0;
		return NULL;
	}
	if (bodyLength)
		*bodyLength = e->valueLength;
	return e->value;
}

/* ------------------------------------------------------------------------- */

char *cgiFormFileContentType(const char *name)
{
	cgiFormEntry *e;

	e = cgiFormEntryFindFirst(name);
	if (e == NULL)
		return NULL;
	if (e->contentType == NULL)
		return g_strdup("");
	return g_strdup(e->contentType);
}

/* ------------------------------------------------------------------------- */

cgiFormResultType cgiFormFileSize(const char *name, int *sizeP)
{
	cgiFormEntry *e;

	e = cgiFormEntryFindFirst(name);
	if (!e)
	{
		if (sizeP)
		{
			*sizeP = 0;
		}
		return cgiFormNotFound;
	} else
	{
		if (sizeP)
		{
			*sizeP = e->valueLength;
		}
		return cgiFormSuccess;
	}
}

/* ------------------------------------------------------------------------- */

cgiFormResultType cgiFormStringMultiple(const char *name, char ***result)
{
	char **stringArray;
	cgiFormEntry *e;
	int i;
	int total = 0;

	/* Make two passes. One would be more efficient, but this
	   function is not commonly used. The select menu and
	   radio box functions are faster. */
	e = cgiFormEntryFindFirst(name);
	if (e != 0)
	{
		do
		{
			total++;
		} while ((e = cgiFormEntryFindNext()) != 0);
	}
	*result = 0;
	/* Now go get the entries */
	e = cgiFormEntryFindFirst(name);
	if (e)
	{
		stringArray = g_new0(char *, total + 1);
		if (!stringArray)
		{
			return cgiFormMemory;
		}
		i = 0;
		do
		{
			int max = (int) (strlen(e->value) + 1);

			stringArray[i] = g_new(char, max);
			if (stringArray[i] == 0)
			{
				/* Memory problems */
				cgiStringArrayFree(stringArray);
				*result = 0;
				return cgiFormMemory;
			}
			strcpy(stringArray[i], e->value);
			cgiFormEntryString(e, stringArray[i], max);
			i++;
		} while ((e = cgiFormEntryFindNext()) != 0);
		*result = stringArray;
		return cgiFormSuccess;
	} else
	{
		return cgiFormNotFound;
	}
}

/* ------------------------------------------------------------------------- */

cgiFormResultType cgiFormStringSpaceNeeded(const char *name, int *result)
{
	cgiFormEntry *e;

	e = cgiFormEntryFindFirst(name);
	if (!e)
	{
		*result = 1;
		return cgiFormNotFound;
	}
	*result = ((int) strlen(e->value)) + 1;
	return cgiFormSuccess;
}

/* ------------------------------------------------------------------------- */

static int cgiFirstNonspaceChar(const char *s)
{
	int len = strspn(s, " \n\r\t");

	return s[len];
}

/* ------------------------------------------------------------------------- */

cgiFormResultType cgiFormInteger(const char *name, int *result, int defaultV)
{
	cgiFormEntry *e;

	int ch;

	e = cgiFormEntryFindFirst(name);
	if (!e)
	{
		*result = defaultV;
		return cgiFormNotFound;
	}
	if (!strlen(e->value))
	{
		*result = defaultV;
		return cgiFormEmpty;
	}
	ch = cgiFirstNonspaceChar(e->value);
	if (!(g_ascii_isdigit(ch)) && (ch != '-') && (ch != '+'))
	{
		*result = defaultV;
		return cgiFormBadType;
	} else
	{
		*result = (int)strtol(e->value, NULL, 10);
		return cgiFormSuccess;
	}
}

/* ------------------------------------------------------------------------- */

cgiFormResultType cgiFormIntegerBounded(const char *name, int *result, int min, int max, int defaultV)
{
	cgiFormResultType error = cgiFormInteger(name, result, defaultV);

	if (error != cgiFormSuccess)
	{
		return error;
	}
	if (*result < min)
	{
		*result = min;
		return cgiFormConstrained;
	}
	if (*result > max)
	{
		*result = max;
		return cgiFormConstrained;
	}
	return cgiFormSuccess;
}

/* ------------------------------------------------------------------------- */

cgiFormResultType cgiFormDouble(const char *name, double *result, double defaultV)
{
	cgiFormEntry *e;
	int ch;

	e = cgiFormEntryFindFirst(name);
	if (!e)
	{
		*result = defaultV;
		return cgiFormNotFound;
	}
	if (!strlen(e->value))
	{
		*result = defaultV;
		return cgiFormEmpty;
	}
	ch = cgiFirstNonspaceChar(e->value);
	if (!(g_ascii_isdigit(ch)) && (ch != '.') && (ch != '-') && (ch != '+'))
	{
		*result = defaultV;
		return cgiFormBadType;
	} else
	{
		*result = atof(e->value);
		return cgiFormSuccess;
	}
}

/* ------------------------------------------------------------------------- */

cgiFormResultType cgiFormDoubleBounded(const char *name, double *result, double min, double max, double defaultV)
{
	cgiFormResultType error = cgiFormDouble(name, result, defaultV);

	if (error != cgiFormSuccess)
	{
		return error;
	}
	if (*result < min)
	{
		*result = min;
		return cgiFormConstrained;
	}
	if (*result > max)
	{
		*result = max;
		return cgiFormConstrained;
	}
	return cgiFormSuccess;
}

/* ------------------------------------------------------------------------- */

cgiFormResultType cgiFormSelectSingle(const char *name, char **choicesText, int choicesTotal, int *result, int defaultV)
{
	cgiFormEntry *e;
	int i;

	e = cgiFormEntryFindFirst(name);
	if (!e)
	{
		*result = defaultV;
		return cgiFormNotFound;
	}
	for (i = 0; (i < choicesTotal); i++)
	{
		if (cgiStrEq(choicesText[i], e->value))
		{
			*result = i;
			return cgiFormSuccess;
		}
	}
	*result = defaultV;
	return cgiFormNoSuchChoice;
}

/* ------------------------------------------------------------------------- */

cgiFormResultType cgiFormSelectMultiple(const char *name, char **choicesText, int choicesTotal, int *result, int *invalid)
{
	cgiFormEntry *e;
	int i;
	int hits = 0;
	int invalidE = 0;

	for (i = 0; (i < choicesTotal); i++)
	{
		result[i] = 0;
	}
	e = cgiFormEntryFindFirst(name);
	if (!e)
	{
		*invalid = invalidE;
		return cgiFormNotFound;
	}
	do
	{
		int hit = 0;

		for (i = 0; (i < choicesTotal); i++)
		{
			if (cgiStrEq(choicesText[i], e->value))
			{
				result[i] = 1;
				hits++;
				hit = 1;
				break;
			}
		}
		if (!(hit))
		{
			invalidE++;
		}
	} while ((e = cgiFormEntryFindNext()) != 0);

	*invalid = invalidE;

	if (hits)
	{
		return cgiFormSuccess;
	} else
	{
		return cgiFormNotFound;
	}
}

/* ------------------------------------------------------------------------- */

cgiFormResultType cgiFormCheckboxSingle(const char *name)
{
	cgiFormEntry *e;

	e = cgiFormEntryFindFirst(name);
	if (!e)
	{
		return cgiFormNotFound;
	}
	return cgiFormSuccess;
}

/* ------------------------------------------------------------------------- */

cgiFormResultType cgiFormCheckboxMultiple(const char *name, char **valuesText, int valuesTotal, int *result, int *invalid)
{
	/* Implementation is identical to cgiFormSelectMultiple. */
	return cgiFormSelectMultiple(name, valuesText, valuesTotal, result, invalid);
}

/* ------------------------------------------------------------------------- */

cgiFormResultType cgiFormRadio(const char *name, char **valuesText, int valuesTotal, int *result, int defaultV)
{
	/* Implementation is identical to cgiFormSelectSingle. */
	return cgiFormSelectSingle(name, valuesText, valuesTotal, result, defaultV);
}

/* ------------------------------------------------------------------------- */

cgiFormResultType cgiCookieString(const char *name, char *value, int space)
{
	const char *p = cgiCookie;

	while (*p)
	{
		const char *n = name;

		/* 2.02: if cgiCookie is exactly equal to name, this
		   can cause an overrun. The server probably wouldn't
		   allow it, since a name without values makes no sense 
		   -- but then again it might not check, so this is a
		   genuine security concern. Thanks to Nicolas 
		   Tomadakis. */
		while (*p == *n)
		{
			if ((p == '\0') && (n == '\0'))
			{
				/* Malformed cookie header from client */
				return cgiFormNotFound;
			}
			p++;
			n++;
		}
		if ((!*n) && (*p == '='))
		{
			p++;
			while ((*p != ';') && (*p != '\0') && (space > 1))
			{
				*value = *p;
				value++;
				p++;
				space--;
			}
			if (space > 0)
			{
				*value = '\0';
			}
			/* Correct parens: 2.02. Thanks to
			   Mathieu Villeneuve-Belair. */
			if (!(((*p) == ';') || ((*p) == '\0')))
			{
				return cgiFormTruncated;
			} else
			{
				return cgiFormSuccess;
			}
		} else
		{
			/* Skip to next cookie */
			while (*p)
			{
				if (*p == ';')
				{
					break;
				}
				p++;
			}
			if (!*p)
			{
				/* 2.01: default to empty */
				if (space)
				{
					*value = '\0';
				}
				return cgiFormNotFound;
			}
			p++;
			/* Allow whitespace after semicolon */
			while ((*p) && g_ascii_isspace(*p))
			{
				p++;
			}
		}
	}
	/* 2.01: actually the above loop never terminates except
	   with a return, but do this to placate gcc */
	if (space)
	{
		*value = '\0';
	}
	return cgiFormNotFound;
}

/* ------------------------------------------------------------------------- */

cgiFormResultType cgiCookieInteger(const char *name, int *result, int defaultV)
{
	char buffer[256];

	cgiFormResultType r = cgiCookieString(name, buffer, sizeof(buffer));

	if (r != cgiFormSuccess)
	{
		*result = defaultV;
	} else
	{
		*result = (int)strtol(buffer, NULL, 0);
	}
	return r;
}

/* ------------------------------------------------------------------------- */

void cgiHeaderCookieSetInteger(FILE *out, const char *name, int value, int secondsToLive, const char *path, const char *domain)
{
	char svalue[256];

	sprintf(svalue, "%d", value);
	cgiHeaderCookieSetString(out, name, svalue, secondsToLive, path, domain);
}

/* ------------------------------------------------------------------------- */

void cgiHeaderCookieSetString(FILE *out, const char *name, const char *value, int secondsToLive, const char *path, const char *domain)
{
	/* cgic 2.02: simpler and more widely compatible implementation.
	   Thanks to Chunfu Lai. 
	   cgic 2.03: yes, but it didn't work. Reimplemented by
	   Thomas Boutell. ; after last element was a bug. 
	   Examples of real world cookies that really work:
	   Set-Cookie: MSNADS=UM=; domain=.slate.com; 
	   expires=Tue, 26-Apr-2022 19:00:00 GMT; path=/
	   Set-Cookie: MC1=V=3&ID=b5bc08af2b8a43ff85fcb5efd8b238f0; 
	   domain=.slate.com; expires=Mon, 04-Oct-2021 19:00:00 GMT; path=/
	 */
	time_t now;
	time_t then;
	struct tm *gt;

	ASSERT(!empty(name));
	time(&now);
	then = now + secondsToLive;
	gt = gmtime(&then);
	fprintf(out,
			"Set-Cookie: %s=%s; domain=%s; expires=%s, %02d-%s-%04d %02d:%02d:%02d GMT; path=%s\015\012",
			name, value ? value : "", domain,
			days[gt->tm_wday],
			gt->tm_mday, months[gt->tm_mon], gt->tm_year + 1900, gt->tm_hour, gt->tm_min, gt->tm_sec, empty(path) ? "/" : path);
}

/* ------------------------------------------------------------------------- */

static int cgiStrBeginsNc(const char *s1, const char *s2)
{
	return g_ascii_strncasecmp(s1, s2, strlen(s2)) == 0;
}

/* ------------------------------------------------------------------------- */

void cgiStringArrayFree(char **stringArray)
{
	char *p;
	char **arrayItself = stringArray;
	
	if (arrayItself == NULL)
		return;
	p = *stringArray;
	while (p)
	{
		g_free(p);
		stringArray++;
		p = *stringArray;
	}
	/* 2.0: free the array itself! */
	g_free(arrayItself);
}

/* ------------------------------------------------------------------------- */

cgiFormResultType cgiCookies(char ***result)
{
	char **stringArray;
	int i;
	int total = 0;
	const char *p;
	const char *n;

	p = cgiCookie;
	while (*p)
	{
		if (*p == '=')
		{
			total++;
		}
		p++;
	}
	stringArray = g_new0(char *, total + 1);
	if (!stringArray)
	{
		*result = 0;
		return cgiFormMemory;
	}
	i = 0;
	p = cgiCookie;
	while (*p)
	{
		while (*p && g_ascii_isspace(*p))
		{
			p++;
		}
		n = p;
		while (*p && (*p != '='))
		{
			p++;
		}
		if (p != n)
		{
			stringArray[i] = g_new(char, (p - n) + 1);
			if (!stringArray[i])
			{
				cgiStringArrayFree(stringArray);
				*result = 0;
				return cgiFormMemory;
			}
			memcpy(stringArray[i], n, p - n);
			stringArray[i][p - n] = '\0';
			i++;
		}
		while (*p && (*p != ';'))
		{
			p++;
		}
		if (!*p)
		{
			break;
		}
		if (*p == ';')
		{
			p++;
		}
	}
	*result = stringArray;
	return cgiFormSuccess;
}

/* ------------------------------------------------------------------------- */

cgiFormResultType cgiFormEntries(char ***result)
{
	char **stringArray;
	cgiFormEntry *e, *pe;
	int i;
	int total = 0;

	e = cgiFormEntryFirst;
	while (e)
	{
		/* Don't count a field name more than once if
		   multiple values happen to be present for it */
		pe = cgiFormEntryFirst;
		while (pe != e)
		{
			if (!strcmp(e->attr, pe->attr))
			{
				goto skipSecondValue;
			}
			pe = pe->next;
		}
		total++;
	  skipSecondValue:
		e = e->next;
	}
	stringArray = g_new0(char *, total + 1);
	if (!stringArray)
	{
		*result = 0;
		return cgiFormMemory;
	}
	/* Now go get the entries */
	e = cgiFormEntryFirst;
	i = 0;
	while (e)
	{
		/* Don't return a field name more than once if
		   multiple values happen to be present for it */
		pe = cgiFormEntryFirst;
		while (pe != e)
		{
			if (!strcmp(e->attr, pe->attr))
			{
				goto skipSecondValue2;
			}
			pe = pe->next;
		}
		stringArray[i] = g_strdup(e->attr);
		if (stringArray[i] == 0)
		{
			/* Memory problems */
			cgiStringArrayFree(stringArray);
			*result = 0;
			return cgiFormMemory;
		}
		i++;
	  skipSecondValue2:
		e = e->next;
	}
	*result = stringArray;
	return cgiFormSuccess;
}

/* ------------------------------------------------------------------------- */

#define TRYPUTC(ch) \
	{ \
		if (g_string_append_c(cgiOut, (ch)) == NULL) { \
			return cgiFormIO; \
		} \
	}

cgiFormResultType cgiHtmlEscapeData(const char *data, int len)
{
	while (len--)
	{
		if (*data == '<')
		{
			TRYPUTC('&');
			TRYPUTC('l');
			TRYPUTC('t');
			TRYPUTC(';');
		} else if (*data == '&')
		{
			TRYPUTC('&');
			TRYPUTC('a');
			TRYPUTC('m');
			TRYPUTC('p');
			TRYPUTC(';');
		} else if (*data == '>')
		{
			TRYPUTC('&');
			TRYPUTC('g');
			TRYPUTC('t');
			TRYPUTC(';');
		} else if (*data == '"')
		{
			TRYPUTC('&');
			TRYPUTC('q');
			TRYPUTC('u');
			TRYPUTC('o');
			TRYPUTC('t');
			TRYPUTC(';');
		} else
		{
			TRYPUTC(*data);
		}
		data++;
	}
	return cgiFormSuccess;
}

/* ------------------------------------------------------------------------- */

cgiFormResultType cgiHtmlEscape(const char *s)
{
	return cgiHtmlEscapeData(s, (int) strlen(s));
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

static void cgiSetupConstants(void)
{
	int i;

	for (i = 0; (i < 256); i++)
	{
		cgiHexValue[i] = 0;
	}
	cgiHexValue['0'] = 0;
	cgiHexValue['1'] = 1;
	cgiHexValue['2'] = 2;
	cgiHexValue['3'] = 3;
	cgiHexValue['4'] = 4;
	cgiHexValue['5'] = 5;
	cgiHexValue['6'] = 6;
	cgiHexValue['7'] = 7;
	cgiHexValue['8'] = 8;
	cgiHexValue['9'] = 9;
	cgiHexValue['A'] = 10;
	cgiHexValue['B'] = 11;
	cgiHexValue['C'] = 12;
	cgiHexValue['D'] = 13;
	cgiHexValue['E'] = 14;
	cgiHexValue['F'] = 15;
	cgiHexValue['a'] = 10;
	cgiHexValue['b'] = 11;
	cgiHexValue['c'] = 12;
	cgiHexValue['d'] = 13;
	cgiHexValue['e'] = 14;
	cgiHexValue['f'] = 15;
}

/* ------------------------------------------------------------------------- */

static void cgiFreeResources(void)
{
	cgiFormEntry *c = cgiFormEntryFirst;
	cgiFormEntry *n;

	while (c)
	{
		n = c->next;
		g_free(c->attr);
		g_free(c->value);
		g_free(c->fileName);
		g_free(c->contentType);
		g_free(c);
		c = n;
	}
	/* 2.0: to clean up the environment for cgiReadEnvironment,
	   we must set these correctly */
	cgiFormEntryFirst = 0;
}

/* ------------------------------------------------------------------------- */

int cgiInit(GString *out)
{
	const char *cgiContentLengthString;
	const char *e;

	cgiOut = out;
	
	cgiSetupConstants();
	cgiGetenv(&cgiServerSoftware, "SERVER_SOFTWARE");
	cgiGetenv(&cgiServerName, "SERVER_NAME");
	cgiGetenv(&cgiGatewayInterface, "GATEWAY_INTERFACE");
	cgiGetenv(&cgiServerProtocol, "SERVER_PROTOCOL");
	cgiGetenv(&cgiServerPort, "SERVER_PORT");
	cgiGetenv(&cgiRequestMethod, "REQUEST_METHOD");
	if (cgiRequestMethod == NULL)
		cgiRequestMethod = "GET";
	cgiGetenv(&cgiPathInfo, "PATH_INFO");
	cgiGetenv(&cgiPathTranslated, "PATH_TRANSLATED");
	cgiGetenv(&cgiScriptName, "SCRIPT_NAME");
	cgiGetenv(&cgiScriptFilename, "SCRIPT_FILENAME");
	cgiGetenv(&cgiQueryString, "QUERY_STRING");
	cgiGetenv(&cgiRemoteHost, "REMOTE_HOST");
	cgiGetenv(&cgiRemoteAddr, "REMOTE_ADDR");
	if (empty(cgiRemoteHost))
		cgiRemoteHost = cgiRemoteAddr;
	cgiGetenv(&cgiAuthType, "AUTH_TYPE");
	cgiGetenv(&cgiRemoteUser, "REMOTE_USER");
	cgiGetenv(&cgiRemoteIdent, "REMOTE_IDENT");
	cgiGetenv(&cgiDocumentRoot, "DOCUMENT_ROOT");
	/* 2.0: the content type string needs to be parsed and modified, so
	   copy it to a buffer. */
	e = getenv("CONTENT_TYPE");
	if (e)
	{
		/* Truncate safely in the event of what is almost certainly
		   a hack attempt */
		strncpy(cgiContentTypeData, e, sizeof(cgiContentTypeData));
		cgiContentTypeData[sizeof(cgiContentTypeData) - 1] = '\0';
	} else
	{
		cgiContentTypeData[0] = '\0';
	}
	cgiContentType = cgiContentTypeData;
	
	/* Never null */
	cgiMultipartBoundary = "";
	/* 2.0: parse semicolon-separated additional parameters of the
	   content type. The one we're interested in is 'boundary'.
	   We discard the rest to make cgiContentType more useful
	   to the typical programmer. */
	if (strchr(cgiContentType, ';'))
	{
		char *sat = strchr(cgiContentType, ';');

		while (sat)
		{
			*sat = '\0';
			sat++;
			while (g_ascii_isspace(*sat))
			{
				sat++;
			}
			if (cgiStrBeginsNc(sat, "boundary="))
			{
				const char *s;

				cgiMultipartBoundary = sat + strlen("boundary=");
				s = cgiMultipartBoundary;
				while ((*s) && (!g_ascii_isspace(*s)))
				{
					s++;
				}
				cgiContentTypeData[s - cgiContentTypeData] = '\0';
				break;
			} else
			{
				sat = strchr(sat, ';');
			}
		}
	}
	cgiGetenv(&cgiContentLengthString, "CONTENT_LENGTH");
	cgiContentLength = cgiContentLengthString ? strtol(cgiContentLengthString, NULL, 0) : 0;
	cgiGetenv(&cgiAccept, "HTTP_ACCEPT");
	cgiGetenv(&cgiUserAgent, "HTTP_USER_AGENT");
	cgiGetenv(&cgiReferrer, "HTTP_REFERER");
	cgiGetenv(&cgiCookie, "HTTP_COOKIE");
#ifdef _WIN32
		/* 1.07: Must set stdin and stdout to binary mode */
		/* 2.0: this is particularly crucial now and must not be removed */
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);
#endif
	cgiFormEntryFirst = 0;

	if (g_ascii_strcasecmp(cgiRequestMethod, "post") == 0)
	{
		if (g_ascii_strcasecmp(cgiContentType, "application/x-www-form-urlencoded") == 0)
		{
			if (cgiParsePostFormInput() != cgiParseSuccess)
			{
				cgiFreeResources();
				return FALSE;
			}
		} else if (g_ascii_strcasecmp(cgiContentType, "multipart/form-data") == 0)
		{
			if (cgiParsePostMultipartInput() != cgiParseSuccess)
			{
				cgiFreeResources();
				return FALSE;
			}
		}
	} else if (g_ascii_strcasecmp(cgiRequestMethod, "get") == 0)
	{
		/* The spec says this should be taken care of by
		   the server, but... it isn't */
		cgiContentLength = cgiQueryString ? strlen(cgiQueryString) : 0;
		if (cgiParseFormInput(cgiQueryString, cgiContentLength) != cgiParseSuccess)
		{
			cgiFreeResources();
			return FALSE;
		}
	}
	return TRUE;
}

/* ------------------------------------------------------------------------- */

void cgiExit(void)
{
	cgiFreeResources();
}
