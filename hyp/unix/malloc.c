#include "hypdefs.h"

#if !defined(__TOS__) && !defined(__atarist__) && DEBUG_ALLOC >= 2

#include <stddef.h>	/* for size_t */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>


#undef malloc
#undef calloc
#undef strdup
#undef realloc
#undef free
#undef g_malloc
#undef g_try_malloc
#undef g_free
#undef g_realloc
#undef g_strdup

#ifndef HAVE_GLIB
#undef g_malloc0
#define g_malloc(n) malloc(n)
#define g_malloc0(n) calloc(n, 1)
#define g_realloc(ptr, s) realloc(ptr, s)
#define g_free(t) free(t)
#define g_try_malloc(n) malloc(n)
#endif

#define MEM_MAGIC_START_INTERN 8154711L

#define MEM_MAGIC_NEW_ALLOCATED 0xbb
#define MEM_MAGIC_END			0xaa
#define MEM_MAGIC_FREED         0x99

#ifndef ALLOC_ALIGN_SIZE
#  define ALLOC_ALIGN_SIZE (sizeof(double) > 8 ? 16 : 8)
#endif

#define SIZEOF_MEM_CONTROL (((sizeof(MEM_CONTROL) + ALLOC_ALIGN_SIZE - 1) / ALLOC_ALIGN_SIZE) * ALLOC_ALIGN_SIZE)

#if DEBUG_ALLOC
#define EXTRA_MEM (SIZEOF_MEM_CONTROL + sizeof(unsigned char) * MEM_MAGIC_SIZE)
#else
#define EXTRA_MEM (SIZEOF_MEM_CONTROL)
#endif


static gboolean got_errors;
#if DEBUG_ALLOC
static size_t memUse;
static size_t memUseSize;
static size_t startMemCount;
static size_t startMemSize;
static _BOOL mem_test_ignored;
#if DEBUG_ALLOC >= 3
static size_t memMaxUse;
static size_t memMaxUseSize;
static size_t memMaxUseBlock;
#endif
#if DEBUG_ALLOC >= 4
static FILE *trc_file;
#define TRC_FILENAME "/tmp/dbgalloc.trc"
#endif
#endif

#define MEM_MAGIC_SIZE 4

typedef struct _mem_control
{
	size_t size;
#if DEBUG_ALLOC
#if DEBUG_ALLOC >= 2
	const char *file;
	long line;
	struct _mem_control *next;
#endif
#endif
	long magic;
#if DEBUG_ALLOC
	unsigned char checker[MEM_MAGIC_SIZE];
#endif
} MEM_CONTROL;


#if DEBUG_ALLOC >= 2
LOCAL MEM_CONTROL *alloc_list;

/*** ---------------------------------------------------------------------- ***/

static const char *dbg_basename(const char *filename)
{
	char *slash;
	char *backslash;

	if (filename == NULL)
		return "(null)";
	slash = strrchr(filename, '/');
	backslash = strrchr(filename, '\\');

	if (slash > backslash)
		return slash + 1;
	if (backslash != NULL)
		return backslash + 1;
	return filename;
}

/*** ---------------------------------------------------------------------- ***/

__attribute__((format(printf, 3, 4)))
static void dbg_trace(const char *file, long line, const char *fmt, ...)
{
	va_list args;
	
	va_start(args, fmt);
#if DEBUG_ALLOC >= 4
	if (trc_file)
	{
		if (file)
			fprintf(trc_file, "%s:%ld: ", dbg_basename(file), line);
		vfprintf(trc_file, fmt, args);
		fflush(trc_file);
	}
	va_end(args);
	va_start(args, fmt);
#endif
#if DEBUG_ALLOC >= 2
	if (file)
		fprintf(stderr, "%s:%ld: ", dbg_basename(file), line);
#endif
	vfprintf(stderr, fmt, args);
	va_end(args);
}

/*** ---------------------------------------------------------------------- ***/

