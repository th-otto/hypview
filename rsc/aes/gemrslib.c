/*      GEMRSLIB.C      5/14/84 - 06/23/85      Lowell Webster          */
/*      merge High C vers. w. 2.2               8/24/87         mdf     */

/*
 *		Copyright 1999, Caldera Thin Clients, Inc.
 *				  2002-2016 The EmuTOS development team
 *
 *		This software is licenced under the GNU Public License.
 *		Please see LICENSE.TXT for further information.
 *
 *				   Historical Copyright
 *		-------------------------------------------------------------
 *		GEM Application Environment Services			  Version 2.3
 *		Serial No.	XXXX-0000-654321			  All Rights Reserved
 *		Copyright (C) 1987						Digital Research Inc.
 *		-------------------------------------------------------------
 */


#include "aes.h"
#include "gemlib.h"

RSHDR *rs_hdr;
AES_GLOBAL *rs_global;


/*
 *	Fix up a character position, from offset,row/col to a pixel value.
 *	If width is 80 then convert to full screen width.
 */
static void fix_chpos(_WORD *pfix, _WORD offset)
{
	_WORD coffset;
	_WORD cpos;

	cpos = *pfix;
	coffset = (cpos >> 8) & 0x00ff;
	cpos &= 0x00ff;

	switch (offset)
	{
	case 0:
		cpos *= gl_wchar;
		break;
	case 1:
		cpos *= gl_hchar;
		break;
	case 2:
		if (cpos == 80)
			cpos = gl_width;
		else
			cpos *= gl_wchar;
		break;
	case 3:
		cpos *= gl_hchar;
		break;
	}

	cpos += coffset > 128 ? (coffset - 256) : coffset;
	*pfix = cpos;
}


/************************************************************************
 * AES #114 - rsrc_obfix - Resource object fix
 *
 * rs_obfix
 ************************************************************************/
void rs_obfix(OBJECT *tree, _WORD curob)
{
	_WORD offset;
	_WORD *p;

	/* set X,Y,W,H */
	p = &tree[curob].ob_x;

	for (offset = 0; offset < 4; offset++)
		fix_chpos(&p[offset], offset);
}


static void *get_sub(_UWORD rsindex, _UWORD offset, _UWORD rsize)
{
	/* get base of objects and then index in */
	return (char *)rs_hdr + offset + rsize * rsindex;
}


/*
 *	return address of given type and index, INTERNAL ROUTINE
 */
static void *get_addr(_UWORD rstype, _UWORD rsindex)
{
	_WORD size;
	_UWORD offset;
	RSHDR *hdr = rs_hdr;
	
	switch (rstype)
	{
	case R_TREE:
		return rs_global->ap_ptree[rsindex];
	case R_OBJECT:
		offset = hdr->rsh_object;
		size = sizeof(OBJECT);
		break;
	case R_TEDINFO:
	case R_TEPTEXT: /* same, because te_ptext is first field of TEDINFO */
		offset = hdr->rsh_tedinfo;
		size = sizeof(TEDINFO);
		break;
	case R_ICONBLK:
	case R_IBPMASK: /* same, because ib_pmask is first field of ICONBLK */
		offset = hdr->rsh_iconblk;
		size = sizeof(ICONBLK);
		break;
	case R_BITBLK:
	case R_BIPDATA: /* same, because bi_pdata is first field of BITBLK */
		offset = hdr->rsh_bitblk;
		size = sizeof(BITBLK);
		break;
	case R_OBSPEC:
		{
			OBJECT *obj = (OBJECT *)get_addr(R_OBJECT, rsindex);
			return &obj->ob_spec;
		}
	case R_TEPTMPLT:
	case R_TEPVALID:
		{
			TEDINFO *tedinfo;

			tedinfo = (TEDINFO *)get_addr(R_TEDINFO, rsindex);
			if (rstype == R_TEPTMPLT)
				return &tedinfo->te_ptmplt;
			else
				return &tedinfo->te_pvalid;
		}
	case R_IBPDATA:
	case R_IBPTEXT:
		{
			ICONBLK *iconblk;

			iconblk = (ICONBLK *)get_addr(R_ICONBLK, rsindex);
			if (rstype == R_IBPDATA)
				return &iconblk->ib_pdata;
			else
				return &iconblk->ib_ptext;
		}
	case R_STRING:
		return *((void **)get_sub(rsindex, hdr->rsh_frstr, sizeof(uint32_t)));
	case R_IMAGEDATA:
		return *((void **)get_sub(rsindex, hdr->rsh_frimg, sizeof(uint32_t)));
	case R_FRSTR:
		offset = hdr->rsh_frstr;
		size = sizeof(uint32_t);
		break;
	case R_FRIMG:
		offset = hdr->rsh_frimg;
		size = sizeof(uint32_t);
		break;
	default:
		return (void *)-1L;
	}
	return get_sub(rsindex, offset, size);
}


