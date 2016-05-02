/* The CGI_C library, by Thomas Boutell, version 2.01. CGI_C is intended
	to be a high-quality API to simplify CGI programming tasks. */

/* Make sure this is only included once. */

#ifndef CGI_C
#define CGI_C 1

/* Bring in standard I/O since some of the functions refer to
	types defined by it, such as FILE *. */

#include <stdio.h>

/* The various CGI environment variables. Instead of using getenv(),
	the programmer should refer to these, which are always
	valid null-terminated strings (they may be empty, but they 
	will never be null). If these variables are used instead
	of calling getenv(), then it will be possible to save
	and restore CGI environments, which is highly convenient
	for debugging. */

extern const char *cgiServerSoftware;
extern const char *cgiServerName;
extern const char *cgiGatewayInterface;
extern const char *cgiServerProtocol;
extern const char *cgiServerPort;
extern const char *cgiRequestMethod;
extern const char *cgiPathInfo;
extern const char *cgiPathTranslated;
extern const char *cgiScriptName;
extern const char *cgiScriptFilename;
extern const char *cgiQueryString;
extern const char *cgiRemoteHost;
extern const char *cgiRemoteAddr;
extern const char *cgiAuthType;
extern const char *cgiRemoteUser;
extern const char *cgiRemoteIdent;
extern const char *cgiContentType;
extern const char *cgiAccept;
extern const char *cgiUserAgent;
extern const char *cgiReferrer;
extern const char *cgiDocumentRoot;

/* Cookies as sent to the server. You can also get them
	individually, or as a string array; see the documentation. */
extern const char *cgiCookie;

/* The number of bytes of data received.
	Note that if the submission is a form submission
	the library will read and parse all the information
	directly from cgiIn; the programmer need not do so. */

extern int cgiContentLength;

/* Possible return codes from the cgiForm family of functions (see below). */

typedef enum
{
	cgiFormSuccess,
	cgiFormTruncated,
	cgiFormBadType,
	cgiFormEmpty,
	cgiFormNotFound,
	cgiFormConstrained,
	cgiFormNoSuchChoice,
	cgiFormMemory,
	cgiFormNotAFile,
	cgiFormOpenFailed,
	cgiFormIO,
	cgiFormEOF
} cgiFormResultType;

/* These functions are used to retrieve form data. See
	cgic.html for documentation. */

char *cgiFormString(const char *name);


cgiFormResultType cgiFormStringSpaceNeeded(const char *name, int *length);


cgiFormResultType cgiFormStringMultiple(const char *name, char ***ptrToStringArray);

void cgiStringArrayFree(char **stringArray);

cgiFormResultType cgiFormInteger(const char *name, int *result, int defaultV);

cgiFormResultType cgiFormIntegerBounded(const char *name, int *result, int min, int max, int defaultV);

cgiFormResultType cgiFormDouble(const char *name, double *result, double defaultV);

cgiFormResultType cgiFormDoubleBounded(const char *name, double *result, double min, double max, double defaultV);

cgiFormResultType cgiFormSelectSingle(const char *name, char **choicesText, int choicesTotal, int *result, int defaultV);


cgiFormResultType cgiFormSelectMultiple(const char *name, char **choicesText, int choicesTotal, int *result, int *invalid);

/* Just an alias; users have asked for this */
#define cgiFormSubmitClicked cgiFormCheckboxSingle

cgiFormResultType cgiFormCheckboxSingle(const char *name);

cgiFormResultType cgiFormCheckboxMultiple(const char *name, char **valuesText, int valuesTotal, int *result, int *invalid);

cgiFormResultType cgiFormRadio(const char *name, char **valuesText, int valuesTotal, int *result, int defaultV);

/* The paths returned by this function are the original names of files
	as reported by the uploading web browser and shoult NOT be
	blindly assumed to be "safe" names for server-side use! */
char *cgiFormFileName(const char *name, int *bodyLength);

/* The content type of the uploaded file, as reported by the browser.
	It should NOT be assumed that browsers will never falsify
	such information. */
char *cgiFormFileContentType(const char *name);

cgiFormResultType cgiFormFileSize(const char *name, int *sizeP);

const char *cgiFormFileData(const char *name, int *bodyLength);

cgiFormResultType cgiCookieString(const char *name, char *result, int max);

cgiFormResultType cgiCookieInteger(const char *name, int *result, int defaultV);

cgiFormResultType cgiCookies(char ***ptrToStringArray);

/* path can be null or empty in which case a path of / (entire site) is set. 
	domain can be a single web site; if it is an entire domain, such as
	'boutell.com', it should begin with a dot: '.boutell.com' */
void cgiHeaderCookieSetString(FILE *out, const char *name, const char *value, int secondsToLive, const char *path, const char *domain);

void cgiHeaderCookieSetInteger(FILE *out, const char *name, int value, int secondsToLive, const char *path, const char *domain);

extern cgiFormResultType cgiFormEntries(char ***ptrToStringArray);

int cgiInit(GString *out);
void cgiExit(void);

/* Output string with the <, &, and > characters HTML-escaped. 
	's' is null-terminated. Returns cgiFormIO in the event
	of error, cgiFormSuccess otherwise. */
cgiFormResultType cgiHtmlEscape(const char *s);

/* Output data with the <, &, and > characters HTML-escaped. 
	'data' is not null-terminated; 'len' is the number of
	bytes in 'data'. Returns cgiFormIO in the event
	of error, cgiFormSuccess otherwise. */
cgiFormResultType cgiHtmlEscapeData(const char *data, int len);

#endif /* CGI_C */
