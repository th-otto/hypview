#ifndef __HYP_PROFILE_H__
#define __HYP_PROFILE_H__ 1

#include <stdint.h>

typedef struct _profile Profile;

#if !defined(__PROFILE_IMPLEMENTATION__) && defined(ANONYMOUS_STRUCT_DUMMY)
ANONYMOUS_STRUCT_DUMMY(_profile)
#endif

/*
 * generic functions
 */

/*
 * like strtod(), but always run in "C" locale
 */
double c_strtod(const char *s, const char **end);
char *c_dtostr(double value);

const char *x_get_tmp_dir(void);
const char *x_get_home_dir(void);
const char *x_get_user_name(void);
const char *x_get_real_name(void);
const char *x_get_user_appdata(void);
const char *x_get_system_appdata(void);

void x_subst_homedir(char **dirp);
void x_unsubst_homedir(char **dirp);
void x_free_resources(void);

#ifdef G_PLATFORM_WIN32
char *get_special_folder(int csidl);
#endif

#if defined(__WIN32__) || defined(__TOS__)
#define filename_cmp strcasecmp
#define filename_ncmp strncasecmp
#else
#define filename_cmp strcmp
#define filename_ncmp strncmp
#endif

const char *g_secure_getenv(const char *name);

intmax_t xs_strtoimax(const char *nptr, const char **endptr, int base);
char *xs_imaxtostr(uintmax_t val, char *buf, gboolean is_signed);

#define G_ASCII_DTOSTR_BUF_SIZE (29 + 10)

char *g_ascii_dtostr(char *buffer, int buf_len, double d);
char *g_ascii_formatd(char *buffer, int buf_len, const char *format, double d);

Profile *Profile_New(const char *filename);
void Profile_Delete(Profile *profile);
gboolean Profile_Read(Profile *profile, const char *creator);
gboolean Profile_IsNew(Profile *profile);

Profile *Profile_Load(const char *filename, const char *creator);
gboolean Profile_Save(Profile *profile);
const char *Profile_GetFilename(Profile *profile);

char *Profile_GetSectionNames(Profile *profile, unsigned long *pNumSections);
char *Profile_GetKeyNames(Profile *profile, const char *section, unsigned long *pNumKeys);
gboolean Profile_DeleteKey(Profile *profile, const char *section, const char *key);


gboolean Profile_ReadString(Profile *profile, const char *section, const char *key, char **strp);
gboolean Profile_ReadByte(Profile *profile, const char *section, const char *key, unsigned char *pval);
gboolean Profile_ReadInt(Profile *profile, const char *section, const char *key, int *pval);
gboolean Profile_ReadLong(Profile *profile, const char *section, const char *key, intmax_t *pval);
gboolean Profile_ReadCard(Profile *profile, const char *section, const char *key, unsigned int *pval);
gboolean Profile_ReadLongCard(Profile *profile, const char *section, const char *key, uintmax_t *pval);
gboolean Profile_ReadBool(Profile *profile, const char *section, const char *key, gboolean *pval);
gboolean Profile_ReadChar(Profile *profile, const char *section, const char *key, char *pval);

void Profile_WriteString(Profile *profile, const char *section, const char *key, const char *str);
void Profile_WriteByte(Profile *profile, const char *section, const char *key, unsigned char val);
void Profile_WriteInt(Profile *profile, const char *section, const char *key, int val);
void Profile_WriteCard(Profile *profile, const char *section, const char *key, unsigned int val);
void Profile_WriteLong(Profile *profile, const char *section, const char *key, intmax_t val);
void Profile_WriteLongCard(Profile *profile, const char *section, const char *key, uintmax_t val);
void Profile_WriteBool(Profile *profile, const char *section, const char *key, gboolean val);
void Profile_WriteChar(Profile *profile, const char *section, const char *key, char val);

#endif
