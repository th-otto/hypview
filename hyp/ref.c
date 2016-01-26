#include "hypdefs.h"
#include "hypdebug.h"

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

gboolean ref_list(REF_FILE *ref, FILE *outfile, gboolean all)
{
	REF_MODULE *mod;
	long num;
	char *str;

	for (mod = ref->modules; mod != NULL; mod = mod->next)
	{
		hyp_utf8_fprintf(outfile, _("Hypertext file: %s with %ld entries\n"),
			mod->filename ? mod->filename : _("<unnamed file>"),
			mod->num_entries);
		for (num = 0; num < mod->num_entries; num++)
		{
			switch (mod->entries[num].type)
			{
			case REF_FILENAME:
				if (all)
				{
					str = hyp_conv_to_utf8(mod->charset, mod->entries[num].name, STR0TERM);
					hyp_utf8_fprintf(outfile, _(" Module         %s\n"), str);
					g_free(str);
				}
				break;
			case REF_NODENAME:
				if (all)
				{
					str = hyp_conv_to_utf8(mod->charset, mod->entries[num].name, STR0TERM);
					hyp_utf8_fprintf(outfile, _(" Node           %s\n"), str);
					g_free(str);
				}
				break;
			case REF_ALIASNAME:
				if (all)
				{
					str = hyp_conv_to_utf8(mod->charset, mod->entries[num].name, STR0TERM);
					hyp_utf8_fprintf(outfile, _(" Alias          %s\n"), str);
					g_free(str);
				}
				break;
			case REF_LABELNAME:
				if (all)
				{
					str = hyp_conv_to_utf8(mod->charset, mod->entries[num].name, STR0TERM);
					hyp_utf8_fprintf(outfile, _(" Label %5u    %s\n"), mod->entries[num].lineno, str);
					g_free(str);
				}
				break;
			case REF_DATABASE:
				if (all)
					hyp_utf8_fprintf(outfile, _(" Database       %s\n"), mod->database);
				break;
			case REF_OS:
				if (all)
					hyp_utf8_fprintf(outfile, _(" OS             %s\n"), hyp_osname(mod->os));
				break;
			case REF_CHARSET:
				if (all)
					hyp_utf8_fprintf(outfile, _(" Charset        %s\n"), hyp_charset_name(mod->charset));
				break;
			case REF_TITLE:
				if (all)
				{
					str = hyp_conv_to_utf8(mod->charset, mod->entries[num].name, STR0TERM);
					hyp_utf8_fprintf(outfile, _(" Title          %s\n"), str);
					g_free(str);
				}
				break;
			case REF_UNKNOWN:
			default:
				if (all)
				{
					str = hyp_conv_to_utf8(mod->charset, mod->entries[num].name, STR0TERM);
					hyp_utf8_fprintf(outfile, _(" Unknown REF type %u: %s\n"), mod->entries[num].type, str);
					g_free(str);
				} else
				{
					hyp_utf8_fprintf(outfile, _(" Unknown REF type %u\n"), mod->entries[num].type);
				}
				break;
			}
		}
	}
	return TRUE;
}

/* ------------------------------------------------------------------------- */

const char *ref_osname(HYP_OS os)
{
	static char ret[2];
	
	switch (os)
	{
	case HYP_OS_UNKNOWN:
		return "0";
	case HYP_OS_AMIGA:
		return "1";
	case HYP_OS_ATARI:
		return "2";
	case HYP_OS_MAC:
		return "3";
	case HYP_OS_WIN32:
		return "4";
	case HYP_OS_UNIX:
		return "5";
	case HYP_OS_RES1:
	case HYP_OS_RES2:
	case HYP_OS_RES3:
	case HYP_OS_RES4:
		ret[0] = os + '0';
		ret[1] = '\0';
		return ret;
	}
	ret[0] = os;
	ret[1] = '\0';
	return ret;
}

/* ------------------------------------------------------------------------- */

