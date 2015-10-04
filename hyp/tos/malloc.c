/*
 * malloc, free, realloc: dynamic memory allocation
 *
 * Substitute for Pure-C implementation,
 * which keeps getting out of memory
 * with lots of malloc()/free calls.
 *
 * Taken from MiNTlib 0.60.0
 */

#include "hypdefs.h"

#if defined(__TOS__) || defined(__atarist__) /* rest of file */

#include <stddef.h>	/* for size_t */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include <osbind.h>
#include <errno.h>
#include <mint/arch/nf_ops.h>


#define MINFREE      (8L * 1024L)            /* free at least this much mem on top */
#define MINKEEP (64L * 1024L)                /* keep at least this much mem on stack */

/* CAUTION: use _mallocChunkSize() to tailor to your environment,
 *          do not make the default too large, as the compiler
 *          gets screwed on a 1M machine otherwise (stack/heap clash)
 */
/* minimum chunk to ask OS for */
static size_t MINHUNK =	8192L;	/* default */
static size_t MAXHUNK = 32 * 1024L; /* max. default */

/* tune chunk size */
void _mallocChunkSize(size_t chunksize)
{
	if (chunksize == 0)
	{
		_DISKINFO d;
		size_t count;
		size_t thissize;
		size_t coresize;
		void **buf;
		void **list;
		
		/* this is like fsstat(u:/proc) */
		if (Dfree(&d, 35) >= 0)
		{
			coresize = d.b_free * d.b_secsiz * d.b_clsiz;
		} else
		{
			coresize = 0;
			list = NULL;
			while ((thissize = (size_t)Malloc(-1l)) >= sizeof(void *))
			{
				buf = (void **)Malloc(thissize);
				if (buf == NULL)
					continue;
				coresize += thissize;
				*buf = list;
				list = buf;
			}
			while (list != NULL)
			{
				buf = (void **)(*list);
				Mfree(list);
				list = buf;
			}
		}
		
		chunksize = 8192;
		while (chunksize < 0x00100000UL && (chunksize * 2048UL) < coresize)
		{
			chunksize <<= 1;
		}
		/*
		 * increase chunksize until we can cover at least half of memory.
		 * This is mainly for poor old SingleTOS which may fail
		 * after a certain amount of Malloc's()
		 */
		for (;;)
		{
			list = NULL;
			count = 0;
			while ((buf = (void **)Malloc(chunksize)) != NULL)
			{
				count++;
				*buf = list;
				list = buf;
				if ((count * chunksize * 2) >= coresize)
					break;
			}
			while (list != NULL)
			{
				buf = (void **)(*list);
				Mfree(list);
				list = buf;
			}
			if ((count * chunksize * 2) >= coresize)
				break;
			chunksize <<= 1;
			if (chunksize == 0)
				break;
		}
		if (chunksize == 0)
			chunksize = 8192;
		/*
		 * increase chunksize until we get a reasonable count.
		 */
		while (count > 1024 && chunksize < 0x40000000UL)
		{
			chunksize <<= 1;
			count >>= 1;
		}
#if 0
		fprintf(stderr, "coresize = %lu, allocation %lu blocks of size %lu\n", coresize, count, chunksize);
#endif
	}
	MAXHUNK = MINHUNK = chunksize;
}

#if 1

#undef malloc
#undef calloc
#undef strdup
#undef g_strdup
#undef realloc
#undef free


#ifdef MAIN
#undef DEBUG_ALLOC
#define DEBUG_ALLOC 3
#endif

#define MEM_MAGIC_NEW_ALLOCATED 0xbb
#define MEM_MAGIC_FREED         0x99

#if DEBUG_ALLOC >= 3
static unsigned long cur_mallocs;
static unsigned long max_mallocs;
static unsigned long total_mallocs;
static unsigned long total_frees;
#endif


/* definitions needed in malloc.c and realloc.c */

