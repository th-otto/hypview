#ifndef __ORCS_RSO_H__
#define __ORCS_RSO_H__

#define RSO_MAGIC   0x52534f48ul

/* Initial format */
#define RSO_VER1_SIZE 262l
/* new fields: rso_date_created, rso_date_changed, rso_edition */
#define RSO_VER2_SIZE 274l
/* fixed minor bug in writing TEDINFOs; no additional fields */
#define RSO_VER3_SIZE 274l
/* included extended types in flags; no additional fields */
#define RSO_VER4_SIZE 274l
/* changed format of timestamps from DOS to Unix; no additional fields */
#define RSO_VER5_SIZE 274l
/* new fields: rso_ted_fillchar */
#define RSO_VER6_SIZE 276l
/* new fields: rso_menu_lmar, rso_menu_rmar, rso_menu_minspace, rso_menu_fillspace */
#define RSO_VER7_SIZE 284l
/* new fields: rso_flags2, rso_languages */
#define RSO_VER8_SIZE 292l
/* new fields: rso_overlay */
#define RSO_VER9_SIZE 324l
/* new fields: rso_rsc_crc, rso_rsc_modules, rso_rsc_langs */
#define RSO_VER10_MIN_SIZE 330l

#define RSO_VERSION 10
#define RSO_HEADER_SIZE RSO_VER10_MIN_SIZE

typedef enum {
	MOD_UNKNOWN = 0,
	MOD_OVL_EXTOB = 1,
	MOD_OVL_RSM = 2,
	MOD_OUT_RSM = 3,
	MOD_IN_RSM = 4
} module_type;

typedef struct rsc_module rsc_module;
struct rsc_module {
	module_type type;
	char *id;
	char *name;
	char *displayname;
	char *param1;
	char *param2;
	char *param3;
	char *param4;
	char *param5;
	_BOOL for_all_layers;
	_ULONG flags1;
	_ULONG flags2;
	_BOOL active;
	rsc_module *next;
};

typedef struct {
	_ULONG rso_magic;
	_ULONG rso_hsize;
	_UWORD rso_version;
	_ULONG rso_size;
	_ULONG rso_flags;
	_UWORD rso_namelen;
	NAMERULE rso_rule1;
	NAMERULE rso_rule2;
	/* Added in version 2, changed in version 5 from DOS format to Unix format: */
	_ULONG rso_date_created;
	_ULONG rso_date_changed;
	/* Added in version 6 format: */
	_UWORD rso_ted_fillchar;
	/* Added in version 7 format: */
	_UWORD rso_menu_leftmargin;
	_UWORD rso_menu_rightmargin;
	_UWORD rso_menu_minspace;
	_UWORD rso_menu_fillspace;
	/* Added in version 8 format: */
	_ULONG rso_flags2;
	_ULONG rso_languages; /* obsolete; was never actually used */
	/* Added in version 9 format: */
	_UBYTE rso_overlay[32 + 1];
	/* Added in version 10 format: */
	RSC_RSM_CRC rso_rsm_crc; /* RSM compatible crc over RSC file */
	rsc_module *rso_modules;
	rsc_lang *rso_langs;
	/* extend here */
	_ULONG rso_edition;
	_UWORD rso_checksum;
} RSO_HEADER;

#define RSO_ENDMARK 0xffff


#endif /* __ORCS_RSO_H__ */