static gboolean ref_load_modules(REF_FILE *ref, gboolean verbose)
{
	REF_MODULE *module;
	REF_MODULE **last;
	const unsigned char *pos;
	const unsigned char *end;
	const unsigned char *this_end;
	long this_size;
	long num, this_entries;
	char *str;
	gboolean os_found;
	gboolean charset_found;
	
	if (ref == NULL)
		return FALSE;
	
	last = &ref->modules;
	pos = ref->data;
	end = pos + ref->data_size;
	
	for (;;)
	{
		if ((pos + 2 * SIZEOF_LONG) >= end)
			break;
		this_size = long_from_chars(pos);
		this_entries = long_from_chars(pos + SIZEOF_LONG);
		if (this_entries <= 0 || this_size <= 0)
			break;
		pos += 2 * SIZEOF_LONG;
		this_end = pos + this_size;
		if (this_end > end)
		{
			this_end = end;
			if (verbose)
				hyp_utf8_fprintf(stderr, _("module len %ld exceeds file size\n"), (long)this_size);
		}
		module = g_new0(REF_MODULE, 1);
		module->module_len = this_size;
		module->module_offset = (long)(pos - ref->data) - 2 * SIZEOF_LONG + SIZEOF_LONG;
		module->num_entries = this_entries;
		/*
		 * allocate some entries more than we need so we
		 * can add missing entries below if neccessary
		 */
		module->entries = g_new(REF_ENTRY, this_entries + 3);
		module->data = pos;
		*last = module;
		last = &(*last)->next;
		if (module->entries == NULL)
		{
			module->num_entries = 0;
			return FALSE;
		}
		num = 0;
		os_found = FALSE;
		charset_found = FALSE;
		while (num < this_entries && pos < this_end)
		{
			unsigned char type;
			unsigned char size;

			type = *pos++;
			size = *pos++;
			if (size <= 2)
			{
				if (verbose)
					hyp_utf8_fprintf(stderr, _("premature end of REF module %s in %s\n"), printnull(module->filename), ref->filename);
				return FALSE;
			}
			module->entries[num].type = (hyp_reftype) type;
			module->entries[num].name = pos;
			module->entries[num].lineno = 0;
			size -= 2;
			/* FIXME: restrict strcmps() & strcpys() to result string size, entry size & ref->data_size */
			/* FIXME: check if the names are properly EOS terminated */
			switch (type)
			{
			case REF_FILENAME:
				if (module->module_filename != NULL)
				{
					if (verbose)
						hyp_utf8_fprintf(stderr, _("more than 1 filename in REF module %s of %s\n"), printnull(module->filename), ref->filename);
					g_free(module->filename);
				}
				module->module_filename = pos;
				/* convert now, because we might need it for error messages */
				module->filename = hyp_conv_to_utf8(module->charset, module->module_filename, STR0TERM);
				/* if file extension is missing: append '.hyp' */
				if (strrchr(module->filename, '.') == NULL)
				{
					 str = g_strconcat(module->filename, HYP_EXT_HYP, NULL);
					 g_free(module->filename);
					 module->filename = str;
				}
				break;
			case REF_NODENAME:
				/* name will be converted later */
				break;
			case REF_ALIASNAME:
				/* name will be converted later */
				break;
			case REF_TITLE:
				/* name will be converted later */
				break;
			case REF_LABELNAME:
				/* name will be converted later */
				module->entries[num].lineno = short_from_chars(pos + size - 2);
				break;
			case REF_DATABASE:
				if (module->module_database != NULL)
				{
					if (verbose)
						hyp_utf8_fprintf(stderr, _("more than 1 database in REF module %s of %s\n"), printnull(module->filename), ref->filename);
				}
				module->module_database = pos;
				break;
			case REF_OS:
				/*
				 * in a ref, these seem to be '0' + ostype
				 */
				if (os_found)
				{
					if (verbose)
						hyp_utf8_fprintf(stderr, _("OS specified more than once in REF module %s of %s\n"), printnull(module->filename), ref->filename);
				}
				os_found = TRUE;
				switch (*pos)
				{
				case '0':
					module->os = HYP_OS_UNKNOWN;
					if (verbose)
						hyp_utf8_fprintf(stderr, _("unknown OS specified in REF module %s of %s\n"), printnull(module->filename), ref->filename);
					break;
				case '1':
					module->os = HYP_OS_AMIGA;
					break;
				case '2':
					module->os = HYP_OS_ATARI;
					break;
				case '3':
					module->os = HYP_OS_MAC;
					break;
				case '4':
					module->os = HYP_OS_WIN32;
					break;
				case '5':
					module->os = HYP_OS_UNIX;
					break;
				case '6':
				case '7':
				case '8':
				case '9':
					module->os = (HYP_OS)(*pos - '0');
					if (verbose)
						hyp_utf8_fprintf(stderr, _("unknown os type '%c' in module %s of %s\n"), *pos, printnull(module->filename), ref->filename);
					break;
				default:
					module->os = (HYP_OS) *pos;
					if (verbose)
						hyp_utf8_fprintf(stderr, _("unknown os type '%s' in module %s of %s\n"), pos, printnull(module->filename), ref->filename);
					break;
				}
				break;
			case REF_CHARSET:
				if (charset_found)
				{
					if (verbose)
						hyp_utf8_fprintf(stderr, _("character set specified more than once in REF module %s of %s\n"), printnull(module->filename), ref->filename);
				}
				charset_found = TRUE;
				module->charset = hyp_charset_from_name((const char *) pos);
				if (module->charset == HYP_CHARSET_NONE)
				{
					module->charset = HYP_CHARSET_UTF8;
					if (verbose)
						hyp_utf8_fprintf(stderr, _("unknown character set %s in module %s of %s\n"), pos, printnull(module->filename), ref->filename);
				}
				break;
			}
			
			num++;
			pos += size;
		}

		if (pos > this_end)
		{
			if (verbose)
				hyp_utf8_fprintf(stderr, _("read beyond end of REF module %s in %s\n"), printnull(module->filename), ref->filename);
			pos = this_end;
		}
		if (pos != this_end)
		{
			if (verbose)
				hyp_utf8_fprintf(stderr, _("REF module %s in %s too short\n"), printnull(module->filename), ref->filename);
		}
		if (num != module->num_entries)
		{
			if (verbose)
				hyp_utf8_fprintf(stderr, _("only %ld entries out of %ld read for module %s in %s\n"), num, module->num_entries, printnull(module->filename), ref->filename);
			module->num_entries = num;
		}
		if (module->module_filename == NULL)
		{
			if (verbose)
				hyp_utf8_fprintf(stderr, _("no filename found in %s\n"), ref->filename);
		}
		if (!os_found)
		{
			if (verbose > 1)
				hyp_utf8_fprintf(stderr, _("No OS specified in REF module %s of %s\n"), printnull(module->filename), ref->filename);
			module->os = HYP_OS_ATARI;
		} else
		{
			module->had_os = TRUE;
		}

		if (!charset_found)
		{
			if (verbose > 1)
				hyp_utf8_fprintf(stderr, _("No character set specified in REF module %s of %s\n"), printnull(module->filename), ref->filename);
			module->charset = hyp_default_charset(module->os);
		} else
		{
			module->had_charset = TRUE;
		}

		if (module->module_filename != NULL)
		{
			char *name1, *name2;
			char *p;
			
			g_free(module->filename);
			module->filename = hyp_conv_to_utf8(module->charset, module->module_filename, STR0TERM);
			/* if file extension is missing: append '.hyp' */
			if (strrchr(module->filename, '.') == NULL)
			{
				 str = g_strconcat(module->filename, HYP_EXT_HYP, NULL);
				 g_free(module->filename);
				 module->filename = str;
			}
			module->had_filename = TRUE;
			
			name1 = g_strdup(hyp_basename(ref->filename));
			p = strrchr(name1, '.');
			if (p != NULL)
				*p = '\0';
			name2 = ref_hyp_basename(module->filename);
			module->mod_name_matches = g_utf8_strcasecmp(name1, name2) == 0;
			g_free(name2);
			g_free(name1);
		} else
		{
			char *str;
			char *p;
			size_t size;
			unsigned char *entry;
			gboolean converror = FALSE;
			
			/*
			 * if module did not contain a filename, add one
			 */
			g_free(module->filename);
			str = hyp_utf8_to_charset(module->charset, hyp_basename(ref->filename), STR0TERM, &converror);
			p = strrchr(str, '.');
			if (p != NULL)
				*p = '\0';
			size = strlen(str) + sizeof(HYP_EXT_HYP) + 2 + strlen(str) + sizeof(HYP_EXT_HYP);
			module->filename = g_new(char, size);
			strcat(strcpy(module->filename, str), HYP_EXT_HYP);
			size = strlen(str) + sizeof(HYP_EXT_HYP) + 2;
			entry = (unsigned char *)module->filename + size;
			entry[-2] = REF_FILENAME;
			entry[-1] = (unsigned char) size;
			module->module_filename = entry;
			strcat(strcpy((char *)entry, str), HYP_EXT_HYP);
			g_free(str);
			module->mod_name_matches = TRUE;
			
			/*
			 * add entry to table, but dont update module_len yet,
			 * since the data is not contained in module->data
			 */
			module->entries[num].name = module->module_filename;
			module->entries[num].type = REF_FILENAME;
			module->entries[num].lineno = 0;
			num++;
			module->num_entries++;
		}

		if (module->module_database != NULL)
		{
			module->database = hyp_conv_to_utf8(module->charset, module->module_database, STR0TERM);
		}
	}
	
	return TRUE;
}

