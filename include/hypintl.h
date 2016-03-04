#ifndef __HYP_INTL_H__
#define __HYP_INTL_H__

#ifdef __TOS__
#undef ENABLE_NLS
#endif

#ifdef ENABLE_NLS
#ifndef RC_INVOKED
# if (defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__MSYS__)) && !defined(HAVE_GTK)
#  include "../rcintl/rcintl.h"
#  define textdomain(String) rc_textdomain(String)
#  define bindtextdomain(Domain, Directory) rc_bindtextdomain(Domain, GetModuleHandleW(NULL)) 
#  define bind_textdomain_codeset(Domain, codeset) rc_bind_textdomain_codeset(Domain, codeset)
# else
#  include <libintl.h>
# endif
# include <locale.h>
#endif

/* The separator between msgctxt and msgid in a .mo file. */
#ifndef GETTEXT_CONTEXT_GLUE
#  define GETTEXT_CONTEXT_GLUE "\004"
#endif

#undef   GETTEXT_PACKAGE
#define  GETTEXT_PACKAGE "hypview"

# define _(String) xs_dgettext(GETTEXT_PACKAGE, String)
# define P_(String, Stringp, n) xs_dngettext(GETTEXT_PACKAGE, String, Stringp, n)
# define C_(Context, String) xs_dgettext(GETTEXT_PACKAGE, Context GETTEXT_CONTEXT_GLUE String)
# define N_(String) String
# define NC_(Context, String) Context GETTEXT_CONTEXT_GLUE String

#undef printf
#undef fprintf
#undef sprintf
#undef vfprintf
#undef vprintf
#undef vsprintf
#undef snprintf
#undef vsnprintf
#undef asprintf
#undef vasprintf
#undef fwprintf
#undef vfwprintf
#undef wprintf
#undef vwprintf
#undef swprintf
#undef vswprintf

#else  /* !ENABLE_NLS */

# define _(String) (String)
# define C_(Context, String) (String)
# define P_(String, Stringp, n) ((n) == 1 ? (String) : (Stringp))
# define N_(String) String
# define NC_(Context, String) String
# define textdomain(String)
# define gettext(String) (String)
# define dgettext(Domain, String) (String)
# define dcgettext(Domain, String, Type) (String)
# define bindtextdomain(Domain, Directory)
# define bind_textdomain_codeset(Domain, codeset)

#endif /* !ENABLE_NLS */

#ifndef RC_INVOKED

EXTERN_C_BEG

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

const char *xs_dgettext(const char *domain, const char *msgid) __THROW __attribute_format_arg__ (2);
const char *xs_dngettext(const char *domain, const char *msgid, const char *msgid_plural, unsigned long n) __THROW __attribute_format_arg__ (1) __attribute_format_arg__ (2);

const char *xs_get_locale_dir(void);
void xs_locale_exit(void);
char *g_get_package_installation_directory(void);
char *g_get_package_bindir(void);

#ifdef __TOS__
extern const char *_argv0;
extern char *(*g_tos_get_bindir)(void);
extern char *g_ttp_get_bindir(void);
extern char *g_gem_get_bindir(void);
#endif


EXTERN_C_END

#endif /* RC_INVOKED */

#endif /* __HYP_INTL_H__ */
