#ifndef __RSC_LANG_H__
#define __RSC_LANG_H__

#ifndef __PORTAB_H__
#  include <portab.h>
#endif

#define RSC_LANG_DELIM '\n'

EXTERN_C_BEG

char *rsc_language_str(char *str, RSC_LANG language);
char *rsc_language_strdup(const char *str, RSC_LANG language);
_BOOL rsc_lang_split(LANG_ARRAY arr, const char *str);
void rsc_lang_unsplit(LANG_ARRAY arr);
char *rsc_lang_make_str(LANG_ARRAY arr);
char *MsgLangString(LANG_ARRAY arr, _WORD Nr);
const char *rsc_lang_id_to_name(RSC_LANG lang);
const char *rsc_lang_id_to_english(RSC_LANG lang);
const char *rsc_lang_id_to_native(RSC_LANG lang);
RSC_LANG rsc_lang_name_to_id(const char *name);
RSC_LANG rsc_lang_english_name_to_id(const char *name);
RSC_LANG rsc_lang_find(RSC_LANG lang, const char *available, _BOOL return_idx);
#define RSC_LANG_FLAG_NATIVE 0x01
OBJECT *rsc_lang_popup(const char *languages, RSC_LANG lang, _WORD width, _UWORD flags);
void rsc_lang_popup_free(OBJECT *tree);

EXTERN_C_END

#endif /* __RSC_LANG_H__ */
