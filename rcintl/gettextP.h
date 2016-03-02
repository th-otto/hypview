#ifndef _GETTEXTP_H
#define _GETTEXTP_H

#include <stddef.h>		/* Get size_t.  */
#include <locale.h>

#if !defined LC_MESSAGES && !(defined __LOCALE_H || (defined _LOCALE_H && defined __sun))
# define LC_MESSAGES 1729
#endif

#if !(defined _WIN32 || defined __WIN32__ || defined __CYGWIN__ || defined(__MSYS__))
typedef struct _instance *HINSTANCE;
#endif

union _strptr {
	const char *cptr;
	char *ptr;
};

#include "loadinfo.h"

#include "gmo.h"		/* Get nls_uint32.  */

#include "plural-exp.h"

/* @@ end of prolog @@ */

/* Tell the compiler when a conditional or integer expression is
   almost always true or almost always false.  */
#ifndef likely
#ifndef HAVE_BUILTIN_EXPECT
# define likely(expr) (expr)
# define unlikely(expr) (expr)
#else
# define likely(expr) __builtin_expect((expr), 1)
# define unlikely(expr) __builtin_expect((expr), 0)
#endif
#endif

#ifndef W
# define W(flag, data) ((flag) ? SWAP (data) : (data))
#endif


static __inline nls_uint32 SWAP (nls_uint32 i)
{
  return (i << 24) | ((i & 0xff00) << 8) | ((i >> 8) & 0xff00) | (i >> 24);
}


/* In-memory representation of system dependent string.  */
struct sysdep_string_desc
{
  /* Length of addressed string, including the trailing NUL.  */
  size_t length;
  /* Pointer to addressed string.  */
  const char *pointer;
};

/* The representation of an opened message catalog.  */
struct loaded_domain
{
  /* Pointer to memory containing the .mo file.  */
  struct mo_file_header *header;
  HGLOBAL hglbl;
  HRSRC hrsrc;
  /* Size of mmap()ed memory.  */
  size_t mmap_size;
  /* 1 if the .mo file uses a different endianness than this machine.  */
  int must_swap;
  /* Pointer to additional malloc()ed memory.  */
  void *malloced;

  /* Number of static strings pairs.  */
  nls_uint32 nstrings;
  /* Pointer to descriptors of original strings in the file.  */
  const struct string_desc *orig_tab;
  /* Pointer to descriptors of translated strings in the file.  */
  const struct string_desc *trans_tab;

  /* Number of system dependent strings pairs.  */
  nls_uint32 n_sysdep_strings;
  /* Pointer to descriptors of original sysdep strings.  */
  const struct sysdep_string_desc *orig_sysdep_tab;
  /* Pointer to descriptors of translated sysdep strings.  */
  const struct sysdep_string_desc *trans_sysdep_tab;

  /* Size of hash table.  */
  nls_uint32 hash_size;
  /* Pointer to hash table.  */
  const nls_uint32 *hash_tab;
  /* 1 if the hash table uses a different endianness than this machine.  */
  int must_swap_hash_tab;

  union {
  	const struct expression *cptr;
  	struct expression *ptr;
  } plural;
  unsigned long int nplurals;
};

/* We want to allocate a string at the end of the struct.  But ISO C
   doesn't allow zero sized arrays.  */
#ifdef __GNUC__
# define ZERO 0
#else
# define ZERO 1
#endif

/* A set of settings bound to a message domain.  Used to store settings
   from bindtextdomain() and bind_textdomain_codeset().  */
struct binding
{
  struct binding *next;
  HINSTANCE instance;
  union _strptr codeset;
  char domainname[ZERO];
};

/* A counter which is incremented each time some previous translations
   become invalid.
   This variable is part of the external ABI of the GNU libintl.  */
extern int _rc_msg_cat_cntr;

char *gl_locale_name_posify (const char *name);
#if defined _WIN32 || defined __WIN32__ || defined __CYGWIN__ || defined(__MSYS__)
const char *gl_locale_name_from_win32_LCID (LCID lcid);
const char *gl_locale_name_from_win32_LANGID (LANGID langid);
#endif
const char *gl_locale_name_thread_unsafe (int category, const char *categoryname);
const char *gl_locale_name_thread (int category, const char *categoryname);
const char *gl_locale_name_posix (int category, const char *categoryname);
const char *gl_locale_name_environ (int category, const char *categoryname);
const char *gl_locale_name_default (void);
const char *gl_locale_name (int category, const char *categoryname);

struct loaded_l10nfile *_rc_find_domain (HINSTANCE __dirname, char *__locale,
					 const char *__domainname,
					 struct binding *__domainbinding);
void _rc_load_domain (struct loaded_l10nfile *__domain, struct binding *__domainbinding);

const char *_rc_find_msg (struct loaded_l10nfile *domain_file, struct binding *domainbinding, const char *msgid, size_t *lengthp);

#define GETTEXT rc_gettext
#define DGETTEXT rc_dgettext
#define NGETTEXT rc_ngettext
#define DNGETTEXT rc_dngettext
#define TEXTDOMAIN rc_textdomain
#define BINDTEXTDOMAIN rc_bindtextdomain
#define BIND_TEXTDOMAIN_CODESET rc_bind_textdomain_codeset

#define DCIGETTEXT rc_dcigettext

const char *GETTEXT(const char *__msgid);
const char *DGETTEXT(const char *__domainname, const char *__msgid);
const char *NGETTEXT(const char *__msgid1, const char *__msgid2, unsigned long int __n);
const char *DNGETTEXT(const char *__domainname, const char *__msgid1, const char *__msgid2, unsigned long int __n);
const char *TEXTDOMAIN(const char *__domainname);
HINSTANCE BINDTEXTDOMAIN(const char *__domainname, HINSTANCE __dirname);
const char *BIND_TEXTDOMAIN_CODESET(const char *__domainname, const char *__codeset);

const char *DCIGETTEXT(const char *__domainname, const char *__msgid1, const char *__msgid2, int __plural, unsigned long int __n);

/* Contains the default location of the message catalogs.  */
HINSTANCE _rc_default_dirname(void);

/* List with bindings of specific domains.  */
extern struct binding *_rc_domain_bindings;

/* Name of the default text domain.  */
extern const char _rc_default_default_domain[];

/* Default text domain in which entries for gettext(3) are to be found.  */
extern union _strptr _rc_current_default_domain;

void _rc_free_mem(void);
void _rc_finddomain_subfreeres(void);

#endif /* gettextP.h  */
