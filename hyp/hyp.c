#include "hypdefs.h"
#include "hypdebug.h"
#include <stddef.h>
#include "hv_vers.h"


char const hyp_default_main_node_name[] = "Main";
char const hyp_default_index_node_name[] = "Index";
char const hyp_default_help_node_name[] = "Help";


/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

char *hyp_lib_version(void)
{
	return g_strdup_printf(_("Version %s for %s, %s"), HYP_VERSION, hyp_osname(hyp_get_current_os()), HYP_RELEASE_DATE);
}

/* ------------------------------------------------------------------------- */

char *hyp_compiler_version(void)
{
#define bitvers (sizeof(int) < 4 ? "16-bit" : sizeof(void *) >= 16 ? "128-bit" : sizeof(void *) >= 8 ? "64-bit" : "32-bit")
#define stringify1(x) #x
#define stringify(x) stringify1(x)

#if defined(_MSC_VER)
#  if _MSC_VER >= 2000
	return g_strdup_printf("MS Visual Studio, Compiler-Version %d.%d (%s)", _MSC_VER / 100, _MSC_VER % 100, bitvers);
#  elif _MSC_VER >= 1900
	return g_strdup_printf("Visual Studio 2015, Compiler-Version %d.%d (%s)", _MSC_VER / 100, _MSC_VER % 100, bitvers);
#  elif _MSC_VER >= 1800
	return g_strdup_printf("Visual Studio 2013, Compiler-Version %d.%d (%s)", _MSC_VER / 100, _MSC_VER % 100, bitvers);
#  elif _MSC_VER >= 1700
	return g_strdup_printf("Visual Studio 2012, Compiler-Version %d.%d (%s)", _MSC_VER / 100, _MSC_VER % 100, bitvers);
#  elif _MSC_VER >= 1600
	return g_strdup_printf("Visual Studio 2010 Compiler-Version %d.%d (%s)", _MSC_VER / 100, _MSC_VER % 100, bitvers);
#  elif _MSC_VER >= 1500
	return g_strdup_printf("Visual Studio 2008 Compiler-Version %d.%d (%s)", _MSC_VER / 100, _MSC_VER % 100, bitvers);
#  elif _MSC_VER >= 1400
	return g_strdup_printf("Visual Studio 2005 Compiler-Version %d.%d (%s)", _MSC_VER / 100, _MSC_VER % 100, bitvers);
#  elif _MSC_VER >= 1310
	return g_strdup_printf("Visual Studio 2003 Compiler-Version %d.%d (%s)", _MSC_VER / 100, _MSC_VER % 100, bitvers);
#  elif _MSC_VER >= 1300
	return g_strdup_printf("Visual Studio 2002 Compiler-Version %d.%d (%s)", _MSC_VER / 100, _MSC_VER % 100, bitvers);
#  elif _MSC_VER >= 1200
	return g_strdup_printf("Visual Studio 6.0 Compiler-Version %d.%d (%s)", _MSC_VER / 100, _MSC_VER % 100, bitvers);
#  elif _MSC_VER >= 1100
	return g_strdup_printf("Visual Studio 5.0 Compiler-Version %d.%d (%s)", _MSC_VER / 100, _MSC_VER % 100, bitvers);
#  else
	return g_strdup_printf("Ancient VC++ Compiler-Version %d.%d (%s)", _MSC_VER / 100, _MSC_VER % 100, bitvers);
#  endif
#elif defined(__INTEL_COMPILER)
	return g_strdup_printf("ICC version %d.%d.%d (%s)", __INTEL_COMPILER / 100, (__INTEL_COMPILER / 10) %10), __INTEL_COMPILER % 10, bitvers);
#elif defined(__clang__)
	return g_strdup_printf("clang version %s.%s.%s (%s)", stringify(__clang_major__), stringify(__clang_minor__), stringify(__clang_patchlevel__), bitvers);
#elif defined(__GNUC__)
	return g_strdup_printf("GNU-C version %s.%s.%s (%s)", stringify(__GNUC__), stringify(__GNUC_MINOR__), stringify(__GNUC_PATCHLEVEL__), bitvers);
