/*****************************************************************************
 * EXTOB.H
 *****************************************************************************/

#ifndef __EXTOB_H__
#define __EXTOB_H__

#ifndef __PORTAB_H__
#  include <portab.h>
#endif

EXTERN_C_BEG


/*
 * Deklarationen fuer erweiterte Objekt-Typen
 */
typedef enum extob_mode {
	EXTOB_UNKNOWN = -1,
	EXTOB_NONE = 0,
	EXTOB_ORCS,
	EXTOB_FLYDIAL,
	EXTOB_MYDIAL,
	EXTOB_GEISS1,
	EXTOB_GEISS2,
	EXTOB_MAGIC,
	EXTOB_WEGA,
	EXTOB_WIN,
	EXTOB_HONKA,
	EXTOB_MAGIX,
	EXTOB_AES,
	EXTOB_FACEVALUE,
	EXTOB_SYSGEM,
	EXTOB_OVERLAY
} EXTOB_MODE;

#define EXTOB_MAX		EXTOB_OVERLAY


typedef enum extob_lang {
	RSC_LANG_NONE = -1,
	/* en   */  RSC_LANG_ENGLISH = 0,
	/* de   */  RSC_LANG_GERMAN,
	/* es   */  RSC_LANG_SPANISH,
	/* fr   */  RSC_LANG_FRENCH,
	/* it   */  RSC_LANG_ITALIAN,
	/* nl   */  RSC_LANG_DUTCH,

	RSC_LANG_MAX,
	/* en_GB */ RSC_LANG_ENGLISH_GB = 6,
	/* sv    */ RSC_LANG_SWEDISH,
	/* de_CH */ RSC_LANG_SWISS_GERMAN,
	/* fr_CH */ RSC_LANG_SWISS_FRENCH,
	/* tr    */ RSC_LANG_TURKISH,
	/* fi    */ RSC_LANG_FINNISH,
	/* nb    */ RSC_LANG_NORWEGIAN,
	/* da    */ RSC_LANG_DANISH,
	/* ar    */ RSC_LANG_ARABIC,
	/* cs    */ RSC_LANG_CZECH,
	/* hu    */ RSC_LANG_HUNGARIAN,
	/* pl    */ RSC_LANG_POLISH,
	/* lt    */ RSC_LANG_LITHUANIAN,
	/* ru    */ RSC_LANG_RUSSIAN,
	/* et    */ RSC_LANG_ESTONIAN,
	/* be    */ RSC_LANG_BELARUSIAN,
	/* uk    */ RSC_LANG_UKRAINIAN,
	/* sk    */ RSC_LANG_SLOVAK,
	/* ro    */ RSC_LANG_ROMANIAN,
	/* bg    */ RSC_LANG_BULGARIAN,
	/* sl    */ RSC_LANG_SLOVENIAN,
	/* hr    */ RSC_LANG_CROATIAN,
	/* sr    */ RSC_LANG_SERBIAN,
	/* sr_ME */ RSC_LANG_MONTENEGRIN,
	/* mk    */ RSC_LANG_MACEDONIAN,
	/* el    */ RSC_LANG_GREEK,
	/* lv    */ RSC_LANG_LATVIAN,
	/* he    */ RSC_LANG_HEBREW,
	/* af    */ RSC_LANG_AFRIKAANS_SOUTH_AFRICA,
	/* pt    */ RSC_LANG_PORTUGUESE,
	/* nl_BE */ RSC_LANG_BELGIAN,
	/* ja    */ RSC_LANG_JAPANESE,
	/* zh    */ RSC_LANG_CHINESE,
	/* ko    */ RSC_LANG_KOREAN,
	/* vi    */ RSC_LANG_VIETNAMESE,
	/* hi    */ RSC_LANG_HINDI,
	/* fa    */ RSC_LANG_FARSI,
	/* mn    */ RSC_LANG_MONGOLIAN,
	/* ne    */ RSC_LANG_NEPALI,
	/* lo    */ RSC_LANG_LAO,
	/* km    */ RSC_LANG_CAMBODIAN,
	/* id    */ RSC_LANG_INDONESIAN,
	/* bn    */ RSC_LANG_BENGALI,
	
	RSC_LANG_LAST
} RSC_LANG;
#define RSC_LANG_FIRST RSC_LANG_ENGLISH
#define RSC_LANG_DEFAULT RSC_LANG_ENGLISH

