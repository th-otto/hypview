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
   ((c) 1994 Martin Osieka)

   Application example:
   mxMask = Mxmask();
   p = mxMask ? Mxalloc( size, 0x43 & mxMask) : Malloc( size); */

static unsigned short Mxmask(void)
{
	static unsigned short mxmask = 1;
    void *svStack;         /* Supervisor-Stack */
    int32_t sRAM, sRAMg;   /* ST-RAM */
    int32_t aRAM, aRAMg;   /* Alternate RAM */

	if (mxmask != 1)
		return mxmask;
	
	/*
	// Sample table of possible values:
	//           | newfashion  | oldfashion
	// sRAM aRAM | sRAMg aRAMg | sRAMg aRAMg
	//   1    0  |   1     0   |   1     1
	//   0    2  |   0     2   |   2     2
	//   1    2  |   1     2   |   3     3
	*/

    svStack = (void *) Super( 0);  /* Disallow task-switching */

    sRAM  = (int32_t) Mxalloc( -1, MX_STRAM);
    sRAMg = (int32_t) Mxalloc( -1, MX_READABLE | MX_STRAM);
    aRAM  = (int32_t) Mxalloc( -1, MX_TTRAM);
    aRAMg = (int32_t) Mxalloc( -1, MX_READABLE | MX_TTRAM);

    Super( svStack);  /* Permit task-switching */

    if (sRAM == -32)
        mxmask = 0x0000;  /* Mxalloc is not implemented */

    else if ( ((sRAM + aRAM) == sRAMg) && ((sRAM + aRAM) == aRAMg) )
        mxmask = 0x0003;  /* oldfashion Mxalloc() */

    else
        mxmask = 0xFFFF;

	return mxmask;
	
} /* Mxmask */

/*** ---------------------------------------------------------------------- ***/

/*
 * allocate memory to be used by IPC
 */
void *g_alloc_shared(size_t size)
{
	void *ptr;
	unsigned short mxmask;
	
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
