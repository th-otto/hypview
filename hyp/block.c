#include "hypdoc.h"
#include "hypdebug.h"

#ifdef __PUREC__
#include "hv_defs.h" /* Pure-C does not like anonymous struct _window_data_ */
#endif

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

gboolean HypBlockOperations(WINDOW_DATA *win, hyp_blockop op, BLOCK *block, void *param)
{
	HYP_NODE *node;
	node = hypwin_node(win);

	switch (op)
	{
	case BLK_ASCIISAVE:
		{
			long line;
			ssize_t ret;
			ssize_t len;
			char *line_buffer;
			const char *src;
			int *handle = (int *) param;
			char *txt;
			
			if (!node)					/* no page loaded */
			{
				HYP_DBG(("Error: Can't save, no page loaded"));
				return FALSE;
			}

			line = block->start.line;
			while (line <= block->end.line)
			{
				line_buffer = HypGetTextLine(win, node, line);

				if (line_buffer != NULL)
				{
					len = strlen(line_buffer);

					if (line == block->end.line && block->end.offset < len)
					{
						line_buffer[block->end.offset] = 0;
						len = block->end.offset;
					}
					if (line == block->start.line)
					{
						if (block->start.offset > len)
						{
							src = line_buffer + len;
							len = 0;
						} else
						{
							src = &line_buffer[block->start.offset];
							len -= block->start.offset;
						}
					} else
					{
						src = line_buffer;
					}
					
					txt = hyp_utf8_to_charset(hyp_get_current_charset(), src, len, NULL);
					len = strlen(txt);
					ret = write(*handle, txt, len);
					g_free(txt);
					g_free(line_buffer);
					if (ret != len)
					{
						HYP_DBG(("Error %s writing file. Abort.", strerror(errno)));
						return FALSE;
					}
				}
									
				if (line != block->end.line || block->end.offset == 0)
					write(*handle, "\n", 1);

				line++;
			}
		}
		break;

	case BLK_PRINT:
		/* TODO */
		break;
	}
	return TRUE;
}
