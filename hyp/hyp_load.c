#include "hypdefs.h"
#include "hypdebug.h"



/* decrypt encrypted data */
void hyp_decrypt(unsigned char *ptr, long bytes)
{
	while (bytes--)
	{
		*ptr = *ptr ^ 127;
		ptr++;
	}
}


/*
 * return uncompressed size of index entry <num>
 */
unsigned long GetDataSize(HYP_DOCUMENT *hyp, hyp_nodenr num)
{
	unsigned long data_size;

	data_size = GetCompressedSize(hyp, num);
	data_size += hyp->indextable[num]->comp_diff;

	if (hyp->indextable[num]->type == HYP_NODE_IMAGE)
		data_size += ((unsigned long) hyp->indextable[num]->next) << 16;

	return data_size;
}


/*
 * Load index entry.
 * Returns buffer and size of compressed data.
 */
unsigned char *hyp_loaddata(HYP_DOCUMENT *hyp, hyp_nodenr num)
{
	INDEX_ENTRY *idxent = hyp->indextable[num];
	int handle;
	ssize_t ret;
	unsigned char *data;
	unsigned long data_size;
	gboolean close_here = FALSE;
	
	if (!hypnode_valid(hyp, num))
		return NULL;
	
	idxent = hyp->indextable[num];
	handle = hyp->handle;
	if (handle < 0)
	{
		handle = hyp_utf8_open(hyp->file, O_RDONLY | O_BINARY, HYP_DEFAULT_FILEMODE);
		if (handle < 0)
		{
			FileErrorErrno(hyp->file);
			return NULL;
		}
		hyp->handle = handle;
		close_here = TRUE;
	}
	
	ret = lseek(handle, idxent->seek_offset, SEEK_SET);
	if (ret < 0)
	{
		FileErrorErrno(hyp->file);
		data = NULL;
	} else
	{
		data_size = GetCompressedSize(hyp, num);
	
		data = g_new(unsigned char, data_size == 0 ? 1 : data_size);
		if (data != NULL)
		{
			/* load (possible compressed and encrypted) data */
			ret = read(handle, data, data_size);
			if (ret < 0 || (unsigned long)ret != data_size)
			{
				FileErrorErrno(hyp->file);
				g_free(data);
				data = NULL;
			}
		}
	}
	
	if (close_here)
	{
		hyp_utf8_close(handle);
		hyp->handle = -1;
	}
	return data;
}

/* ------------------------------------------------------------------------- */

HYP_NODE *hyp_loadtext(HYP_DOCUMENT *hyp, hyp_nodenr node_num)
{
	long size;
	unsigned char *data;
	gboolean ret;
	HYP_NODE *node;
	
	size = GetDataSize(hyp, node_num);
	node = hyp_node_alloc(size);

	if (node != NULL)
	{
		node->number = node_num;
	
		/* load compressed data */
		data = hyp_loaddata(hyp, node_num);
		if (data == NULL)
		{
			hyp_node_free(node);
			HYP_DBG(("ERROR: Entry %u: hyp_loadtext failed", node_num));
			node = NULL;
		} else
		{
			/* decompress and decrypt data to target */
			ret = GetEntryBytes(hyp, node_num, data, node->start, size);
			g_free(data);
	
			*node->end = 0;
	
			if (ret == FALSE)
			{
				HYP_DBG(("ERROR: while preparing page for display!"));
				hyp_node_free(node);
				node = NULL;
			}
		}
	}
	return node;
}


/*
 * Copy <bytes> entry bytes (read by hyp_loaddata())
 * from <src> to <dst>.
 * Decodes and decrypts data if neccessary.
 */
gboolean GetEntryBytes(HYP_DOCUMENT *hyp, hyp_nodenr num, const unsigned char *src, unsigned char *dst, long bytes)
{
	INDEX_ENTRY *idxent;

	if (!hypnode_valid(hyp, num))
		return TRUE;
	
	idxent = hyp->indextable[num];
	if (idxent->comp_diff)				/* data compressed? */
	{
		unsigned long data_size = GetCompressedSize(hyp, num);

		if (!lh5_decode(dst, bytes, src, data_size))
			return FALSE;
	} else
	{
		memcpy(dst, src, bytes);
	}
	
	/* need to decrypt? */
	if (hyp->st_guide_flags & STG_ENCRYPTED)
		hyp_decrypt(dst, bytes);
	return TRUE;
}
