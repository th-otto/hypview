/* Load needed message catalogs.
   Copyright (C) 1995-1999, 2000-2008, 2010 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 2.1 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* Tell glibc's <string.h> to provide a prototype for mempcpy().
   This must come before <config.h> because <config.h> may include
   <features.h>, and once <features.h> has been included, it's too late.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE    1
#endif

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "windows_.h"

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#ifdef _MSC_VER
#include <io.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __GNUC__
# undef  alloca
# define alloca __builtin_alloca
# define HAVE_ALLOCA 1
#else
# ifdef _MSC_VER
#  include <malloc.h>
#  define alloca _alloca
# else
#  if defined HAVE_ALLOCA_H || defined _LIBC
#   include <alloca.h>
#  else
#   ifdef _AIX
#pragma alloca
#   else
#    ifndef alloca
char *alloca();
#    endif
#   endif
#  endif
# endif
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <stdint.h>
#include <inttypes.h>

#include "gmo.h"
#include "gettextP.h"
#include "hash-string.h"
#include "plural-exp.h"

/* Handle multi-threaded applications.  */
#include "rcintl.h"
#include "rcsysdep.h"

/* @@ end of prolog @@ */

#ifdef _LIBC
/* Rename the non ISO C functions.  This is required by the standard
   because some ISO C functions will require linking with this object
   file and the name space must not be polluted.  */
# define open(name, flags)	open_not_cancel_2 (name, flags)
# define close(fd)		close_not_cancel_no_status (fd)
# define read(fd, buf, n)	read_not_cancel (fd, buf, n)
# define mmap(addr, len, prot, flags, fd, offset) \
  __mmap (addr, len, prot, flags, fd, offset)
# define munmap(addr, len)	__munmap (addr, len)
#else
#undef read
#endif

/* For those losing systems which don't have `alloca' we have to add
   some additional code emulating it.  */
#ifdef HAVE_ALLOCA
# define freea(p)						/* nothing */
#else
# define alloca(n) malloc (n)
# define freea(p) free (p)
#endif

/* For systems that distinguish between text and binary I/O.
   O_BINARY is usually declared in <fcntl.h>. */
#if !defined O_BINARY && defined _O_BINARY
  /* For MSC-compatible compilers.  */
# define O_BINARY _O_BINARY
# define O_TEXT _O_TEXT
#endif
#ifdef __BEOS__
  /* BeOS 5 has O_BINARY and O_TEXT, but they have no effect.  */
# undef O_BINARY
# undef O_TEXT
#endif
/* On reasonable systems, binary I/O is the default.  */
#ifndef O_BINARY
# define O_BINARY 0
#endif


/* We need a sign, whether a new catalog was loaded, which can be associated
   with all translations.  This is important if the translations are
   cached by one of GCC's features.  */
int _rc_msg_cat_cntr;


/* Load the message catalogs specified by FILENAME.  If it is no valid
   message catalog do nothing.  */
