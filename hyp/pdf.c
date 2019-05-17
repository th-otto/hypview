#include "hypdefs.h"
#include "hypdebug.h"
#include "hcp_opts.h"
#include "pdf.h"

/*
 * gzip header:
 *    magic 0x1f 0x8b
 *    cm    0x08
 *    flg   0x00
 *    mtime 0x00 0x00 0x00 0x00
 *    xfl   0x02
 *    os    0x03
 * 
 *
 * deflate stream header:
 *   CMF FLG
 *
 *   CMF: compression method and flags
 *        0..3: CM compression method (8)
 *        4..7: CINFO compression info (ln2 window-size, minus 8)
 *              eg. value of 7 = 1 << 15 = 32k windows size
 *
 *   FLG: Flags
 *        0..4 FCHECK check bits for CMF and FLG
 *             (CMF*256 + FLG) must be multiple of 31
 *        5    FDICT preset dictionary
 *             if set, a DICTID identifier follows
 *        6..7 Compression level (informative only)
 */

/*
 * node/label names are case sensitiv
 */
#define namecmp strcmp

/* ------------------------------------------------------------------------- */

PDF *pdf_new(void)
{
	PDF *pdf;
	
	pdf = g_new(PDF, 1);
	if (pdf == NULL)
		return pdf;
	pdf->out = g_string_new(NULL);
	pdf->objects = NULL;
	pdf->obj_num = 0;
	pdf->last_obj = &pdf->objects;
	return pdf;
}
	
/* ------------------------------------------------------------------------- */

void pdf_delete(PDF *pdf)
{
	if (pdf == NULL)
		return;
	g_string_free(pdf->out, TRUE);
	g_free(pdf);
}

/* ------------------------------------------------------------------------- */

PDF_OBJ *pdf_obj_new(PDF *pdf)
{
	PDF_OBJ *obj;
	
	obj = g_new(PDF_OBJ, 1);
	if (obj == NULL)
		return obj;
	obj->num = pdf->obj_num++;
	obj->out = g_string_new(NULL);
	obj->stream = NULL;
	obj->next = NULL;
	obj->fileoffset = 0;
	obj->len = 0;
	*pdf->last_obj = obj;
	pdf->last_obj = &obj->next;
	return obj;
}

static char *pdf_quote_name(const char *name, unsigned int flags, size_t *lenp)
{
	unsigned short *str, *ret;
	size_t len;
	int need_utfbe = FALSE;

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define BE16(x) x
#else
#define BE16(x) (((unsigned short)((x) & 0xff) << 8) | ((unsigned short)((x) & 0xff00) >> 8))
#endif

	if (name == NULL)
	{
		*lenp = 0;
		return NULL;
	}
	len = strlen(name);
	str = ret = g_new(unsigned short, len * 2 + 3);
	if (str != NULL)
	{
		*str++ = BE16(0xfeffu);
		while (*name)
		{
			unsigned char c = *name++;
			switch (c)
			{
			case '\\':
			case '(':
			case ')':
			case '<':
			case '>':
			case '[':
			case ']':
			case '{':
			case '}':
			case '/':
			case '%':
				*str++ = BE16('\\');
				*str++ = BE16(c);
				break;
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
			case 0x05:
			case 0x06:
			case 0x07:
			case 0x08:
			case 0x09:
			case 0x0a:
			case 0x0b:
			case 0x0c:
			case 0x0d:
			case 0x0e:
			case 0x0f:
			case 0x10:
			case 0x11:
			case 0x12:
			case 0x13:
			case 0x14:
			case 0x15:
			case 0x16:
			case 0x17:
			case 0x18:
			case 0x19:
			case 0x1a:
			case 0x1b:
			case 0x1c:
			case 0x1D:
			case 0x1E:
			case 0x1F:
				*str++ = BE16(c);
				break;
			default:
				if (c >= 0x80)
				{
					h_unichar_t wc;
					--name;
					name = hyp_utf8_getchar(name, &wc);
					if (wc >= 0x10000u)
					{
						*str++ = BE16(wc);
					} else
					{
						*str++ = BE16(wc);
					}
					need_utfbe = TRUE;
				} else
				{
					*str++ = BE16(c);
				}
				break;
			}
		}
		*str++ = BE16(0);
		len = str - ret;
		if (!need_utfbe)
		{
			size_t i = len - 1;
			char *dst = (char *)ret + i;
			
			for (; i != 0; i++)
				*--dst = *--str;
		} else
		{
			len *= sizeof(*ret);
		}
	}
#undef BE16
	(void) flags;
	*lenp = len;
	return (char *)ret;
}

/* ------------------------------------------------------------------------- */

static void pdf_write_objects(PDF *pdf, hcp_opts *opts)
{
	PDF_OBJ *obj;
	
	for (obj = pdf->objects; obj != NULL; obj = obj->next)
	{
		obj->fileoffset = pdf->out->len;
		g_string_append_printf(pdf->out, "%d 0 obj\n", obj->num);
		if (obj->stream)
		{
			if (obj->out->len == 0)
			{
				g_string_append_printf(obj->out, "<</Length %ld>>\n", (long)obj->stream->len);
			} else if (obj->out->len < 5 ||
				obj->out->str[0] != '<' ||
				obj->out->str[1] != '<' ||
				obj->out->str[obj->out->len - 3] != '>' ||
				obj->out->str[obj->out->len - 2] != '>' ||
				obj->out->str[obj->out->len - 1] != '\n')
			{
				hyp_utf8_fprintf(opts->errorfile, _("obj without dictionary\n"));
			} else
			{
				g_string_truncate(obj->out, obj->out->len - 3);
				g_string_append_printf(obj->out, "/Length %ld>>\n", (long)obj->stream->len);
			}
			if (obj->stream->len != 0)
				g_string_append_c(obj->stream, '\n');
		}
		g_string_append_len(pdf->out, obj->out->str, obj->out->len);
		if (obj->stream)
		{
			g_string_append(pdf->out, "stream\n");
			g_string_append(pdf->out, "endstream\n");
		}
		g_string_append(pdf->out, "endobj\n");
	}
}

