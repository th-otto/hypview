#include "hypdefs.h"
#include "hypdebug.h"

/*
 * HypTree header data:
 * 4 bytes: sum of the length of all explicit titles (big-endian)
 * Array of word bit-vectors:
 *   1st word: Bit n == 1 -> Page n has an explicit title
 *   2nd word: Bit n == 1 -> Page 16+n has an explicit title
 *   etc.
 */

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

gboolean hyp_tree_isset(HYP_DOCUMENT *hyp, hyp_nodenr node)
{
	hyp_nodenr bitlen;
	const unsigned char *p;
	hyp_nodenr pos;
	
	if (hyp == NULL || hyp->hyptree_len <= SIZEOF_LONG || hyp->hyptree_data == NULL || node > hyp->last_text_page)
		return FALSE;
	p = hyp->hyptree_data + SIZEOF_LONG;
	bitlen = hyp->hyptree_len - SIZEOF_LONG;
	pos = (node >> 4) << 1;
	if (!(node & 0x08))
		pos++;
	if (pos >= bitlen)
		return FALSE;
	return (p[pos] & (0x01 << (node & 0x07))) != 0;
}

/* ------------------------------------------------------------------------- */

void hyp_tree_setbit(HYP_DOCUMENT *hyp, hyp_nodenr node)
{
	hyp_nodenr bitlen;
	unsigned char *p;
	hyp_nodenr pos;
	
	if (hyp == NULL || hyp->hyptree_len <= SIZEOF_LONG || hyp->hyptree_data == NULL || node > hyp->last_text_page)
		return;
	p = hyp->hyptree_data + SIZEOF_LONG;
	bitlen = hyp->hyptree_len - SIZEOF_LONG;
	pos = (node >> 4) << 1;
	if (!(node & 0x08))
		pos++;
	if (pos >= bitlen)
		return;
	p[pos] |= (0x01 << (node & 0x07));
}

/* ------------------------------------------------------------------------- */

gboolean hyp_tree_alloc(HYP_DOCUMENT *hyp)
{
	hyp_nodenr node;
	size_t bitlen;
	long titlelen = 0;
	HYP_NODE *nodeptr;
	
	if (hyp == NULL)
		return FALSE;
	if (hyp->hyptree_data != NULL)
	{
		HYP_DBG(("hyptree already allocated"));
		return FALSE;
	}
	hyp->first_text_page = hyp_first_text_page(hyp);
	hyp->last_text_page = hyp_last_text_page(hyp);
	bitlen = SIZEOF_LONG + (((hyp->last_text_page + 16u) >> 4) << 1);

	hyp->hyptree_data = g_new0(unsigned char, bitlen);
	if (hyp->hyptree_data == NULL)
		return FALSE;
	hyp->hyptree_len = bitlen;
	hyp->handle = -1;

	for (node = 0; node < hyp->num_index; node++)
	{
		if ((nodeptr = hyp_loadtext(hyp, node)) != NULL)
		{
			hyp_node_find_windowtitle(nodeptr);
			if (nodeptr->window_title)
			{
				titlelen += ustrlen(nodeptr->window_title) + 1;
				hyp_tree_setbit(hyp, node);
			}
			hyp_node_free(nodeptr);
		}
	}
	/*
	 * we have a maximum of 65000 nodes,
	 * and a title can have 255 chars max,
	 * so the sum cannot overflow the
	 * the range of a 32-bit int, and we don't need to check that here
	 */
	long_to_chars(titlelen, hyp->hyptree_data);
	
	/*
	 * do not write the bit table when there are no titlss
	 */
	if (titlelen == 0)
		hyp->hyptree_len = SIZEOF_LONG;
	
	return TRUE;
}

/* ------------------------------------------------------------------------- */

HYPTREE *hyp_tree_build(HYP_DOCUMENT *hyp, int handle)
{
	hyp_nodenr node;
	HYPTREE *tree;
	INDEX_ENTRY *entry;

	hyp->handle = handle;
	tree = g_new(HYPTREE, hyp->num_index);
	if (tree == NULL)
	{
		return NULL;
	}
	
	for (node = 0; node < hyp->num_index; node++)
	{
		tree[node].next = HYP_NOINDEX;
		tree[node].prev = HYP_NOINDEX;
		tree[node].parent = HYP_NOINDEX;
		tree[node].head = HYP_NOINDEX;
		tree[node].tail = HYP_NOINDEX;
		tree[node].flags = HYPTREE_IS_EXPANDED;
		tree[node].level = 0;
		tree[node].num_childs = 0;
		tree[node].name = NULL;
		tree[node].title = NULL;
	}

	/*
	 * gather available window titles
	 */
	for (node = 0; node < hyp->num_index; node++)
	{
		entry = hyp->indextable[node];
		if (entry->type == HYP_NODE_INTERNAL)
		{
			HYP_NODE *nodeptr;
			size_t namelen;
			
			tree[node].flags |= HYPTREE_IS_NODE;
			namelen = entry->length - SIZEOF_INDEX_ENTRY;
#ifdef NO_UTF8
			tree[node].name = g_strndup((const char *)entry->name, namelen);
#else
			tree[node].name = hyp_conv_to_utf8(hyp->comp_charset, entry->name, namelen);
#endif
			if (hyp_tree_isset(hyp, node) && (nodeptr = hyp_loadtext(hyp, node)) != NULL)
			{
				hyp_node_find_windowtitle(nodeptr);
				if (nodeptr->window_title)
				{
#ifdef NO_UTF8
					tree[node].title = g_strdup((const char *)nodeptr->window_title);
#else
					tree[node].title = hyp_conv_to_utf8(hyp->comp_charset, nodeptr->window_title, STR0TERM);
#endif
				}
				hyp_node_free(nodeptr);
			}
			if (tree[node].title == NULL)
			{
				tree[node].title = tree[node].name;
			}
		}
	}

	/*
	 * construct tree
	 */
	for (node = 0; node < hyp->num_index; node++)
	{
		hyp_nodenr nr;
		
		entry = hyp->indextable[node];
		if (entry->type != HYP_NODE_INTERNAL)
			continue;
		nr = entry->toc_index;
		if (hypnode_valid(hyp, nr) && nr != node)
		{
			tree[node].parent = nr;
			if (tree[nr].head == HYP_NOINDEX)
			{
				ASSERT(tree[nr].num_childs == 0);
				ASSERT(tree[nr].tail == HYP_NOINDEX);
				tree[nr].head = node;
			} else
			{
				ASSERT(tree[node].prev == HYP_NOINDEX);
				tree[node].prev = tree[nr].tail;
				ASSERT(tree[tree[nr].tail != HYP_NOINDEX);
				ASSERT(tree[tree[nr].tail].next == HYP_NOINDEX);
				tree[tree[nr].tail].next = node;
			}
			tree[nr].tail = node;
			tree[nr].num_childs++;
		}
	}

	return tree;
}

/* ------------------------------------------------------------------------- */

void hyp_tree_free(HYP_DOCUMENT *hyp, HYPTREE *tree)
{
	hyp_nodenr node;
	INDEX_ENTRY *entry;

	if (hyp == NULL || tree == NULL)
		return;
	/*
	 * cleanup
	 */
	for (node = 0; node < hyp->num_index; node++)
	{
		entry = hyp->indextable[node];
		if (entry->type != HYP_NODE_INTERNAL)
			continue;
		if (tree[node].title != tree[node].name)
			g_free(tree[node].title);
		g_free(tree[node].name);
	}
	g_free(tree);
}
