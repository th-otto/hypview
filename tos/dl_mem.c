#include "hv_defs.h"
#include "tos/mem.h"

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*
   Mxmask returns a bit-mask with which one should mask the mode
   WORD of a Mxalloc call if one wants to use protection bits.
   This is necessary as Mxalloc unfortunately returns erroneous
   results in some GEMDOS implementations when protection bits are
   specified, which can result in a system crash.
   (c) 2016 Thorsten Otto, newly written

   Application example:
   mxMask = Mxmask();
   p = mxMask ? Mxalloc( size, 0x43 & mxMask) : Malloc( size); */

static unsigned short Mxmask(void)
{
	static unsigned short mxmask = 1;

	if (mxmask == 1)
	{
		if ((long)Mxalloc(-1, MX_STRAM) == -32L)
			mxmask = 0x0000;  /* Mxalloc is not implemented */
		else if (Sysconf(-1) == -32L)
			mxmask = MX_PREFTTRAM;  /* oldfashion Mxalloc() */
		else
			mxmask = -1;
	}

	return mxmask;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * allocate memory to be used by IPC
 */
void *g_alloc_shared(size_t size)
{
	void *ptr;
	unsigned short mxmask;
	SavePD();
	
	/*
	 * MX_READABLE would be good enough for programs
	 * that are memory-protection-aware,
	 * but since memory allocated here is used to
	 * communicate with (maybe old) programs that might
	 * try to write to their memory, we use MX_GLOBAL in order
	 * not to crash others.
	 */
	mxmask = Mxmask();
	ptr = mxmask ? Mxalloc(size, (MX_PREFTTRAM | MX_GLOBAL) & mxmask) : Malloc(size);
	RestorePD();
	return ptr;
}

/*** ---------------------------------------------------------------------- ***/

char *g_strdup2_shared(const char *str1, const char *str2, char **ptr2)
{
	char *ptr;
	size_t len1, len;
	
	*ptr2 = NULL;
	if (str1 == NULL)
		return NULL;
	len = len1 = strlen(str1) + 1;
	if (str2)
		len += strlen(str2) + 1;
	ptr = (char *) g_alloc_shared(len);
	if (ptr != NULL)
	{
		strcpy(ptr, str1);
		if (str2)
		{
			strcpy(ptr + len1, str2);
			*ptr2 = ptr + len1;
		}
	}
	return ptr;
}

/*** ---------------------------------------------------------------------- ***/

char *g_strdup_shared(const char *str)
{
	char *ptr;
	
	if (str == NULL)
		return NULL;
	ptr = (char *) g_alloc_shared(strlen(str) + 1);
	if (ptr != NULL)
	{
		strcpy(ptr, str);
	}
	return ptr;
}

/*** ---------------------------------------------------------------------- ***/

void g_free_shared(void *ptr)
{
	if (ptr != NULL)
		Mfree(ptr);
}
