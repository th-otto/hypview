#include "hypdefs.h"
#include "hypdebug.h"
#include "hobstack.h"

#define POINTER void *

/* Determine default alignment. */
struct fooalign
{
	char x;
	double d;
};

#define DEFAULT_ALIGNMENT  \
  ((PTR_INT_TYPE) ((char *) &((struct fooalign *) 0)->d - (char *) 0))
/* If malloc were really smart, it would round addresses to DEFAULT_ALIGNMENT.
   But in fact it might be less smart and round addresses to as much as
   DEFAULT_ROUNDING.  So we prepare for it to do that. */
union fooround
{
	long x;
	double d;
};

#define DEFAULT_ROUNDING (sizeof (union fooround))

/* When we copy a long block of data, this is the unit to do it with.
   On some machines, copying successive ints does not work;
   in such a case, redefine COPYING_UNIT to `long' (if that works)
   or `char' as a last resort. */
#ifndef COPYING_UNIT
#  define COPYING_UNIT int
#endif	/* COPYING_UNIT */


#define CALL_CHUNKFUN(h, size) ((h)->chunkfun(size))

#define CALL_FREEFUN(h, old_chunk) (h)->freefun(old_chunk)


/* Initialize an obstack H for use.  Specify chunk size SIZE (0 means default).
   Objects start on multiples of ALIGNMENT (0 means use default).
   CHUNKFUN is the function to use to allocate chunks,
   and FREEFUN the function to free them.

   Return nonzero if successful, calls obstack_alloc_failed_handler if
   allocation fails. */

gboolean _hyp_obstack_begin(struct obstack *h, PTR_INT_TYPE size, PTR_INT_TYPE alignment, POINTER(*chunkfun) (size_t), void (*freefun) (POINTER))
{
	register struct _obstack_chunk *chunk;	/* points to new chunk */

	if (alignment == 0)
		alignment = DEFAULT_ALIGNMENT;
	if (size == 0)
		/* Default size is what GNU malloc can fit in a 4096-byte block. */
	{
		/* 12 is sizeof (mhead) and 4 is EXTRA from GNU malloc.
		   Use the values for range checking, because if range checking is off,
		   the extra bytes won't be missed terribly, but if range checking is on
		   and we used a larger request, a whole extra 4096 bytes would be
		   allocated.

		   These number are irrelevant to the new GNU malloc.  I suspect it is
		   less sensitive to the size of the request. */
		PTR_INT_TYPE extra = ((((12 + DEFAULT_ROUNDING - 1) & ~(DEFAULT_ROUNDING - 1))
					  + 4 + DEFAULT_ROUNDING - 1)
					 & ~(DEFAULT_ROUNDING - 1));

		size = 4096 - extra;
	}
	h->chunkfun = chunkfun;
	h->freefun = freefun;
	h->chunk_size = size;
	h->alignment_mask = alignment - 1;

	chunk = h->chunk = CALL_CHUNKFUN(h, h->chunk_size);
	if (!chunk)
		return FALSE;
	h->next_free = h->object_base = chunk->contents;
	h->chunk_limit = chunk->limit
		= (char *) chunk + h->chunk_size;
	chunk->prev = 0;
	/* The initial chunk now contains no empty object. */
	h->maybe_empty_object = 0;
	h->alloc_failed = 0;
	return TRUE;
}


/* Allocate a new current chunk for the obstack *H
   on the assumption that LENGTH bytes need to be added
   to the current object, or a new object of length LENGTH allocated.
   Copies any partial object from the end of the old chunk
   to the beginning of the new one. */

gboolean _hyp_obstack_newchunk(struct obstack *h, PTR_INT_TYPE length)
{
	register struct _obstack_chunk *old_chunk = h->chunk;
	register struct _obstack_chunk *new_chunk;
	register long new_size;
	register long obj_size = h->next_free - h->object_base;

	/* Compute size for new chunk. */
	new_size = (obj_size + length) + (obj_size >> 3) + 100;
	if (new_size < h->chunk_size)
		new_size = h->chunk_size;

	/* Allocate and initialize the new chunk. */
	new_chunk = CALL_CHUNKFUN(h, new_size);
	if (!new_chunk)
		return FALSE;
	h->chunk = new_chunk;
	new_chunk->prev = old_chunk;
	new_chunk->limit = h->chunk_limit = (char *) new_chunk + new_size;

	/* Move the existing object to the new chunk. */
	memcpy(new_chunk->contents, h->object_base, obj_size);

	/* If the object just copied was the only data in OLD_CHUNK,
	   free that chunk and remove it from the chain.
	   But not if that chunk might contain an empty object. */
	if (h->object_base == old_chunk->contents && !h->maybe_empty_object)
	{
		new_chunk->prev = old_chunk->prev;
		CALL_FREEFUN(h, old_chunk);
	}
	h->object_base = new_chunk->contents;
	h->next_free = h->object_base + obj_size;
	/* The new chunk certainly contains no empty object yet. */
	h->maybe_empty_object = 0;
	return TRUE;
}