struct mem_chunk 
{
	unsigned long valid;
#define VAL_FREE   0xf4ee0abcUL
#define VAL_ALLOC  0xa11c0abcUL
#define VAL_SBRK   0xb04d0abcUL

	struct mem_chunk *next;
	struct mem_chunk *prev;
	size_t size;
#if DEBUG_ALLOC >= 2
	const char *file;
	long line;
#endif
	/* size_t alloc_size; */
};

#define MALLOC_ALIGNMENT 4
#define SBRK_SIZE(ch) (*(size_t *)((char *)(ch) + sizeof(*(ch))))
#define ALLOC_EXTRA ((sizeof(struct mem_chunk) + MALLOC_ALIGNMENT - 1) & ~(MALLOC_ALIGNMENT - 1))
#define SBRK_EXTRA ((sizeof(struct mem_chunk) + sizeof(size_t) + (MALLOC_ALIGNMENT - 1)) & ~(MALLOC_ALIGNMENT - 1))

/* flag to control zero'ing of malloc'ed chunks */
#define ZeroMallocs 0

/* linked list of free blocks struct defined in lib.h */
static struct mem_chunk _mchunk_free_list = { VAL_FREE, &_mchunk_free_list, &_mchunk_free_list, 0
#if DEBUG_ALLOC >= 2
, 0, 0
#endif
};
#if DEBUG_ALLOC >= 1
static struct mem_chunk _mchunk_alloc_list = { VAL_ALLOC, &_mchunk_alloc_list, &_mchunk_alloc_list, 0
#if DEBUG_ALLOC >= 2
, 0, 0
#endif
};
#endif


#define DEF_PAGESIZE 8192				/* default page size for TOS */

int getpagesize(void)
{
	return DEF_PAGESIZE;
}


#if DEBUG_ALLOC >= 2
void *dbg_malloc(size_t n, const char *file, long line)
#else
void *malloc(size_t n)
#endif
{
	struct mem_chunk *head, *q, *p, *s;
	size_t sz;

	/* add a mem_chunk to required size and round up */
	n = ALLOC_EXTRA + ((n + MALLOC_ALIGNMENT - 1) & ~(MALLOC_ALIGNMENT - 1));

	/* look for first block big enough in free list */
	head = &_mchunk_free_list;
	q = head->next;
	while (q != head && (q->size < n || q->valid == VAL_SBRK))
		q = q->next;

	/* if not enough memory, get more from the system */
	if (q == head)
	{
		if ((n + SBRK_EXTRA) > MINHUNK)
		{
			sz = n;
			sz += SBRK_EXTRA;
		} else
		{
			sz = MINHUNK;
			if (MINHUNK < MAXHUNK)
				MINHUNK <<= 1;
		}
		{
			size_t page_size;
			
			page_size = getpagesize();
			
			sz = (sz + page_size - 1) & -page_size;
		}

		q = (struct mem_chunk * ) sbrk(sz);
		if (q == (void *)-1) /* can't alloc any more? */
			return NULL;

#if DEBUG_ALLOC >= 3
		{
			++total_mallocs;
			++cur_mallocs;
			if (cur_mallocs > max_mallocs)
				max_mallocs = cur_mallocs;
		}
#endif

		/* Note: q may be below the highest allocated chunk */
		p = head->next;
		while (p != head && q > p)
			p = p->next;
		
		q->size = SBRK_EXTRA;
		sz -= SBRK_EXTRA;
		q->valid = VAL_SBRK;
		SBRK_SIZE(q) = sz;
		q->next = s = (struct mem_chunk *) ((char *) q + SBRK_EXTRA);
		q->prev = p->prev;
		q->prev->next = q;
		q->next->prev = q;
		
		s->size = sz;
		s->valid = VAL_FREE;
		s->next = p;
		s->next->prev = s;
		
		q = s;
	}

	if (q->size > (n + ALLOC_EXTRA))
	{
		/* split, leave part of free list */
		q->size -= n;
		q = (struct mem_chunk * )(((char *) q) + q->size);
		q->size = n;
		q->valid = VAL_ALLOC;
	} else
	{
		/* just unlink it */
		q->next->prev = q->prev;
		q->prev->next = q->next;
		q->valid = VAL_ALLOC;
	}

#if DEBUG_ALLOC >= 1
	head = &_mchunk_alloc_list;
	q->next = head->next;
	q->prev = head;
	q->next->prev = q;
	q->prev->next = q;
#endif

#if DEBUG_ALLOC >= 2
	q->file = file;
	q->line = line;
#endif

	/* hand back ptr to after chunk desc */
	s = (struct mem_chunk * )(((char *) q) + ALLOC_EXTRA);

#if DEBUG_ALLOC >= 5
	memset(s, MEM_MAGIC_NEW_ALLOCATED, n - ALLOC_EXTRA);
#elif ZeroMallocs
	memset(s, 0, n - ALLOC_EXTRA);
#endif

	return (void *) s;
}