typedef struct _lang_arr {
	char *start;
	char *end;
} LANG_ARRAY[RSC_LANG_MAX];


typedef struct {
	EXTOB_MODE mode;
	RSC_LANG lang;
} EXTOB_OPTIONS;

enum ext_type {
	EXTTYPE_NONE,
	EXTTYPE_MOVER,
	EXTTYPE_RADIO,
	EXTTYPE_CHECK,
	EXTTYPE_CHECK_SWITCH,
	EXTTYPE_CHECK_BOOLEAN,
	EXTTYPE_POPUP,
	EXTTYPE_EXIT,
	EXTTYPE_DEFAULT,
	EXTTYPE_UNDO,
	EXTTYPE_LINE,
	EXTTYPE_INNERFRAME,
	EXTTYPE_OUTERFRAME,
	EXTTYPE_HELP,
	EXTTYPE_CIRCLE,
	EXTTYPE_UNDO2,
	EXTTYPE_CLOSER,
	
	/* Button */
	EXTTYPE_TITLE,
	EXTTYPE_SLIDE,
	EXTTYPE_THREESTATE,
	EXTTYPE_SYSGEM_HELP,
	EXTTYPE_SYSGEM_NOTEBOOK,
	EXTTYPE_SYSGEM_BUTTON,
	EXTTYPE_SYSGEM_RADIO,
	EXTTYPE_SYSGEM_SELECT,
	EXTTYPE_SYSGEM_TOUCHEXIT,
	EXTTYPE_SYSGEM_FRAME,
	EXTTYPE_SYSGEM_STRING,
	/* BOXTEXT */
	EXTTYPE_LIST_BOX,
	/* FTEXT | FBOXTEXT */
	EXTTYPE_LONGEDIT,
	EXTTYPE_SYSGEM_EDIT,
	/* TEXT | FTEXT | BOXTEXT | FBOXTEXT */
	EXTTYPE_LIST,
	EXTTYPE_SYSGEM_LISTBOX,
	EXTTYPE_SYSGEM_CIRCLE,
	EXTTYPE_SYSGEM_TEXT,
	
	/* BOX */
	EXTTYPE_SYSGEM_BAR1,
	EXTTYPE_SYSGEM_BAR2,
	EXTTYPE_SYSGEM_BAR3,
	EXTTYPE_SYSGEM_BOX,
	EXTTYPE_SYSGEM_BOXCHAR,
	
	/* BOXCHAR */
	EXTTYPE_OBJWIND,

	/* String */
	EXTTYPE_NICELINE,

	/* FTEXT, FBOXTEXT */
	EXTTYPE_POPUP_SIMPLE,
	EXTTYPE_POPUP_SINGLE,
	EXTTYPE_POPUP_MULTIPLE,
	
	/* I-Box */
	EXTTYPE_FLYMOVER
};

#define MYDIAL_UNDO_FLAG 0x0800

#define G_OBJX_MY_USERDEF       98

/* G_BUTTON */
#define G_OBJX_SLIDE 		    50
#define G_OBJX_SGSCROLLBAR	    51

#define G_OBJX_MOVER            17
#define G_OBJX_SHORTCUT         18
#define G_OBJX_LINE             19
#define G_OBJX_FRAME            20
#define G_OBJX_NICELINE         21
#define G_OBJX_NOTEBOOK         21
#define G_OBJX_CIRCLE           22
#define G_OBJX_THREESTATE       23
#define G_OBJX_LONGEDIT         24
#define G_OBJX_DEFAULT          31

/* G_FTEXT | G_FBOXTEXT */
#define G_OBJX_POPUP_MULTIPLE   28
#define G_OBJX_POPUP_SINGLE     29
#define G_OBJX_POPUP_SIMPLE     30
#define G_OBJX_TITLE            31
#define G_OBJX_LIST	           	17		/* G_FBOXTEXT */
#define G_OBJX_LIST_BOX         99

#define G_OBJX_NOLANG           1

#define G_OBJX_MUST_CONVERT (0x01 << 8)

enum ext_type is_ext_type(const EXTOB_OPTIONS *extob, _UWORD type, _UWORD flags, _UWORD state);


EXTERN_C_END

#endif /* __EXTOB_H__ */