static _BOOL fix_long(intptr_t *plong)
{
	intptr_t lngval;

	lngval = *plong;
	if (lngval != -1L)
	{
		*plong = (intptr_t)rs_hdr + lngval;
		return TRUE;
	}
	return FALSE;
}


static void fix_trindex(void)
{
	_WORD ii;
	intptr_t *ptreebase;

	ptreebase = (intptr_t *)get_sub(R_TREE, rs_hdr->rsh_trindex, sizeof(uint32_t));
	rs_global->ap_ptree = (OBJECT **)ptreebase;

	for (ii = rs_hdr->rsh_ntree - 1; ii >= 0; ii--)
		fix_long(ptreebase + ii);
}


#if COLORICON_SUPPORT
/*
 * Fix up the G_ICON table
 */
static void fix_cicon(void)
{
	intptr_t *ctable;
	RSHDR *header;

	header = rs_hdr;
	if (header->rsh_vrsn & 0x0004)		/* if extended type */
	{
		ctable = (intptr_t *)((intptr_t)rs_hdr + (intptr_t) header->rsh_rssize);
		if (ctable[1] && (ctable[1] != -1))
			get_color_rsc((CICONBLK **)(ctable[1] + (intptr_t)rs_hdr));
	}
}
#endif


/*
 * Fix up the objects including color icons
 */
static void fix_objects(void)
{
	_WORD ii;
	_WORD obtype;
#if COLORICON_SUPPORT
	intptr_t *ctable;
	RSHDR *header;

	header = rs_hdr;

	if (header->rsh_vrsn & 0x0004)
	{
		ctable = (intptr_t *)((intptr_t)rs_hdr + (intptr_t) header->rsh_rssize);
		ctable = (intptr_t *)(ctable[1] + (intptr_t)rs_hdr);
	} else
	{
		ctable = NULL;
	}
#endif
	
	for (ii = rs_hdr->rsh_nobs - 1; ii >= 0; ii--)
	{
		OBJECT *obj = (OBJECT *)get_addr(R_OBJECT, ii);
		rs_obfix(obj, 0);
		obtype = obj->ob_type & 0xff;
#if COLORICON_SUPPORT
		if ((obtype == G_CICON) && ctable)
			obj->ob_spec.index = ctable[obj->ob_spec.index];
#endif

		if (obtype != G_BOX && obtype != G_IBOX && obtype != G_BOXCHAR
#if COLORICON_SUPPORT
			&& obtype != G_CICON
#endif
			)
			fix_long(&obj->ob_spec.index);
	}
}


static void fix_nptrs(_WORD cnt, _WORD type)
{
	while (--cnt >= 0)
		fix_long((intptr_t *)get_addr(type, cnt));
}


static _BOOL fix_ptr(_WORD type, _WORD index)
{
	return fix_long((intptr_t *)get_addr(type, index));
}


static void fix_tedinfo(void)
{
	_WORD ii;
	TEDINFO *ted;

	for (ii = rs_hdr->rsh_nted - 1; ii >= 0; ii--)
	{
		ted = (TEDINFO *)get_addr(R_TEDINFO, ii);
		if (fix_ptr(R_TEPTEXT, ii))
			ted->te_txtlen = strlen(ted->te_ptext) + 1;
		if (fix_ptr(R_TEPTMPLT, ii))
			ted->te_tmplen = strlen(ted->te_ptmplt) + 1;
		fix_ptr(R_TEPVALID, ii);
	}
}


/*
 *	Set global addresses that are used by the resource library sub-
 *	routines
 */
void rs_sglobe(AES_GLOBAL *pglobal)
{
	rs_global = pglobal;
	rs_hdr = (RSHDR *)pglobal->ap_rscmem;
}


/*
 * AES #111 - rsrc_free - Resource free
 *
 *	Free the memory associated with a particular resource load.
 */
_WORD rs_free(AES_GLOBAL *pglobal)
{
#if COLORICON_SUPPORT
	RSHDR *header;
	intptr_t *ctable;

	rs_sglobe(pglobal);					/* set global values */

	header = rs_hdr;

	if (header->rsh_vrsn & 0x0004)		/* extended format */
	{
		ctable = (intptr_t *)((intptr_t)rs_hdr + (uint32_t) header->rsh_rssize);
		if (ctable[1] && (ctable[1] != -1))
		{
			ctable = (intptr_t *)(ctable[1] + (intptr_t)rs_hdr);
			free_cicon((CICONBLK **)ctable);
		}
	}
	return dos_free(rs_hdr) == 0;

#else

	rs_global = pglobal;
	return dos_free(pglobal->ap_rscmem) == 0;

#endif
}