void _rc_load_domain(struct loaded_l10nfile *domain_file, struct binding *domainbinding)
{
	HRSRC hrsrc;
	HGLOBAL hglbl;
	size_t size;
	struct mo_file_header *data = NULL;
	struct loaded_domain *domain;
	nls_uint32 revision;
	const char *nullentry;
	size_t nullentrylen;
	LPCWSTR type = MAKEINTRESOURCEW(RT_MOFILE);
	LPCWSTR name = MAKEINTRESOURCEW(1);
	
	if (domain_file->decided != 0)
	{
		/* There are two possibilities:

		   + this is the same thread calling again during this initialization
		   via _rc_find_msg.  We have initialized everything this call needs.

		   + this is another thread which tried to initialize this object.
		   Not necessary anymore since if the lock is available this
		   is finished.
		 */
		goto done;
	}

	domain_file->decided = -1;
	domain_file->domain = NULL;

	/* Note that it would be useless to store domainbinding in domain_file
	   because domainbinding might be == NULL now but != NULL later (after
	   a call to bind_textdomain_codeset).  */

	/* If the record does not represent a valid locale the FILENAME
	   might be NULL.  This can happen when according to the given
	   specification the locale file name is different for XPG and CEN
	   syntax.  */
	if (domain_file->instance == NULL)
		goto out;

	hrsrc = FindResourceExW(domain_file->instance, type, name, domain_file->langid);
	if (!hrsrc && SUBLANGID(domain_file->langid) != SUBLANG_DEFAULT)
		hrsrc = FindResourceExW(domain_file->instance, type, name, MAKELANGID(PRIMARYLANGID(domain_file->langid), SUBLANG_DEFAULT));
	if (!hrsrc && SUBLANGID(domain_file->langid) != SUBLANG_NEUTRAL)
		hrsrc = FindResourceExW(domain_file->instance, type, name, MAKELANGID(PRIMARYLANGID(domain_file->langid), SUBLANG_NEUTRAL));
	if (!hrsrc && domain_file->langid != MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL))
		hrsrc = FindResourceExW(domain_file->instance, type, name, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
	if (!hrsrc)
		goto out;

	/* We must know about the size of the file.  */
	if (unlikely((size = SizeofResource(domain_file->instance, hrsrc)) < sizeof(struct mo_file_header)) ||
		unlikely((hglbl = LoadResource(domain_file->instance, hrsrc)) == NULL) ||
		unlikely((data = (struct mo_file_header *)LockResource(hglbl)) == NULL))
		/* Something went wrong.  */
		goto out;

	/* Using the magic number we can test whether it really is a message
	   catalog file.  */
	if (unlikely(data->magic != _MAGIC && data->magic != _MAGIC_SWAPPED))
	{
		/* The magic number is wrong: not a message catalog file.  */
		(void) UnlockResource(hglbl);
		FreeResource(hglbl);
		goto out;
	}

	domain = (struct loaded_domain *) malloc(sizeof(struct loaded_domain));
	if (domain == NULL)
		goto invalid;
	domain_file->domain = domain;

	domain->header = data;
	domain->hrsrc = hrsrc;
	domain->hglbl = hglbl;
	domain->mmap_size = size;
	domain->must_swap = data->magic != _MAGIC;
	domain->malloced = NULL;

	/* Fill in the information about the available tables.  */
	revision = W(domain->must_swap, data->revision);
	/* We support only the major revisions 0 and 1.  */
	switch ((int) (revision >> 16))
	{
	case 0:
	case 1:
		domain->nstrings = W(domain->must_swap, data->nstrings);
		domain->orig_tab = (const struct string_desc *) ((char *) data + W(domain->must_swap, data->orig_tab_offset));
		domain->trans_tab = (const struct string_desc *) ((char *) data + W(domain->must_swap, data->trans_tab_offset));
		domain->hash_size = W(domain->must_swap, data->hash_tab_size);
		domain->hash_tab =
			(domain->hash_size > 2
			 ? (const nls_uint32 *) ((char *) data + W(domain->must_swap, data->hash_tab_offset)) : NULL);
		domain->must_swap_hash_tab = domain->must_swap;

		/* Now dispatch on the minor revision.  */
		switch ((int) (revision & 0xffff))
		{
		case 0:
			domain->n_sysdep_strings = 0;
			domain->orig_sysdep_tab = NULL;
			domain->trans_sysdep_tab = NULL;
			break;
		case 1:
		default:
			{
				nls_uint32 n_sysdep_strings;

				if (domain->hash_tab == NULL)
					/* This is invalid.  These minor revisions need a hash table.  */
					goto invalid;

				n_sysdep_strings = W(domain->must_swap, data->n_sysdep_strings);
				if (n_sysdep_strings > 0)
				{
					nls_uint32 n_sysdep_segments;
					const struct sysdep_segment *sysdep_segments;
					const char **sysdep_segment_values;
					const nls_uint32 *orig_sysdep_tab;
					const nls_uint32 *trans_sysdep_tab;
					nls_uint32 n_inmem_sysdep_strings;
					size_t memneed;
					char *mem;
					struct sysdep_string_desc *inmem_orig_sysdep_tab;
					struct sysdep_string_desc *inmem_trans_sysdep_tab;
					nls_uint32 *inmem_hash_tab;
					nls_uint32 i, j;

					/* Get the values of the system dependent segments.  */
					n_sysdep_segments = W(domain->must_swap, data->n_sysdep_segments);
					sysdep_segments = (const struct sysdep_segment *)
						((char *) data + W(domain->must_swap, data->sysdep_segments_offset));
					sysdep_segment_values = (const char **) alloca(n_sysdep_segments * sizeof(const char *));
					for (i = 0; i < n_sysdep_segments; i++)
					{
						const char *name = (char *) data + W(domain->must_swap, sysdep_segments[i].offset);

						nls_uint32 namelen = W(domain->must_swap, sysdep_segments[i].length);

						if (!(namelen > 0 && name[namelen - 1] == '\0'))
						{
							freea(sysdep_segment_values);
							goto invalid;
						}

						sysdep_segment_values[i] = get_sysdep_segment_value(name);
					}

					orig_sysdep_tab = (const nls_uint32 *)
						((char *) data + W(domain->must_swap, data->orig_sysdep_tab_offset));
					trans_sysdep_tab = (const nls_uint32 *)
						((char *) data + W(domain->must_swap, data->trans_sysdep_tab_offset));

					/* Compute the amount of additional memory needed for the
					   system dependent strings and the augmented hash table.
					   At the same time, also drop string pairs which refer to
					   an undefined system dependent segment.  */
					n_inmem_sysdep_strings = 0;
					memneed = domain->hash_size * sizeof(nls_uint32);
					for (i = 0; i < n_sysdep_strings; i++)
					{
						int valid = 1;
						size_t needs[2];

						for (j = 0; j < 2; j++)
						{
							const struct sysdep_string *sysdep_string =
								(const struct sysdep_string *) ((char *) data + W(domain->must_swap,
																				  j == 0
																				  ? orig_sysdep_tab[i]
																				  : trans_sysdep_tab[i]));
							size_t need = 0;

							const struct segment_pair *p = sysdep_string->segments;

							if (W(domain->must_swap, p->sysdepref) != SEGMENTS_END)
								for (p = sysdep_string->segments;; p++)
								{
									nls_uint32 sysdepref;

									need += W(domain->must_swap, p->segsize);

									sysdepref = W(domain->must_swap, p->sysdepref);
									if (sysdepref == SEGMENTS_END)
										break;

									if (sysdepref >= n_sysdep_segments)
									{
										/* Invalid.  */
										freea(sysdep_segment_values);
										goto invalid;
									}

									if (sysdep_segment_values[sysdepref] == NULL)
									{
										/* This particular string pair is invalid.  */
										valid = 0;
										break;
									}

									need += strlen(sysdep_segment_values[sysdepref]);
								}

							needs[j] = need;
							if (!valid)
								break;
						}

						if (valid)
						{
							n_inmem_sysdep_strings++;
							memneed += needs[0] + needs[1];
						}
					}
					memneed += 2 * n_inmem_sysdep_strings * sizeof(struct sysdep_string_desc);

					if (n_inmem_sysdep_strings > 0)
					{
						nls_uint32 k;

						/* Allocate additional memory.  */
						mem = (char *) malloc(memneed);
						if (mem == NULL)
							goto invalid;

						domain->malloced = mem;
						inmem_orig_sysdep_tab = (struct sysdep_string_desc *) mem;
						mem += n_inmem_sysdep_strings * sizeof(struct sysdep_string_desc);
						inmem_trans_sysdep_tab = (struct sysdep_string_desc *) mem;
						mem += n_inmem_sysdep_strings * sizeof(struct sysdep_string_desc);
						inmem_hash_tab = (nls_uint32 *) mem;
						mem += domain->hash_size * sizeof(nls_uint32);

						/* Compute the system dependent strings.  */
						k = 0;
						for (i = 0; i < n_sysdep_strings; i++)
						{
							int valid = 1;

							for (j = 0; j < 2; j++)
							{
								const struct sysdep_string *sysdep_string =
									(const struct sysdep_string *) ((char *) data + W(domain->must_swap,
																					  j == 0
																					  ? orig_sysdep_tab[i]
																					  : trans_sysdep_tab[i]));
								const struct segment_pair *p = sysdep_string->segments;

								if (W(domain->must_swap, p->sysdepref) != SEGMENTS_END)
									for (p = sysdep_string->segments;; p++)
									{
										nls_uint32 sysdepref;

										sysdepref = W(domain->must_swap, p->sysdepref);
										if (sysdepref == SEGMENTS_END)
											break;

										if (sysdep_segment_values[sysdepref] == NULL)
										{
											/* This particular string pair is
											   invalid.  */
											valid = 0;
											break;
										}
									}

								if (!valid)
									break;
							}

							if (valid)
							{
								for (j = 0; j < 2; j++)
								{
									const struct sysdep_string *sysdep_string =
										(const struct sysdep_string *) ((char *) data + W(domain->must_swap,
																						  j == 0
																						  ? orig_sysdep_tab[i]
																						  : trans_sysdep_tab[i]));
									const char *static_segments =
										(char *) data + W(domain->must_swap, sysdep_string->offset);
									const struct segment_pair *p = sysdep_string->segments;

									/* Concatenate the segments, and fill
									   inmem_orig_sysdep_tab[k] (for j == 0) and
									   inmem_trans_sysdep_tab[k] (for j == 1).  */

									struct sysdep_string_desc *inmem_tab_entry =
										(j == 0 ? inmem_orig_sysdep_tab : inmem_trans_sysdep_tab) + k;

									if (W(domain->must_swap, p->sysdepref) == SEGMENTS_END)
									{
										/* Only one static segment.  */
										inmem_tab_entry->length = W(domain->must_swap, p->segsize);
										inmem_tab_entry->pointer = static_segments;
									} else
									{
										inmem_tab_entry->pointer = mem;

										for (p = sysdep_string->segments;; p++)
										{
											nls_uint32 segsize = W(domain->must_swap, p->segsize);
											nls_uint32 sysdepref = W(domain->must_swap, p->sysdepref);
											size_t n;

											if (segsize > 0)
											{
												memcpy(mem, static_segments, segsize);
												mem += segsize;
												static_segments += segsize;
											}

											if (sysdepref == SEGMENTS_END)
												break;

											n = strlen(sysdep_segment_values[sysdepref]);
											memcpy(mem, sysdep_segment_values[sysdepref], n);
											mem += n;
										}

										inmem_tab_entry->length = (const char *) mem - inmem_tab_entry->pointer;
									}
								}

								k++;
							}
						}
						if (k != n_inmem_sysdep_strings)
							abort();

						/* Compute the augmented hash table.  */
						for (i = 0; i < domain->hash_size; i++)
							inmem_hash_tab[i] = W(domain->must_swap_hash_tab, domain->hash_tab[i]);
						for (i = 0; i < n_inmem_sysdep_strings; i++)
						{
							const char *msgid = inmem_orig_sysdep_tab[i].pointer;
							nls_uint32 hash_val = __hash_string(msgid);
							nls_uint32 idx = hash_val % domain->hash_size;
							nls_uint32 incr = 1 + (hash_val % (domain->hash_size - 2));

							for (;;)
							{
								if (inmem_hash_tab[idx] == 0)
								{
									/* Hash table entry is empty.  Use it.  */
									inmem_hash_tab[idx] = 1 + domain->nstrings + i;
									break;
								}

								if (idx >= domain->hash_size - incr)
									idx -= domain->hash_size - incr;
								else
									idx += incr;
							}
						}

						domain->n_sysdep_strings = n_inmem_sysdep_strings;
						domain->orig_sysdep_tab = inmem_orig_sysdep_tab;
						domain->trans_sysdep_tab = inmem_trans_sysdep_tab;

						domain->hash_tab = inmem_hash_tab;
						domain->must_swap_hash_tab = 0;
					} else
					{
						domain->n_sysdep_strings = 0;
						domain->orig_sysdep_tab = NULL;
						domain->trans_sysdep_tab = NULL;
					}

					freea(sysdep_segment_values);
				} else
				{
					domain->n_sysdep_strings = 0;
					domain->orig_sysdep_tab = NULL;
					domain->trans_sysdep_tab = NULL;
				}
			}
			break;
		}
		break;
	default:
		/* This is an invalid revision.  */
	  invalid:
		/* This is an invalid .mo file or we ran out of resources.  */
		if (domain)
			free(domain->malloced);
		(void) UnlockResource(hglbl);
		FreeResource(hglbl);
		free(domain);
		domain_file->domain = NULL;
		goto out;
	}

	/* Get the header entry and look for a plural specification.  */
	nullentry = _rc_find_msg(domain_file, domainbinding, "", &nullentrylen);
	if (unlikely(nullentry == (const char *) -1))
	{
		goto invalid;
	}
	EXTRACT_PLURAL_EXPRESSION(nullentry, &domain->plural.cptr, &domain->nplurals);

  out:

	domain_file->decided = 1;

  done:
	;
}