/* ------------------------------------------------------------------------- */

REF_FILE *ref_new(const char *filename, size_t size)
{
	REF_FILE *ref = NULL;
	
	/*
	 * Allocate memory for header and index data.
	 * In case the file does not contain an EOF marker,
	 * allocate 8 more so we can add one if we have to.
	 */
	ref = (REF_FILE *)g_malloc(sizeof(REF_FILE) + size + 2 * SIZEOF_LONG);
	if (ref != NULL)
	{
		ref->data_size = size;
		ref->filename = g_strdup(filename);
		ref->modules = NULL;
	}
	return ref;
}

/* ------------------------------------------------------------------------- */

REF_FILE *ref_load(const char *filename, int handle, gboolean verbose)
{
	REF_FILE *ref = NULL;
	ssize_t size, pos, ret;
	unsigned char magic[SIZEOF_LONG];

	if (lseek(handle, 0L, SEEK_SET) != 0)
		return NULL;
	if (read(handle, magic, SIZEOF_LONG) != SIZEOF_LONG)
		return NULL;

	/* "magic value" in file header? */
	if (long_from_chars(magic) != HYP_MAGIC_REF)			/* 'HREF' */
	{
		if (verbose)
			hyp_utf8_fprintf(stderr, _("ERROR: Illegal REF file. Magic value not found.\n"));
		return NULL;
	}
	pos = lseek(handle, 0, SEEK_CUR);
	if (pos < 0)
		return NULL;
	size = lseek(handle, 0, SEEK_END) - pos;
	if (size < 0)
		return NULL;
	if (lseek(handle, pos, SEEK_SET) != pos)
		return NULL;
	/*
	 * file should contain at least 1 module
	 */
	if (size < 2 * SIZEOF_LONG)
	{
		if (verbose)
			hyp_utf8_fprintf(stderr, _("Illegal REF file. file too short.\n"));
		return NULL;
	}

	ref = ref_new(filename, size);
	if (ref == NULL)
		return NULL;
	
	if ((ret = read(handle, ref->data, size)) != size)
	{
		if (verbose)
			hyp_utf8_fprintf(stderr, _("read error, got %ld of %ld bytes\n"), (long) ret, (long)size);
		ref_close(ref);
		return NULL;
	}
	
	if (ref_load_modules(ref, verbose) == FALSE)
	{
		ref_close(ref);
		ref = NULL;
	}
	
	return ref;
}

