#include "hypdefs.h"
#include "hypdebug.h"

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

void InitCache(HYP_DOCUMENT *hyp)
{
	hyp->cache = g_new0(HYP_NODE *, hyp->num_index);
}

/* ------------------------------------------------------------------------- */

void ClearCache(HYP_DOCUMENT *hyp)
{
	hyp_nodenr i;

	if (hyp->cache != NULL)
	{
		for (i = 0; i < hyp->num_index; i++)
		{
			if (hyp->cache[i] != NULL)
			{
				switch (hyp->indextable[i]->type)
				{
				case HYP_NODE_INTERNAL:
				case HYP_NODE_POPUP:
					hyp_node_free(hyp->cache[i]);
					break;
				case HYP_NODE_IMAGE:
					hyp_image_free((HYP_IMAGE *)hyp->cache[i]);
					break;
				default:
					unreachable();
					break;
				}
				hyp->cache[i] = NULL;
			}
		}
		g_free(hyp->cache);
		hyp->cache = NULL;
	}
}

/* ------------------------------------------------------------------------- */

gboolean TellCache(HYP_DOCUMENT *hyp, hyp_nodenr node_num, HYP_NODE *node)
{
	if (hyp->cache && node_num < hyp->num_index)
	{
		hyp->cache[node_num] = node;
		return TRUE;
	}
	return FALSE;
}

/* ------------------------------------------------------------------------- */

HYP_NODE *AskCache(HYP_DOCUMENT *hyp, hyp_nodenr node_num)
{
	if (hyp->cache && node_num < hyp->num_index)
		return hyp->cache[node_num];
	return NULL;
}

/* ------------------------------------------------------------------------- */

/*
 * Remove text pages from cache
 */
void RemoveNodes(HYP_DOCUMENT *hyp)
{
	hyp_nodenr i;

	if (hyp->cache == NULL)
		return;
	for (i = 0; i < hyp->num_index; i++)
	{
		if (hyp->cache[i] != NULL && HYP_NODE_IS_TEXT(hyp->indextable[i]->type))
		{
			hyp_node_free(hyp->cache[i]);
			hyp->cache[i] = NULL;
		}
	}
}

/* ------------------------------------------------------------------------- */

/*
 * Remove pictures from cache
 */
void RemovePictures(HYP_DOCUMENT *hyp, gboolean reload)
{
	hyp_nodenr i;

	if (hyp->cache == NULL)
		return;
	for (i = 0; i < hyp->num_index; i++)
	{
		if (hyp->cache[i] != NULL && hyp->indextable[i]->type == HYP_NODE_IMAGE)
		{
			hyp_image_free((HYP_IMAGE *)hyp->cache[i]);
			hyp->cache[i] = NULL;
		}
	}
	if (reload)
	{
		for (i = 0; i < hyp->num_index; i++)
		{
			if (hyp->cache[i] != NULL && HYP_NODE_IS_TEXT(hyp->indextable[i]->type))
			{
				hyp_free_graphics(hyp->cache[i]);
				hyp_prep_graphics(hyp, hyp->cache[i]);
			}
		}
	}
}