#if DEBUG_ALLOC >= 2
void *dbg_calloc(size_t nitems, size_t size, const char *file, long line)
#else
void *calloc(size_t nitems, size_t size)
#endif
{
	void *ptr;
	
	nitems *= size;
#if DEBUG_ALLOC >= 2
	ptr = dbg_malloc(nitems, file, line);
#else
	ptr = malloc(nitems);
#endif
#if !ZeroMallocs
	if (ptr != NULL)
		memset(ptr, 0, nitems);
#endif
	return ptr;
}


#if DEBUG_ALLOC >= 1
void dbg_free(void *param, const char *file, long line)
#else
void free(void *param)
#endif
{
	struct mem_chunk *head, *next, *prev;
	struct mem_chunk *r = (struct mem_chunk *) param;
	
	/* free(NULL) should do nothing */
	if (r == NULL)
		return;

	/* move back to uncover the mem_chunk */
	r = (struct mem_chunk * )(((char *) r) - ALLOC_EXTRA);

	if (r->valid != VAL_ALLOC)
	{
#if DEBUG_ALLOC >= 1
		fprintf(stderr, "Memory Block not found %s:%ld\n", file, line);
		nf_debugprintf("free: Memory Block not found %s:%ld\n", file, line);
#endif
		return;
	}
	
	r->valid = VAL_FREE;

#if DEBUG_ALLOC >= 1
	/* remove it from the alloc_list */
	r->next->prev = r->prev;
	r->prev->next = r->next;
#endif

	/* stick it into free list, preserving ascending address order */
	head = &_mchunk_free_list;
	next = head->next;
	while (next != head && next < r) 
		next = next->next;

	r->next = next;
	r->prev = next->prev;
	r->prev->next = r;
	r->next->prev = r;
	/* merge after if possible */
	if ((struct mem_chunk *)(((char *) r) + r->size) == next && next->valid == VAL_FREE)
	{
		r->size += next->size;
		r->next = next->next;
		next->next->prev = r;
	}

	/* merge before if possible, otherwise link it in */
	prev = r->prev;
	if (prev != head && prev->valid == VAL_FREE && (struct mem_chunk *)(((char *) (prev) + (prev)->size)) == r)
	{
		prev->size += r->size;
		prev->next = r->next;
		r->next->prev = prev;
		r = prev;
	}
	
	prev = r->prev;
	if (prev != head && prev->valid == VAL_SBRK && SBRK_SIZE(prev) == r->size)
	{
		prev->prev->next = r->next;
		r->next->prev = prev->prev;
#if DEBUG_ALLOC >= 5
		memset(prev, MEM_MAGIC_FREED, SBRK_SIZE(prev) + SBRK_EXTRA);
#endif
		Mfree(prev);
#if DEBUG_ALLOC >= 3
		++total_frees;
		--cur_mallocs;
#endif
	} else
	{
#if DEBUG_ALLOC >= 5
		memset((((char *) r) + ALLOC_EXTRA), MEM_MAGIC_FREED, r->size - ALLOC_EXTRA);
#endif
	}
}


