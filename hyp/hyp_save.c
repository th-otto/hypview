#include "hypdefs.h"
#include "hypdebug.h"


/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*
 * set compressed size of index entry <num>
 */
void SetCompressedSize(HYP_DOCUMENT *hyp, hyp_nodenr num, unsigned long prev_pos, unsigned long curr_pos)
{
	ASSERT(hyp->num_index != 0 && num < hyp->num_index);
	hyp->indextable[num]->seek_offset = prev_pos;
	hyp->indextable[num + 1]->seek_offset = curr_pos;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * set uncompressed size of index entry <num>
 */
gboolean SetDataSize(HYP_DOCUMENT *hyp, hyp_nodenr num, unsigned long datasize)
{
	unsigned long compressed_size;
	unsigned long comp_diff;
	
	ASSERT(hyp->num_index != 0 && num < hyp->num_index);
	compressed_size = GetCompressedSize(hyp, num);
	comp_diff = datasize - compressed_size;
	hyp->indextable[num]->comp_diff = comp_diff;

	if (hyp->indextable[num]->type == HYP_NODE_IMAGE)
		hyp->indextable[num]->next = (comp_diff >> 16);

	/* check for overflow */
	return datasize == GetDataSize(hyp, num);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * Write <bytes> entry bytes from <src> to output file.
 * Encodes and encrypts data if neccessary
 * (encryption will modify the input buffer)
 * Updates <*bytes> to contain the compressed size.
 */
gboolean WriteEntryBytes(HYP_DOCUMENT *hyp, hyp_nodenr num, unsigned char *src, unsigned long *bytes, FILE *outfile, gboolean compress)
{
	unsigned long compressed_size;
	
	if (!hypnode_valid(hyp, num))
		return FALSE;
	
	/* encryption requested? */
	if (hyp->st_guide_flags & STG_ENCRYPTED)
		hyp_encrypt(src, *bytes);

	if (compress)				/* compression requested? */
	{
		if (lh5_encode(outfile, src, *bytes, 0, &compressed_size) == FALSE)
			return FALSE;
		*bytes = compressed_size;
	} else
	{
		if ((unsigned long)fwrite(src, 1, *bytes, outfile) != *bytes)
			return FALSE;
	}
	
	return TRUE;
}