/* ------------------------------------------------------------------------- */

void ref_close(REF_FILE *ref)
{
	REF_MODULE *module, *next;
	
	if (ref == NULL)
		return;
	for (module = ref->modules; module != NULL; module = next)
	{
		next = module->next;
		g_free(module->filename);
		g_free(module->database);
		g_free(module->entries);
		g_free(module);
	}
	g_free(ref->filename);
	g_free(ref);
}

/* ------------------------------------------------------------------------- */

int ref_num_modules(REF_FILE *ref)
{
	int count = 0;
	REF_MODULE *module;
	
	if (ref != NULL)
	{
		for (module = ref->modules; module != NULL; module = module->next)
			count++;
	}
	return count;
}

/* ------------------------------------------------------------------------- */

char *ref_hyp_basename(const char *filename)
{
	char *name = g_strdup(hyp_basename(filename));
	char *p;
	
	if ((p = strrchr(name, '.')) != NULL && g_ascii_strcasecmp(p, HYP_EXT_HYP) == 0)
		*p = '\0';
	return name;
}

/* ------------------------------------------------------------------------- */

/*
 * Find node by name in (first module of) REF file.
 */
char *ref_findnode(REF_FILE *ref, const char *search, hyp_lineno *line, gboolean only_first)
{
	hyp_nodenr node_num;
	REF_MODULE *mod;
	char *name;
	int res;
	
	if (ref == NULL)
		return FALSE;
	for (mod = ref->modules; mod != NULL; mod = mod->next)
	{
		for (node_num = 0; node_num < mod->num_entries; node_num++)
		{
			switch (mod->entries[node_num].type)
			{
			case REF_NODENAME:
			case REF_ALIASNAME:
			case REF_LABELNAME:
				name = hyp_conv_to_utf8(mod->charset, mod->entries[node_num].name, STR0TERM);
				res = strcmp(name, search);
				if (res == 0)
				{
					*line = mod->entries[node_num].lineno;
					if (mod->entries[node_num].type != REF_NODENAME)
					{
						while (node_num > 0)
						{
							--node_num;
							if (mod->entries[node_num].type == REF_NODENAME)
								break;
						}
						g_free(name);
						name = hyp_conv_to_utf8(mod->charset, mod->entries[node_num].name, STR0TERM);
					}
					return name;
				}
				g_free(name);
				break;
			case REF_FILENAME:
			case REF_DATABASE:
			case REF_OS:
			case REF_TITLE:
			case REF_UNKNOWN:
			case REF_CHARSET:
				break;
			default:
				break;
			}
		}
		if (only_first)
			break;
	}
	return NULL;
}

