#include "hypdoc.h"
#include "hypdebug.h"
#include <stddef.h>

#ifdef __PUREC__
struct _window_data_ { int dummy; };
#endif

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static gboolean HypGotoNode(WINDOW_DATA *win, const char *chapter, hyp_nodenr node_num)
{
	DOCUMENT *doc = hypwin_doc(win);
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
			/*
			 * no default page: use first text page
			 */
			node_num = hyp->first_text_page;
			if (node_num == HYP_NOINDEX)
			{
				/*
				 * This could only happen if we loaded a non-empty file
				 * without any text page.
				 * The compiler should prevent that: any command that can
				 * create an entry which is not a text node can only
				 * appear inside a text node.
				 */
				FileError(hyp_basename(doc->path), _("no start page found."));
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

	/* update toolbar state */
	doc->buttons.help = hypnode_valid(hyp, hyp->help_page) && node_num != hyp->help_page;
	doc->buttons.index = hypnode_valid(hyp, hyp->index_page) && node_num != hyp->index_page;
	doc->buttons.previous = hypnode_valid(hyp, node_num) && hypnode_valid(hyp, hyp->indextable[node_num]->previous) && node_num != hyp->indextable[node_num]->previous;
	doc->buttons.next = hypnode_valid(hyp, node_num) && hypnode_valid(hyp, hyp->indextable[node_num]->next) && node_num != hyp->indextable[node_num]->next;
	doc->buttons.nextphys = hypnode_valid(hyp, hyp->last_text_page) && node_num < hyp->last_text_page;
	doc->buttons.prevphys = hypnode_valid(hyp, hyp->first_text_page) && node_num > hyp->first_text_page;
	doc->buttons.first = hypnode_valid(hyp, hyp->first_text_page) && node_num != hyp->first_text_page;
	doc->buttons.last = hypnode_valid(hyp, hyp->last_text_page) && node_num != hyp->last_text_page;
	doc->buttons.home = hypnode_valid(hyp, node_num) && hypnode_valid(hyp, hyp->indextable[node_num]->toc_index) && node_num != hyp->indextable[node_num]->toc_index;
	doc->buttons.references = HypCountExtRefs(node) != 0;
	
	/* ASCII Export supported */
	doc->buttons.ascii = TRUE;
	
	if (node != NULL)
	{
		doc->prepNode(win, node);
	}
	
	return node != NULL;
}

/*** ---------------------------------------------------------------------- ***/

static void HypClose(DOCUMENT *doc)
{
	HYP_DOCUMENT *hyp;
	
	hyp = (HYP_DOCUMENT *) doc->data;
	doc->data = NULL;
	hyp_unref(hyp);
}

/*** ---------------------------------------------------------------------- ***/

static hyp_nodenr HypGetNode(WINDOW_DATA *win)
{
	HYP_NODE *node = hypwin_node(win);
	if (node)
		return node->number;
	HYP_DBG(("Document %s has no open page", printnull(hypwin_doc(win)->path)));
	return HYP_NOINDEX;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Check for file being hyp-file, and load index.
 * Set document to HYP_DOCUMENT if successful.
 */
hyp_filetype HypLoad(DOCUMENT *doc, int handle, gboolean return_if_ref)
{
	HYP_DOCUMENT *hyp;
	REF_FILE *ref;
	hyp_filetype ftype;
	
	/* back to beginning o file */
	if (lseek(handle, 0, SEEK_SET) != 0)
		return HYP_FT_UNKNOWN;

	/* guess file type by checking filename extension */
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
	doc->autolocProc = HypAutolocator;
	doc->getCursorProc = HypGetCursorPosition;
	doc->blockProc = HypBlockOperations;
	doc->prepNode = HypPrepNode;
	
	doc->start_line = 0;

	if (ref != NULL)
		hyp_utf8_close(handle);
	
	return doc->type;
}

/*** ---------------------------------------------------------------------- ***/

hyp_nodenr HypFindNode(DOCUMENT *doc, const char *chapter)
{
	HYP_DOCUMENT *hyp = (HYP_DOCUMENT *) doc->data;
	hyp_nodenr node_num;
	char *nodename;
	
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

		if ((nodename = ref_findnode(hyp->ref, chapter, &line, TRUE)) != NULL)
		{
			node_num = find_nr_by_title(hyp, nodename);
			g_free(nodename);
			doc->start_line = line;
		}
	}
	return node_num;
}

/*** ---------------------------------------------------------------------- ***/

hyp_filetype LoadFile(DOCUMENT *doc, int handle, gboolean return_if_ref)
{
	hyp_filetype type;

	type = HypLoad(doc, handle, return_if_ref);
	if (type == HYP_FT_UNKNOWN)
		type = AsciiLoad(doc, handle);

	return type;
}

/*** ---------------------------------------------------------------------- ***/

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

	/* create a new document */
	doc = g_new0(DOCUMENT, 1);

	doc->path = g_strdup(path);
	doc->buttons.load = TRUE;
	doc->type = HYP_FT_UNKNOWN;
	doc->ref_count = 1;
	
	/* type-spezific loading follows */
	if (LoadFile(doc, handle, return_if_ref) < 0)
	{
		FileError(hyp_basename(doc->path), _("format not recognized"));
		hypdoc_unref(doc);
		doc = NULL;
	} else
	{
		struct stat st;

		/* save file modification time&date */
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

DOCUMENT *hypdoc_unref(DOCUMENT *doc)
{
	if (G_UNLIKELY(doc == NULL))
		return doc;
	ASSERT(doc->ref_count >= 1);
	if (doc->data && doc->type == HYP_FT_HYP)
	{
		HYP_DOCUMENT *hyp = doc->data;
		doc->data = hyp_unref(hyp);
	}
	if (--doc->ref_count == 0)
	{
		HypCloseFile(doc);
		doc = NULL;
	}
	return doc;
}

/* ------------------------------------------------------------------------- */

DOCUMENT *hypdoc_ref(DOCUMENT *doc)
{
	if (G_UNLIKELY(doc == NULL))
		return doc;
	++doc->ref_count;
	ASSERT(doc->ref_count >= 2);
	if (doc->data && doc->type == HYP_FT_HYP)
	{
		HYP_DOCUMENT *hyp = doc->data;
		hyp_ref(hyp);
	}
	return doc;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Remove DOCUMENT from memory.
 */
void HypCloseFile(DOCUMENT *doc)
{
	if (G_UNLIKELY(doc == NULL))
		return;
	ASSERT(doc->ref_count == 0);
	/* type-specific cleanup follows */
	if (doc->data)
		doc->closeProc(doc);
	g_free(doc->path);
	g_free(doc);
}
