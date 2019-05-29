#define _GNU_SOURCE

#include "config.h"
#include <stdint.h>
#include <gem.h>
#include <s_endian.h>
#include <debug.h>
#include <xrsrc.h>
#include <rsrcload.h>
#include <ro_mem.h>
#include <rsc.h>
#include <errno.h>
#include "fileio.h"
#include "time_.h"
#include "stat_.h"
#include "tos/nogem.h"
#include "hyp.h"
#ifdef __MINGW32__
#include <libgen.h>
#define basename(x) basename((char *)NO_CONST(x))
#endif

#undef max
#define max(a, b) ((a) > (b) ? (a) : (b))


#ifndef O_RDONLY
#  define O_RDONLY 0
#endif

#define SWAP_ALLOWED 1


static char empty[1];
static _WORD imdata[1];
static TEDINFO empty_ted = {
	empty,
	empty,
	empty,
	IBM,
	0,
	TE_LEFT,
	COLSPEC_MAKE(G_BLACK, G_BLACK, IP_HOLLOW, 0, G_WHITE),
	0,
	0,
	1,
	1
};
static BITBLK empty_bit = {
	imdata,
	2,
	1,
	0,
	0,
	G_BLACK
};
static ICONBLK empty_icon = {
	imdata,
	imdata,
	empty,
	ICOLSPEC_MAKE(G_BLACK, G_WHITE, '\0'),
	0, 0,
	0, 0, 16, 1,
	0, 0, 7, 1
};
static USERBLK empty_user = { 0, 0 };
static OBJECT empty_object = { NIL, NIL, NIL, G_BOX, OF_NONE, OS_NORMAL, {0x1181L}, 0,0, 43,5 };

#undef CHECK

#define OFFSET(p1, p2, type) \
	((type)((char *)(p1) - (char *)(p2)))

#define L(p) ((_LONG)(p))

#define MAKEPTR(p1, offset) \
	((char *)(p1) + (size_t)(offset))

#define CHECK_SIZE(offset, size) \
	(L(offset) < 0 || L(offset) >= L(size))

#define CHECK(p) \
	CHECK_SIZE(p, file->header.rsh_rssize)

/*
 * do not check for odd addresses
 * of little-endian resources;
 * they seem to be quite common
 */
#define IS_REALLY_ODD(p) (!file->rsc_little_endian && (L(p) & 1))

#define CHECK_ODD(p) \
	(CHECK(p) || IS_REALLY_ODD(p))


#define RSC_OK     0 /* all fine */
#define RSC_ERROR  1 /* is RSC file, but can't load it */
#define RSC_NORSC  2 /* not a RSC file at all */
#define RSC_ABORT  3 /* error loading, but don't issue message */
#define RSC_NOFILE 4 /* file not found */

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*
 * xrsrc_gaddr: like rsrc_gaddr(), taking an extra parameter for the resource
 */
