/*****************************************************************************
 * OBJECT.C
 *****************************************************************************/

#include "config.h"
#include <stdint.h>
#include <gem.h>
#include <object.h>
#include <ro_mem.h>
#include "rsc.h"

/*** ---------------------------------------------------------------------- ***/

/* Find the parent object of	*/
/* by traversing right until	*/
/* we find nodes whose NEXT     */
/* and TAIL links point to		*/
/* each other.					*/
_WORD Objc_Get_Parent(const OBJECT *tree, _WORD obj)
{
	_WORD pobj;

	if (obj == NIL)
		return NIL;
	pobj = tree[obj].ob_next;
	while (pobj != NIL && tree[pobj].ob_tail != obj)
	{
		obj = pobj;
		pobj = tree[obj].ob_next;
	}
	return pobj;
}

/*** ---------------------------------------------------------------------- ***/

_WORD Objc_Get_Parent_Root(const OBJECT *tree, _WORD obj)
{
	_WORD root = Objc_Get_Parent(tree, obj);

	if (root < ROOT)
		return ROOT;
	return root;
}

/*** ---------------------------------------------------------------------- ***/

void Objc_Offset(const OBJECT *tree, _WORD idx, _WORD *xp, _WORD *yp)
{
	_WORD x = 0, y = 0;

	do
	{
		x += tree[idx].ob_x;
		y += tree[idx].ob_y;
		idx = Objc_Get_Parent(tree, idx);
	} while (idx != NIL);
	*xp = x;
	*yp = y;
}

/*** ---------------------------------------------------------------------- ***/

void Objc_Xywh(const OBJECT *tree, _WORD idx, GRECT *gr)
{
	gr->g_w = tree[idx].ob_width;
	gr->g_h = tree[idx].ob_height;
	Objc_Offset(tree, idx, &gr->g_x, &gr->g_y);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * calculate the number of objects in a sub-tree
 */
_WORD Objc_Count(OBJECT *tree, _WORD start)
{
	_WORD i, n;

	n = 1;
	if (tree[start].ob_head != NIL)
	{
		for (i = tree[start].ob_head; i != start; i = tree[i].ob_next)
		{
			n += Objc_Count(tree, i);
		}
	}
	return n;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * delete a subtree from a tree
 * returns the number of deleted objects
 */
_WORD Objc_Delete(OBJECT *tree, _WORD idx)
{
	_WORD n, pn;
	_WORD i;
	_WORD parent;

	pn = Objc_Count(tree, idx);
	/* if this was the root, don't do anything at all */
	if (idx == ROOT)
		return pn;
	n = Objc_Count(tree, ROOT);
	parent = Objc_Get_Parent(tree, idx);
	if (parent == NIL)
		return pn;				/* this shouldn't happen */
	DelObjFlags(tree, n - 1, OF_LASTOB);
	if (tree[parent].ob_head == tree[parent].ob_tail)
	{
		/*
		 * deleted the only child
		 */
		tree[parent].ob_head = NIL;
		tree[parent].ob_tail = NIL;
	} else
	{
		i = tree[parent].ob_head;
		if (idx == i)
		{
			tree[parent].ob_head = tree[idx].ob_next;
		} else
		{
			while (tree[i].ob_next != idx)
				i = tree[i].ob_next;
		}
		tree[i].ob_next = tree[idx].ob_next;
		while (tree[i].ob_next != parent)
			i = tree[i].ob_next;
		tree[parent].ob_tail = i;
	}
	n -= pn;
	for (i = idx; i != n; i++)
	{
		tree[i] = tree[i + pn];
	}
	idx += pn;
	for (i = 0; i != n; i++)
	{
		if (tree[i].ob_next != NIL && tree[i].ob_next >= idx)
			tree[i].ob_next -= pn;
		if (tree[i].ob_head != NIL && tree[i].ob_head >= idx)
			tree[i].ob_head -= pn;
		if (tree[i].ob_tail != NIL && tree[i].ob_tail >= idx)
			tree[i].ob_tail -= pn;
	}
	SetObjFlags(tree, n - 1, OF_LASTOB);
	return pn;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * disconnect a subtree from a tree,
 * but leave the rest alone
 */
void Objc_Unlink(OBJECT *tree, _WORD idx)
{
	_WORD i;
	_WORD parent;

	/* if this was the root, don't do anything at all */
	if (idx == ROOT)
		return;
	parent = Objc_Get_Parent(tree, idx);
	if (parent == NIL)
		return;             /* this shouldn't happen */
	if (tree[parent].ob_head == tree[parent].ob_tail)
	{
		/*
		 * deleted the only child
		 */
		tree[parent].ob_head = NIL;
		tree[parent].ob_tail = NIL;
	} else
	{
		i = tree[parent].ob_head;
		if (idx == i)
		{
			tree[parent].ob_head = tree[idx].ob_next;
		} else
		{
			while (tree[i].ob_next != idx)
				i = tree[i].ob_next;
		}
		tree[i].ob_next = tree[idx].ob_next;
		while (tree[i].ob_next != parent)
			i = tree[i].ob_next;
		tree[parent].ob_tail = i;
	}
}

/*** ---------------------------------------------------------------------- ***/

/*
 * connect a new object to the tree
 */
_WORD Objc_Add(OBJECT *tree, _WORD parent, _WORD idx)
{
	_WORD tail;

	if ((tail = tree[parent].ob_tail) != NIL)
		tree[tail].ob_next = idx;
	else
		tree[parent].ob_head = idx;
	tree[parent].ob_tail = idx;
	tree[idx].ob_next = parent;
	return 1;
}

/*** ---------------------------------------------------------------------- ***/

_LONG bitblk_datasize(BITBLK *bit)
{
	return (_ULONG)((bit->bi_wb + 1) & ~1) * (_ULONG)bit->bi_hl;
}

/*** ---------------------------------------------------------------------- ***/

_LONG iconblk_masksize(ICONBLK *icon)
{
	return (_ULONG)((icon->ib_wicon + 15) >> 4) * 2l * (_ULONG)icon->ib_hicon;
}

