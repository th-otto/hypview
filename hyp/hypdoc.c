#include "hypdoc.h"
#include "hypdebug.h"
#include <stddef.h>


/* ------------------------------------------------------------------------- */

static gboolean HypGotoNode(DOCUMENT *doc, const char *chapter, hyp_nodenr node_num)
{
	HYP_DOCUMENT *hyp = (HYP_DOCUMENT *) doc->data;
	HYP_NODE *node;
	
	/* locate page based on name? */
	if (!empty(chapter))
	{
		node_num = HypFindNode(doc, chapter);
	} else if (node_num == HYP_NOINDEX)
	{
		node_num = hyp->default_page;
		if (node_num == HYP_NOINDEX)
		{
			node_num = 0;
			
			/*
			 * no default page: use first text page
			 */
			while (node_num < hyp->num_index)
			{
				if (HYP_NODE_IS_TEXT(hyp->indextable[node_num]->type))
					break;
				node_num++;
			}
	
			if (node_num >= hyp->num_index)
			{
				FileError(hyp_basename(doc->path), _("no start page found."));
				node_num = HYP_NOINDEX;
			}
		}
	}
	
	if (node_num != HYP_NOINDEX && !hypnode_valid(hyp, node_num))
	{
		HYP_DBG(("ERROR: No entry with number %u found in this document (max number is %u)", node_num, hyp->num_index));
		node_num = HYP_NOINDEX;
	}

	if (hypnode_valid(hyp, node_num) && !HYP_NODE_IS_TEXT(hyp->indextable[node_num]->type))
	{
		HYP_DBG(("ERROR: Entry %u (Type: %d) is not a 'page' or a 'popup'", node_num, hyp->indextable[node_num]->type));
		node_num = HYP_NOINDEX;
	}

	node = AskCache(hyp, node_num);

	if (hypnode_valid(hyp, node_num) && node == NULL)
	{
		hyp->handle = -1;
		node = hyp_loadtext(hyp, node_num);
		if (node == NULL)
		{
			FileErrorErrno(hyp->file);
			node_num = HYP_NOINDEX;
		} else
		{
			TellCache(hyp, node_num, node);
			hyp_node_find_windowtitle(node);
			hyp_prep_graphics(hyp, node);
		}
	}

	if (node != NULL)
	{
		doc->displayed_node = node;
	
		/* update document with node data XXFIXME */
		doc->lines = node->lines;
		doc->height = node->height;
		doc->columns = node->columns;
		g_free(doc->window_title);
		doc->window_title = hyp_conv_to_utf8(hyp->comp_charset, node->window_title, STR0TERM);
	
		if (doc->window_title == NULL)
			doc->window_title = hyp_conv_to_utf8(hyp->comp_charset, hyp->indextable[node_num]->name, STR0TERM);
	}
	
	/* update toolbar state */
	doc->buttons.help = hypnode_valid(hyp, hyp->help_page);
	doc->buttons.index = hypnode_valid(hyp, hyp->index_page);
	doc->buttons.previous = hypnode_valid(hyp, node_num) && hypnode_valid(hyp, hyp->indextable[node_num]->previous) && node_num != hyp->indextable[node_num]->previous;
	doc->buttons.next = hypnode_valid(hyp, node_num) && hypnode_valid(hyp, hyp->indextable[node_num]->next) && node_num != hyp->indextable[node_num]->next;
	doc->buttons.home = hypnode_valid(hyp, node_num) && hypnode_valid(hyp, hyp->indextable[node_num]->next) && node_num != hyp->indextable[node_num]->toc_index;
	doc->buttons.references = HypCountExtRefs(node) != 0;
	
	/* ASCII Export supported */
	doc->buttons.ascii = TRUE;
	
	return node != NULL;
}

/* ------------------------------------------------------------------------- */

static void HypClose(DOCUMENT *doc)
{
	HYP_DOCUMENT *hyp;
	
	hyp = (HYP_DOCUMENT *) doc->data;
	doc->data = NULL;
	HypDeleteIfLast(doc, hyp);
}

/* ------------------------------------------------------------------------- */

static hyp_nodenr HypGetNode(DOCUMENT *doc)
{
	if (doc->displayed_node)
		return doc->displayed_node->number;
	HYP_DBG(("Document %s has no open page", printnull(doc->path)));
	return HYP_NOINDEX;
}

/* ------------------------------------------------------------------------- */

/*
 *		Ueberprueft ob es sich um einen Hypertext handelt und ladet
 *		danach die wichtigsten Daten in den Speicher.
 */
