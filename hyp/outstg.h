/*
 * HypView - (c)      - 2019 Thorsten Otto
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
 * along with HypView; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __HYPOUTSTG_H__
#define __HYPOUTSTG_H__ 1

void stg_out_globals(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out);
gboolean stg_out_nodedata(HYP_DOCUMENT *hyp, hcp_opts *opts, GString *out, HYP_NODE *nodeptr, symtab_entry *syms);
gboolean recompile_stg(HYP_DOCUMENT *hyp, hcp_opts *opts, int argc, const char **argv);

#endif /* __HYPOUTSTG_H__ */