/* ------------------------------------------------------------------------- */

/*
 * Find node(s) by name in all modules of REF file.
 * Returns a linked list of entries found.
 */
RESULT_ENTRY *ref_findall(REF_FILE *ref, const char *string, long *num_results)
{
	RESULT_ENTRY *list;
	RESULT_ENTRY **last;
	RESULT_ENTRY prototype;
	REF_MODULE *mod;
	long num;
	int res;
	char *name;
	gboolean foundone;
	
	*num_results = 0;
	
	if (ref == NULL)
		return NULL;
	
	list = NULL;
	last = &list;

	for (mod = ref->modules; mod != NULL; mod = mod->next)
	{
		memset(&prototype, 0, sizeof(prototype));
		prototype.path = mod->filename;
		prototype.dbase_description = mod->database;
		for (num = 0; num < mod->num_entries; num++)
		{
			foundone = FALSE;
			switch (mod->entries[num].type)
			{
			case REF_FILENAME:
				break;
			case REF_NODENAME:
				name = hyp_conv_to_utf8(mod->charset, mod->entries[num].name, STR0TERM);
				g_free(prototype.node_name);
				prototype.node_name = name;
				res = strcmp(name, string);
				foundone = res == 0;
				break;
			case REF_ALIASNAME:
			case REF_LABELNAME:
				name = hyp_conv_to_utf8(mod->charset, mod->entries[num].name, STR0TERM);
				res = strcmp(name, string);
				if (res == 0)
				{
					foundone = TRUE;
					if (mod->entries[num].type == REF_LABELNAME)
						prototype.label_name = name;
					else
						prototype.alias_name = name;
					name = NULL;
					prototype.lineno = mod->entries[num].lineno;
				}
				g_free(name);
				break;
			case REF_DATABASE:
			case REF_OS:
			case REF_CHARSET:
			case REF_TITLE:
			case REF_UNKNOWN:
				break;
			default:
				break;
			}
			if (foundone)
			{
				RESULT_ENTRY *entry;

				entry = g_new(RESULT_ENTRY, 1);
				if (entry == NULL)
				{
					HYP_DBG(("Out of Memory in ref_findall()"));
					return list;
				}
				*entry = prototype;
				entry->node_name = g_strdup(prototype.node_name);
				(*num_results)++;

				*last = entry;
				last = (RESULT_ENTRY **)(&(*last)->item.next);
				prototype.alias_name = NULL;
				prototype.label_name = NULL;
			}
		}
		g_free(prototype.node_name);
	}
	
	return list;
}

/* ------------------------------------------------------------------------- */

void ref_freeresults(RESULT_ENTRY **result_list)
{
	RESULT_ENTRY *ptr, *next;

	ptr = *result_list;
	*result_list = NULL;

	while (ptr != NULL)
	{
		next = (RESULT_ENTRY *)ptr->item.next;
		g_free(ptr->node_name);
		g_free(ptr->alias_name);
		g_free(ptr->label_name);
		g_free(ptr);
		ptr = next;
	}
}