#if DEBUG_ALLOC >= 1
void *dbg_realloc(void *r, size_t n, const char *file, long line)
#else
void *realloc(void *r, size_t n)
#endif
{
	struct mem_chunk *p;
	size_t sz;

	/* obscure features:
	 * 
	 * realloc(NULL,n) is the same as malloc(n)
	 * realloc(p, 0) is the same as free(p)
	 */
	if (!r)
#if DEBUG_ALLOC >= 2
		return dbg_malloc(n, file, line);
#else
		return malloc(n);
#endif

	if (n == 0)
	{
#if DEBUG_ALLOC >= 2
		dbg_free(r, file, line);
#else
		free(r);
#endif
		return NULL;
	}

	p = (struct mem_chunk * )(((char *) r) - ALLOC_EXTRA);

	if (p->valid != VAL_ALLOC)
	{
#if DEBUG_ALLOC >= 1
		fprintf(stderr, "Memory Block not found %s:%ld\n", file, line);
		nf_debugprintf("realloc: Memory Block not found %s:%ld\n", file, line);
#endif
		__set_errno(EINVAL);
		return NULL;
	}

	sz = ALLOC_EXTRA + ((n + MALLOC_ALIGNMENT - 1) & ~(MALLOC_ALIGNMENT - 1));

	if (p->size > (sz + 2 * ALLOC_EXTRA))
	{
		/* resize down */
		void *newr;

#if DEBUG_ALLOC >= 2
		newr = dbg_malloc(n, file, line);
#else
		newr = malloc(n);
#endif
		if (newr)
		{
			memcpy(newr, r, n);
#if DEBUG_ALLOC >= 2
		    dbg_free(r, file, line);
#else
		    free(r);
#endif

			r = newr;
		}
		/* else
		 * malloc failed; can be safely ignored as the new block
		 * is smaller
		 */
	} else if (p->size < sz)
	{
		/* block too small, get new one */
		struct mem_chunk *head, *s, *next;

		head = &_mchunk_free_list;
		next = head->next;
		while (next != head && next < p)
		{
			next = next->next;
		}

		/* merge after if possible */
		s = (struct mem_chunk * )(((char *) p) + p->size);
		if (s == next && (p->size + next->size) >= sz && next->valid == VAL_FREE)
		{
			p->size += next->size;
			p->next = next->next;
			next->next->prev = p;
		} else
		{
			void *newr;

#if DEBUG_ALLOC >= 2
			newr = dbg_malloc(n, file, line);
#else
			newr = malloc(n);
#endif
			if (newr)
			{
				memcpy(newr, r, p->size - ALLOC_EXTRA);
#if DEBUG_ALLOC >= 2
			    dbg_free(r, file, line);
#else
			    free(r);
#endif
			}
			r = newr;
		}
	}

	return (void *) r;
}


void *sbrk(intptr_t n)
{
	void *rval;

	rval = (void *) Malloc(n);
	if (rval == NULL)
	{
		__set_errno(ENOMEM);
		rval = (void *) (-1L);
	}
	return rval;
}

#endif



#if DEBUG_ALLOC >= 2
char *dbg_strdup(const char *str, const char *file, long line)
{
	char *dst;
	
	if (str == NULL)
		return NULL;
	dst = (char *)dbg_malloc(strlen(str) + 1, file, line);
	if (dst == NULL)
		return NULL;
	return strcpy(dst, str);
}


char *dbg_strndup(const char *str, size_t len, const char *file, long line)
{
	char *dst;
	
	if (str == NULL)
		return NULL;
	if (len == STR0TERM)
		len = strlen(str);
	dst = (char *)dbg_malloc(len + 1, file, line);
	if (dst == NULL)
		return NULL;
	memcpy(dst, str, sizeof(char) * len);
	dst[len] = '\0';
	return dst;
}
#endif


