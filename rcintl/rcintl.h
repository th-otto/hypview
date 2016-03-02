#ifndef _LIBINTL_H
#define _LIBINTL_H 1

#include <locale.h>

/* The LC_MESSAGES locale category is the category used by the functions
   gettext() and dgettext().  It is specified in POSIX, but not in ANSI C.
   On systems that don't define it, use an arbitrary value instead.
   On Solaris, <locale.h> defines __LOCALE_H (or _LOCALE_H in Solaris 2.5)
   then includes <libintl.h> (i.e. this file!) and then only defines
   LC_MESSAGES.  To avoid a redefinition warning, don't define LC_MESSAGES
   in this case.  */
#if !defined LC_MESSAGES && !(defined __LOCALE_H || (defined _LOCALE_H && defined __sun))
# define LC_MESSAGES 1729
#endif

/* resource type for embedded *.mo-files */
#define RT_MOFILE 257

#ifndef __GNUC_PREREQ
# ifdef __GNUC__
#   define __GNUC_PREREQ(maj, min) ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
# else
#   define __GNUC_PREREQ(maj, min) 0
# endif
#endif

#ifndef __LEAF
# if __GNUC_PREREQ (4, 6) && !defined _LIBC
#  define __LEAF , __leaf__
#  define __LEAF_ATTR __attribute__ ((__leaf__))
# else
#  define __LEAF
#  define __LEAF_ATTR
# endif
#endif

#ifndef __THROW
# if !defined __cplusplus && __GNUC_PREREQ (3, 3)
#  define __THROW	__attribute__ ((__nothrow__ __LEAF))
# else
#  if defined __cplusplus && __GNUC_PREREQ (2,8)
#   define __THROW	throw ()
#  else
#   define __THROW
#  endif
# endif
#endif

#ifndef __attribute_format_arg__
#if __GNUC_PREREQ (2,8)
# define __attribute_format_arg__(x) __attribute__ ((__format_arg__ (x)))
#else
# define __attribute_format_arg__(x) /* Ignore */
#endif
#endif

/* We define an additional symbol to signal that we use the GNU
   implementation of gettext.  */
#define __USE_GNU_GETTEXT 1

/* Provide information about the supported file formats.  Returns the
   maximum minor revision number supported for a given major revision.  */
#define __GNU_GETTEXT_SUPPORTED_REVISION(major) \
  ((major) == 0 || (major) == 1 ? 1 : -1)

#ifndef RC_INVOKED

#ifdef __cplusplus
extern "C" {
#endif

#undef gettext
#undef dgettext
#undef ngettext
#undef dngettext

#if !(defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__MSYS__) || defined(_GETTEXTP_H))
typedef struct _instance *HINSTANCE;
#endif

/* Look up MSGID in the current default message catalog for the current
   LC_MESSAGES locale.  If not found, returns MSGID itself (the default
   text).  */
extern const char *rc_gettext (const char *__msgid)
     __THROW __attribute_format_arg__ (1);

/* Look up MSGID in the DOMAINNAME message catalog for the current
   LC_MESSAGES locale.  */
extern const char *rc_dgettext (const char *__domainname, const char *__msgid)
     __THROW __attribute_format_arg__ (2);



/* Similar to `gettext' but select the plural form corresponding to the
   number N.  */
extern const char *rc_ngettext (const char *__msgid1, const char *__msgid2,
		       unsigned long int __n)
     __THROW __attribute_format_arg__ (1) __attribute_format_arg__ (2);

/* Similar to `dgettext' but select the plural form corresponding to the
   number N.  */
extern const char *rc_dngettext (const char *__domainname, const char *__msgid1,
			const char *__msgid2, unsigned long int __n)
     __THROW __attribute_format_arg__ (2) __attribute_format_arg__ (3);


/* Set the current default message catalog to DOMAINNAME.
   If DOMAINNAME is null, return the current default.
   If DOMAINNAME is "", reset to the default of "messages".  */
extern const char *rc_textdomain (const char *__domainname) __THROW;

/* Specify that the DOMAINNAME message catalog will be found
   in DIRNAME rather than in the system locale data base.  */
extern HINSTANCE rc_bindtextdomain (const char *__domainname, HINSTANCE __dirname) __THROW;

/* Specify the character encoding in which the messages from the
   DOMAINNAME message catalog will be returned.  */
extern const char *rc_bind_textdomain_codeset (const char *__domainname, const char *__codeset) __THROW;

/* Optimized version of the function above.  */
#if defined __OPTIMIZE__ && !defined __cplusplus

/* We need NULL for `gettext'.  */
# define __need_NULL
# include <stddef.h>

/* We need LC_MESSAGES for `dgettext'.  */
# include <locale.h>

/* These must be macros.  Inlined functions are useless because the
   `__builtin_constant_p' predicate in dcgettext would always return
   false.  */

# define rc_gettext(msgid) rc_dgettext (NULL, msgid)

# define rc_ngettext(msgid1, msgid2, n) rc_dngettext (NULL, msgid1, msgid2, n)

#endif	/* Optimizing.  */

#ifdef __cplusplus
}
#endif

#endif /* RC_INVOKED */

#endif /* libintl.h */