#elif defined(__AHCC__)
	return g_strdup_printf("AHCC version %d.%02x (%s)", __AHCC__ >> 8, __AHCC__ & 0xff, bitvers);
#elif defined(__PUREC__)
	return g_strdup_printf("Pure-C version %d.%02x (%s)", __PUREC__ >> 8, __PUREC__ & 0xff, bitvers);
#elif defined(SOZOBON)
	return g_strdup_printf("Sozobon version %s (%s)", stringify(SOZOBON), bitvers);
#else
	return g_strdup_printf("Unknown Compiler (%s)", bitvers);
#endif
#undef bitvers

}

/*** ---------------------------------------------------------------------- ***/

char *chomp(char *str)
{
	if (str != NULL)
	{
		str = g_strchug(g_strchomp(str));
	}
	return str;
}

/* ------------------------------------------------------------------------- */

HYP_NODE *hyp_node_alloc(long size)
{
	HYP_NODE *node;
	
	node = (HYP_NODE *) g_malloc(sizeof(HYP_NODE) + size + 1);
	if (node != NULL)
	{
		node->number = HYP_NOINDEX;
		node->start = (unsigned char *)node + sizeof(*node);
		node->end = &node->start[size];
		node->height = 0;
		node->width = 0;
		node->window_title = NULL;
		node->line_ptr = NULL;
		node->gfx = NULL;
	}
	return node;
}

/* ------------------------------------------------------------------------- */

void hyp_node_free(HYP_NODE *node)
{
	if (node != NULL)
	{
		g_free(node->line_ptr);
		hyp_free_graphics(node);
		g_free(node);
	}
}

/* ------------------------------------------------------------------------- */

void hyp_image_free(HYP_IMAGE *image)
{
	if (image)
	{
		if (image->decompressed)
			W_Release_Bitmap(&image->pic.fd_addr, image->pic.fd_w, image->pic.fd_h, image->pic.fd_nplanes);
		else
			g_free(image->pic.fd_addr);
		g_free(image);
	}
}

/* ------------------------------------------------------------------------- */

const char *hyp_osname(HYP_OS os)
{
	static char ret[20];
	
	switch (os)
	{
	case HYP_OS_UNKNOWN:
		return _("Unknown");
	case HYP_OS_AMIGA:
		return "Amiga";
	case HYP_OS_ATARI:
		return "Atari";
	case HYP_OS_MAC:
		return "Macintosh";
	case HYP_OS_WIN32:
		return "Windows";
	case HYP_OS_UNIX:
		return "Unix";
	case HYP_OS_RES1:
	case HYP_OS_RES2:
	case HYP_OS_RES3:
	case HYP_OS_RES4:
		sprintf(ret, _("OS %c"), os + '0');
		return ret;
	}
	sprintf(ret, _("Type 0x%02x"), os);
	return ret;
}

/* ------------------------------------------------------------------------- */

HYP_OS hyp_os_from_name(const char *name)
{
	if (g_ascii_strcasecmp(name, "amiga") == 0)
		return HYP_OS_AMIGA;
	if (g_ascii_strcasecmp(name, "atari") == 0)
		return HYP_OS_ATARI;
	if (g_ascii_strcasecmp(name, "macintosh") == 0)
		return HYP_OS_MAC;
	if (g_ascii_strcasecmp(name, "windows") == 0)
		return HYP_OS_WIN32;
	if (g_ascii_strcasecmp(name, "unix") == 0)
		return HYP_OS_UNIX;

	/* names that might be generated by UDO */
	if (g_ascii_strcasecmp(name, "TOS") == 0)
		return HYP_OS_ATARI;
	if (g_ascii_strcasecmp(name, "Mac OS X") == 0)
		return HYP_OS_MAC;
	if (g_ascii_strcasecmp(name, "Mac OS") == 0)
		return HYP_OS_MAC;
	if (g_ascii_strcasecmp(name, "Win32") == 0)
		return HYP_OS_WIN32;
	if (g_ascii_strcasecmp(name, "Linux") == 0)
		return HYP_OS_UNIX;
	if (g_ascii_strcasecmp(name, "SunOS") == 0)
		return HYP_OS_UNIX;
	if (g_ascii_strcasecmp(name, "Sinix") == 0)
		return HYP_OS_UNIX;
	if (g_ascii_strcasecmp(name, "IRIX") == 0)
		return HYP_OS_UNIX;
	if (g_ascii_strcasecmp(name, "NeXTStep") == 0)
		return HYP_OS_UNIX;
	if (g_ascii_strcasecmp(name, "OS/2") == 0)
		return HYP_OS_UNIX; /* a lie */
	if (g_ascii_strcasecmp(name, "DOS") == 0)
		return HYP_OS_UNIX; /* another lie */
	if (g_ascii_strcasecmp(name, "BeOS") == 0)
		return HYP_OS_UNIX;
	return HYP_OS_UNKNOWN;
}

