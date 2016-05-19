/**************************************************/
/*                                                */
/*  H E L P F I L E - D E C O M P I L E R  V 1.0  */
/*                                                */
/*  Headerfile HELP_RC.H                          */
/*                                                */
/*  Author: Volker Reichel                        */
/*         BÅhlstraûe 8                           */
/*         7507 Pfinztal 2                        */
/*                                                */
/*  Last change: 31.01.1992                       */
/**************************************************/


/*---------- General constants----------------*/
#define EOS        '\0'
#define CR         0x0d
#define LF         0x0a
#define ESC_CHR    0x1d
#define BACKSLASH  '\\'

/*---------- For the output of messages -------*/
#define TO_SCREEN   0x01
#define TO_LOG      0x02
#define TO_ALL      (TO_SCREEN + TO_LOG)


#define BOLD_ON         "\033E"
#define BOLD_OFF        "\033F"
#define FORM_FEED       "\f"

/*------ Constants for memory allocation ---------*/
#define TXTBUFSIZE      0x8000L
#define MAXCODEDSIZE    0x4000L

/*------ Constants for the decoding process ------*/
#define CHAR_DIR   0x0C
#define STR_TABLE  0x0E

/*-------- HC-version-dependent constants --------*/
#define INDEX_SCR   1   /* 2nd entry ScreenTab    */
#define HC_VERS "H2.0"  /* Help compiler version  */
#define INDEX_CNT   27  /* Entries in INDEX       */

/*----------- Attributes of a name ---------------*/
#define SCR_NAME    0             /* ScreenName   */
#define CAP_SENS    1 /* Caps/l.c. distinction    */ 
#define SENSITIVE   2   /* No distinction         */
#define LINK        3     /* is a \link-name      */
#define ATTR_CNT    4   /* Number of attributes   */

/*--------- Types of search-word tables ----------*/
#define CAP_TABLE   0
#define SENS_TABLE  1

/*------ Header structure of a help-file ---------*/

#define HEADER_SIZE  0x30  /* Length of header    */

typedef struct {
  int32_t scr_tab_size; /* Length of screen table    */
  int32_t str_offset;   /* String-table start        */
  int32_t str_size;              /* Length in bytes  */
  unsigned char char_table[12];  /* Most common character */
  int32_t caps_offset;    /* Start capsens-Table     */
  int32_t caps_size;             /* Length in Bytes  */
  int32_t caps_cnt;           /* No. of search-words */
  int32_t sens_offset;    /* Start sensitive-Tab.    */
  int32_t sens_size;             /* Length in bytes  */
  int32_t sens_cnt;           /* No. of search-words */
} HLPHDR;


/*--------- Description of an Index entry --------*/
typedef uint16_t SUB_IDX_ENTRY;

/*----------- Description of a name --------------*/
typedef struct name_entry {
  uint16_t scr_code;       /* Index-Code ScreenTab   */
  uint16_t link_index;   	/* link-Index ScreenTab   */
  unsigned int name_idx;
  unsigned char name_attr;      /* Attribute of the name  */
  char  *name;         		/* The name itself        */
  struct name_entry *next;  /* Follower     */
} NAME_ENTRY;

/*--------- Structure of the Keyword-Tables ---------*/
typedef struct {
  uint32_t pos;  /* Word-start for current position+pos */
  uint16_t code;       /* Word has this coding          */
} SRCHKEY_ENTRY;
#define SIZEOF_SRCHKEY_ENTRY 6

/*------------------ End of HELP_RC.H ---------------*/
