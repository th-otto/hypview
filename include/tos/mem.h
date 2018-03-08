/*
 * HypView - (c)      - 2006 Philipp Donze
 *               2006 -      Philipp Donze & Odd Skancke
 *
 * A replacement hypertext viewer
 *
 * This file is part of HypView.
 *
 * HypView is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * HypView is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HypView; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _mem_h
#define _mem_h

/*	Mxalloc()-Modus	*/
#define MX_STRAM		0x00
#define MX_TTRAM		0x01
#define	MX_PREFSTRAM	0x02
#define MX_PREFTTRAM	0x03

/* Protection bits.  */
#define MX_MPROT		0x08			/* change protection of already allocated block */

#define MX_HEADER		0x00			/* default protection from program header */
#define MX_PRIVATE		0x10			/* only owner can read or write */
#define MX_GLOBAL		0x20 			/* anyone can read or write */
#define MX_SUPERVISOR	0x30			/* any super access OK */
#define MX_READABLE		0x40			/* any read OK, no write */

void *g_alloc_shared(size_t size);
void g_free_shared(void *ptr);
char *g_strdup_shared(const char *str);
char *g_strdup2_shared(const char *str1, const char *str2, char **ptr2);

#endif