/* Return nonzero if object OBJ has been allocated from obstack H.
   This is here for debugging.
   If you use it in a program, you are probably losing. */

/* Suppress -Wmissing-prototypes warning.  We don't want to declare this in
   obstack.h because it is just for debugging. */
gboolean _hyp_obstack_allocated_p(struct obstack *h, POINTER obj);

gboolean _hyp_obstack_allocated_p(struct obstack *h, POINTER obj)
{
	register struct _obstack_chunk *lp;	/* below addr of any objects in this chunk */
	register struct _obstack_chunk *plp;	/* point to previous chunk if any */

	lp = (h)->chunk;
	/* We use >= rather than > since the object cannot be exactly at
	   the beginning of the chunk but might be an empty object exactly
	   at the end of an adjacent chunk. */
	while (lp != 0 && ((POINTER) lp >= obj || (POINTER) (lp)->limit < obj))
	{
		plp = lp->prev;
		lp = plp;
	}
	return lp != 0;
}

/* Free objects in obstack H, including OBJ and everything allocate
   more recently than OBJ.  If OBJ is zero, free everything in H. */

gboolean hyp_obstack_free(struct obstack *h, POINTER obj)
{
	register struct _obstack_chunk *lp;	/* below addr of any objects in this chunk */
	register struct _obstack_chunk *plp;	/* point to previous chunk if any */
	register char * temp;
	
	temp = (char *) obj;
	if (temp > (char *)h->chunk && temp < h->chunk_limit)
	{
   		h->next_free = h->object_base = temp;
   	} else
   	{
   		obj = (POINTER)temp;

		lp = h->chunk;
		/* We use >= because there cannot be an object at the beginning of a chunk.
		   But there can be an empty object at that address
		   at the end of another chunk. */
		while (lp != 0 && ((POINTER) lp >= obj || (POINTER) (lp)->limit < obj))
		{
			plp = lp->prev;
			CALL_FREEFUN(h, lp);
			lp = plp;
			/* If we switch chunks, we can't tell whether the new current
			   chunk contains an empty object, so assume that it may. */
			h->maybe_empty_object = 1;
		}
		if (lp)
		{
			h->object_base = h->next_free = (char *) obj;
			h->chunk_limit = lp->limit;
			h->chunk = lp;
		} else if (obj != NULL)
		{
			/* obj is not in any of the chunks! */
			return FALSE;
		}
	}
	return TRUE;
}


PTR_INT_TYPE hyp_obstack_memory_used(struct obstack *h)
{
	register struct _obstack_chunk *lp;
	register PTR_INT_TYPE nbytes = 0;

	for (lp = h->chunk; lp != 0; lp = lp->prev)
	{
		nbytes += lp->limit - (char *) lp;
	}
	return nbytes;
}


/* Pointer to beginning of object being allocated or to be allocated next.
   Note that this might not be the final address of the object
   because a new chunk might be needed to hold the final size. */
POINTER hyp_obstack_base(struct obstack *obstack)
{
	return obstack->object_base;
}


/* Pointer to next byte not yet allocated in current chunk. */
POINTER hyp_obstack_next_free(struct obstack *obstack)
{
	return obstack->next_free;
}


/* Size for allocating ordinary chunks. */
long hyp_obstack_chunk_size (struct obstack *obstack)
{
	return obstack->chunk_size;
}

/* Mask specifying low bits that should be clear in address of an object. */
PTR_INT_TYPE hyp_obstack_alignment_mask (struct obstack *obstack)
{
	return obstack->alignment_mask;
}


PTR_INT_TYPE hyp_obstack_object_size(struct obstack *obstack)
{
	return obstack->next_free - obstack->object_base;
}


PTR_INT_TYPE hyp_obstack_room (struct obstack *obstack)
{
	return obstack->chunk_limit - obstack->next_free;
}


gboolean hyp_obstack_empty_p (struct obstack *obstack)
{
	return obstack->chunk->prev == 0 && obstack->next_free - obstack->chunk->contents == 0;
}


gboolean hyp_obstack_make_room(struct obstack *obstack, PTR_INT_TYPE length)
{
	if (obstack->next_free + length > obstack->chunk_limit)			\
		return _hyp_obstack_newchunk(obstack, length);
	return TRUE;
}


