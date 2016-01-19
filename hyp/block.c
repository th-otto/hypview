#include "hypdoc.h"
#include "hypdebug.h"

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

gboolean HypBlockOperations(DOCUMENT *doc, hyp_blockop op, BLOCK *block, void *param)
{
	HYP_NODE *node;
	HYP_DOCUMENT *hyp;
	
	hyp = (HYP_DOCUMENT *)doc->data;

	node = doc->displayed_node;

	switch (op)
	{
	case BLK_ASCIISAVE:
		{
			long line;
			ssize_t ret;
			size_t len;
			char *line_buffer;
			const char *src;
			int *handle = (int *) param;

			if (!node)					/* no page loaded */
			{
				HYP_DBG(("Error: Can't save, no page loaded"));
				return FALSE;
			}

			line = block->start.line;

			while ((line < doc->lines) && (line <= block->end.line))
			{
				line_buffer = HypGetTextLine(hyp, node, line);

				if (line_buffer != NULL)
				{
					if (line == block->start.line)
					{
						if (block->start.offset > (long)strlen(line_buffer))
							src = line_buffer + strlen(line_buffer);
						else
							src = &line_buffer[block->start.offset];
					} else
					{
						src = line_buffer;
					}
					
					if (line == block->end.line && block->end.offset < (long)strlen(line_buffer))
						line_buffer[block->end.offset] = 0;
	
					len = strlen(src);
					ret = write(*handle, src, len);
					if (ret != (ssize_t)len)
					{
						HYP_DBG(("Error %s writing file. Abort.", strerror(errno)));
						return FALSE;
					}
				}
									
				if (line != block->end.line || block->end.offset == 0)
					write(*handle, "\n", 1);

				line++;
				g_free(line_buffer);
			}
		}
		break;

	case BLK_PRINT:
		/* TODO */
		break;
	}
	return TRUE;
}
