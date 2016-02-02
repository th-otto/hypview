#include "hypdefs.h"
#include "hypdebug.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

gboolean ref_list_entries(const char *filename, FILE *outfile, gboolean all, gboolean verbose)
{
	int handle;
	REF_FILE *ref;
	char *filename2 = NULL;
	
	if (hyp_guess_filetype(filename) == HYP_FT_HYP)
	{
		filename2 = replace_ext(filename, HYP_EXT_HYP, HYP_EXT_REF);
		filename = filename2;
	}
	
	handle = hyp_utf8_open(filename, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
	if (handle < 0)
	{
		FileErrorErrno(filename);
		g_free(filename2);
		return FALSE;
	}
	
	ref = ref_load(filename, handle, verbose);
	hyp_utf8_close(handle);
	g_free(filename2);
	
	if (ref == NULL)
		return FALSE;
	
	fprintf(outfile, _("Contents of %s:\n"), filename);

	ref_list(ref, outfile, all);
	
	ref_close(ref);
	return TRUE;
}

/* ------------------------------------------------------------------------- */

gboolean ref_write_module(int handle, REF_MODULE *mod, gboolean verbose)
{
	long size;
	long modsize;
	long num_entries;
	unsigned char buf[2 * SIZEOF_LONG];
	const char *charsetname = NULL;
	
	UNUSED(verbose);
	size = mod->module_len;
	num_entries = mod->num_entries;
	
	modsize = size;
	/*
	 * if module did not contain a filename, update module_len
	 */
	if (!mod->had_filename)
	{
		/* module_filename[-1] contains the size of the entry */
		modsize += mod->module_filename[-1];
		/* num_entries was already updated when loading */
	}

	if (!mod->had_os)
	{
		modsize += 2 + 2;
		num_entries++;
	}
	
	if (!mod->had_charset)
	{
		charsetname = hyp_charset_name(mod->charset);
		modsize += strlen(charsetname) + 1 + 2;
		num_entries++;
	}
	
	long_to_chars(modsize, buf);
	long_to_chars(num_entries, buf + SIZEOF_LONG);
	if (write(handle, buf, sizeof(buf)) != (ssize_t)sizeof(buf))
		return FALSE;

	if (!mod->had_filename)
	{
		/* write the module_filename out */
		if (write(handle, mod->module_filename - 2, mod->module_filename[-1]) != mod->module_filename[-1])
			return FALSE;
	}
	if (!mod->had_os)
	{
		unsigned char entry[4];
		ssize_t size;
		
		size = 2 + 2;
		entry[0] = REF_OS;
		entry[1] = (unsigned char) size;
		entry[2] = mod->os + '0';
		entry[3] = '\0';
		if (write(handle, entry, size) != size)
			return FALSE;
	}
	if (!mod->had_charset)
	{
		unsigned char entry[2];
		ssize_t size;
		
		size = 2 + strlen(charsetname) + 1;
		entry[0] = REF_CHARSET;
		entry[1] = (unsigned char) size;
		if (write(handle, entry, 2) != 2)
			return FALSE;
		size -= 2;
		if (write(handle, charsetname, size) != size)
			return FALSE;
	}
	
	/* write the data */
	if (write(handle, mod->data, size) != (ssize_t)size)
		return FALSE;
	
	return TRUE;
}

/* ------------------------------------------------------------------------- */

gboolean ref_del_entries(const char *refname, int argc, const char **argv, FILE *outfile, gboolean verbose)
{
	int ref_handle = -1;
	REF_FILE *ref = NULL;
	char *dir, *tmpname = NULL;
	char *tmp;
	REF_MODULE *mod1;
	gboolean found;
	char *name1, *name2;
	int i;
	int new_num_modules;
	
	ref_handle = hyp_utf8_open(refname, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
	if (ref_handle < 0)
	{
		FileErrorErrno(refname);
		goto error;
	}
		
	ref = ref_load(refname, ref_handle, verbose);
	if (ref == NULL)
		goto error;
	hyp_utf8_close(ref_handle);
	ref_handle = -1;
	
	dir = hyp_path_get_dirname(refname);
	tmp = g_strdup_printf("hy(%u).ref", (int)getpid());
	tmpname = g_build_filename(dir, tmp, NULL);
	g_free(tmp);
	g_free(dir);
	
	ref_handle = hyp_utf8_open(tmpname, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, HYP_DEFAULT_FILEMODE);
	if (ref_handle < 0)
	{
		FileErrorErrno(tmpname);
		goto error;
	}
	
	/*
	 * write the file header
	 */
	if (!ref_write_header(ref_handle))
		goto write_error;
	
	new_num_modules = 0;
	for (mod1 = ref->modules; mod1 != NULL; mod1 = mod1->next)
	{
		found = FALSE;
		for (i = 0; i < argc; i++)
		{
			if (argv[i] != NULL)
			{
				name1 = ref_hyp_basename(mod1->filename ? mod1->filename : ref->filename);
				name2 = ref_hyp_basename(argv[i]);
				found = g_utf8_strcasecmp(name1, name2) == 0;
				g_free(name1);
				g_free(name2);
				if (found)
					break;
			}
		}
		if (!found)
		{
			if (!ref_write_module(ref_handle, mod1, verbose))
				goto write_error;
			new_num_modules++;
		} else
		{
			if (verbose)
				fprintf(outfile, _("Deleting %s\n"), argv[i]);
			argv[i] = NULL;
		}
	}
	if (verbose)
	{
		for (i = 0; i < argc; i++)
		{
			if (argv[i] != NULL)
				fprintf(outfile, _("Not found: %s\n"), argv[i]);
		}
	}
	
	/*
	 * write an empty module header as terminator
	 */
	if (!ref_write_trailer(ref_handle))
		goto write_error;
	hyp_utf8_close(ref_handle);
	ref_handle = -1;
	
	if (hyp_utf8_unlink(refname) < 0)
		goto write_error;
	ref_close(ref);
	ref = NULL;
	if (new_num_modules == 0)
	{
		if (verbose)
			fprintf(outfile, _("No files left\n"));
		if (hyp_utf8_unlink(refname) < 0)
		{
			FileErrorErrno(refname);
			goto error;
		}
		if (hyp_utf8_unlink(tmpname) < 0)
		{
			FileErrorErrno(tmpname);
			goto error;
		}
	} else
	{
		if (hyp_utf8_rename(tmpname, refname) < 0)
		{
			FileErrorErrno(refname);
			goto error;
		}
	}
	
	g_free(tmpname);
	
	return TRUE;

write_error:
	FileErrorErrno(tmpname);
	
error:
	ref_close(ref);
	if (ref_handle >= 0)
		hyp_utf8_close(ref_handle);
	if (tmpname)
	{
		hyp_utf8_unlink(tmpname);
		g_free(tmpname);
	}
	return FALSE;
}

/* ------------------------------------------------------------------------- */

gboolean ref_extract_entries(const char *refname, int argc, const char **argv, FILE *outfile, gboolean verbose)
{
	int ref_handle = -1;
	REF_FILE *ref = NULL;
	char *tmpname = NULL;
	char *modname = NULL;
	REF_MODULE *mod1;
	gboolean found;
	char *name1, *name2;
	int i;
	
	ref_handle = hyp_utf8_open(refname, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
	if (ref_handle < 0)
	{
		FileErrorErrno(refname);
		goto error;
	}
		
	ref = ref_load(refname, ref_handle, verbose);
	if (ref == NULL)
		goto error;
	hyp_utf8_close(ref_handle);
	ref_handle = -1;
	
	for (mod1 = ref->modules; mod1 != NULL; mod1 = mod1->next)
	{
		found = FALSE;
		for (i = 0; i < argc; i++)
		{
			if (argv[i] != NULL)
			{
				name1 = ref_hyp_basename(mod1->filename ? mod1->filename : ref->filename);
				name2 = ref_hyp_basename(argv[i]);
				found = g_utf8_strcasecmp(name1, name2) == 0;
				g_free(name1);
				g_free(name2);
				if (found)
					break;
			}
		}
		if (found || argc == 0)
		{
			tmpname = g_strdup_printf("hy(%u).ref", (int)getpid());
			
			name2 = ref_hyp_basename(mod1->filename);
			modname = g_strconcat(name2, HYP_EXT_REF, NULL);
			if (verbose)
				fprintf(outfile, _("Extracting %s\n"), name2);
			g_free(name2);
			
			ref_handle = hyp_utf8_open(tmpname, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, HYP_DEFAULT_FILEMODE);
			if (ref_handle < 0)
			{
				FileErrorErrno(tmpname);
				goto error;
			}
	
			/*
			 * write the file header
			 */
			if (!ref_write_header(ref_handle))
				goto write_error;

			if (!ref_write_module(ref_handle, mod1, verbose))
				goto write_error;
			
			/*
			 * write an empty module header as terminator
			 */
			if (!ref_write_trailer(ref_handle))
				goto write_error;
			hyp_utf8_close(ref_handle);
			ref_handle = -1;
	
			hyp_utf8_unlink(modname);
			if (hyp_utf8_rename(tmpname, modname) < 0)
			{
				FileErrorErrno(modname);
				goto error;
			}
			
			g_freep(&tmpname);
			g_freep(&modname);
			
			if (argc != 0)
				argv[i] = NULL;
		}
	}
	if (verbose)
	{
		for (i = 0; i < argc; i++)
		{
			if (argv[i] != NULL)
				fprintf(outfile, _("Not found: %s\n"), argv[i]);
		}
	}
	
	ref_close(ref);
	ref = NULL;
	
	return TRUE;

write_error:
	FileErrorErrno(tmpname);
	
error:
	ref_close(ref);
	if (ref_handle >= 0)
		hyp_utf8_close(ref_handle);
	if (tmpname)
	{
		hyp_utf8_unlink(tmpname);
		g_free(tmpname);
	}
	g_free(modname);
	return FALSE;
}

/* ------------------------------------------------------------------------- */

gboolean ref_add_entries(const char *refname, const char *modname, gboolean delete_mod, FILE *outfile, gboolean verbose)
{
	int ref_handle = -1, mod_handle = -1;
	int lck_handle = -1;
	char *refname2 = NULL, *modname2 = NULL;
	REF_FILE *ref = NULL, *mod = NULL;
	char *dir = NULL, *tmpname = NULL;
	char *lckname = NULL;
	char *tmp;
	REF_MODULE *mod1, *mod2;
	gboolean found;
	char *name1, *name2;
	hyp_filetype ftype;
	
	mod_handle = hyp_utf8_open(modname, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
	if (mod_handle < 0 && (ftype = hyp_guess_filetype(modname)) != HYP_FT_REF)
	{
		if (ftype == HYP_FT_HYP)
		{
			char *base, *filename;
			
			base = g_strdup(modname);
			*strrchr(base, '.') = '\0';
			filename = g_strconcat(base, HYP_EXT_REF, NULL);
			g_free(base);
			modname2 = hyp_find_file(filename);
			g_free(filename);
		} else
		{
			modname2 = g_strconcat(modname, HYP_EXT_REF, NULL);
		}
		if (modname2 != NULL)
		{
			mod_handle = hyp_utf8_open(modname2, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
			if (mod_handle >= 0)
			{
				modname = modname2;
			}
		}
	}
	if (mod_handle < 0)
	{
		FileErrorErrno(modname);
		goto error;
	}
	
	dir = hyp_path_get_dirname(refname);
	lckname = g_build_filename(dir, "hypview.lck", NULL);
	while ((lck_handle = hyp_utf8_open(lckname, O_WRONLY | O_CREAT | O_EXCL | O_BINARY, HYP_DEFAULT_FILEMODE)) < 0 && errno == EEXIST)
	{
#ifdef __TOS__
		sleep(100);
#else
		usleep(100000);
#endif
	}
	
	ref_handle = hyp_utf8_open(refname, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
	if (ref_handle < 0 && hyp_guess_filetype(refname) != HYP_FT_REF)
	{
		refname2 = g_strconcat(refname, HYP_EXT_REF, NULL);
		refname = refname2;
		ref_handle = hyp_utf8_open(refname, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
	}
		
	mod = ref_load(modname, mod_handle, verbose);
	if (mod == NULL)
		goto error;
	hyp_utf8_close(mod_handle);
	mod_handle = -1;

	if (verbose)
		fprintf(outfile, _("adding %s\n"), modname);
		
	if (ref_handle < 0)
	{
		ref = ref_new(refname, 0);
	} else
	{
		ref = ref_load(refname, ref_handle, verbose);
		if (ref == NULL)
			goto error;
		hyp_utf8_close(ref_handle);
		ref_handle = -1;
	}
	
	tmp = g_strdup_printf("hy(%u).ref", (int)getpid());
	tmpname = g_build_filename(dir, tmp, NULL);
	g_free(tmp);
	g_free(dir);
	dir = NULL;
	
	ref_handle = hyp_utf8_open(tmpname, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, HYP_DEFAULT_FILEMODE);
	if (ref_handle < 0)
	{
		FileErrorErrno(tmpname);
		goto error;
	}
	
	/*
	 * write the file header
	 */
	if (!ref_write_header(ref_handle))
		goto write_error;
	
	for (mod1 = ref->modules; mod1 != NULL; mod1 = mod1->next)
	{
		found = FALSE;
		for (mod2 = mod->modules; mod2 != NULL; mod2 = mod2->next)
		{
			name1 = ref_hyp_basename(mod1->filename ? mod1->filename : ref->filename);
			name2 = ref_hyp_basename(mod2->filename ? mod2->filename : mod->filename);
			found = g_utf8_strcasecmp(name1, name2) == 0;
			g_free(name1);
			g_free(name2);
			if (found)
				break;
		}
		if (!found)
		{
			if (!ref_write_module(ref_handle, mod1, verbose))
				goto write_error;
		}
	}
	for (mod2 = mod->modules; mod2 != NULL; mod2 = mod2->next)
	{
		if (!ref_write_module(ref_handle, mod2, verbose))
			goto write_error;
		if (verbose)
			fprintf(outfile, _("adding references for %s\n"), mod2->filename);
	}
	
	/*
	 * write an empty module header as terminator
	 */
	if (!ref_write_trailer(ref_handle))
		goto write_error;
	ref_close(mod);
	mod = NULL;
	hyp_utf8_close(ref_handle);
	ref_handle = -1;
	
	if (hyp_utf8_unlink(refname) < 0)
		if (ref->modules != NULL)
		{
			FileErrorErrno(refname);
			goto error;
		}
	ref_close(ref);
	ref = NULL;
	if (hyp_utf8_rename(tmpname, refname) < 0)
	{
		FileErrorErrno(refname);
		goto error;
	}
	
	if (delete_mod)
		if (hyp_utf8_unlink(modname) < 0)
			FileErrorErrno(modname);
	
	g_free(modname2);
	g_free(refname2);
	g_free(tmpname);
	if (lck_handle >= 0)
		hyp_utf8_close(lck_handle);
	if (lckname)
		hyp_utf8_unlink(lckname);
	g_free(lckname);
	
	return TRUE;

write_error:
	FileErrorErrno(tmpname);
	
error:
	ref_close(ref);
	ref_close(mod);
	if (ref_handle >= 0)
		hyp_utf8_close(ref_handle);
	if (mod_handle >= 0)
		hyp_utf8_close(mod_handle);
	g_free(modname2);
	g_free(refname2);
	g_free(dir);
	if (tmpname)
	{
		hyp_utf8_unlink(tmpname);
		g_free(tmpname);
	}
	if (lck_handle >= 0)
		hyp_utf8_close(lck_handle);
	if (lckname)
		hyp_utf8_unlink(lckname);
	g_free(lckname);
	return FALSE;
}

/* ------------------------------------------------------------------------- */

gboolean ref_write_header(int ref_handle)
{
	unsigned char magic[SIZEOF_LONG];
	
	long_to_chars(HYP_MAGIC_REF, magic);
	if (write(ref_handle, magic, SIZEOF_LONG) != SIZEOF_LONG)
		return FALSE;
	return TRUE;
}

/* ------------------------------------------------------------------------- */

gboolean ref_write_trailer(int ref_handle)
{
	unsigned char buf[2 * SIZEOF_LONG];
	
	long_to_chars(0, buf);
	long_to_chars(0, buf + SIZEOF_LONG);
	if (write(ref_handle, buf, 2 * SIZEOF_LONG) != 2 * SIZEOF_LONG)
		return FALSE;
	return TRUE;
}
