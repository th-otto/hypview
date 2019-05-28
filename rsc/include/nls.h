/*
 * nls.h - Definitions for Native Language Support
 *
 * Copyright (C) 2001 The Emutos Development Team
 *
 * Authors:
 *  LVL   Laurent Vogel
 *
 * This file is distributed under the GPL, version 2 or at your
 * option any later version.  See doc/license.txt for details.
 */

#ifndef NLS_H
#define NLS_H

/* the gettext-like macros */

#ifndef N_
#define N_(a) a
#define _(x) x
#endif

/* Indexes of font sets inside font_sets[] */
#define CHARSET_ST 1
#define CHARSET_L9 2
#define CHARSET_L2 3
#define CHARSET_GR 4
#define CHARSET_RU 5
#define CHARSET_L1 6

typedef unsigned short nls_wchar_t;
#define UTF8_CHARMAX 6

#endif /* NLS_H */