/*
 * AES #112 - rsrc_gaddr - Resource get address
 *
 *	Get a particular ADDRess out of a resource file that has been
 *	loaded into memory.
 */
_WORD rs_gaddr(AES_GLOBAL *pglobal, _UWORD rtype, _UWORD rindex, void **rsaddr)
{
	rs_sglobe(pglobal);

	*rsaddr = get_addr(rtype, rindex);
	return *rsaddr != (void *)-1L;
}


/*
 * AES #113 - rsrc_saddr - Resource store address
 *
 *	Set a particular ADDRess in a resource file that has been
 *	loaded into memory.
 */
_WORD rs_saddr(AES_GLOBAL *pglobal, _UWORD rtype, _UWORD rindex, void *rsaddr)
{
	void **psubstruct;

	rs_sglobe(pglobal);

	psubstruct = (void **)get_addr(rtype, rindex);
	if (psubstruct != (void **)-1L)
	{
		*psubstruct = rsaddr;
		return TRUE;
	}
	return FALSE;
}


/*
 * do all the fixups. rs_hdr must be initialized
 */
void do_rsfix(RSHDR *hdr, _LONG size)
{
	_WORD ibcnt;

	rs_global->ap_rscmem = hdr;
	rs_global->ap_rsclen = size;

	/*
	 * transfer RT_TRINDEX to global and turn all offsets from
	 * base of file into pointers
	 */
#if COLORICON_SUPPORT
	fix_cicon();						/* fix color icon */
#endif
	fix_trindex();
	fix_tedinfo();
	ibcnt = hdr->rsh_nib;
	fix_nptrs(ibcnt, R_IBPMASK);
	fix_nptrs(ibcnt, R_IBPDATA);
	fix_nptrs(ibcnt, R_IBPTEXT);
	fix_nptrs(hdr->rsh_nbb, R_BIPDATA);
	fix_nptrs(hdr->rsh_nstring, R_FRSTR);
	fix_nptrs(hdr->rsh_nimages, R_FRIMG);
}


/*
 *	Read resource file into memory and fix everything up except the
 *	x,y,w,h, parts which depend upon a GSX open workstation.  In the
 *	case of the GEM resource file this workstation will not have
 *	been loaded into memory yet.
 */
static _WORD rs_readit(AES_GLOBAL *pglobal, int fd)
{
	int32_t rslsize;
	RSHDR hdr_buff;
	
	/* read the header */
	if (dos_read(fd, sizeof(hdr_buff), &hdr_buff) != sizeof(hdr_buff))
		return FALSE;			/* error or short read */

	/* get size of resource & allocate memory */

#if COLORICON_SUPPORT
	if (hdr_buff.rsh_vrsn & 0x0004)	/* New format? */
	{
		/* seek to the 1st entry of the table */
		if (dos_lseek(fd, SEEK_SET, (int32_t)hdr_buff.rsh_rssize) != hdr_buff.rsh_rssize)
			return FALSE;
		/* read the size */
		if (dos_read(fd, sizeof(int32_t), &rslsize) != sizeof(int32_t))
			return FALSE;
	} else
#endif
	{
		rslsize = hdr_buff.rsh_rssize;
	}

	/* allocate memory */
	rs_hdr = (RSHDR *)dos_alloc_anyram((int32_t) rslsize);
	if (rs_hdr == NULL)
		return FALSE;
	
	/* read it all in */
	if (dos_lseek(fd, 0, 0x0L) < 0L)	/* mode 0: absolute offset */
		return FALSE;
	if (dos_read(fd, rslsize, rs_hdr) != rslsize)
		return FALSE;			/* error or short read */

	/* init global */
	rs_global = pglobal;

	do_rsfix(rs_hdr, rslsize);	/* do all the fixups */
	
	return TRUE;
}


/*
 *	Fix up objects separately so that we can read GEM resource before we
 *	do an open workstation, then once we know the character sizes we
 *	can fix up the objects accordingly.
 */
void rs_fixit(AES_GLOBAL *pglobal)
{
	rs_sglobe(pglobal);
	fix_objects();
}


/*
 * AES #110 - rsrc_load - Resource load
 *	RS_LOAD		mega resource load
 */
_WORD rs_load(AES_GLOBAL *pglobal, const char *rsfname)
{
	_LONG dosrc;
	_WORD ret;
	int fd;
	char rspath[MAXPATHLEN];

	/*
	 * use shel_find() to get resource location
	 */
	strcpy(rspath, rsfname);
	if (!sh_find(rspath))
		return FALSE;

	dosrc = dos_open(rspath, 0); /* mode 0: read only */
	if (dosrc < 0)
		return FALSE;
	fd = (int)dosrc;

	ret = rs_readit(pglobal, fd);
	if (ret)
		rs_fixit(pglobal);
	
	/* close file and return */
	dos_close(fd);
	return ret;
}