gboolean hyp_obstack_grow(struct obstack *obstack, POINTER pointer, PTR_INT_TYPE length)
{
	if (obstack->next_free + length > obstack->chunk_limit)
		if (_hyp_obstack_newchunk(obstack, length) == FALSE)
			return FALSE;
	memcpy(obstack->next_free, (char *) pointer, length);
	obstack->next_free += length;
	return TRUE;
}


gboolean hyp_obstack_grow0 (struct obstack *obstack, POINTER pointer, PTR_INT_TYPE length)
{
	if (obstack->next_free + length + 1 > obstack->chunk_limit)
		if (_hyp_obstack_newchunk(obstack, length + 1) == FALSE)
			return FALSE;
	memcpy(obstack->next_free, (char *) pointer, length);
	obstack->next_free += length;
	*(obstack->next_free)++ = 0;
	return TRUE;
}


gboolean hyp_obstack_1grow(struct obstack *obstack, int character)
{
	if (((obstack)->next_free + 1 > (obstack)->chunk_limit))
		if (_hyp_obstack_newchunk(obstack, 1) == FALSE)
			return FALSE;
	*(obstack->next_free)++ = (char)character;
	return TRUE;
}


gboolean hyp_obstack_ptr_grow(struct obstack *obstack, void *data)
{
	if (((obstack)->next_free + sizeof(char *) > (obstack)->chunk_limit))
		if (_hyp_obstack_newchunk(obstack, sizeof(char *)) == FALSE)
			return FALSE;
	*((char **) (((obstack)->next_free += sizeof(char *)) - sizeof(char *))) = (char *)data;
	return TRUE;
}


gboolean hyp_obstack_int_grow(struct obstack *obstack, int data)
{
	if (((obstack)->next_free + sizeof(int) > (obstack)->chunk_limit))
		if (_hyp_obstack_newchunk(obstack, sizeof(int)) == FALSE)
			return FALSE;
	*((int *) (((obstack)->next_free += sizeof(int)) - sizeof(int))) = data;
	return TRUE;
}


gboolean hyp_obstack_long_grow(struct obstack *obstack, long data)
{
	if (((obstack)->next_free + sizeof(long) > (obstack)->chunk_limit))
		if (_hyp_obstack_newchunk(obstack, sizeof(long)) == FALSE)
			return FALSE;
	*((long *) (((obstack)->next_free += sizeof(long)) - sizeof(long))) = data;
	return TRUE;
}


gboolean hyp_obstack_blank(struct obstack *obstack, PTR_INT_TYPE length)
{
	if (((obstack)->next_free + length > (obstack)->chunk_limit))
		if (_hyp_obstack_newchunk(obstack, length) == FALSE)
			return FALSE;
	obstack->next_free += length;
	return TRUE;
}


void hyp_obstack_1grow_fast (struct obstack *obstack, int character)
{
	*(obstack->next_free)++ = (char)character;
}


void hyp_obstack_blank_fast(struct obstack *obstack, PTR_INT_TYPE length)
{
	obstack->next_free += length;
}


POINTER hyp_obstack_finish(struct obstack *obstack)
{
	POINTER temp;
	
	if (obstack->next_free == obstack->object_base)
		obstack->maybe_empty_object = 1;
	temp = obstack->object_base;
	obstack->next_free = __INT_TO_PTR ((__PTR_TO_INT ((obstack)->next_free) + (obstack)->alignment_mask)
		    & ~ ((obstack)->alignment_mask));
	if (obstack->next_free - (char *) obstack->chunk > obstack->chunk_limit - (char *) obstack->chunk)
		obstack->next_free = obstack->chunk_limit;
	obstack->object_base = obstack->next_free;
	return temp;
}


POINTER hyp_obstack_alloc(struct obstack *obstack, PTR_INT_TYPE length)
{
	if (hyp_obstack_blank(obstack, length) == FALSE)
		return NULL;
	return hyp_obstack_finish(obstack);
}


POINTER hyp_obstack_copy(struct obstack *obstack, POINTER pointer, PTR_INT_TYPE length)
{
	if (hyp_obstack_grow(obstack, pointer, length) == FALSE)
		return NULL;
	return hyp_obstack_finish(obstack);
}


POINTER hyp_obstack_copy0(struct obstack *obstack, POINTER pointer, PTR_INT_TYPE length)
{
	if (hyp_obstack_grow0(obstack, pointer, length) == FALSE)
		return NULL;
	return hyp_obstack_finish(obstack);
}