/* ------------------------------------------------------------------------- */

hyp_filetype hyp_guess_filetype(const char *name)
{
	const char *p = strrchr(hyp_basename(name), '.');
	
	if (p == NULL)
		return HYP_FT_NONE;
	if (g_ascii_strcasecmp(p, HYP_EXT_HYP) == 0)
		return HYP_FT_HYP;
	if (g_ascii_strcasecmp(p, HYP_EXT_REF) == 0)
		return HYP_FT_REF;
	if (g_ascii_strcasecmp(p, HYP_EXT_STG) == 0)
		return HYP_FT_STG;
	if (g_ascii_strcasecmp(p, HYP_EXT_RSC) == 0)
		return HYP_FT_RSC;
	if (g_ascii_strcasecmp(p, HYP_EXT_GUIDE) == 0)
		return HYP_FT_GUIDE;
	if (g_ascii_strcasecmp(p, ".gui") == 0)
		return HYP_FT_GUIDE;
	if (g_ascii_strcasecmp(p, HYP_EXT_HTML) == 0)
		return HYP_FT_HTML;
	if (g_ascii_strcasecmp(p, ".htm") == 0)
		return HYP_FT_HTML;
	if (g_ascii_strcasecmp(p, HYP_EXT_XML) == 0)
		return HYP_FT_XML;
	if (g_ascii_strcasecmp(p, ".h") == 0)
		return HYP_FT_CHEADER;
	if (g_ascii_strcasecmp(p, ".rsh") == 0)
		return HYP_FT_CHEADER;
	if (g_ascii_strcasecmp(p, ".iff") == 0)
		return HYP_FT_IMAGE;
	if (g_ascii_strcasecmp(p, ".ilbm") == 0)
		return HYP_FT_IMAGE;
	if (g_ascii_strcasecmp(p, ".icn") == 0)
		return HYP_FT_IMAGE;
	if (g_ascii_strcasecmp(p, ".img") == 0)
		return HYP_FT_IMAGE;
	if (g_ascii_strcasecmp(p, ".bmp") == 0)
		return HYP_FT_IMAGE;
	if (g_ascii_strcasecmp(p, ".gif") == 0)
		return HYP_FT_IMAGE;
	if (g_ascii_strcasecmp(p, ".png") == 0)
		return HYP_FT_IMAGE;
	if (g_ascii_strcasecmp(p, ".jpeg") == 0)
		return HYP_FT_IMAGE;
	if (g_ascii_strcasecmp(p, ".jpg") == 0)
		return HYP_FT_IMAGE;
	return HYP_FT_UNKNOWN;
}

/* ------------------------------------------------------------------------- */

HYP_OS hyp_get_current_os(void)
{
#if defined(__TOS__)
	return HYP_OS_ATARI;
#elif defined(__AMIGA__)
	return HYP_OS_AMIGA;
#elif defined(__MACOS__)
	return HYP_OS_MAC;
#elif defined(__WIN32__)
	return HYP_OS_WIN32;
#elif defined(__unix__) || defined(__unix) || defined(unix)
	return HYP_OS_UNIX;
#else
	return HYP_OS_UNKNOWN;
#endif
}

/* ------------------------------------------------------------------------- */

