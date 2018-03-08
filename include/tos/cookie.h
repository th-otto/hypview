/*****************************************************************************
 *	TOS/COOKIE.H
 *****************************************************************************/

#ifndef __COOKIE_H__
#define __COOKIE_H__

#define COOKIE_SND      0x5f534e44l /* '_SND' */
#define COOKIE_CPU      0x5f435055l /* '_CPU' */
#define COOKIE_FLK      0x5f464c4bl /* '_FLK' */

#define COOKIE_MINT     0x4d694e54l /* 'MiNT' */
#define COOKIE_FSEL     0x4653454cl /* 'FSEL' */
#define COOKIE_RSVF     0x52535646l /* 'RSVF' */
#define COOKIE_ICFS     0x49434653l /* 'ICFS' */
#define COOKIE_MAGX     0x4d616758l /* 'MagX' */
#define COOKIE_GNVA     0x476e7661l /* 'Gnva' */
#define COOKIE_XSSI     0x58535349l /* 'XSSI' */
#define COOKIE_XFSL     0x7846534Cl /* 'xFSL' */
#define COOKIE_UFSL     0x5546534Cl /* 'UFSL' */
#define COOKIE_MagicMac 0x4d674d63l /* 'MgMc', Magic Mac */

int CK_JarInstalled(void);
_WORD CK_UsedEntries(void);
_WORD CK_JarSize(void);
_BOOL CK_ResizeJar(_WORD newsize);
_BOOL CK_ReadJar(_ULONG id, _LONG *value);
_BOOL CK_WriteJar(_ULONG id, _LONG value);
void CK_SetOptions(_WORD increment, _ULONG xbra_id);

#endif /* __COOKIE_H__ */
