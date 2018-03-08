#include "hypdefs.h"
#include "hypdebug.h"



short HypCountExtRefs(HYP_NODE *node)
{
	const unsigned char *pos;
	short count = 0;

	if (node == NULL)
		return 0;
	pos = node->start;
	while (pos < node->end && *pos == HYP_ESC)
	{
		if (pos[1] == HYP_ESC_EXTERNAL_REFS)
			count++;
		pos = hyp_skip_esc(pos);
	}
	return count;
}