char *hyp_find_file(const char *path)
{
	int ret;
	const char *list, *end;
	char *real_path;
	char *dir, *tmp;
	char *filename;
	
	filename = g_strdup(path);
	/* try original path first */
	ret = hyp_utf8_open(filename, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
	if (ret >= 0)
	{
		hyp_utf8_close(ret);
		if (g_path_is_absolute(filename))
		{
			real_path = g_strdup(filename);
		} else
		{
			dir = g_get_current_dir();
			real_path = g_build_filename(dir, filename, NULL);
			g_free(dir);
		}
		g_free(filename);
		return real_path;
	}

	if (g_path_is_absolute(filename))
	{
		path = hyp_basename(path);
		g_free(filename);
		filename = g_strdup(path);
	}

	/* locate file on pathlist */
	if (!empty(gl_profile.general.path_list))
	{
		list = gl_profile.general.path_list;
		while (*list)
		{
			end = list;
			while (*end != '\0' && (*end != G_SEARCHPATH_SEPARATOR))
				end++;
			tmp = g_strndup(list, end - list);
			dir = path_subst(tmp);
			real_path = g_build_filename(dir, filename, NULL);
			g_free(tmp);
			list = end;
			if (*list != '\0')
				list++;
			ret = hyp_utf8_open(real_path, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
#if defined(G_OS_UNIX) /* should be: if filesystem of <dir> is case-sensitive */
			/*
			 * filenames from external references are unpredictable,
			 * try lowercase version, too.
			 */
			if (ret < 0)
			{
				char *lower = hyp_utf8_strdown(filename, -1);
				if (strcmp(lower, filename) != 0)
				{
					g_free(real_path);
					real_path = g_build_filename(dir, lower, NULL);
					ret = hyp_utf8_open(real_path, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
				}
				g_free(lower);
			}
#endif
			g_free(dir);
			if (ret >= 0)
			{
				hyp_utf8_close(ret);
				g_free(filename);
				return real_path;
			}
			g_free(real_path);
		}
	}
	
	g_free(filename);
	return NULL;
}

/* ------------------------------------------------------------------------- */

static char *load_string(HYP_CHARSET charset, int handle, unsigned short len)
{
	char *str;
	char *res;
	
	if (len == 0)
		return NULL;
	str = g_new(char, len + 1);
	if (str == NULL)
		return str;
	if (read(handle, str, len) != len)
	{
		g_free(str);
		return NULL;
	}
	str[len] = '\0';
	if ((len > 1 && str[len - 1] != '\0' && str[len - 2] != '\0') ||
		(len == 1 && str[0] != '\0'))
	{
		HYP_DBG(("str not zero-terminated: %s", printnull(str)));
	}
	res = hyp_conv_to_utf8(charset, str, len);
	g_free(str);
	return res;
}

/* ------------------------------------------------------------------------- */

static gboolean is_big_endian(void)
{
	static unsigned char const c[4] = { 0x12, 0x34, 0x45, 0x78 };
	return long_from_chars(c) == 0x12345678UL;
}

/* ------------------------------------------------------------------------- */

/*
 * check if layout on disk is same as in memory;
 * this allows us to skip decomposing the structure
 */
static gboolean flat_indexentry(void)
{
	return offsetof(INDEX_ENTRY, length) == 0 &&
	       offsetof(INDEX_ENTRY, type) == 1 &&
	       offsetof(INDEX_ENTRY, seek_offset) == 2 &&
	       offsetof(INDEX_ENTRY, comp_diff) == 6 &&
	       offsetof(INDEX_ENTRY, next) == 8 &&
	       offsetof(INDEX_ENTRY, previous) == 10 &&
	       offsetof(INDEX_ENTRY, toc_index) == 12 &&
	       offsetof(INDEX_ENTRY, name) == SIZEOF_INDEX_ENTRY &&
	       is_big_endian();
}

/* ------------------------------------------------------------------------- */

void hyp_delete(HYP_DOCUMENT *hyp)
{
	hyp_nodenr node;

	if (G_UNLIKELY(hyp == NULL))
		return;
	ASSERT(hyp->ref_count == 0);
	ClearCache(hyp);
	ref_close(hyp->ref);
	g_free(hyp->database);
	{
		HYP_HOSTNAME *h, *next;
		
		for (h = hyp->hostname; h != NULL; h = next)
		{
			next = h->next;
			g_free(h->name);
			g_free(h);
		}
	}
	g_free(hyp->author);
	g_free(hyp->version);
	g_free(hyp->hcp_options);
	g_free(hyp->subject);
	g_free(hyp->help_name);
	g_free(hyp->default_name);
	g_free(hyp->language);
	g_free(hyp->hyptree_data);
	
	if (hyp->num_index > 0)
	{
		if (hyp->indextable != NULL)
		{
			/* last entry is always malloced */
			g_free(hyp->indextable[hyp->num_index]);
			if (hyp->flatindex)
			{
				/* first entry contains all of the data */
				g_free(hyp->indextable[0]);
			} else
			{
				for (node = 0; node < hyp->num_index; node++)
				{
					g_free(hyp->indextable[node]);
				}
			}
			g_free(hyp->indextable);
		}
	}
	g_free(hyp);
}

/* ------------------------------------------------------------------------- */

HYP_DOCUMENT *hyp_unref(HYP_DOCUMENT *hyp)
{
	if (G_UNLIKELY(hyp == NULL))
		return hyp;
	ASSERT(hyp->ref_count >= 1);
	if (--hyp->ref_count == 0)
	{
		hyp_delete(hyp);
		hyp = NULL;
	}
	return hyp;
}

/* ------------------------------------------------------------------------- */

HYP_DOCUMENT *hyp_ref(HYP_DOCUMENT *hyp)
{
	if (G_UNLIKELY(hyp == NULL))
		return hyp;
	++hyp->ref_count;
	ASSERT(hyp->ref_count >= 2);
	return hyp;
}

/* ------------------------------------------------------------------------- */

HYP_DOCUMENT *hyp_new(void)
{
	HYP_DOCUMENT *hyp = g_new0(HYP_DOCUMENT, 1);
	
	if (G_UNLIKELY(hyp == NULL))
		return hyp;

	/* default-values for information later read from exteneded headers */
	hyp->comp_os = hyp_get_current_os();
	hyp->comp_charset = hyp_default_charset(hyp->comp_os);
	hyp->st_guide_flags = 0;
	hyp->line_width = HYP_STGUIDE_DEFAULT_LINEWIDTH;
	
	hyp->default_page = HYP_NOINDEX;
	hyp->help_page = HYP_NOINDEX;
	hyp->main_page = HYP_NOINDEX;
	hyp->index_page = HYP_NOINDEX;
	hyp->first_text_page = HYP_NOINDEX;
	hyp->last_text_page = HYP_NOINDEX;

	hyp->handle = -1;
	hyp->ref_count = 1;
	
	return hyp;
}

/* ------------------------------------------------------------------------- */

char *hyp_invalid_page(hyp_nodenr page)
{
	return g_strdup_printf(_("<invalid destination page %u>"), page);
}

/* ------------------------------------------------------------------------- */

HYP_DOCUMENT *hyp_load(const char *filename, int handle, hyp_filetype *err)
{
	HYP_DOCUMENT *hyp;
	HYP_HEADER head;
	unsigned char rawhead[SIZEOF_HYP_HEADER];
	unsigned char *idxent;
	unsigned char *current_pos;
	unsigned char info[2 * SIZEOF_SHORT];
	hyp_nodenr i;
	ssize_t ret;

	/* read file header */
	if (lseek(handle, 0L, SEEK_SET) != 0)
	{
		*err = HYP_FT_UNKNOWN;
		return NULL;
	}
	ret = read(handle, rawhead, SIZEOF_HYP_HEADER);
	if (ret != SIZEOF_HYP_HEADER)
	{
		/* error... */
		HYP_DBG(("Error %s in read(%s)", hyp_utf8_strerror(errno), printnull(filename)));
		*err = HYP_FT_UNKNOWN;
		return NULL;
	}
	
	head.magic = long_from_chars(rawhead);
	head.itable_size = long_from_chars(rawhead + 4);
	head.itable_num = short_from_chars(rawhead + 8);
	head.compiler_vers = rawhead[10];
	head.compiler_os = rawhead[11];
	
	HYP_DBG(("hyp-Header: Magic %08lx (%c%c%c%c) itable_size %lu itable_num %u vers %d sys %d",
		head.magic, rawhead[0], rawhead[1], rawhead[2], rawhead[3],
		head.itable_size,
		head.itable_num,
		head.compiler_vers,
		head.compiler_os));

	if (head.magic != HYP_MAGIC_HYP)	/* 'HDOC' */
	{
		/* "magic value" not found... */
		HYP_DBG(("ERROR: Wrong file format. Magic value not found."));
		*err = HYP_FT_UNKNOWN;
		return NULL;
	}

	if (head.itable_size < 0)
	{
		HYP_DBG(("ERROR: negative size of index table size found!"));
		*err = HYP_FT_UNKNOWN;
		return NULL;
	}

	hyp = hyp_new();
	if (hyp == NULL)
	{
		*err = HYP_FT_LOADERROR;
		return NULL;
	}
	hyp->handle = handle;
	hyp->file = filename;
	
	hyp->num_index = head.itable_num;
	hyp->itable_size = head.itable_size;
	hyp->comp_vers = head.compiler_vers;
	hyp->comp_os = (HYP_OS) head.compiler_os;
	hyp->comp_charset = hyp_default_charset(hyp->comp_os);
	
	if (hyp->itable_size > 0 && hyp->num_index > 0)
	{
		const unsigned char *end;
		
		/* read index data, just beyond the file header */
		hyp->indextable = g_new0(INDEX_ENTRY *, hyp->num_index + 1);
		if (hyp->indextable == NULL)
		{
			hyp_unref(hyp);
			*err = HYP_FT_LOADERROR;
			return NULL;
		}
		
		/* allocate buffer, either temp or permanent */
		idxent = g_new(unsigned char, head.itable_size);
	
		ret = read(handle, idxent, head.itable_size);	/* load entries */
		if (ret != head.itable_size)
		{
			g_free(idxent);
			hyp_unref(hyp);
			*err = HYP_FT_LOADERROR;
			return NULL;
		}
		
		/*
		 * convert entries, if not flat,
		 * and generate index table
		 */
		current_pos = idxent;
		end = current_pos + head.itable_size;
		
		if (flat_indexentry())
		{
			/* index data kept in first entry */
			hyp->flatindex = TRUE;
			for (i = 0; i < hyp->num_index; i++)
			{
				/*
				 * address must be kept even; could maybe
				 * fall back to code below,
				 * but since the compiler is doing the same,
				 * something has gone wrong in that case
				 */
				if (current_pos >= end ||
					(current_pos[0] < SIZEOF_INDEX_ENTRY) ||
					(current_pos[0] & 1))
				{
					g_free(idxent);
					hyp->indextable[0] = NULL;
					hyp_unref(hyp);
					*err = HYP_FT_LOADERROR;
					return NULL;
				}
				hyp->indextable[i] = (INDEX_ENTRY *)current_pos;
				current_pos += current_pos[0];
			}
		} else
		{
			for (i = 0; i < hyp->num_index; i++)
			{
				INDEX_ENTRY *entry;
				size_t namelen;
				
				if (current_pos >= end ||
					(current_pos[0] < SIZEOF_INDEX_ENTRY) ||
					(current_pos[0] & 1))
				{
					g_free(idxent);
					hyp_unref(hyp);
					*err = HYP_FT_LOADERROR;
					return NULL;
				}
				namelen = current_pos[0] - SIZEOF_INDEX_ENTRY;
				entry = (INDEX_ENTRY *)g_malloc(sizeof(INDEX_ENTRY) + namelen);
				hyp->indextable[i] = entry;
				entry->length = current_pos[0];
				entry->type = current_pos[1];
				entry->seek_offset = long_from_chars(current_pos + 2);
				entry->comp_diff = short_from_chars(current_pos + 6);
				entry->next = short_from_chars(current_pos + 8);
				entry->previous = short_from_chars(current_pos + 10);
				entry->toc_index = short_from_chars(current_pos + 12);
				memcpy(entry->name, current_pos + SIZEOF_INDEX_ENTRY, namelen);
				entry->name[namelen] = '\0';
				current_pos += entry->length;
			}
			/*
			 * all data has been copied, free buffer
			 */
			g_free(idxent);
			hyp->flatindex = FALSE;
		}
		
		/*
		 * looks like original hcp had a bug,
		 * setting the comp_diff member of the EOF
		 * entry to the value from the previous entry
		 */
		if (hyp->indextable[hyp->num_index - 1]->type == HYP_NODE_EOF)
			hyp->indextable[hyp->num_index - 1]->comp_diff = 0;
		
		hyp->indextable[hyp->num_index] = g_new0(INDEX_ENTRY, 1);
		if (hyp->indextable[hyp->num_index] == NULL)
		{
			hyp_unref(hyp);
			*err = HYP_FT_LOADERROR;
			return NULL;
		}
		hyp->indextable[hyp->num_index]->seek_offset = 0;
	} else
	{
		/*
		 * empty file.
		 * dunno wether it makes sense to continue,
		 * but hcp will refuse to create such files anyway.
		 */
	}
	
	/* load extra headers */
	for (;;)
	{
		hyp_ext_header type;
		unsigned short len;
		
		ret = read(handle, info, 2 * SIZEOF_SHORT);
		if (ret != 2 * SIZEOF_SHORT)
			break;
		type = (hyp_ext_header)(int)short_from_chars(info);
		if (type == HYP_EXTH_EOF)
			break;
		len = short_from_chars(info + SIZEOF_SHORT);
		
		switch (type)
		{
		case HYP_EXTH_EOF:
			break;
		case HYP_EXTH_DATABASE:						/* @database */
			hyp->database = load_string(hyp->comp_charset, handle, len);
			break;
		case HYP_EXTH_DEFAULT:						/* @default */
			hyp->default_name = load_string(hyp->comp_charset, handle, len);
			hyp->default_page = find_nr_by_title(hyp, hyp->default_name);
			if (!hypnode_valid(hyp, hyp->default_page))
			{
				HYP_DBG(("default page %s not found", printnull(hyp->default_name)));
			}
			break;
		case HYP_EXTH_HOSTNAME:						/* @hostname */
			{
				char *name = load_string(hyp->comp_charset, handle, len);
				HYP_HOSTNAME **last;
				HYP_HOSTNAME *h;
				
				if (name != NULL)
				{
					unsigned short p, start;
					
					/*
					 * not originally documented, but the string is
					 * a doubly NUL-terminated list of names
					 */
					p = 0;
					while (p < len && name[p] != '\0')
					{
						start = p;
						while (p < len && name[p] != '\0')
							p++;
						if (p > start)
						{
							h = g_new(HYP_HOSTNAME, 1);
							h->name = g_strndup(name + start, p - start);
							h->next = NULL;
							last = &hyp->hostname;
							while (*last)
								last = &(*last)->next;
							*last = h;
						}
						if (p < len && name[p] == '\0')
							p++;
					}
					g_free(name);
				}
			}
			break;
		case HYP_EXTH_OPTIONS:						/* @options */
			hyp->hcp_options = load_string(hyp->comp_charset, handle, len);
			break;
		case HYP_EXTH_AUTHOR:						/* @author */
			hyp->author = load_string(hyp->comp_charset, handle, len);
			break;
		case HYP_EXTH_VERSION:						/* @$VER */
			hyp->version = load_string(hyp->comp_charset, handle, len);
			if (hyp->version && strlen(hyp->version) >= 6 && g_ascii_strncasecmp(hyp->version, "$VER: ", 6) == 0)
			{
				memmove(hyp->version, hyp->version + 6, strlen(hyp->version + 6) + 1);
				if (strlen(hyp->version) == 0)
				{
					g_free(hyp->version);
					hyp->version = NULL;
				}
			}
			break;
		case HYP_EXTH_HELP:							/* @help */
			hyp->help_name = load_string(hyp->comp_charset, handle, len);
			hyp->help_page = find_nr_by_title(hyp, hyp->help_name);
			if (!hypnode_valid(hyp, hyp->help_page))
			{
				HYP_DBG(("help page %s not found", printnull(hyp->help_name)));
			}
			break;
		case HYP_EXTH_SUBJECT:						/* @subject */
			hyp->subject = load_string(hyp->comp_charset, handle, len);
			break;
		case HYP_EXTH_TREEHEADER:					/* HypTree-Header */
			/* FIXME: NYI: tree header */
			hyp->hyptree_len = len;
			if (len != 0)
			{
				hyp->hyptree_data = g_new(unsigned char, hyp->hyptree_len);
				if (hyp->hyptree_data != NULL)
					read(handle, hyp->hyptree_data, len);
				else
					lseek(handle, len, SEEK_CUR);
			}
			break;
		case HYP_EXTH_STGUIDE_FLAGS:				/* ST-Guide flags */
			if (len != SIZEOF_SHORT)
			{
				HYP_DBG(("skipping flags entry with illegal len %u", len));
				lseek(handle, len, SEEK_CUR);
			} else
			{
				/*
				 * Apparently ST-Guide HCP only uses the first byte here
				 * (same as with linewidth below).
				 * if you start hcp with -k1, that field
				 * will contain 0x01 0x00.
				 * To be able to use the extra byte later,
				 * we therefore read it in little endian order.
				 */
				if (read(handle, info, SIZEOF_SHORT) == SIZEOF_SHORT)
					hyp->st_guide_flags = short_from_lechars(info);
			}
			break;
		case HYP_EXTH_WIDTH:						/* @width */
			if (len != SIZEOF_SHORT)
			{
				HYP_DBG(("skipping width entry with illegal len %u", len));
				lseek(handle, len, SEEK_CUR);
			} else
			{
				if (read(handle, info, SIZEOF_SHORT) == SIZEOF_SHORT)
				{
					/*
					 * this should actually use a big-endian short,
					 * but apparently HCP only uses the first byte,
					 * then aligns the entry to an even address.
					 * Same as with compat flags above.
					 */
					hyp->line_width = short_from_lechars(info);
				}
			}
			break;
		case HYP_EXTH_CHARSET:						/* @charset */
			{
				char *charset = load_string(hyp->comp_charset, handle, len);
				HYP_CHARSET cset = hyp_charset_from_name(charset);
				g_free(charset);
				if (cset != HYP_CHARSET_NONE)
					hyp->comp_charset = cset;
			}
			break;
		default:
			HYP_DBG(("skipping unknown entry type %u len %u", type, len));
			lseek(handle, len, SEEK_CUR);
			break;
		}
	}

	/*
	 * find standard help page name if not specified in file
	 */
	if (!hypnode_valid(hyp, hyp->help_page))
	{
		hyp->help_page = find_nr_by_title(hyp, hyp_default_help_node_name);
	}
	
	/*
	 * Find standard index page. Note that there is no
	 * command to specify a different name in the file.
	 */
	if (!hypnode_valid(hyp, hyp->index_page))
	{
		hyp->index_page = find_nr_by_title(hyp, hyp_default_index_node_name);
	}
	
	/*
	 * Find main page. Note that there is no
	 * command to specify a different name in the file.
	 */
	if (!hypnode_valid(hyp, hyp->main_page))
	{
		hyp->main_page = find_nr_by_title(hyp, hyp_default_main_node_name);
	}
	
	/* there is no standard name for a default page */
	
	/*
	 * set page size of last entry
	 */
	if (hyp->num_index > 0)
		hyp->indextable[hyp->num_index]->seek_offset = lseek(handle, 0, SEEK_END);
	
	hyp->first_text_page = hyp_first_text_page(hyp);
	hyp->last_text_page = hyp_last_text_page(hyp);

	if (!hypnode_valid(hyp, hyp->main_page))
		hyp->main_page = hyp->first_text_page;
	
	*err = HYP_FT_HYP;
	return hyp;
}
