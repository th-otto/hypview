#ifndef __GEMX_H__
#define __GEMX_H__

#ifdef __PUREC__
#include <aes.h>
#include <vdi.h>
#include <tos.h>
#include <wdlgevnt.h>
#include <wdlglbox.h>
#include <wdlgwdlg.h>
#include <wdlgfslx.h>
#else
#include <gem.h>
#include <gemx.h>
extern short _app;
#include <mint/osbind.h>
#include <mint/mintbind.h>

/* MagiC XTED Struktur */
#ifndef __XTED
#define __XTED
typedef struct
{
	char *xte_ptmplt;
	char *xte_pvalid;
	_WORD xte_vislen;
	_WORD xte_scroll;
} XTED;
#endif

#ifndef _AESrscmem
#define	_AESrscmem   (*((void **)&aes_global[7]))
#endif
#ifndef _AESrsclen
#define	_AESrsclen   (aes_global[9])
#endif
#undef _AESrscfile
#define	_AESrscfile   (*((OBJECT ***)&aes_global[5]))

#endif

#include "tos/scancode.h"

#ifdef __PUREC__
#define inline
#endif

#ifndef __EVNTDATA
#define __EVNTDATA
typedef struct
{
	_WORD x;
	_WORD y;
	_WORD bstate;
	_WORD kstate;
} EVNTDATA;
#endif

extern int __mint;

#endif /* __GEMX_H__ */