_BOOL xrsrc_gaddr(RSCFILE *file, _WORD type, _WORD idx, void *gaddr)
{
	if (gaddr == NULL)
		return FALSE;
	*(void **)gaddr = NULL;
	if (file == NULL)
	{
		return FALSE;
	}
	switch (type)
	{
	case R_TREE:
		if (idx < 0 || (_ULONG)idx >= file->header.rsh_ntree)
		{
			return FALSE;
		}
		*((OBJECT **)gaddr) = file->rs_trindex[idx];
		break;
	case R_OBJECT:
		if (idx < 0 || (_ULONG)idx >= file->header.rsh_nobs)
		{
			return FALSE;
		}
		*((OBJECT **)gaddr) = &file->rs_object[idx];
		break;
	case R_TEDINFO:
		if (idx < 0 || (_ULONG)idx >= file->header.rsh_nted)
		{
			return FALSE;
		}
		*((TEDINFO **)gaddr) = &file->rs_tedinfo[idx];
		break;
	case R_ICONBLK:
		if (idx < 0 || (_ULONG)idx >= file->header.rsh_nib)
		{
			return FALSE;
		}
		*((ICONBLK **)gaddr) = &file->rs_iconblk[idx];
		break;
	case R_BITBLK:
		if (idx < 0 || (_ULONG)idx >= file->header.rsh_nbb)
		{
			return FALSE;
		}
		*((BITBLK **)gaddr) = &file->rs_bitblk[idx];
		break;
	case R_STRING:
		if (idx < 0 || (_ULONG)idx >= file->header.rsh_nstring)
		{
			return FALSE;
		}
		*((char **)gaddr) = file->rs_frstr[idx];
		break;
	case R_IMAGEDATA:
		if (idx < 0 || (_ULONG)idx >= file->header.rsh_nimages)
		{
			return FALSE;
		}
		*((BITBLK **)gaddr) = file->rs_frimg[idx];
		break;
	case R_OBSPEC:
		if (idx < 0 || (_ULONG)idx >= file->header.rsh_nobs)
		{
			return FALSE;
		}
		*((_LONG_PTR **)gaddr) = &file->rs_object[idx].ob_spec.index;
		break;
	case R_TEPTEXT:
		if (idx < 0 || (_ULONG)idx >= file->header.rsh_nted)
		{
			return FALSE;
		}
		*((char ***)gaddr) = &file->rs_tedinfo[idx].te_ptext;
		break;
	case R_TEPTMPLT:
		if (idx < 0 || (_ULONG)idx >= file->header.rsh_nted)
		{
			return FALSE;
		}
		*((char ***)gaddr) = &file->rs_tedinfo[idx].te_ptmplt;
		break;
	case R_TEPVALID:
		if (idx < 0 || (_ULONG)idx >= file->header.rsh_nted)
		{
			return FALSE;
		}
		*((char ***)gaddr) = &file->rs_tedinfo[idx].te_pvalid;
		break;
	case R_IBPMASK:
		if (idx < 0 || (_ULONG)idx >= file->header.rsh_nib)
		{
			return FALSE;
		}
		*((char ***)gaddr) = (char **)&file->rs_iconblk[idx].ib_pmask;
		break;
	case R_IBPDATA:
		if (idx < 0 || (_ULONG)idx >= file->header.rsh_nib)
		{
			return FALSE;
		}
		*((char ***)gaddr) = (char **)&file->rs_iconblk[idx].ib_pdata;
		break;
	case R_IBPTEXT:
		if (idx < 0 || (_ULONG)idx >= file->header.rsh_nib)
		{
			return FALSE;
		}
		*((char ***)gaddr) = &file->rs_iconblk[idx].ib_ptext;
		break;
	case R_BIPDATA:
		if (idx < 0 || (_ULONG)idx >= file->header.rsh_nbb)
		{
			return FALSE;
		}
		*((char ***)gaddr) = (char **)&file->rs_bitblk[idx].bi_pdata;
		break;
	case R_FRSTR:
		if (idx < 0 || (_ULONG)idx >= file->header.rsh_nstring)
		{
			return FALSE;
		}
		*((char ***)gaddr) = &file->rs_frstr[idx];
		break;
	case R_FRIMG:
		if (idx < 0 || (_ULONG)idx >= file->header.rsh_nimages)
		{
			return FALSE;
		}
		*((BITBLK ***)gaddr) = &file->rs_frimg[idx];
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL test_header(RS_HEADER *header, _LONG filesize)
{
	_UWORD vrsn;

	vrsn = header->rsh_vrsn & RSC_VERSION_MASK;
	if (vrsn != 0 && vrsn != 1)
		return FALSE;
	if (header->rsh_rssize > filesize)
		return FALSE;
#if 0
	/* some broken construction kits fail to put correct values here */
	if (header->rsh_rssize < (filesize - sizeof(*header)))
		return FALSE;
#endif
	if (header->rsh_object > filesize)
		return FALSE;
	if (header->rsh_tedinfo > filesize)
		return FALSE;
	if (header->rsh_iconblk > filesize)
		return FALSE;
	if (header->rsh_bitblk > filesize)
		return FALSE;
	if (header->rsh_frstr > filesize)
		return FALSE;
	if (header->rsh_string > filesize)
		return FALSE;
	if (header->rsh_imdata > filesize)
		return FALSE;
	if (header->rsh_frimg > filesize)
		return FALSE;
	if (header->rsh_trindex > filesize)
		return FALSE;
	if (header->rsh_nobs > 2339) /* (65536 - sizeof(RS_HEADER)) / (RSC_SIZEOF_OBJECT + sizeof(OBJECT *)) */
		return FALSE;
	if (header->rsh_ntree > 2339)
		return FALSE;
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL test_xrsc_header(XRS_HEADER *xrsc_header, _ULONG filesize)
{
	if ((xrsc_header->rsh_vrsn & RSC_VERSION_MASK) != 3)
		return FALSE;
	if (xrsc_header->rsh_rssize > filesize)
		return FALSE;
	if (xrsc_header->rsh_object > filesize)
		return FALSE;
	if (xrsc_header->rsh_tedinfo > filesize)
		return FALSE;
	if (xrsc_header->rsh_iconblk > filesize)
		return FALSE;
	if (xrsc_header->rsh_bitblk > filesize)
		return FALSE;
	if (xrsc_header->rsh_frstr > filesize)
		return FALSE;
	if (xrsc_header->rsh_string > filesize)
		return FALSE;
	if (xrsc_header->rsh_imdata > filesize)
		return FALSE;
	if (xrsc_header->rsh_frimg > filesize)
		return FALSE;
	if (xrsc_header->rsh_trindex > filesize)
		return FALSE;
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

void xrsc_hdr2xrsc(XRS_HEADER *xrsc_header, RS_HEADER *header, size_t diff_size)
{
	xrsc_header->rsh_vrsn	 = 3 | (header->rsh_vrsn & ~RSC_VERSION_MASK);
	xrsc_header->rsh_extvrsn = XRSC_VRSN_ORCS;
	xrsc_header->rsh_object  = header->rsh_object + diff_size;
	xrsc_header->rsh_tedinfo = header->rsh_tedinfo + diff_size;
	xrsc_header->rsh_iconblk = header->rsh_iconblk + diff_size;
	xrsc_header->rsh_bitblk  = header->rsh_bitblk + diff_size;
	xrsc_header->rsh_frstr	 = header->rsh_frstr + diff_size;
	xrsc_header->rsh_string  = header->rsh_string + diff_size;
	xrsc_header->rsh_imdata  = header->rsh_imdata + diff_size;
	xrsc_header->rsh_frimg	 = header->rsh_frimg + diff_size;
	xrsc_header->rsh_trindex = header->rsh_trindex + diff_size;
	xrsc_header->rsh_nobs	 = header->rsh_nobs;
	xrsc_header->rsh_ntree	 = header->rsh_ntree;
	xrsc_header->rsh_nted	 = header->rsh_nted;
	xrsc_header->rsh_nib	 = header->rsh_nib;
	xrsc_header->rsh_nbb	 = header->rsh_nbb;
	xrsc_header->rsh_nstring = header->rsh_nstring;
	xrsc_header->rsh_nimages = header->rsh_nimages;
	xrsc_header->rsh_rssize  = header->rsh_rssize + diff_size;
}

/*** ---------------------------------------------------------------------- ***/

void xrsc_xrsc2hdr(RS_HEADER *header, XRS_HEADER *xrsc_header, size_t diff_size)
{
	header->rsh_vrsn	= 0 | (xrsc_header->rsh_vrsn & ~RSC_VERSION_MASK);
	header->rsh_object	= (_UWORD)(xrsc_header->rsh_object - diff_size);
	header->rsh_tedinfo = (_UWORD)(xrsc_header->rsh_tedinfo - diff_size);
	header->rsh_iconblk = (_UWORD)(xrsc_header->rsh_iconblk - diff_size);
	header->rsh_bitblk	= (_UWORD)(xrsc_header->rsh_bitblk - diff_size);
	header->rsh_frstr	= (_UWORD)(xrsc_header->rsh_frstr - diff_size);
	header->rsh_string	= (_UWORD)(xrsc_header->rsh_string - diff_size);
	header->rsh_imdata	= (_UWORD)(xrsc_header->rsh_imdata - diff_size);
	header->rsh_frimg	= (_UWORD)(xrsc_header->rsh_frimg - diff_size);
	header->rsh_trindex = (_UWORD)(xrsc_header->rsh_trindex - diff_size);
	header->rsh_nobs	= (_UWORD)xrsc_header->rsh_nobs;
	header->rsh_ntree	= (_UWORD)xrsc_header->rsh_ntree;
	header->rsh_nted	= (_UWORD)xrsc_header->rsh_nted;
	header->rsh_nib     = (_UWORD)xrsc_header->rsh_nib;
	header->rsh_nbb     = (_UWORD)xrsc_header->rsh_nbb;
	header->rsh_nstring = (_UWORD)xrsc_header->rsh_nstring;
	header->rsh_nimages = (_UWORD)xrsc_header->rsh_nimages;
	header->rsh_rssize	= (_UWORD)(xrsc_header->rsh_rssize - diff_size);
}

/*** ---------------------------------------------------------------------- ***/

#if SWAP_ALLOWED

#define bswap_ptr(p) bswap_32((uint32_t)(p))

static void flip_header(RS_HEADER *header)
{
	header->rsh_vrsn = bswap_16(header->rsh_vrsn);
	header->rsh_object = bswap_16(header->rsh_object);
	header->rsh_tedinfo = bswap_16(header->rsh_tedinfo);
	header->rsh_iconblk = bswap_16(header->rsh_iconblk);
	header->rsh_bitblk = bswap_16(header->rsh_bitblk);
	header->rsh_frstr = bswap_16(header->rsh_frstr);
	header->rsh_string = bswap_16(header->rsh_string);
	header->rsh_imdata = bswap_16(header->rsh_imdata);
	header->rsh_frimg = bswap_16(header->rsh_frimg);
	header->rsh_trindex = bswap_16(header->rsh_trindex);
	header->rsh_nobs = bswap_16(header->rsh_nobs);
	header->rsh_ntree = bswap_16(header->rsh_ntree);
	header->rsh_nted = bswap_16(header->rsh_nted);
	header->rsh_nib = bswap_16(header->rsh_nib);
	header->rsh_nbb = bswap_16(header->rsh_nbb);
	header->rsh_nstring = bswap_16(header->rsh_nstring);
	header->rsh_nimages = bswap_16(header->rsh_nimages);
	header->rsh_rssize = bswap_16(header->rsh_rssize);
}

static void flip_xrsrc_header(XRS_HEADER *header)
{
	header->rsh_vrsn = bswap_16(header->rsh_vrsn);
	header->rsh_extvrsn = bswap_16(header->rsh_extvrsn);
	header->rsh_object = bswap_32(header->rsh_object);
	header->rsh_tedinfo = bswap_32(header->rsh_tedinfo);
	header->rsh_iconblk = bswap_32(header->rsh_iconblk);
	header->rsh_bitblk = bswap_32(header->rsh_bitblk);
	header->rsh_frstr = bswap_32(header->rsh_frstr);
	header->rsh_string = bswap_32(header->rsh_string);
	header->rsh_imdata = bswap_32(header->rsh_imdata);
	header->rsh_frimg = bswap_32(header->rsh_frimg);
	header->rsh_trindex = bswap_32(header->rsh_trindex);
	header->rsh_nobs = bswap_32(header->rsh_nobs);
	header->rsh_ntree = bswap_32(header->rsh_ntree);
	header->rsh_nted = bswap_32(header->rsh_nted);
	header->rsh_nib = bswap_32(header->rsh_nib);
	header->rsh_nbb = bswap_32(header->rsh_nbb);
	header->rsh_nstring = bswap_32(header->rsh_nstring);
	header->rsh_nimages = bswap_32(header->rsh_nimages);
	header->rsh_rssize = bswap_32(header->rsh_rssize);
}

/*** ---------------------------------------------------------------------- ***/

/*
 * whether to swap the image data of BITBLKs/ICONs/CICONs to native format.
 * image data is normally treated as array of unsigned shorts, if
 * FLIP_DATA is unset it is left in big-endian order.
 * If unset, compiled-in resources must be stored as bytes.
 * If you change this, you also have to change vdi_put_image
 * and vro_cpy_to_screen in the VDI.
 */
#define FLIP_DATA 0

#if FLIP_DATA
static void flip_image(size_t words, _WORD *data)
{
	size_t j;

	for (j = 0; j < words; j++)
		data[j] = bswap_16(data[j]);
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL xrsc_flip_data(RSCFILE *file, _ULONG filesize)
{
	_UWORD i;
	size_t words;
	_ULONG offset;
	
	{
		/***** flip data and mask of iconblocks *****/
		ICONBLK *p;

		p = file->rs_iconblk;
		for (i = 0; i < file->header.rsh_nib; i++, p++)
		{
			words = iconblk_masksize(p) >> 1;

			if (filesize != 0)
			{
				offset = OFFSET(p->ib_pmask, file->data, _ULONG);
				if ((offset + words + words) > filesize)
				{
					KINFO(("flip_data: iconblk[%u]: ip_pmask $%lx out of range\n", i, offset));
					*p = empty_icon;
					continue;
				}
				offset = OFFSET(p->ib_pdata, file->data, _ULONG);
				if ((offset + words + words) > filesize)
				{
					KINFO(("flip_data: iconblk[%u]: ip_pdata $%lx out of range\n", i, offset));
					*p = empty_icon;
					continue;
				}
			}
			flip_image(words, p->ib_pmask);
			flip_image(words, p->ib_pdata);
		}
	}

	{
		/***** flip data of bitblocks *****/
		BITBLK *pbitblk;

		pbitblk = file->rs_bitblk;
		for (i = 0; i < file->header.rsh_nbb; i++, pbitblk++)
		{
			words = bitblk_datasize(pbitblk) >> 1;
			if (filesize != 0)
			{
				offset = OFFSET(pbitblk->bi_pdata, file->data, _ULONG);
				if ((offset + words + words) > filesize)
				{
					KINFO(("flip_data: bitblk[%u]: bi_pdata $%lx out of range\n", i, offset));
					*pbitblk = empty_bit;
					continue;
				}
			}
			flip_image(words, pbitblk->bi_pdata);
		}
	}

	return TRUE;
}
#endif

/*** ---------------------------------------------------------------------- ***/

static INLINE uint16_t get_word(const char *p)
{
	return *((const uint16_t *)(p));
}

/*** ---------------------------------------------------------------------- ***/

static INLINE uint32_t get_long(const char *p)
{
	return *((const uint32_t *)(p));
}

/*** ---------------------------------------------------------------------- ***/

static INLINE void swap_word(char *p)
{
	*((uint16_t *)(p)) = bswap_16(*((uint16_t *)(p)));
}

/*** ---------------------------------------------------------------------- ***/

static INLINE void swap_long(char *p)
{
	*((uint32_t *)(p)) = bswap_32(*((uint32_t *)(p)));
}

/*** ---------------------------------------------------------------------- ***/

void xrsc_get_header(XRS_HEADER *xrsc_header, const char *buf)
{
	xrsc_header->rsh_vrsn = get_word(buf + 0x00);
	xrsc_header->rsh_extvrsn = get_word(buf + 0x02);
	xrsc_header->rsh_object = get_long(buf + 0x04);
	xrsc_header->rsh_tedinfo = get_long(buf + 0x08);
	xrsc_header->rsh_iconblk = get_long(buf + 0x0c);
	xrsc_header->rsh_bitblk = get_long(buf + 0x10);
	xrsc_header->rsh_frstr = get_long(buf + 0x14);
	xrsc_header->rsh_string = get_long(buf + 0x18);
	xrsc_header->rsh_imdata = get_long(buf + 0x1c);
	xrsc_header->rsh_frimg = get_long(buf + 0x20);
	xrsc_header->rsh_trindex = get_long(buf + 0x24);
	xrsc_header->rsh_nobs = get_long(buf + 0x28);
	xrsc_header->rsh_ntree = get_long(buf + 0x2c);
	xrsc_header->rsh_nted = get_long(buf + 0x30);
	xrsc_header->rsh_nib = get_long(buf + 0x34);
	xrsc_header->rsh_nbb = get_long(buf + 0x38);
	xrsc_header->rsh_nstring = get_long(buf + 0x3c);
	xrsc_header->rsh_nimages = get_long(buf + 0x40);
	xrsc_header->rsh_rssize = get_long(buf + 0x44);
}

/*** ---------------------------------------------------------------------- ***/

static void rsc_get_header(RS_HEADER *header, const char *buf)
{
	header->rsh_vrsn = get_word(buf + 0x00);
	header->rsh_object = get_word(buf + 0x02);
	header->rsh_tedinfo = get_word(buf + 0x04);
	header->rsh_iconblk = get_word(buf + 0x06);
	header->rsh_bitblk = get_word(buf + 0x08);
	header->rsh_frstr = get_word(buf + 0x0a);
	header->rsh_string = get_word(buf + 0x0c);
	header->rsh_imdata = get_word(buf + 0x0e);
	header->rsh_frimg = get_word(buf + 0x10);
	header->rsh_trindex = get_word(buf + 0x12);
	header->rsh_nobs = get_word(buf + 0x14);
	header->rsh_ntree = get_word(buf + 0x16);
	header->rsh_nted = get_word(buf + 0x18);
	header->rsh_nib = get_word(buf + 0x1a);
	header->rsh_nbb = get_word(buf + 0x1c);
	header->rsh_nstring = get_word(buf + 0x1e);
	header->rsh_nimages = get_word(buf + 0x20);
	header->rsh_rssize = get_word(buf + 0x22);
}

/*** ---------------------------------------------------------------------- ***/

static void flip_iconblk(char *blk)
{
	swap_long(blk + 0);
	swap_long(blk + 4);
	swap_long(blk + 8);
	swap_word(blk + 12);
	swap_word(blk + 14);
	swap_word(blk + 16);
	swap_word(blk + 18);
	swap_word(blk + 20);
	swap_word(blk + 22);
	swap_word(blk + 24);
	swap_word(blk + 26);
	swap_word(blk + 28);
	swap_word(blk + 30);
	swap_word(blk + 32);
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL xrsc_flip_rsc(RSCFILE *file, _ULONG filesize)
{
	_UWORD i;
	char *rsc_buffer = file->data;
	XRS_HEADER *header = &file->header;
	
#if FLIP_DATA
	/*
	 * swap it now when saving a file,
	 * before their pointers are also swapped
	 */
	if (filesize == 0)
		xrsc_flip_data(file, filesize);
#endif

	{
		/***** fix objects *****/
		char *obj;

		if (filesize != 0)
		{
			if (header->rsh_object > header->rsh_rssize ||
				(header->rsh_nobs > 0 && header->rsh_object + header->rsh_nobs * RSC_SIZEOF_OBJECT > header->rsh_rssize))
			{
				KINFO(("swap: rsh_object %lx[%lx] out of range\n", header->rsh_object, header->rsh_nobs));
				return FALSE;
			}
		}
		obj = MAKEPTR(rsc_buffer, header->rsh_object);
		for (i = 0; i < header->rsh_nobs; i++, obj += RSC_SIZEOF_OBJECT)
		{
			swap_word(obj + 0);
			swap_word(obj + 2);
			swap_word(obj + 4);
			swap_word(obj + 6);
			swap_word(obj + 8);
			swap_word(obj + 10);
			swap_long(obj + 12);
			swap_word(obj + 16);
			swap_word(obj + 18);
			swap_word(obj + 20);
			swap_word(obj + 22);
			if ((get_word(obj + 6) & OBTYPEMASK) == G_USERDEF)
			{
				/* swap userblk.ub_parm */
				_ULONG offset = get_long(obj + 12);
				if (offset >= RSC_SIZEOF_RS_HEADER && offset < filesize)
				{
					char *user = MAKEPTR(rsc_buffer, offset);
					swap_long(user + 4);
				}
			}
		}
	}

	{
		/***** flip tedinfos *****/
		char *ted;

		if (filesize != 0)
		{
			if (header->rsh_tedinfo > header->rsh_rssize ||
				(header->rsh_nted != 0 && header->rsh_tedinfo + header->rsh_nted * RSC_SIZEOF_TEDINFO > header->rsh_rssize))
				return FALSE;
		}
		ted = MAKEPTR(rsc_buffer, header->rsh_tedinfo);
		for (i = 0; i < header->rsh_nted; i++, ted += RSC_SIZEOF_TEDINFO)
		{
			swap_long(ted + 0);
			swap_long(ted + 4);
			swap_long(ted + 8);
			swap_word(ted + 12);
			swap_word(ted + 14);
			swap_word(ted + 16);
			swap_word(ted + 18);
			swap_word(ted + 20);
			swap_word(ted + 22);
			swap_word(ted + 24);
			swap_word(ted + 26);
		}
	}

	{
		/***** flip iconblocks *****/
		char *p;

		if (filesize != 0)
		{
			if (header->rsh_iconblk > header->rsh_rssize ||
				(header->rsh_nib != 0 && header->rsh_iconblk + header->rsh_nib * RSC_SIZEOF_ICONBLK > header->rsh_rssize))
				return FALSE;
		}
		p = MAKEPTR(rsc_buffer, header->rsh_iconblk);
		for (i = 0; i < header->rsh_nib; i++, p += RSC_SIZEOF_ICONBLK)
		{
			flip_iconblk(p);
		}
	}

	{
		/***** flip bitblocks *****/
		char *bit;

		if (filesize != 0)
		{
			if (header->rsh_bitblk > header->rsh_rssize ||
				(header->rsh_nbb != 0 && header->rsh_bitblk + header->rsh_nbb * RSC_SIZEOF_BITBLK > header->rsh_rssize))
				return FALSE;
		}
		bit = MAKEPTR(rsc_buffer, header->rsh_bitblk);
		for (i = 0; i < header->rsh_nbb; i++, bit += RSC_SIZEOF_BITBLK)
		{
			swap_long(bit + 0);
			swap_word(bit + 4);
			swap_word(bit + 6);
			swap_word(bit + 8);
			swap_word(bit + 10);
			swap_word(bit + 12);
		}
	}

	{
		/***** flip free strings *****/
		char *pfrstr;

		if (filesize != 0)
		{
			if (header->rsh_frstr > header->rsh_rssize ||
				(header->rsh_nstring != 0 && header->rsh_frstr + header->rsh_nstring * RSC_SIZEOF_PTR > header->rsh_rssize))
				return FALSE;
		}
		pfrstr = MAKEPTR(rsc_buffer, header->rsh_frstr);
		for (i = 0; i < header->rsh_nstring; i++, pfrstr += RSC_SIZEOF_PTR)
		{
			swap_long(pfrstr);
		}
	}

	{
		/***** flip free images *****/
		char *pfrimg;

		if (filesize != 0)
		{
			if (header->rsh_frimg > header->rsh_rssize ||
				(header->rsh_nimages != 0 && header->rsh_frimg + header->rsh_nimages * RSC_SIZEOF_PTR > header->rsh_rssize))
				return FALSE;
		}
		pfrimg = MAKEPTR(rsc_buffer, header->rsh_frimg);
		for (i = 0; i < header->rsh_nimages; i++, pfrimg += RSC_SIZEOF_PTR)
		{
			swap_long(pfrimg);
		}
	}

	{
		/***** flip trees *****/
		char *ptrindex;

		if (filesize != 0)
		{
			if (header->rsh_trindex > header->rsh_rssize ||
				(header->rsh_ntree != 0 && header->rsh_trindex + header->rsh_ntree * RSC_SIZEOF_PTR > header->rsh_rssize))
				return FALSE;
		}
		ptrindex = rsc_buffer + header->rsh_trindex;
		for (i = 0; i < header->rsh_ntree; i++, ptrindex += RSC_SIZEOF_PTR)
		{
			swap_long(ptrindex);
		}
	}

	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static _BOOL intel_2_m68k(RSCFILE *file, _LONG filesize)
{
#if 0
	/* fix font from IBM to ATARI */
	char *pstring;
	char *pend;
	_ULONG end;
	_ULONG start;

	start = xrsc_header->rsh_string;
	end = xrsc_header->rsh_rssize;
	if (xrsc_header->rsh_object  < end && xrsc_header->rsh_object  > start)
		end = xrsc_header->rsh_object;
	if (xrsc_header->rsh_tedinfo < end && xrsc_header->rsh_tedinfo > start)
		end = xrsc_header->rsh_tedinfo;
	if (xrsc_header->rsh_iconblk < end && xrsc_header->rsh_iconblk > start)
		end = xrsc_header->rsh_iconblk;
	if (xrsc_header->rsh_bitblk  < end && xrsc_header->rsh_bitblk  > start)
		end = xrsc_header->rsh_bitblk;
	if (xrsc_header->rsh_frstr	 < end && xrsc_header->rsh_frstr   > start)
		end = xrsc_header->rsh_frstr;
	if (xrsc_header->rsh_imdata  < end && xrsc_header->rsh_imdata  > start)
		end = xrsc_header->rsh_imdata;
	if (xrsc_header->rsh_frimg	 < end && xrsc_header->rsh_frimg   > start)
		end = xrsc_header->rsh_frimg;
	if (xrsc_header->rsh_trindex < end && xrsc_header->rsh_trindex > start)
		end = xrsc_header->rsh_trindex;
	pstring = (char *)(rsc_buffer + (size_t)start);
	pend = (char *)(rsc_buffer + (size_t)end);
	while (pstring < pend)
	{
		switch (*pstring)
		{
		case 0x15:
			*pstring = 0xdd; /* change paragraph from IBM to ATARI ST font */
			break;
		case 0xe1:
			*pstring = 0x9e; /* change sz from IBM to ATARI ST font */
			break;
		}
		pstring++;
	} /* while */
#endif
	return xrsc_flip_rsc(file, filesize);
}
#endif /* SWAP_ALLOWED */

/*** ---------------------------------------------------------------------- ***/

static void rsc_obfix(OBJECT *tree, _ULONG count, _WORD wchar, _WORD hchar)
{
	/* Koordinaten fuer alle Objekte umrechnen */
	while (count)
	{
		tree->ob_x = wchar * (tree->ob_x & 0xff) + (tree->ob_x >> 8);
		tree->ob_y = hchar * (tree->ob_y & 0xff) + (tree->ob_y >> 8);
		tree->ob_width = wchar * (tree->ob_width & 0xff) + (tree->ob_width >> 8);
		tree->ob_height = hchar * (tree->ob_height & 0xff) + (tree->ob_height >> 8);
		tree++;
		count--;
	}
}

/*** ---------------------------------------------------------------------- ***/

/*
 * check wether we need to convert the resource
 * when loading into memory
 */
static _BOOL rsrc_load_works(void)
{
/*
 * we assume that it works for PureC;
 * avoid the warning from not being able
 * to optimize out the constant checks
 */
#if !defined(__PUREC__)
#define check_size(struct, size) \
	if (sizeof(struct) != size) \
	{ \
		return FALSE; \
	}
	check_size(TEDINFO, RSC_SIZEOF_TEDINFO);
	check_size(ICONBLK, RSC_SIZEOF_ICONBLK);
	check_size(BITBLK, RSC_SIZEOF_BITBLK);
	check_size(CICON, RSC_SIZEOF_CICON);
	check_size(CICONBLK, RSC_SIZEOF_CICONBLK);
	check_size(USERBLK, RSC_SIZEOF_USERBLK);
	check_size(OBJECT, RSC_SIZEOF_OBJECT);
	check_size(OBJECT *, RSC_SIZEOF_PTR);
#endif
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static _WORD _CDECL draw_userdef(PARMBLK *pb)
{
	UNUSED(pb);
	return 0;
}

/*** ---------------------------------------------------------------------- ***/

static void report_problem(XRS_HEADER *xrsc_header, const char *what, _LONG offset, const char **whats_wrong)
{
	if (whats_wrong)
		*whats_wrong = what;
	if (offset == 0)
	{
		KINFO(("%s: offset is NULL\n", what));
	} else if (offset < (_LONG)RSC_SIZEOF_RS_HEADER)
	{
		KINFO(("%s: offset $%lx points to RSC file header\n", what, offset));
	} else if (offset >= L(xrsc_header->rsh_rssize))
	{
		KINFO(("%s: offset $%lx is beyond EOF\n", what, offset));
	} else if (offset & 1)
	{
		KINFO(("%s: offset $%lx is odd\n", what, offset));
	} else
	{
		KINFO(("%s: offset $%lx\n", what, offset));
	}
}

/*** ---------------------------------------------------------------------- ***/

_BOOL W_Cicon_Setpalette(_WORD *palette)
{
	UNUSED(palette);
	return TRUE;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * xrsrc_load: like rsrc_load()
 */
RSCFILE *xrsrc_load(const char *filename, _WORD wchar, _WORD hchar, _UWORD flags)
{
	_ULONG UObj;
	CICONBLK *cicon_p;
	CICONBLK *cicon_dst;
	char headerbuf[max(RSC_SIZEOF_XRS_HEADER, RSC_SIZEOF_RS_HEADER)];
	RS_HEADER rs_header;
	XRS_HEADER xrsc_header;
	char *buf = NULL;
	_BOOL swap_flag = FALSE;
	_BOOL xrsc_flag = FALSE;
	_WORD error = RSC_OK;
	FILE *fp;
	_ULONG filesize;
	RSCFILE *file;
	_ULONG n_userblks;
	_ULONG offset;
	_ULONG idx;
	const char *whats_wrong = NULL;
	RSC_RSM_CRC crc_for_string = RSC_CRC_NONE;
	
	cicon_p = NULL;
	cicon_dst = NULL;
	fp = hyp_utf8_fopen(filename, "rb");

	if (fp == NULL)
	{
		err_fopen(filename);
		return NULL;
	}
#ifdef HAVE_FSTAT
	{
		struct stat st;
		
		if (fstat(fileno(fp), &st) < 0)
			filesize = 0, error = RSC_ERROR;
		else
			filesize = st.st_size;
	}
#else
	if (fseek(fp, 0l, SEEK_END) != 0)
		filesize = 0, error = RSC_ERROR;
	else
		filesize = ftell(fp);
	if (fseek(fp, 0l, SEEK_SET) != 0)
		error = RSC_ERROR;
#endif
	
	if (error == RSC_OK && filesize < RSC_SIZEOF_RS_HEADER)
		error = RSC_NORSC;
	
	if (error == RSC_OK && fread(headerbuf, 1, sizeof(headerbuf), fp) != sizeof(headerbuf))
	{
		KINFO(("reading header failed\n"));
		error = RSC_ERROR;
	}
	
	if (error == RSC_OK)
	{
		rsc_get_header(&rs_header, headerbuf);
		xrsc_get_header(&xrsc_header, headerbuf);
		if (test_header(&rs_header, filesize) == FALSE)
		{
			if (test_xrsc_header(&xrsc_header, filesize) == FALSE)
			{
#if SWAP_ALLOWED
				flip_header(&rs_header);
				flip_xrsrc_header(&xrsc_header);
				if (test_header(&rs_header, filesize) != FALSE)
				{
					if (filesize > 65534l && !(rs_header.rsh_vrsn & RSC_EXT_FLAG))
					{
						KINFO(("normal header && filesize >64k but no extension flag set\n"));
						error = RSC_NORSC;
					} else
					{
						swap_flag = TRUE;
						xrsc_hdr2xrsc(&xrsc_header, &rs_header, 0);
					}
				} else if (test_xrsc_header(&xrsc_header, filesize) != FALSE)
				{
					swap_flag = TRUE;
					xrsc_flag = TRUE;
				} else
#endif
				{
					KINFO(("header check failed\n"));
					error = RSC_NORSC;
				}
			} else
			{
				xrsc_flag = TRUE;
			}
		} else
		{
			if (filesize > 65534l && !(rs_header.rsh_vrsn & RSC_EXT_FLAG))
			{
				KINFO(("normal header && filesize >64k but no extension flag set\n"));
				error = RSC_NORSC;
			} else
			{
				xrsc_hdr2xrsc(&xrsc_header, &rs_header, 0);
			}
		}
	}
	if (error == RSC_OK)
	{
		if (test_xrsc_header(&xrsc_header, filesize) == FALSE ||
			fseek(fp, 0l, SEEK_SET) != 0)
		{
			KINFO(("xrsc_header check failed\n"));
			error = RSC_NORSC;
		}
	}
	if (error == RSC_OK && filesize >= 65534l && sizeof(size_t) <= 2)
	{
		warn_toobig();
		error = RSC_ABORT;
	}
	if (error == RSC_OK)
	{
		if ((buf = g_new(char, sizeof(RSCFILE) + (size_t)filesize)) == NULL)
		{
			fclose(fp);
			return NULL;
		}
	}

	if (error == RSC_OK && (_ULONG)fread(buf + sizeof(RSCFILE), 1, (size_t)filesize, fp) != filesize)
	{
		error = RSC_ERROR;
	}

	fclose(fp);
	file = (RSCFILE *)buf;
	if (error == RSC_OK)
	{
		memset(file, 0, sizeof(RSCFILE));
		rsc_init_file(file);
		strcpy(file->rsc_rsxfilename, filename);
		strcpy(file->rsc_rsxname, basename(filename));
		buf += sizeof(RSCFILE);
		if (error == RSC_OK)
			file->rsc_rsm_crc = rsc_rsm_calc_crc(buf, filesize);
		file->data = buf;
		file->header = xrsc_header;
		file->rsc_swap_flag = swap_flag;
		file->rsc_xrsc_flag = xrsc_flag;
		file->rsc_little_endian = swap_flag ^ (HOST_BYTE_ORDER != BYTE_ORDER_BIG_ENDIAN);
	}
	
#if SWAP_ALLOWED
	if (error == RSC_OK && swap_flag)
	{
		if (intel_2_m68k(file, filesize) == FALSE)
		{
			KINFO(("flipping resource failed\n"));
			warn_damaged(filename, "Data");
			error = RSC_ABORT;
		}
	}
#endif
	
	switch (error)
	{
	case RSC_OK:
		break;
	case RSC_NORSC:
		err_nota_rsc(filename);
		errno = EINVAL;
		break;
	case RSC_ERROR:
		err_fread(filename);
		errno = EIO;
		break;
	case RSC_ABORT:
		errno = EFAULT;
		break;
	}
	if (error != RSC_OK)
	{
		xrsrc_free(file);
		return NULL;
	}
			
	/*
	 * Some resource editors fail to mark an extended RSC in the header,
	 * do a quick check first wether any color icons are present
	 */
	file->rsc_nciconblks = 0;
	n_userblks = 0;
	if (xrsc_header.rsh_nobs > 0 &&
		xrsc_header.rsh_object >= RSC_SIZEOF_RS_HEADER)
	{
		char *pobject;
		_ULONG i;
		_UWORD type;
		
		pobject = MAKEPTR(buf, xrsc_header.rsh_object);
		for (i = 0; i < xrsc_header.rsh_nobs; i++, pobject += RSC_SIZEOF_OBJECT)
		{
			type = get_word(pobject + 6) & OBTYPEMASK;
			if (type == G_CICON)
			{
				xrsc_header.rsh_vrsn |= RSC_EXT_FLAG;
				file->rsc_nciconblks++;
			}
			if (type == G_USERDEF)
				n_userblks++;
		}
	}

	if (xrsc_header.rsh_vrsn & RSC_EXT_FLAG)
	{
		int32_t *p;
		_BOOL ok = TRUE;

		/*
		 * an extended resource has a list of extension ptrs
		 * located at the standard rsh_rssize offset.
		 * It has at least:
		 * - the real filesize
		 * - an offset to the CICON ptr list
		 * - an endmarker (zero)
		 */
		p = (int32_t *)(MAKEPTR(buf, xrsc_header.rsh_rssize));
		if (swap_flag)
		{
			p[RSC_EXT_FILESIZE] = bswap_32(p[RSC_EXT_FILESIZE]);
			p[RSC_EXT_CICONBLK] = bswap_32(p[RSC_EXT_CICONBLK]);
		}
		if ((uint32_t)p[RSC_EXT_FILESIZE] != filesize)
		{
			ok = FALSE;
			KINFO(("extension hdr filesize %ld does not match real filesize %ld\n", (long)p[RSC_EXT_FILESIZE], filesize));
		} else if (p[RSC_EXT_CICONBLK] < 0 || (uint32_t)p[RSC_EXT_CICONBLK] >= filesize)
		{
			ok = FALSE;
			report_problem(&file->header, "extension hdr ciconblk offset out of range\n", (long)p[RSC_EXT_CICONBLK], &whats_wrong);
		} else
		{
			int32_t *cp = p + RSC_EXT_CICONBLK + 1;
			
			/*
			 * check wether there is also a palette
			 */
			while (ok != FALSE)
			{
				if ((uint32_t)((char *)cp - buf) >= filesize)
				{
					ok = FALSE;
				} else if (*cp == 0)
				{
					break;
				} else if (*cp != -1)
				{
					if (swap_flag)
						*cp = bswap_32(*cp);
					/* ok = FALSE; */
					if (cp == (p + RSC_EXT_PALETTE))
					{
						_WORD *palette = (_WORD *)(buf + (size_t)(*cp));
						if (swap_flag)
						{
							_WORD i;

							for (i = 0; i < 1024; i++)
							{
								palette[i] = bswap_16(palette[i]);
							}
						}
						W_Cicon_Setpalette(palette);
					}
					break;
				}
				cp++;
			}
		}

		/*
		 * the CICONBLK slot of the extensions
		 * points to an empty list of slots to store
		 * the starting offset of the color icons.
		 * It is terminated by -1
		 */
		if (ok != FALSE)
		{
			p = (int32_t *)(buf + (size_t)p[RSC_EXT_CICONBLK]);
			while (ok != FALSE)
			{
				if ((uint32_t)((char *)p - buf) >= filesize)
					ok = FALSE;
				else if (*p == -1)
					break;
				else if (*p != 0)
					ok = FALSE;
				else
					p++;
			}
			if (ok != FALSE && *p == -1)
			{
				/*
 				 * The CICONBLK structures immediately follow this list
				 */
				p++;
				cicon_p = (CICONBLK *)p;
			}
		}
		if (ok == FALSE)
		{
			xrsrc_free(file);
			return NULL;
		}
	}

	if (rsrc_load_works() && !(flags & XRSC_SAFETY_CHECKS))
	{
		/*
		 * simple case, we just have to translate file offsets in memory addresses
		 */
		file->rs_trindex = (OBJECT **)(buf + (size_t)file->header.rsh_trindex);
		file->rs_object = (OBJECT *)(buf + (size_t)file->header.rsh_object);
		file->rs_tedinfo = (TEDINFO *)(buf + (size_t)file->header.rsh_tedinfo);
		file->rs_iconblk = (ICONBLK *)(buf + (size_t)file->header.rsh_iconblk);
		file->rs_bitblk = (BITBLK *)(buf + (size_t)file->header.rsh_bitblk);
		file->rs_frstr = (char **)(buf + (size_t)file->header.rsh_frstr);
		file->rs_frimg = (BITBLK **)(buf + (size_t)file->header.rsh_frimg);
		file->rs_ciconblk = cicon_p;

		{
			OBJECT **rs_trindex;

			rs_trindex = file->rs_trindex;
			for (UObj = 0; UObj < xrsc_header.rsh_ntree; UObj++, rs_trindex++)
			{
				*rs_trindex = (OBJECT *)(buf + (uintptr_t)(*rs_trindex));
			}
		}
	
		{
			TEDINFO *rs_tedinfo;
	
			rs_tedinfo = file->rs_tedinfo;
			for (UObj = 0; UObj < xrsc_header.rsh_nted; UObj++, rs_tedinfo++)
			{
				rs_tedinfo->te_ptext += (uintptr_t)buf;
				rs_tedinfo->te_ptmplt += (uintptr_t)buf;
				rs_tedinfo->te_pvalid += (uintptr_t)buf;
			}
		}

		{
			ICONBLK *rs_iconblk;

			rs_iconblk = file->rs_iconblk;
			for (UObj = 0; UObj < xrsc_header.rsh_nib; UObj++, rs_iconblk++)
			{
				rs_iconblk->ib_pmask = (_WORD *)(buf + (uintptr_t)(rs_iconblk->ib_pmask));
				rs_iconblk->ib_pdata = (_WORD *)(buf + (uintptr_t)(rs_iconblk->ib_pdata));
				rs_iconblk->ib_ptext = buf + (uintptr_t)rs_iconblk->ib_ptext;
			}
		}

		{
			BITBLK *rs_bitblk;
	
			rs_bitblk = file->rs_bitblk;
			for (UObj = 0; UObj < xrsc_header.rsh_nbb; UObj++, rs_bitblk++)
			{
				rs_bitblk->bi_pdata = (_WORD *)(buf + (uintptr_t)(rs_bitblk->bi_pdata));
			}
		}

		{
			char **rs_frstr;
	
			rs_frstr = file->rs_frstr;
			for (UObj = 0; UObj < xrsc_header.rsh_nstring; UObj++, rs_frstr++)
			{
				*rs_frstr += (uintptr_t)buf;
			}
		}

		{
			BITBLK **rs_frimg;

			rs_frimg = file->rs_frimg;
			for (UObj = 0; UObj < xrsc_header.rsh_nimages; UObj++, rs_frimg++)
			{
				*rs_frimg = (BITBLK *)(buf + (uintptr_t)(*rs_frimg));
			}
		}

	} else
	{
		/*
		 * we have to do it the hard way
		 */
		if (file->header.rsh_nobs > 0)
		{
			char *src;
			OBJECT *dst;
			
			file->rs_object = g_new(OBJECT, file->header.rsh_nobs);
			if (file->rs_object == NULL)
			{
				xrsrc_free(file);
				return NULL;
			}
			file->allocated |= RSC_ALLOC_OBJECT;
			if (file->header.rsh_object < RSC_SIZEOF_RS_HEADER ||
				file->header.rsh_object + file->header.rsh_nobs * RSC_SIZEOF_OBJECT > file->header.rsh_rssize)
			{
				report_problem(&file->header, "rsh_object out of range", file->header.rsh_trindex, NULL);
				warn_damaged(filename, "Objects");
			} else
			{
				src = MAKEPTR(buf, file->header.rsh_object);
				dst = file->rs_object;
				for (UObj = 0; UObj < xrsc_header.rsh_nobs; UObj++, src += RSC_SIZEOF_OBJECT, dst++)
				{
					dst->ob_next = get_word(src + 0);
					dst->ob_head = get_word(src + 2);
					dst->ob_tail = get_word(src + 4);
					dst->ob_type = get_word(src + 6);
					dst->ob_flags = get_word(src + 8);
					dst->ob_state = get_word(src + 10);
					dst->ob_spec.index = get_long(src + 12);
					dst->ob_x = get_word(src + 16);
					dst->ob_y = get_word(src + 18);
					dst->ob_width = get_word(src + 20);
					dst->ob_height = get_word(src + 22);
				}
			}
		}

		if (file->header.rsh_ntree > 0)
		{
			char *src;
			OBJECT **dst;
			
			file->rs_trindex = g_new0(OBJECT *, file->header.rsh_ntree);
			if (file->rs_trindex == NULL)
			{
				xrsrc_free(file);
				return NULL;
			}
			file->allocated |= RSC_ALLOC_TRINDEX;
			dst = file->rs_trindex;
			if (file->header.rsh_trindex < RSC_SIZEOF_RS_HEADER ||
				file->header.rsh_trindex + file->header.rsh_ntree * RSC_SIZEOF_PTR > file->header.rsh_rssize)
			{
				report_problem(&file->header, "rsh_trindex out of range", file->header.rsh_trindex, NULL);
				warn_damaged(filename, "Trees");
				for (UObj = 0; UObj < xrsc_header.rsh_ntree; UObj++, dst++)
					*dst = &empty_object;
			} else
			{
				src = MAKEPTR(buf, file->header.rsh_trindex);
				for (UObj = 0; UObj < xrsc_header.rsh_ntree; UObj++, src += RSC_SIZEOF_PTR, dst++)
				{
					offset = get_long(src);
					if (offset < RSC_SIZEOF_RS_HEADER || CHECK_ODD(offset) ||
						offset < file->header.rsh_object ||
						offset >= file->header.rsh_object + file->header.rsh_nobs * RSC_SIZEOF_OBJECT)
					{
						KINFO(("tree offset[%lu] out of range: $%lx\n", UObj, offset));
						warn_damaged(filename, "Trees");
						*dst = &empty_object;
					} else
					{
						offset = (offset - file->header.rsh_object) / RSC_SIZEOF_OBJECT;
						*dst = &file->rs_object[offset];
					}
				}
			}
		}
		
		if (file->header.rsh_nted > 0)
		{
			char *src;
			TEDINFO *dst;
			
			file->rs_tedinfo = g_new(TEDINFO, file->header.rsh_nted);
			if (file->rs_tedinfo == NULL)
			{
				xrsrc_free(file);
				return NULL;
			}
			file->allocated |= RSC_ALLOC_TEDINFO;
			if (file->header.rsh_trindex < RSC_SIZEOF_RS_HEADER)
			{
				report_problem(&xrsc_header, "rsh_trindex out of range", file->header.rsh_trindex, &whats_wrong);
				warn_damaged(filename, "Tedinfos");
			} else
			{
				src = MAKEPTR(buf, file->header.rsh_tedinfo);
				dst = file->rs_tedinfo;
				for (UObj = 0; UObj < xrsc_header.rsh_nted; UObj++, src += RSC_SIZEOF_TEDINFO, dst++)
				{
					idx = get_long(src + 0);
					/*
					 * some broken Digital GEM RSC files have an offset of -1
					 */
					if (idx == 0xffffffffUL && file->rsc_little_endian)
						idx = 0;
					if (idx == 0)
					{
						/* silently ignore NULL ptrs from broken Digital GEM resource files */
						dst->te_ptext = empty;
						dst->te_txtlen = 1;
					} else if (idx < RSC_SIZEOF_RS_HEADER || CHECK(idx))
					{
						dst->te_ptext = empty;
						dst->te_txtlen = 1;
						report_problem(&xrsc_header, "te_ptext out of range", idx, &whats_wrong);
					} else
					{
						dst->te_ptext = MAKEPTR(buf, idx);
						dst->te_txtlen = get_word(src + 24);
					}
					idx = get_long(src + 4);
					if (idx < RSC_SIZEOF_RS_HEADER || CHECK(idx))
					{
						dst->te_ptmplt = empty;
						dst->te_tmplen = 1;
						/* silently ignore NULL ptrs from broken Digital GEM resource files */
						if (idx != 0 || !file->rsc_little_endian)
							report_problem(&xrsc_header, "te_ptmplt out of range", idx, &whats_wrong);
					} else
					{
						dst->te_ptmplt = MAKEPTR(buf, idx);
						dst->te_tmplen = get_word(src + 26);
					}
					idx = get_long(src + 8);
					if (idx < RSC_SIZEOF_RS_HEADER || CHECK(idx))
					{
						dst->te_pvalid = empty;
						/* silently ignore NULL ptrs from broken Digital GEM resource files */
						if (idx != 0 || !file->rsc_little_endian)
							report_problem(&xrsc_header, "te_pvalid out of range", idx, &whats_wrong);
					} else
					{
						dst->te_pvalid = MAKEPTR(buf, idx);
					}
					dst->te_font = get_word(src + 12);
					dst->te_fontid = get_word(src + 14);
					dst->te_just = get_word(src + 16);
					dst->te_color = get_word(src + 18);
					dst->te_fontsize = get_word(src + 20);
					dst->te_thickness = get_word(src + 22);
				}
			}
		}

		if (file->header.rsh_nib > 0)
		{
			char *src;
			ICONBLK *dst;

			file->rs_iconblk = g_new(ICONBLK, file->header.rsh_nib);
			if (file->rs_iconblk == NULL)
			{
				xrsrc_free(file);
				return NULL;
			}
			file->allocated |= RSC_ALLOC_ICONBLK;
			if (file->header.rsh_iconblk < RSC_SIZEOF_RS_HEADER ||
				file->header.rsh_iconblk + file->header.rsh_nib * RSC_SIZEOF_ICONBLK > file->header.rsh_rssize)
			{
				report_problem(&xrsc_header, "rsh_iconblk out of range", file->header.rsh_iconblk, &whats_wrong);
				warn_damaged(filename, "Iconblks");
			} else
			{
				src = MAKEPTR(buf, file->header.rsh_iconblk);
				dst = file->rs_iconblk;
				for (UObj = 0; UObj < xrsc_header.rsh_nib; UObj++, src += RSC_SIZEOF_ICONBLK, dst++)
				{
					idx = get_long(src + 0);
					if (idx < RSC_SIZEOF_RS_HEADER || CHECK_ODD(idx))
					{
						report_problem(&xrsc_header, "ib_pmask out of range", idx, &whats_wrong);
						/*
						 * some broken editors put the icon data in the extended resource part
						 */
						if (idx >= RSC_SIZEOF_RS_HEADER && !CHECK_SIZE(idx, filesize) && !IS_REALLY_ODD(idx))
							dst->ib_pmask = (_WORD *)MAKEPTR(buf, idx);
						else
							dst->ib_pmask = imdata;
					} else
					{
						dst->ib_pmask = (_WORD *)MAKEPTR(buf, idx);
					}
					idx = get_long(src + 4);
					if (idx < RSC_SIZEOF_RS_HEADER || CHECK_ODD(idx))
					{
						report_problem(&xrsc_header, "ib_pdata out of range", idx, &whats_wrong);
						if (idx >= RSC_SIZEOF_RS_HEADER && !CHECK_SIZE(idx, filesize) && !IS_REALLY_ODD(idx))
							dst->ib_pdata = (_WORD *)MAKEPTR(buf, idx);
						else
							dst->ib_pdata = imdata;
					} else
					{
						dst->ib_pdata = (_WORD *)MAKEPTR(buf, idx);
					}
					idx = get_long(src + 8);
					if (idx < RSC_SIZEOF_RS_HEADER || CHECK(idx))
					{
						report_problem(&xrsc_header, "ib_ptext out of range", idx, &whats_wrong);
						if (idx >= RSC_SIZEOF_RS_HEADER && !CHECK_SIZE(idx, filesize))
							dst->ib_ptext = MAKEPTR(buf, idx);
						else
							dst->ib_ptext = empty;
					} else
					{
						dst->ib_ptext = MAKEPTR(buf, idx);
					}
					dst->ib_char = get_word(src + 12);
					dst->ib_xchar = get_word(src + 14);
					dst->ib_ychar = get_word(src + 16);
					dst->ib_xicon = get_word(src + 18);
					dst->ib_yicon = get_word(src + 20);
					dst->ib_wicon = get_word(src + 22);
					dst->ib_hicon = get_word(src + 24);
					dst->ib_xtext = get_word(src + 26);
					dst->ib_ytext = get_word(src + 28);
					dst->ib_wtext = get_word(src + 30);
					dst->ib_htext = get_word(src + 32);
				}
			}
		}

		if (file->header.rsh_nbb > 0)
		{
			char *src;
			BITBLK *dst;
	
			file->rs_bitblk = g_new(BITBLK, file->header.rsh_nbb);
			if (file->rs_bitblk == NULL)
			{
				xrsrc_free(file);
				return NULL;
			}
			file->allocated |= RSC_ALLOC_BITBLK;
			if (file->header.rsh_bitblk < RSC_SIZEOF_RS_HEADER ||
				file->header.rsh_bitblk + file->header.rsh_nbb * RSC_SIZEOF_BITBLK > file->header.rsh_rssize)
			{
				report_problem(&xrsc_header, "rsh_bitblk out of range", file->header.rsh_bitblk, &whats_wrong);
				warn_damaged(filename, "Bitblks");
			} else
			{
				src = MAKEPTR(buf, file->header.rsh_bitblk);
				dst = file->rs_bitblk;
				for (UObj = 0; UObj < xrsc_header.rsh_nbb; UObj++, src += RSC_SIZEOF_BITBLK, dst++)
				{
					idx = get_long(src + 0);
					if (idx < RSC_SIZEOF_RS_HEADER || CHECK_ODD(idx))
					{
						dst->bi_pdata = imdata;
						report_problem(&xrsc_header, "bi_pdata out of range", idx, &whats_wrong);
					} else
					{
						dst->bi_pdata = (_WORD *)MAKEPTR(buf, idx);
					}
					dst->bi_wb = get_word(src + 4);
					dst->bi_hl = get_word(src + 6);
					dst->bi_x = get_word(src + 8);
					dst->bi_y = get_word(src + 10);
					dst->bi_color = get_word(src + 12);
				}
			}
		}

		if (file->header.rsh_nstring > 0)
		{
			char *src;
			char **dst;
	
			file->rs_frstr = g_new(char *, file->header.rsh_nstring);
			if (file->rs_frstr == NULL)
			{
				xrsrc_free(file);
				return NULL;
			}
			file->allocated |= RSC_ALLOC_FRSTR;
			if (file->header.rsh_frstr < RSC_SIZEOF_RS_HEADER ||
				file->header.rsh_frstr + file->header.rsh_nstring * RSC_SIZEOF_PTR > file->header.rsh_rssize)
			{
				report_problem(&xrsc_header, "rsh_frstr out of range", file->header.rsh_frstr, &whats_wrong);
				warn_damaged(filename, "Strings");
			} else
			{
				src = MAKEPTR(buf, file->header.rsh_frstr);
				dst = file->rs_frstr;
				for (UObj = 0; UObj < xrsc_header.rsh_nstring; UObj++, src += RSC_SIZEOF_PTR, dst++)
				{
					offset = get_long(src);
					if (CHECK_SIZE(offset, filesize))
					{
						KINFO(("string offset[%lu] out of range: $%lx\n", UObj, offset));
						warn_damaged(filename, "Strings");
						*dst = empty;
					} else
					{
						*dst = MAKEPTR(buf, offset);
						if ((crc_for_string = rsc_get_crc_string(*dst)) != RSC_CRC_NONE)
						{
							file->rsc_opts.crc_string = TRUE;
						}
					}
				}
			}
		}
		
		if (file->header.rsh_nimages > 0)
		{
			char *src;
			BITBLK **dst;

			file->rs_frimg = g_new(BITBLK *, file->header.rsh_nimages);
			if (file->rs_frimg == NULL)
			{
				xrsrc_free(file);
				return NULL;
			}
			file->allocated |= RSC_ALLOC_FRIMG;
			if (file->header.rsh_frimg < RSC_SIZEOF_RS_HEADER ||
				file->header.rsh_frimg + file->header.rsh_nimages * RSC_SIZEOF_PTR > file->header.rsh_rssize)
			{
				report_problem(&xrsc_header, "rsh_frimg out of range", file->header.rsh_frimg, &whats_wrong);
				warn_damaged(filename, "Images");
			} else
			{
				src = MAKEPTR(buf, file->header.rsh_frimg);
				dst = file->rs_frimg;
				for (UObj = 0; UObj < xrsc_header.rsh_nimages; UObj++, src += RSC_SIZEOF_PTR, dst++)
				{
					offset = get_long(src);
					if (CHECK_SIZE(offset, filesize) || IS_REALLY_ODD(offset) ||
						offset < file->header.rsh_bitblk ||
						offset >= file->header.rsh_bitblk + file->header.rsh_nbb * RSC_SIZEOF_BITBLK)
					{
						KINFO(("image offset[%lu] out of range: $%lx\n", UObj, offset));
						warn_damaged(filename, "Images");
						*dst = &empty_bit;
					} else
					{
						offset = (offset - file->header.rsh_bitblk) / RSC_SIZEOF_BITBLK;
						*dst = &file->rs_bitblk[offset];
					}
				}
			}
		}

		if (file->rsc_nciconblks > 0)
		{
			file->rs_ciconblk = g_new(CICONBLK, file->rsc_nciconblks);
			if (file->rs_ciconblk == NULL)
			{
				xrsrc_free(file);
				return NULL;
			}
			file->allocated |= RSC_ALLOC_CICONBLK;
			cicon_dst = file->rs_ciconblk;
		}

		if (n_userblks > 0)
		{
			file->rs_userblk = g_new(USERBLK, n_userblks);
			if (file->rs_userblk == NULL)
			{
				xrsrc_free(file);
				return NULL;
			}
			file->allocated |= RSC_ALLOC_USERBLK;
		}
	}

	n_userblks = 0;
	{
		OBJECT *rs_object = file->rs_object;
		
		if (!(flags & XRSC_NO_OBFIX))
			rsc_obfix(rs_object, xrsc_header.rsh_nobs, wchar, hchar);
		for (UObj = 0; UObj < xrsc_header.rsh_nobs; UObj++, rs_object++)
		{
			idx = rs_object->ob_spec.index;
			switch (rs_object->ob_type & OBTYPEMASK)
			{
			case G_BOX:
			case G_IBOX:
			case G_BOXCHAR:
			case G_EXTBOX:
				break;

			case G_STRING:
			case G_TITLE:
			case G_BUTTON:
			case G_SHORTCUT:
				/* not changed to G_STRING here;
				   might be displayed in window
				   where we use our own functions
				 */
				if (idx < RSC_SIZEOF_RS_HEADER || CHECK(idx))
				{
					rs_object->ob_spec.free_string = empty;
					report_problem(&xrsc_header, "ob_spec.free_string out of range", idx, &whats_wrong);
				} else
				{
					rs_object->ob_spec.free_string = MAKEPTR(buf, idx);
				}
				break;

			case G_TEXT:
			case G_FTEXT:
			case G_BOXTEXT:
			case G_FBOXTEXT:
				if (idx < RSC_SIZEOF_RS_HEADER || CHECK_ODD(idx) ||
					(offset = (idx - file->header.rsh_tedinfo) / RSC_SIZEOF_TEDINFO) >= file->header.rsh_nted)
				{
					report_problem(&xrsc_header, "ob_spec.tedinfo out of range", idx, &whats_wrong);
					rs_object->ob_spec.tedinfo = &empty_ted;
				} else
				{
					if (file->allocated & RSC_ALLOC_TEDINFO)
					{
						rs_object->ob_spec.tedinfo = &file->rs_tedinfo[offset];
					} else
					{
						rs_object->ob_spec.free_string = MAKEPTR(buf, idx);
					}
				}
				break;
				
			case G_IMAGE:
				if (idx < RSC_SIZEOF_RS_HEADER || CHECK_ODD(idx) ||
					(offset = (idx - file->header.rsh_bitblk) / RSC_SIZEOF_BITBLK) >= file->header.rsh_nbb)
				{
					rs_object->ob_spec.bitblk = &empty_bit;
					report_problem(&xrsc_header, "ob_spec.bitblk out of range", idx, &whats_wrong);
				} else
				{
					if (file->allocated & RSC_ALLOC_BITBLK)
					{
						rs_object->ob_spec.bitblk = &file->rs_bitblk[offset];
					} else
					{
						rs_object->ob_spec.free_string = MAKEPTR(buf, idx);
					}
				}
				break;

			case G_ICON:
				if (idx < RSC_SIZEOF_RS_HEADER || CHECK_ODD(idx) ||
					(offset = (idx - file->header.rsh_iconblk) / RSC_SIZEOF_ICONBLK) >= file->header.rsh_nib)
				{
					rs_object->ob_spec.iconblk = &empty_icon;
					report_problem(&xrsc_header, "ob_spec.iconblk out of range", idx, &whats_wrong);
				} else
				{
					if (file->allocated & RSC_ALLOC_ICONBLK)
					{
						rs_object->ob_spec.iconblk = &file->rs_iconblk[offset];
					} else
					{
						rs_object->ob_spec.free_string = MAKEPTR(buf, idx);
					}
				}
				break;

			case G_CICON:

				if (cicon_p == NULL)
				{
					/* !!! */
				} else
				{
					CICON *dp;
					CICONBLK *cicon;
					_LONG size;
					char *p;
					char *sp;
					_LONG num_cicons;

					offset = OFFSET(cicon_p, buf, _LONG);
					if (offset >= filesize)
					{
						report_problem(&xrsc_header, "color icon data out of range", offset, &whats_wrong);
						xrsrc_free(file);
						return NULL;
					}
					cicon = cicon_p;
					cicon_p = (CICONBLK *)((char *)cicon + RSC_SIZEOF_CICONBLK);
					if (swap_flag)
					{
						flip_iconblk((char *) cicon);
						swap_long((char *) cicon + 34); /* swap cicon->mainlist */
					}
					if (file->allocated & RSC_ALLOC_CICONBLK)
					{
						char *src = (char *)cicon;
						
						rs_object->ob_spec.ciconblk = cicon_dst;
						cicon = cicon_dst++;
						cicon->monoblk.ib_ptext = (char *)(uintptr_t)get_long(src + 8);
						cicon->monoblk.ib_char = get_word(src + 12);
						cicon->monoblk.ib_xchar = get_word(src + 14);
						cicon->monoblk.ib_ychar = get_word(src + 16);
						cicon->monoblk.ib_xicon = get_word(src + 18);
						cicon->monoblk.ib_yicon = get_word(src + 20);
						cicon->monoblk.ib_wicon = get_word(src + 22);
						cicon->monoblk.ib_hicon = get_word(src + 24);
						cicon->monoblk.ib_xtext = get_word(src + 26);
						cicon->monoblk.ib_ytext = get_word(src + 28);
						cicon->monoblk.ib_wtext = get_word(src + 30);
						cicon->monoblk.ib_htext = get_word(src + 32);
						cicon->mainlist = (CICON *)(intptr_t)get_long(src + 34);
					} else
					{
						rs_object->ob_spec.ciconblk = cicon;
						cicon_dst = cicon;
					}
					size = iconblk_masksize(&cicon->monoblk);
					p = (char *)cicon_p;
					cicon->monoblk.ib_pdata = (_WORD *)p;
					p += (size_t)size;
					cicon->monoblk.ib_pmask = (_WORD *)p;
					p += (size_t)size;
					if (p > (buf + (size_t)filesize))
					{
						report_problem(&xrsc_header, "color icon data out of range", p - buf, &whats_wrong);
						xrsrc_free(file);
						return NULL;
					}
#if FLIP_DATA
					if (swap_flag)
					{
						flip_image(size / 2, cicon->monoblk.ib_pdata);
						flip_image(size / 2, cicon->monoblk.ib_pmask);
					}
#endif
					/*
					 * ib_ptext can either be the offset after the iconblk (when less than CICON_STR_SIZE),
					 * or point into the string area
					 */
					idx = (_ULONG)(uintptr_t)cicon->monoblk.ib_ptext;
					if (idx >= filesize)
					{
						report_problem(&xrsc_header, "ib_ptext out of range", idx, &whats_wrong);
						cicon->monoblk.ib_ptext = empty;
					} else if (idx == 0 || (buf + (size_t)idx) == p || idx < xrsc_header.rsh_string || idx >= xrsc_header.rsh_rssize)
					{
						cicon->monoblk.ib_ptext = p;
					} else
					{
						cicon->monoblk.ib_ptext = MAKEPTR(buf, idx);
					}
					p += CICON_STR_SIZE;
					dp = (CICON *)p;
					sp = p;
					num_cicons = (_LONG)(intptr_t)(cicon->mainlist);
					if (p > (buf + (size_t)filesize))
					{
						report_problem(&xrsc_header, "color icon data out of range", p - buf, &whats_wrong);
						xrsrc_free(file);
						return NULL;
					}
					if (num_cicons == 0)
					{
						cicon->mainlist = NULL;
					} else
					{
						CICON **link_p;
						
						link_p = &cicon->mainlist;
						while (num_cicons != 0)
						{
							if (file->allocated & RSC_ALLOC_CICONBLK)
							{
								dp = g_new(CICON, 1);
								if (dp == NULL)
								{
									xrsrc_free(file);
									return NULL;
								}
							}
							*link_p = dp;
							dp->next_res = NULL;
							link_p = &dp->next_res;
							
							offset = OFFSET(p, buf, _LONG);
							if (offset >= filesize)
							{
								report_problem(&xrsc_header, "unexpected end of coloricon list", offset, &whats_wrong);
								break;
							}
							dp->num_planes = get_word(p);
							if (swap_flag)
								dp->num_planes = bswap_16(dp->num_planes);
							p += RSC_SIZEOF_CICON;
							dp->col_data = (_WORD *)p;
							p += (size_t)(size * dp->num_planes);
							dp->col_mask = (_WORD *)p;
							p += (size_t)size;
							offset = OFFSET(p, buf, _LONG);
							if (offset > filesize)
							{
								report_problem(&xrsc_header, "color icon data out of range", offset, &whats_wrong);
								break;
							}
#if FLIP_DATA
							if (swap_flag)
							{
								flip_image(size * dp->num_planes / 2, dp->col_data);
								flip_image(size / 2, dp->col_mask);
							}
#endif
							/* get sel_data pointer */
							offset = get_long(sp + 10);
							if (offset != 0)
							{
								dp->sel_data = (_WORD *)p;
								p += (size_t)(size * dp->num_planes);
								dp->sel_mask = (_WORD *)p;
								p += (size_t)size;
								offset = OFFSET(p, buf, _LONG);
								if (offset > filesize)
								{
									report_problem(&xrsc_header, "color icon data out of range", offset, &whats_wrong);
									break;
								}
#if FLIP_DATA
								if (swap_flag)
								{
									flip_image(size * dp->num_planes / 2, dp->sel_data);
									flip_image(size / 2, dp->sel_mask);
								}
#endif
							} else
							{
								dp->sel_data = NULL;
								dp->sel_mask = NULL;
							}
							num_cicons--;

							dp = (CICON *)p;
							sp = p;
							offset = OFFSET(p, buf, _LONG);
							if (offset > filesize)
							{
								report_problem(&xrsc_header, "color icon data out of range", offset, &whats_wrong);
								break;
							}
						}
					}
					cicon_p = (CICONBLK *)p;
				}
				break;

			case G_USERDEF:
				if (idx < RSC_SIZEOF_RS_HEADER || CHECK_ODD(idx))
				{
					rs_object->ob_spec.userblk = &empty_user;
					rs_object->ob_spec.userblk->ub_parm = 0;
					report_problem(&xrsc_header, "ob_spec.userblk out of range", idx, &whats_wrong);
				} else
				{
					if (file->allocated & RSC_ALLOC_USERBLK)
					{
						char *src = MAKEPTR(buf, idx);
						rs_object->ob_spec.userblk = &file->rs_userblk[n_userblks++];
						rs_object->ob_spec.userblk->ub_parm = get_long(src + 4);
					} else
					{
						rs_object->ob_spec.userblk = (USERBLK *)MAKEPTR(buf, idx);
					}
				}
				/*
				 * It's up to the application to set the appropiate function.
				 * To be on the safe side, let it point to some function
				 * that draws a box only, or simply does nothing.
				 */
				rs_object->ob_spec.userblk->ub_code = draw_userdef;
				break;
			
			default:
				whats_wrong = "unknown object type";
				KINFO(("%s\n", whats_wrong));
				break;
			}
		}
	}

	{
		OBJECT **rs_trindex;

		rs_trindex = file->rs_trindex;
		for (UObj = 0; UObj < xrsc_header.rsh_ntree; UObj++, rs_trindex++)
		{
			if (!(flags & XRSC_NO_ZERO_ROOT))
			{
				(*rs_trindex)[ROOT].ob_x = 0;
				(*rs_trindex)[ROOT].ob_y = 0;
			}
		}
#if 0
		if (!(flags & XRSC_NO_INSERT_POPUPS))
		{
			rsc_insert_popups(file->rs_trindex, xrsc_header.rsh_ntree, xrsc_header.rsh_nobs);
		}
#endif
	}
	
	if (file->rsc_nciconblks == 0 && (xrsc_header.rsh_vrsn & RSC_EXT_FLAG))
	{
		warn_cicons();
	}
	
	{
		RSC_RSM_CRC crc = 0x5555;
		_ULONG i;
		_UBYTE *ob;
		
		crc += (_WORD)xrsc_header.rsh_ntree * 11;
		/*
		 * RSM calculates the crc before adding the string,
		 * so don't count the string containing the crc
		 */
		i = xrsc_header.rsh_nstring;
		if (file->rsc_opts.crc_string)
			i--;
		crc += (_WORD)i * 13;
		crc += (_WORD)xrsc_header.rsh_nimages * 17;
		
		{
			_WORD idx;
			
			ob = (_UBYTE *)MAKEPTR(buf, xrsc_header.rsh_object);
			idx = 0;
			for (i = 0; i < xrsc_header.rsh_nobs; i++, ob += RSC_SIZEOF_OBJECT)
			{
				crc += 19;
				crc += (idx + 1 + (_WORD)get_word((char *)ob + 0)) * 23; /* ob_next */
				crc += (idx + 1 + (_WORD)get_word((char *)ob + 2)) * 29; /* ob_head */
				crc += (idx + 1 + (_WORD)get_word((char *)ob + 4)) * 31; /* ob_tail */
				crc += (idx + 1 + (_WORD)get_word((char *)ob + 6)) * 37; /* ob_type */
				idx++;
				if (get_word((char *)ob + 8) & OF_LASTOB)
					idx = 0;
			}
		}
		
		if (file->rsc_opts.crc_string && crc != crc_for_string)
		{
			KINFO(("orsc_load: crc: calculated $%04x, in file $%04x\n", crc, crc_for_string));
			warn_crc_string_mismatch(filename);
		} else if (file->rsc_opts.crc_string)
		{
			/*
			 * Silently remove the CRC string.
			 * It will be added again when the file is saved.
			 * This is done after loading the definition files,
			 * so we don't barf upon an out-of range name index.
			 */
		}
	}
	
#if FLIP_DATA
	/*
	 * swap it now when loading a file,
	 * after their pointers have been fixed
	 */
	if (file->rsc_swap_flag)
		xrsc_flip_data(file, filesize);
#endif

	return file;
}

/*** ---------------------------------------------------------------------- ***/

_BOOL xrsrc_free(RSCFILE *file)
{
	if (file == NULL)
		return FALSE;
	if (file->allocated & RSC_ALLOC_CICONBLK)
	{
		_ULONG i;
		OBJECT *obj;
		CICON *p, *next;
		for (i = 0, obj = file->rs_object; i < file->header.rsh_nobs; i++, obj++)
		{
			if ((obj->ob_type & OBTYPEMASK) == G_CICON)
			{
				for (p = obj->ob_spec.ciconblk->mainlist; p; p = next)
				{
					next = p->next_res;
					g_free(p);
				}
			}
		}
		g_free(file->rs_ciconblk);
	}
	if (file->allocated & RSC_ALLOC_TRINDEX)
		g_free(file->rs_trindex);
	if (file->allocated & RSC_ALLOC_OBJECT)
		g_free(file->rs_object);
	if (file->allocated & RSC_ALLOC_TEDINFO)
		g_free(file->rs_tedinfo);
	if (file->allocated & RSC_ALLOC_ICONBLK)
		g_free(file->rs_iconblk);
	if (file->allocated & RSC_ALLOC_BITBLK)
		g_free(file->rs_bitblk);
	if (file->allocated & RSC_ALLOC_FRSTR)
		g_free(file->rs_frstr);
	if (file->allocated & RSC_ALLOC_FRIMG)
		g_free(file->rs_frimg);
	if (file->allocated & RSC_ALLOC_USERBLK)
		g_free(file->rs_userblk);
	g_free(file);
	return TRUE;
}
