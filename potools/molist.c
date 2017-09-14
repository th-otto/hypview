#define CC_FOR_BUILD 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#undef HAVE_GLIB
#undef HAVE_GTK
#include "windows_.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "string_.h"
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include "time_.h"
#include <sys/stat.h>
#include <fcntl.h>
#include "portab.h"
#include "hypintl.h"
#include "gmo.h"
#include "gettextP.h"
#include "hash-string.h"
#include "stat_.h"
#include "hypmem.h"
#ifdef _MSC_VER
#include <io.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

/* The separator between msgctxt and msgid in a .mo file. */
#ifndef GETTEXT_CONTEXT_GLUE
#  define GETTEXT_CONTEXT_GLUE "\004"
#endif

/*
 * if this file is used in a cross-compilation environment,
 * it is compiled for the build system.
 * We cannot use the libraries that were compiled for the target,
 * so just include the needed sources here for simplicity
 */
#include "xgetopt.h"
#include "../hyp/xgetopt.c"
#include "../hyp/hyp_glib.c"
#include "../rcintl/localenm.c"
#include "hash-string.c"
#include "rcsysdep.h"

#ifndef CATOBJEXT
#define CATOBJEXT ".gmo"
#endif

char const gl_program_name[] = "molist";

static int num_errors;
static gboolean bShowVersion;
static gboolean bShowHelp;
static GSList *filelist;
static const char *podir = "../po";

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static struct loaded_l10nfile *loadmo(const char *input_filename)
{
	int fd = -1;
	size_t size;
	struct stat st;
	struct mo_file_header *data = NULL;
	struct loaded_domain *domain = NULL;
	nls_uint32 revision;
	struct loaded_l10nfile *domain_file;
	
	domain_file = g_new0(struct loaded_l10nfile, 1);
	if (domain_file == NULL)
		goto invalid;
	
	domain_file->decided = -1;
	domain_file->domain = NULL;

	fd = open(input_filename, O_RDONLY | O_BINARY);

	if (fd == -1)
	{
		fprintf(stderr, "%s: %s: %s\n", gl_program_name, input_filename, strerror(errno));
		goto invalid;
	}

	/* We must know about the size of the file.  */
	if (unlikely(fstat(fd, &st) != 0)
		|| unlikely(((ssize_t) (size = (size_t) st.st_size)) != st.st_size)
		|| unlikely(size < sizeof(struct mo_file_header)))
		/* Something went wrong.  */
	{
		fprintf(stderr, "%s: %s: %s\n", gl_program_name, input_filename, _("cannot determine file size"));
		goto invalid;
	}
	
	/* If the data is not yet available (i.e. mmap'ed) we try to load
	   it manually.  */
	{
		size_t to_read;
		char *read_ptr;

		data = (struct mo_file_header *) g_malloc(size);
		if (data == NULL)
		{
			fprintf(stderr, "%s: %s: %s\n", gl_program_name, input_filename, strerror(errno));
			goto invalid;
		}
		
		to_read = size;
		read_ptr = (char *) data;
		do
		{
			long int nb = (long int) read(fd, read_ptr, to_read);

			if (nb <= 0)
			{
#ifdef EINTR
				if (nb == -1 && errno == EINTR)
					continue;
#endif
				fprintf(stderr, "%s: %s: %s\n", gl_program_name, input_filename, strerror(errno));
				goto invalid;
			}
			read_ptr += nb;
			to_read -= nb;
		} while (to_read > 0);

		close(fd);
		fd = -1;
	}

	/* Using the magic number we can test whether it really is a message
	   catalog file.  */
	if (unlikely(data->magic != _MAGIC && data->magic != _MAGIC_SWAPPED))
	{
		/* The magic number is wrong: not a message catalog file.  */
		g_free(data);
		fprintf(stderr, "%s: %s: %s\n", gl_program_name, input_filename, _("file is not in .mo format"));
		goto invalid;
	}

	domain = g_new(struct loaded_domain, 1);
	if (domain == NULL)
	{
		fprintf(stderr, "%s: %s: %s\n", gl_program_name, input_filename, strerror(errno));
		goto invalid;
	}
	domain_file->domain = domain;

	domain->header = data;
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
				{
					/* This is invalid.  These minor revisions need a hash table.  */
					fprintf(stderr, "%s: %s: %s\n", gl_program_name, input_filename, _("hash table missing"));
					goto invalid;
				}
				
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
					sysdep_segment_values = g_new(const char *, n_sysdep_segments);
					for (i = 0; i < n_sysdep_segments; i++)
					{
						const char *name = (char *) data + W(domain->must_swap, sysdep_segments[i].offset);

						nls_uint32 namelen = W(domain->must_swap, sysdep_segments[i].length);

						if (!(namelen > 0 && name[namelen - 1] == '\0'))
						{
							g_free(sysdep_segment_values);
							fprintf(stderr, "%s: %s: %s\n", gl_program_name, input_filename, _("invalid system dependent name"));
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
										fprintf(stderr, "%s: %s: %s\n", gl_program_name, input_filename, _("system dependent prefix out of range"));
										g_free(sysdep_segment_values);
										goto invalid;
									}

									if (sysdep_segment_values[sysdepref] == NULL)
									{
										/* This particular string pair is invalid.  */
										fprintf(stderr, "%s: %s: %s\n", gl_program_name, input_filename, _("invalid system dependent string pair"));
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
						mem = g_new(char, memneed);
						if (mem == NULL)
						{
							fprintf(stderr, "%s: %s: %s\n", gl_program_name, input_filename, strerror(errno));
							goto invalid;
						}
						
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
											fprintf(stderr, "%s: %s: %s\n", gl_program_name, input_filename, _("invalid system dependent string pair"));
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

					g_free(sysdep_segment_values);
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
		{
			g_free(domain->malloced);
			g_free(data);
			g_free(domain);
		}
		g_free(domain_file);
		domain_file = NULL;
		goto out;
	}

  out:
	if (fd != -1)
		close(fd);

	return domain_file;
}

/*** ---------------------------------------------------------------------- ***/

static void quote(FILE *out, const char *prefix, const char *str, size_t len)
{
	const char *lf;
	const char *end = str + len;
	
	fputs(prefix, out);
	fputc('"', out);
	if ((lf = strchr(str, 0x0a)) != NULL && lf < end && lf[1] != '\0')
	{
		fputs("\"\n\"", out);
	}
	while (str < end)
	{
		unsigned char c = *str;
		
		switch (c)
		{
		case 0x0d:
			fputs("\\r", out);
			break;
		case 0x0a:
			fputs("\\n\"\n", out);
			if (str[1] == '\0')
				return;
			fputc('"', out);
			break;
		case '\t':
			fputs("\\t", out);
			break;
		case '\v':
			fputs("\\v", out);
			break;
		case '\f':
			fputs("\\f", out);
			break;
		case '\b':
			fputs("\\b", out);
			break;
		case '\\':
			fputs("\\\\", out);
			break;
		case '"':
			fputs("\\\"", out);
			break;
		default:
			if (c < 0x20 || c >= 0x80)
			{
				fputc('\\', out);
				fputc('0' + ((c >> 6) & 7), out);
				fputc('0' + ((c >> 3) & 7), out);
				fputc('0' + (c & 7), out);
			} else
			{
				fputc(c, out);
			}
			break;
		}
		str++;
	}
	fputc('"', out);
	fputc('\n', out);
}

/*** ---------------------------------------------------------------------- ***/

static gboolean dofile(const char *input_filename)
{
	gboolean retval = FALSE;
	struct loaded_l10nfile *file;
	nls_uint32 i;
	const char *msgid;
	const char *trans;
	const char *context;
	const char *plural;
	FILE *out = stdout;
	nls_uint32 msglen, translen;
	
	file = loadmo(input_filename);
	if (file)
	{
		const struct loaded_domain *domain = file->domain;
		const struct mo_file_header *header = domain->header;
		
		for (i = 0; i < header->nstrings; i++)
		{
			msgid = (const char *)header + W(domain->must_swap, domain->orig_tab[i].offset);
			msglen = W(domain->must_swap, domain->orig_tab[i].length);
			trans = (const char *)header + W(domain->must_swap, domain->trans_tab[i].offset);
			translen = W(domain->must_swap, domain->trans_tab[i].length);
			
			fprintf(out, "# %u\n", i);
			context = (const char *)memchr(msgid, GETTEXT_CONTEXT_GLUE[0], msglen);
			if (context)
			{
				quote(out, "msgctxt ", msgid, context - msgid);
				++context;
				msgid = context;
				msglen -= context - msgid;
			}
			plural = (const char *)memchr(msgid, '\0', msglen);
			if (plural)
			{
				char buf[40];
				int i;
			
				quote(out, "msgid  ", msgid, plural - msgid);
				plural++;
				msglen -= plural - msgid;
				quote(out, "msgid_plural ", plural, msglen);
				i = 0;
				for (;;)
				{
					++i;
					sprintf(buf, "msgstr[%d] ", i);
					plural = (const char *)memchr(trans, '\0', translen);
					if (plural)
					{
						quote(out, buf, trans, plural - trans);
						plural++;
						translen -= plural - trans;
						trans = plural;
					} else
					{
						quote(out, buf, trans, translen);
						break;
					}
				}
			} else
			{
				quote(out, "msgid  ", msgid, msglen);
				quote(out, "msgstr ", trans, translen);
			}
			fputc('\n', out);
		}
		retval = TRUE;
	}
	return retval;
}

/*** ---------------------------------------------------------------------- ***/

static void addfile(const char *path)
{
	char *filename = NULL;
	struct stat s;
	
	if (stat(path, &s) == 0)
		filename = g_strdup(path);
	if (filename == NULL)
		filename = g_build_filename(podir, path, NULL);
	filelist = g_slist_append(filelist, filename);
}

/*** ---------------------------------------------------------------------- ***/

static gboolean read_linguas(const char *filename)
{
	FILE *fp;
	char buf[1024];
	
	fp = fopen(filename, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "%s: %s: %s:\n", gl_program_name, filename, strerror(errno));
		return FALSE;
	}
	while (fgets(buf, (int)sizeof(buf), fp) != NULL)
	{
		g_strchomp(buf);
		g_strchug(buf);
		if (*buf == '\0' || *buf == '#')
			continue;
		strncat(buf, CATOBJEXT, sizeof(buf));
		addfile(buf);
	}
	
	fclose(fp);
	return TRUE;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

enum {
	OPT_HELP = 'h',
	OPT_VERSION = 'V',
	OPT_LINGUAS = 256,
	OPT_PODIR,
};

static struct option const long_options[] = {
	{ "linguas", required_argument, NULL, OPT_LINGUAS },
	{ "podir", required_argument, NULL, OPT_PODIR },
	{ "help", no_argument, NULL, OPT_HELP },
	{ "version", no_argument, NULL, OPT_VERSION },
	
	{ NULL, no_argument, NULL, 0 }
};

/*** ---------------------------------------------------------------------- ***/

static void show_version(void)
{
}

/*** ---------------------------------------------------------------------- ***/

static void show_help(void)
{
}

/*** ---------------------------------------------------------------------- ***/

int main(int argc, const char **argv)
{
	int i;
	int c;
	gboolean retval = TRUE;
	
	while ((c = getopt_long_only(argc, argv, "hV?", long_options, NULL)) != EOF)
	{
		switch (c)
		{
		case OPT_LINGUAS:
			retval &= read_linguas(optarg);
			break;
		
		case OPT_PODIR:
			podir = optarg;
			break;
		
		case OPT_HELP:
			bShowHelp = TRUE;
			break;
		case OPT_VERSION:
			bShowVersion = TRUE;
			break;
		case '?':
			if (optopt == '?')
			{
				bShowHelp = TRUE;
			} else
			{
				retval = FALSE;
			}
			break;
		
		case 0:
			/* option which just sets a var */
			break;
		
		case 1:
			addfile(optarg);
			break;
		
		default:
			/* error message already issued */
			retval = FALSE;
			break;
		}
	}

	if (bShowHelp)
	{
		show_help();
	} else if (bShowVersion)
	{
		show_version();
	} else if (retval)
	{
		GSList *l;
		
		for (i = optind; i < argc; i++)
		{
			addfile(argv[i]);
		}
		if (filelist == NULL)
		{
			fprintf(stderr, _("%s: no files\n"), gl_program_name);
			retval = FALSE;
		}
		for (l = filelist; l; l = l->next)
		{
			retval &= dofile((const char *)l->data);
		}
	}
	
	g_slist_free_full(filelist, g_free);
	
	return retval == FALSE || num_errors > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
