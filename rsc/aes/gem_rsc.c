#include "aes.h"
#include "gem_rsc.h"
#define rs_fstr rs_frstr
#define N_(a) a
#include "gem_rsc.rsh"
#include "s_endian.h"

OBJECT **aes_rsc_tree = (OBJECT **)rs_trindex;
const char *const *aes_rsc_string = rs_frstr;
const BITBLK *const *aes_rsc_bitblk = rs_frimg;

/* Strings for the alert box */
static char msg_str[MAX_LINENUM][MAX_LINELEN+1];
static char msg_but[MAX_BUTNUM][MAX_BUTLEN+1];

_BOOL rsc_read(void)
{
	OBJECT *tree;
	int i;
	
	/* Copy data from ROM to RAM: */
	memcpy(rs_object, rs_object_rom, sizeof(rs_object));
	memcpy(rs_tedinfo, rs_tedinfo_rom, sizeof(rs_tedinfo));

	/*
	 * Set up message & button buffers for form_alert().
	 */
	tree = rs_trindex[DIALERT];
	for (i = 0; i < MAX_LINENUM; i++)
		tree[MSGOFF + i].ob_spec.free_string = msg_str[i];
	for (i = 0; i < MAX_BUTNUM; i++)
		tree[BUTOFF + i].ob_spec.free_string = msg_but[i];
	return TRUE;
}


void rsc_free(void)
{
}


/* Get a string from the GEM-RSC */
const char *rs_str(_UWORD stnum)
{
	return aes_rsc_string[stnum];
}
