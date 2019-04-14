/* Copyright (C) 1996-2013 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _LOADINFO_H
#define _LOADINFO_H	1

/* Declarations of locale dependent catalog lookup functions.
   Implemented in

     localealias.c    Possibly replace a locale name by another.
     explodename.c    Split a locale name into its various fields.
     l10nflist.c      Generate a list of filenames of possible message catalogs.
     finddomain.c     Find and open the relevant message catalogs.

   The main function _rc_find_domain() in finddomain.c is declared
   in gettextP.h.
 */

/* Separator in PATH like lists of pathnames.  */
#if ((defined _WIN32 || defined __WIN32__) && !defined __CYGWIN__) || defined __EMX__ || defined __DJGPP__
  /* Win32, OS/2, DOS */
# define PATH_SEPARATOR ';'
#else
  /* Unix */
# define PATH_SEPARATOR ':'
#endif

/* Pathname support.
   ISSLASH(C)           tests whether C is a directory separator character.
 */
#if defined _WIN32 || defined __WIN32__ || defined __CYGWIN__ || defined __MSYS__ || defined __EMX__ || defined __DJGPP__ || defined __MINT__ || defined __TOS__
  /* Win32, Cygwin, OS/2, DOS */
#  define ISSLASH(C) ((C) == '/' || (C) == '\\')
#else
  /* Unix */
#  define ISSLASH(C) ((C) == '/')
#endif

/* Encoding of locale name parts.  */
#define XPG_CODESET		2
#define XPG_TERRITORY		4
#define XPG_MODIFIER		8

#if !(defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__) || defined(__MSYS__))
typedef struct _hgblobal *HGLOBAL;
typedef struct _hrsrc *HRSRC;
#endif

struct loaded_l10nfile
{
  union _strptr language;
  LANGID langid;
  HINSTANCE instance;
  int decided;

  struct loaded_domain *domain;

  struct loaded_l10nfile *next;
  struct loaded_l10nfile *successor[1];
};


/* Lookup a locale dependent file.
   *L10NFILE_LIST denotes a pool of lookup results of locale dependent
   files of the same kind, sorted in decreasing order of ->filename.
   DIRLIST and DIRLIST_LEN are an argz list of directories in which to
   look, containing at least one directory (i.e. DIRLIST_LEN > 0).
   MASK, LANGUAGE, TERRITORY, CODESET, NORMALIZED_CODESET, MODIFIER
   are the pieces of the locale name, as produced by _rc_explode_name().
   FILENAME is the filename suffix.
   The return value is the lookup result, either found in *L10NFILE_LIST,
   or - if DO_ALLOCATE is nonzero - freshly allocated, or possibly NULL.
   If the return value is non-NULL, it is added to *L10NFILE_LIST, and
   its ->next field denotes the chaining inside *L10NFILE_LIST, and
   furthermore its ->successor[] field contains a list of other lookup
   results from which this lookup result inherits.  */
extern struct loaded_l10nfile *
_rc_make_l10nflist (struct loaded_l10nfile **l10nfile_list,
		    const HINSTANCE *dirlist, size_t dirlist_len, int mask,
		    const char *language, const char *territory,
		    const char *codeset,
		    const char *modifier,
		    int do_allocate);

/* Split a locale name NAME into its pieces: language, modifier,
   territory, codeset.
   NAME gets destructively modified: NUL bytes are inserted here and
   there.  *LANGUAGE gets assigned NAME.  Each of *MODIFIER, *TERRITORY,
   *CODESET gets assigned either a pointer into the old NAME string, or
   NULL.  *NORMALIZED_CODESET gets assigned the expanded *CODESET, if it
   is different from *CODESET; this one is dynamically allocated and has
   to be freed by the caller.
   The return value is a bitmask, where each bit corresponds to one
   filled-in value:
     XPG_MODIFIER                for *MODIFIER,
     XPG_TERRITORY               for *TERRITORY,
     XPG_CODESET                 for *CODESET,
 */
extern int _rc_explode_name (char *name, const char **language,
			     const char **modifier, const char **territory,
			     const char **codeset);

#endif	/* loadinfo.h */