/* ------------------------------------------------------------------------- */

static gboolean pdf_out_node(HYP_DOCUMENT *hyp, hcp_opts *opts, PDF *pdf, hyp_nodenr node)
{
	(void) hyp;
	(void) opts;
	(void) pdf;
	(void) node;
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static char *pdf_datestr(time_t t)
{
	struct tm tm;
	int gmtoff;
	int c;
	
	localtime_r(&t, &tm);
	gmtoff = (int)(tm.tm_gmtoff / 60);
	c = gmtoff < 0 ? '-' : '+';
	if (gmtoff < 0)
		gmtoff = -gmtoff;
	return g_strdup_printf("D:%04d%02d%02d%02d%02d%02d%c%02d'%02d'",
		tm.tm_year + 1900,
		tm.tm_mon + 1,
		tm.tm_mday,
		tm.tm_hour,
		tm.tm_min,
		tm.tm_sec,
		c,
		gmtoff / 60,
		gmtoff % 60);
}

/* ------------------------------------------------------------------------- */

void pdf_out_globals(HYP_DOCUMENT *hyp, hcp_opts *opts, PDF *pdf)
{
	PDF_OBJ *obj;
	char *str;
	size_t len;
	struct stat s;
	
	UNUSED(opts);
	obj = pdf_obj_new(pdf);
	g_string_append(obj->out, "<<\n");

#define STR(t, x) \
	if (x != NULL) \
	{ \
		g_string_append(obj->out, t); \
		g_string_append(obj->out, " ("); \
		str = pdf_quote_name(x, 0, &len); \
		g_string_append_len(obj->out, str, len); \
		g_free(str); \
		g_string_append(obj->out, ")\n"); \
	}

	STR("/Title", hyp->database);
	STR("/Author", hyp->author);
	STR("/Subject", hyp->subject);

#undef STR

	g_string_append_printf(obj->out, "/Creator (%s)\n", hyp->comp_vers >= 6 || hyp->language != NULL ? gl_program_name : "ST-GUIDE");
	g_string_append_printf(obj->out, "/Producer (%s %s for %s)\n", gl_program_name, gl_program_version, hyp_osname(hyp_get_current_os()));
	if (hyp_utf8_stat(hyp->file, &s) == 0)
	{
		str = pdf_datestr(s.st_mtime);
		g_string_append_printf(obj->out, "/CreationDate (%s)\n", str);
		g_free(str);
	}
	g_string_append(obj->out, ">>\n");
}

/* ------------------------------------------------------------------------- */

gboolean recompile_pdf(HYP_DOCUMENT *hyp, hcp_opts *opts, int argc, const char **argv)
{
	hyp_nodenr node;
	INDEX_ENTRY *entry;
	gboolean ret;
	int i;
	gboolean found;
	PDF *pdf;
		
	/* force_crlf = FALSE; */
	
	ret = TRUE;
		
	if (opts->read_images && hyp->cache == NULL)
		InitCache(hyp);
	
	pdf = pdf_new();
	
	g_string_append(pdf->out, "%PDF-1.4\n");
	g_string_append(pdf->out, "%\320\320\320\320\n");
	g_string_append_printf(pdf->out, "%% This file was automatically generated by %s version %s\n", gl_program_name, gl_program_version);
	pdf_out_globals(hyp, opts, pdf);

	for (node = 0; node < hyp->num_index; node++)
	{
		entry = hyp->indextable[node];
		if (argc == 0)
		{
			found = TRUE;
		} else
		{
			found = FALSE;
			for (i = 0; i < argc; i++)
			{
				if (argv[i])
				{
					char *s1, *s2;
					
					s1 = hyp_conv_to_utf8(hyp->comp_charset, entry->name, STR0TERM);
					s2 = hyp_conv_to_utf8(hyp_get_current_charset(), argv[i], STR0TERM);
					if (namecmp(s1, s2) == 0)
					{
						argv[i] = NULL;
						found = TRUE;
					}
					g_free(s2);
					g_free(s1);
				}
			}
		}
		if (!found)
			continue;

		if (node == hyp->index_page)
		{
			if (!opts->gen_index || argc != 0)
				continue;
		}

		switch ((hyp_indextype) entry->type)
		{
		case HYP_NODE_INTERNAL:
		case HYP_NODE_POPUP:
			ret &= pdf_out_node(hyp, opts, pdf, node);
			break;
		case HYP_NODE_IMAGE:
		case HYP_NODE_EXTERNAL_REF:
		case HYP_NODE_SYSTEM_ARGUMENT:
		case HYP_NODE_REXX_SCRIPT:
		case HYP_NODE_REXX_COMMAND:
		case HYP_NODE_QUIT:
		case HYP_NODE_CLOSE:
		case HYP_NODE_EOF:
		default:
			break;
		}
	}

	pdf_write_objects(pdf, opts);

	fwrite(pdf->out->str, 1, pdf->out->len, opts->outfile);
	g_string_truncate(pdf->out, 0);

	for (i = 0; i < argc; i++)
	{
		if (argv[i] != NULL)
		{
			hyp_utf8_fprintf(opts->errorfile, _("'%s' not found.\n"), argv[i]);
			ret = FALSE;
		}
	}
	
	pdf_delete(pdf);

	return ret;
}