hyp_filetype HypLoad(DOCUMENT *doc, int handle, gboolean return_if_ref)
{
	HYP_DOCUMENT *hyp;
	REF_FILE *ref;
	hyp_filetype ftype;
	
	/* Zurueck zum Dateianfang */
	if (lseek(handle, 0, SEEK_SET) != 0)
		return HYP_FT_UNKNOWN;

	/* Datei Extension ermitteln */
	ftype = hyp_guess_filetype(doc->path);
	if (ftype == HYP_FT_REF)	/* REF file? */
	{
		char *f;
		
		ref = ref_load(doc->path, handle, FALSE);
		f = replace_ext(doc->path, HYP_EXT_REF, HYP_EXT_HYP);
		if (ref != NULL && return_if_ref)
		{
			doc->type = HYP_FT_REF;
			doc->data = ref;
			g_free(doc->path);
			doc->path = f;
			return doc->type;
		}
		handle = hyp_utf8_open(f, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
		if (handle < 0)
		{
			g_free(f);
			ref_close(ref);
			return HYP_FT_UNKNOWN;
		}
		g_free(doc->path);
		doc->path = f;
		ftype = HYP_FT_HYP;
	} else
	{
		ref = NULL;
	}
	
	if (ftype != HYP_FT_HYP)	/* no .HYP file? */
	{
		if (ref != NULL)
			hyp_utf8_close(handle);
		ref_close(ref);
		return HYP_FT_UNKNOWN;
	}

	if ((hyp = hyp_load(handle, &ftype)) == NULL)
	{
		if (ref != NULL)
			hyp_utf8_close(handle);
		ref_close(ref);
		return ftype;
	}
	
	/* initialize cache of node data */
	InitCache(hyp);

	doc->data = hyp;
	hyp->ref = ref;
	hyp->file = doc->path;

	doc->type = HYP_FT_HYP;
	doc->displayProc = HypDisplayPage;
	doc->closeProc = HypClose;
	doc->gotoNodeProc = HypGotoNode;
	doc->getNodeProc = HypGetNode;
	doc->clickProc = HypClick;
	doc->autolocProc = HypAutolocator;
	doc->getCursorProc = HypGetCursorPosition;
	doc->blockProc = HypBlockOperations;

	doc->start_line = 0;
	doc->lines = 0;
	doc->height = 0;
	doc->columns = 0;

	if (ref != NULL)
		hyp_utf8_close(handle);
	
	return doc->type;
}

/* ------------------------------------------------------------------------- */

hyp_nodenr HypFindNode(DOCUMENT *doc, const char *chapter)
{
	HYP_DOCUMENT *hyp = (HYP_DOCUMENT *) doc->data;
	hyp_nodenr node_num;

	/* try to find in node names first */
	node_num = find_nr_by_title(hyp, chapter);

	if (!hypnode_valid(hyp, node_num))
	{
		hyp_lineno line;
		
		/* load REF if not done already */
		if (hyp->ref == NULL)
		{
			char *filename;
			int ret;

			filename = replace_ext(doc->path, HYP_EXT_HYP, HYP_EXT_REF);

			ret = hyp_utf8_open(filename, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
			if (ret >= 0)
			{
				hyp->ref = ref_load(filename, ret, FALSE);
				hyp_utf8_close(ret);
			}
			g_free(filename);
		}

		if (ref_findnode(hyp->ref, chapter, &node_num, &line, TRUE))
			doc->start_line = line;
	}
	return node_num;
}

/* ------------------------------------------------------------------------- */

hyp_filetype LoadFile(DOCUMENT *doc, int handle, gboolean return_if_ref)
{
	hyp_filetype type;

	type = HypLoad(doc, handle, return_if_ref);
	if (type == HYP_FT_UNKNOWN)
		type = AsciiLoad(doc, handle);

	return type;
}

/* ------------------------------------------------------------------------- */

DOCUMENT *HypOpenFile(const char *path, gboolean return_if_ref)
{
	DOCUMENT *doc = NULL;
	int handle;

	handle = hyp_utf8_open(path, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);

	if (handle < 0)
	{
		FileErrorErrno(path);
		return NULL;
	}

	/*  Nicht gefunden? --> neues Dokument erstellen    */
	doc = g_new0(DOCUMENT, 1);

	doc->path = g_strdup(path);
	doc->window_title = g_strdup(doc->path);
	doc->buttons.load = TRUE;
	doc->type = HYP_FT_UNKNOWN;
	doc->autolocator = NULL;
	doc->selection.valid = FALSE;

	/*  Hier folgt die typ-spezifische Lade-Routine */
	if (LoadFile(doc, handle, return_if_ref) < 0)
	{
		FileError(hyp_basename(doc->path), _("format not recognized"));
		HypCloseFile(doc);
		doc = NULL;
	} else
	{
		struct stat st;

		/*  Aenderungsdatum und -zeit sichern   */
		if (rpl_fstat(handle, &st) < 0)
		{
			doc->mtime = 0;
		} else
		{
			doc->mtime = st.st_mtime;
		}
	}

	hyp_utf8_close(handle);

	return doc;
}

/* ------------------------------------------------------------------------- */

/*	Entfernt die Datei <doc> aus dem Speicher und aus der Liste.
	<doc> muss (!!) ein Zeiger auf ein Dokument in der Liste sein.	*/
void HypCloseFile(DOCUMENT *doc)
{
	if (doc == NULL)
		return;
	/*  Hier folgt die typ-spezifische Aufrum-Arbeit. */
	if (doc->data)
		doc->closeProc(doc);
	g_free(doc->window_title);
	g_free(doc->autolocator);
	g_free(doc->path);
	g_free(doc);
}