void _crtexit(void)
{
#if DEBUG_ALLOC >= 3
	fprintf(stderr, "Mallocs: %lu (max %lu), Mfrees %lu, cur %ld\n",
		total_mallocs,
		max_mallocs,
		total_frees,
		cur_mallocs);
#endif
#if DEBUG_ALLOC >= 1
	{
		struct mem_chunk *head, *p;
		head = &_mchunk_alloc_list;
		p = head->next;
		if (p != head)
		{
			fprintf(stderr, "still allocated:\n");
			nf_debugprintf("still allocated:\n");
			for (; p != head; p = p->next)
			{
#if DEBUG_ALLOC >= 2
				fprintf(stderr, "-> %lu %s:%ld\n", p->size - ALLOC_EXTRA, p->file, p->line);
				nf_debugprintf("-> %lu %s:%ld\n", p->size - ALLOC_EXTRA, p->file, p->line);
#else
				fprintf(stderr, "-> %lu %p\n", p->size - ALLOC_EXTRA, (char *)p + ALLOC_EXTRA);
				nf_debugprintf("-> %lu %p\n", p->size - ALLOC_EXTRA, (char *)p + ALLOC_EXTRA);
#endif
			}
		}
	}
#endif
}


#ifdef MAIN
static void *xmalloc(size_t size)
{
	void *p = malloc(size);
	if (p == NULL)
	{
		fprintf(stderr, "malloc(%lu) failed\n", (unsigned long)size);
	fprintf(stderr, "Mallocs: %lu (max %lu), Mfrees %lu, cur %ld\n",
		total_mallocs,
		max_mallocs,
		total_frees,
		cur_mallocs);
		exit(1);
	}
	return p;
}

extern long _stksize;

int main(void)
{
#if 1
	size_t i, count, size;
	void **ptrs;
	
	_mallocChunkSize(0);
	
	{
	BASEPAGE *bp = _base;
	
	printf("_base: 0x%08p\n", _base);
	printf("_stksize: 0x%08lx\n", _stksize);
	printf("bss start & length: 0x%08p 0x%08lx\n", bp->p_bbase, bp->p_blen);
	printf("hitpa: 0x%08p\n", bp->p_hitpa);
	printf("chunksize: 0x%08lx\n", MINHUNK);
	}

	printf("pass 1\n");
	count = (((long)rand() * 1000) / RAND_MAX) + 500;
	ptrs = xmalloc(count * sizeof(void *));
	for (i = 0; i < count; i++)
	{
		size = (((long)rand() * 3000) / RAND_MAX) + 3000;
		ptrs[i] = xmalloc(size);
	}
	for (i = 0; i < count; i++)
		free(ptrs[i]);
	free(ptrs);
	fprintf(stderr, "Mallocs: %lu (max %lu), Mfrees %lu, cur %ld\n",
		total_mallocs,
		max_mallocs,
		total_frees,
		cur_mallocs);
	
	printf("pass 2\n");
	count = (((long)rand() * 5000) / RAND_MAX) + 5000;
	ptrs = xmalloc(count * sizeof(void *));
	for (i = 0; i < count; i++)
	{
		size = (((long)rand() * 5000) / RAND_MAX) + 5000;
		ptrs[i] = xmalloc(size);
	}
	while (count)
	{
		i = (((long)rand() * count) / RAND_MAX);
		--count;
		free(ptrs[i]);
		ptrs[i] = ptrs[count];
	}
	free(ptrs);
#else
	void *p1, *p2, *p3;
	
	p1 = malloc(5000);
	p2 = malloc(5000);
	p3 = malloc(5000);
	free(p1);
	free(p2);
	free(p3);
#endif
	
	_crtexit();
	return 0;
}

#endif

#else

extern int _I_dont_care_that_ISO_C_forbids_an_empty_source_file_;

#endif
