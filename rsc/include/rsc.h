/*****************************************************************************
 * RSC.H
 *****************************************************************************/

#ifndef __RSC_H__
#define __RSC_H__

#ifndef __PORTAB_H__
#  include <portab.h>
#endif
#ifndef __MOBJECT_H__
#  include <mobject.h>
#endif

/*
 * Menue-Indices
 */
/* the box containing the whole menubar */
#define menu_the_bar(menu) (menu[ROOT].ob_head)
/* the ibox containing the titles */
#define menu_the_active(menu) (menu[menu_the_bar(menu)].ob_head)
/* the first title entry */
#define menu_the_first(menu) (menu[menu_the_active(menu)].ob_head)
/* the last title entry */
#define menu_the_last(menu) (menu[menu_the_active(menu)].ob_tail)
/* the ibox containing the menu subboxes */
#define menu_the_menus(menu) (menu[menu_the_bar(menu)].ob_next)



void rsc_init_file(RSCFILE *file);
void rsc_count_all(RSCFILE *file);
void rsc_tree_count(RSCFILE *file);
RSCTREE *rsc_add_tree(RSCFILE *file, _WORD type, const char *name, void *object);
void rsc_tree_delete(RSCTREE *tree);
RSCFILE *rsc_new_file(const char *filename, const char *basename);
_BOOL rsc_file_merge(RSCFILE *dest, RSCFILE *src, _BOOL remove_dups);
void rsc_file_delete(RSCFILE *file, _BOOL all);
void change_changed(RSCFILE *file, _BOOL new_changed);
RSC_RSM_CRC rsc_rsm_calc_crc(const void *buf, _ULONG size);
_BOOL rsc_is_crc_string(const char *str);
RSC_RSM_CRC rsc_get_crc_string(const char *str);
void rsc_remove_crc_string(RSCFILE *file);
void rule_calc(NAMERULE *rule);
_LONG bitblk_datasize(BITBLK *bit);
_LONG iconblk_masksize(ICONBLK *icon);

_BOOL rsc_lang_add(rsc_lang **list, const char *id, const char *charset, const char *filename);
_BOOL nls_set_lang(RSCFILE *file, const char *id);

#endif /* __RSC_H__ */
