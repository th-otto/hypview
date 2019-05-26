#include "aes.h"

/*
 * ob_sst	Routine to set the user variables pspec, pstate, ptype,
 *		pflags, pt, pth.
 *
 *		returns object border/text color or the 3byte of the pointer
 *		to a tedinfo structure (isn't this help full).
 *
 */
char ob_sst(OBJECT *tree, _WORD obj, OBSPEC *pspec, _WORD *pstate, _WORD *ptype, _WORD *pflags, GRECT *pt, _WORD *pth)
{
	_WORD th;
	OBJECT *tmp;

	tmp = &tree[obj];

	pt->g_w = tmp->ob_width;			/* set user grect width */
	pt->g_h = tmp->ob_height;			/* set user grect height */
	*pflags = tmp->ob_flags;			/* set user flags variable */
	*pspec = tmp->ob_spec;				/* set user spec variable */
	*pstate = tmp->ob_state;			/* set user state variable */

	*ptype = tmp->ob_type & OBTYPEMASK;	/* set user type variable */

	/* IF indirect then get pointer */
	if (*pflags & OF_INDIRECT)
		*pspec = *(tmp->ob_spec.indirect);


	th = 0;								/* border thickness */

	switch (*ptype)
	{
	case G_TITLE:						/* menu title thickness = 1 */
		th = 1;
		break;

	case G_TEXT:						/* for these items tedinfo thickness */
	case G_BOXTEXT:
	case G_FTEXT:
	case G_FBOXTEXT:
		th = (*pspec).tedinfo->te_thickness;
		break;

	case G_BOX:							/* for these use object thickness */
	case G_BOXCHAR:
	case G_IBOX:
		th = OBSPEC_GET_FRAMESIZE(*pspec);
		break;

	case G_BUTTON:						/* for a button make thicker */
		th--;
		if (*pflags & OF_EXIT)			/* one thicker ( - (neg) is thicker) */
			th--;
		if (*pflags & OF_DEFAULT)		/* still one more thick */
			th--;
		break;
	}

	if (th > 128)
		th -= 256;						/* clamp it */

	*pth = th;							/* set user variable */

	/*
	 * returns object border/text color
	 * or the 3byte of the pointer to a
	 * tedinfo structure (real helpfull)
	 */
	return OBSPEC_GET_CHARACTER(*pspec);
}


void everyobj(OBJECT *tree, _WORD thisobj, _WORD last, EVERYOBJ_CALLBACK routine, _WORD startx, _WORD starty, _WORD maxdep)
{
	_WORD tmp1;
	_WORD depth;
	_WORD x[MAX_DEPTH + 2], y[MAX_DEPTH + 2];

	x[0] = startx;
	y[0] = starty;
	depth = 1;
	if (maxdep > MAX_DEPTH)
		maxdep = MAX_DEPTH;
	
	/*
	 * non-recursive tree traversal
	 */
	for (;;)
	{
		/* see if we need to stop */
		if (thisobj == last)
			return;

		/* do this object */
		x[depth] = x[depth - 1] + tree[thisobj].ob_x;
		y[depth] = y[depth - 1] + tree[thisobj].ob_y;
		(*routine) (tree, thisobj, x[depth], y[depth]);

		/* if this guy has kids then do them */
		tmp1 = tree[thisobj].ob_head;
		if (tmp1 != NIL)
		{
			if (!(tree[thisobj].ob_flags & OF_HIDETREE) && depth <= maxdep)
			{
				depth++;
				thisobj = tmp1;
				continue;
			}
		}
		for (;;)
		{
			/*
			 * if this is the root (which has no parent),
			 * or it is the last then stop else...
			 */
			tmp1 = tree[thisobj].ob_next;
			if (tmp1 == last || thisobj == ROOT)
				return;
			/*
			 * if this object has a sibling that is not his parent,
			 * then move to him and do him and his kids
			 */
			if (tree[tmp1].ob_tail != thisobj)
			{
				thisobj = tmp1;
				break;
			}
			/*
			 * else move up to the parent and finish off his siblings
			 */
			depth--;
			thisobj = tmp1;
		}
	}
}



/*
 * Routine that will find the parent of a given object.  The
 * idea is to walk to the end of our siblings and return
 * our parent. If object is the root then return NIL as parent.
 */
_WORD get_par(OBJECT *tree, _WORD obj)
{
	_WORD pobj;

	pobj = obj;

	if (obj == ROOT)
		return NIL;

	do
	{
		obj = pobj;
		pobj = tree[obj].ob_next;
	} while (tree[pobj].ob_tail != obj);

	return pobj;
}