static gboolean delete_alloc_list(const char *file, long line, MEM_CONTROL *block)
{
	MEM_CONTROL *search = alloc_list;

	if (block == search)
	{
		alloc_list = search->next;
	} else
	{
		while (search && search->next != block)
			search = search->next;
		if (search && search->next == block)
		{
			search->next = search->next->next;
		} else
		{
			dbg_trace(file, line, "Memory Block not found: %p\n", (void *)((char *)block + SIZEOF_MEM_CONTROL));
			got_errors = TRUE;
			return FALSE;
		}
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static gboolean move_alloc_list(const char *file, long line, MEM_CONTROL *oldblock, MEM_CONTROL *newblock)
{
	MEM_CONTROL *search = alloc_list;

	if (oldblock == search)
	{
		alloc_list = newblock;
	} else
	{
		while (search && search->next != oldblock)
			search = search->next;
		if (search && search->next == oldblock)
		{
			search->next = newblock;
		} else
		{
			dbg_trace(file, line, "Memory Block not found: %p\n", (void *)((char *)oldblock + SIZEOF_MEM_CONTROL));
			got_errors = TRUE;
			return FALSE;
		}
	}
	return TRUE;
}

static gboolean in_alloc_list(MEM_CONTROL *block)
{
	MEM_CONTROL *search = alloc_list;

	while (search)
	{
		if (search == block)
			return TRUE;
		search = search->next;
	}
	got_errors = TRUE;
	return FALSE;
}

#endif							/* DEBUG_ALLOC >= 2 */

void *dbg_malloc(size_t size, const char *file, long line)
{
	MEM_CONTROL *cntl;
	
	if (size == 0)
	{
		dbg_trace(file, line, "MemGet(0)?\n");
		return NULL;
	}

	cntl = (MEM_CONTROL *)g_try_malloc(size + EXTRA_MEM);

	if (cntl == NULL)
	{
		return NULL;
	}
	
	cntl->magic = MEM_MAGIC_START_INTERN;
	
	cntl->size = size;
	{
		size_t i;
		
		for (i = 0; i < SIZEOF_MEM_CONTROL - offsetof(MEM_CONTROL, checker); i++)
			cntl->checker[i] = MEM_MAGIC_END;
	}
#if DEBUG_ALLOC >= 2
	cntl->next = alloc_list;
	alloc_list = cntl;
	cntl->file = file;
	cntl->line = line;
#else
	UNUSED(file);
	UNUSED(line);
#endif
	cntl = (MEM_CONTROL *)((char *)cntl + SIZEOF_MEM_CONTROL);

	memset(cntl, MEM_MAGIC_NEW_ALLOCATED, size);

	{
		unsigned char *test;
		size_t i;
		
		test = (unsigned char *) cntl + size;
		for (i = 0; i < MEM_MAGIC_SIZE; i++)
			test[i] = MEM_MAGIC_END;
	}
	memUseSize += size;
	memUse++;
#if DEBUG_ALLOC >= 3
	if (memUse > memMaxUse)
		memMaxUse = memUse;
	if (memUseSize > memMaxUseSize)
		memMaxUseSize = memUseSize;
	if (size > memMaxUseBlock)
		memMaxUseBlock = size;
#endif


#if DEBUG_ALLOC >= 4
	if (trc_file != NULL)
	{
		fprintf(trc_file, "%s:%ld: Trace: allocated %lu bytes memory at %p\n", dbg_basename(file), line, size, (void *)cntl);
		fflush(trc_file);
	}
#endif
	return (void *) cntl;
}


void *dbg_calloc(size_t nitems, size_t size, const char *file, long line)
{
	void *ptr;
	
	nitems *= size;
#if DEBUG_ALLOC >= 2
	ptr = dbg_malloc(nitems, file, line);
	if (ptr != NULL)
		memset(ptr, 0, nitems);
#else
	ptr = g_malloc0(nitems);
#endif
	return ptr;
}


void dbg_free(void *strPtr, const char *file, long line)
{
	MEM_CONTROL *memCntl;
	gboolean defect;
	
	if (strPtr == NULL)
	{
		return;
	}

	memCntl = (MEM_CONTROL *)((unsigned char *)strPtr - SIZEOF_MEM_CONTROL);

#if DEBUG_ALLOC >= 2
	if (!delete_alloc_list(file, line, memCntl))
		return;
#endif

#if DEBUG_ALLOC >= 4
	if (trc_file != NULL)
	{
		fprintf(trc_file, "%s:%ld: Trace: free %lu bytes memory at %p\n", dbg_basename(file), line, memCntl->size, (void *)strPtr);
		fflush(trc_file);
	}
#endif

	defect = FALSE;
	if (memCntl->magic != MEM_MAGIC_START_INTERN)
		defect = TRUE;

#if DEBUG_ALLOC
	{
		size_t i;
		
		for (i = 0; i < SIZEOF_MEM_CONTROL - offsetof(MEM_CONTROL, checker); i++)
			if (memCntl->checker[i] != MEM_MAGIC_END)
				defect = TRUE;
	}
#endif

	if (defect)
	{
		got_errors = TRUE;
		dbg_trace(file, line, "MemStart defekt (%lu).\n", (unsigned long)memCntl->size);
		return;
	}

#if DEBUG_ALLOC
	{
		unsigned char *test = ((unsigned char *)memCntl) + memCntl->size + SIZEOF_MEM_CONTROL;
		size_t i;
		
		defect = FALSE;
		for (i = 0; i < MEM_MAGIC_SIZE; i++)
			if (test[i] != MEM_MAGIC_END)
				defect = TRUE;
		if (defect)
		{
			got_errors = TRUE;
			/* assumes MAGIC_SIZE >= 4 */
			dbg_trace(file, line, "MemEnd defekt %ld. %02x %02x %02x %02x\n", (unsigned long)memCntl->size,
				test[0], test[1], test[2], test[3]);
		}
	}

	memUse--;
	memUseSize -= memCntl->size;
	memset(memCntl, MEM_MAGIC_FREED, memCntl->size + EXTRA_MEM);
#endif
	g_free(memCntl);
}


void *dbg_realloc(void *ptr, size_t newsize, const char *file, long line)
{
	MEM_CONTROL *cntl;
	MEM_CONTROL *newcntl;
	int defect;
	size_t oldsize;
	
	if (newsize == 0)
	{
		dbg_free(ptr, file, line);
		return NULL;
	}
	if (ptr == NULL)
	{
#if DEBUG_ALLOC >= 4
		if (trc_file != NULL)
		{
			fprintf(trc_file, "%s:%ld: Trace: realloc: ptr is NULL\n", dbg_basename(file), line);
			fflush(trc_file);
		}
#endif
		return dbg_malloc(newsize, file, line);
	}
	
	cntl = (MEM_CONTROL *) ((char *) (ptr) - SIZEOF_MEM_CONTROL);

#if DEBUG_ALLOC >= 2
	if (!in_alloc_list(cntl))
	{
		dbg_trace(file, line, "Memory Block not found: %p\n", ptr);
		return NULL;
	}
#endif
	
	defect = FALSE;
	if (cntl->magic != MEM_MAGIC_START_INTERN)
		defect = TRUE;

	{
		size_t i;
		unsigned char *test;
	
		test = cntl->checker;
		for (i = 0; i < SIZEOF_MEM_CONTROL - offsetof(MEM_CONTROL, checker); i++)
			if (test[i] != MEM_MAGIC_END)
				defect = TRUE;
	}

	if (defect)
	{
		got_errors = TRUE;
		dbg_trace(file, line, "MemStart defekt (%lu).\n", (unsigned long)cntl->size);
		return NULL;
	}

	{
		size_t i;
		unsigned char *test;
	
		defect = FALSE;
		test = ((unsigned char *) (cntl)) + cntl->size + SIZEOF_MEM_CONTROL;
		for (i = 0; i < MEM_MAGIC_SIZE; i++)
			if (test[i] != MEM_MAGIC_END)
				defect = TRUE;
		if (defect)
		{
			got_errors = TRUE;
			dbg_trace(file, line, "MemEnd defekt (%lu).\n", (unsigned long)cntl->size);
			return NULL;
		}
	}

	oldsize = cntl->size;
	
	newcntl = (MEM_CONTROL *)g_realloc(cntl, newsize + EXTRA_MEM);
	if (newcntl == NULL)
	{
		newcntl = (MEM_CONTROL *)g_malloc(newsize + EXTRA_MEM);
		if (newcntl == NULL)
		{
			return NULL;
		}
		memcpy(newcntl, cntl, (newsize < oldsize ? newsize : oldsize) + EXTRA_MEM);
		g_free(cntl);
	}
	newcntl->magic = MEM_MAGIC_START_INTERN;
	
	newcntl->size = newsize;
	{
		size_t i;
		
		for (i = 0; i < SIZEOF_MEM_CONTROL - offsetof(MEM_CONTROL, checker); i++)
			newcntl->checker[i] = MEM_MAGIC_END;
	}
#if DEBUG_ALLOC >= 2
	if (cntl != newcntl)
	{
		if (!move_alloc_list(file, line, cntl, newcntl))
			return NULL;
	}
	newcntl->file = file;
	newcntl->line = line;
#else
	UNUSED(file);
	UNUSED(line);
#endif
	
	newcntl = (MEM_CONTROL *)((char *)newcntl + SIZEOF_MEM_CONTROL);

	if (newsize > oldsize)
		memset((char *)newcntl + oldsize, MEM_MAGIC_NEW_ALLOCATED, newsize - oldsize);

	{
		unsigned char *test;
		size_t i;
		
		test = (unsigned char *) newcntl + newsize;
		for (i = 0; i < MEM_MAGIC_SIZE; i++)
			test[i] = MEM_MAGIC_END;
	}
	
	if (newsize > oldsize)
		memUseSize += newsize - oldsize;
	else
		memUseSize -= oldsize - newsize;
#if DEBUG_ALLOC >= 3
	if (memUseSize > memMaxUseSize)
		memMaxUseSize = memUseSize;
	if (newsize > memMaxUseBlock)
		memMaxUseBlock = newsize;
#endif
#if DEBUG_ALLOC >= 4
	if (trc_file != NULL)
	{
		fprintf(trc_file, "%s:%ld: Trace: realloc (%p,%lu) to (%p,%lu)\n", dbg_basename(file), line, ((char *)cntl + SIZEOF_MEM_CONTROL), oldsize, (void *)newcntl, newsize);
		fflush(trc_file);
	}
#endif
	
	return (void *) newcntl;
}


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


#if DEBUG_ALLOC >= 4
void _crtinit(void)
{
	if (trc_file == NULL)
	{
		trc_file = fopen(TRC_FILENAME, "w");
	}
}
#endif


void _crtexit(void)
{
#if DEBUG_ALLOC
	if ((memUse != startMemCount || memUseSize != startMemSize) && !mem_test_ignored)
	{
		dbg_trace(NULL, 0, "MemTest A:%ld S:%ld\n", (unsigned long)(memUse - startMemCount), (unsigned long)(memUseSize - startMemSize));
		got_errors = TRUE;
#if DEBUG_ALLOC >= 2
		while (alloc_list != NULL)
		{
			dbg_trace(alloc_list->file, alloc_list->line, "-> %7ld\n", (unsigned long)alloc_list->size);
			alloc_list = alloc_list->next;
		}
#endif
	}
#endif
#if DEBUG_ALLOC >= 3
	dbg_trace(NULL, 0, "Max Memory usage: %lu blocks, %lu bytes, largest %lu\n", memMaxUse, memMaxUseSize, memMaxUseBlock);
#endif
#if DEBUG_ALLOC >= 4
	if (trc_file != NULL)
	{
		fclose(trc_file);
		trc_file = NULL;
	}
#endif
}

#else

extern int _I_dont_care_that_ISO_C_forbids_an_empty_source_file_;

#endif
