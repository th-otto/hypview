/*************************************************/
/*                                               */
/*  H E L P F I L E - D E C O M P I L E R  V1.0  */
/*                                               */
/*  Author: Volker Reichel                       */
/*     BÅhlstraûe 8                              */
/*     7507 Pfinztal 2                           */
/*                                               */
/*  Last changes: 31.01.1992                     */
/*************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#undef HAVE_GLIB
#undef HAVE_GTK

#include "hypdefs.h"
#include <portab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "help_rc.h"
#undef fclose

/*-------- VARIABLES: ---------------------------*/
/*-------- For management of name-tables --------*/
NAME_ENTRY *namelist = NULL;			/* Names found      */
int name_cnt = 0;						/* Number of names  */
NAME_ENTRY **name_array;				/* As table         */
NAME_ENTRY *link_list = NULL;			/* Link names found */
int link_cnt = 0;						/* Number of them   */

/*--------- For managing the subindexes ---------*/
SUB_IDX_ENTRY subidx_scrs[INDEX_CNT];

/*--------- The search-word tables --------------*/
SRCHKEY_ENTRY *key_table = NULL;
SRCHKEY_ENTRY *c_key_table = NULL;

/*---------- The screen-table -------------------*/
long *screen_table;						/* File-offsets of screens  */
int screen_cnt = 0;						/* Number of screens        */
unsigned char *screen_done;				/* 'Done' marking           */

/*---------- The string-table -------------------*/
unsigned char *string_tab;				/* Coded strings     */

/*---------- File-streams -----------------------*/
FILE *hlpfile = NULL;					/* Input help-file     */
FILE *txtfile = NULL;					/* Text output-file    */
FILE *scrfile = NULL;					/* Screen output-file  */
FILE *logfile = NULL;					/* Log-file            */

char filename[80];
char hlpname[80];						/* Name of help-file      */
char txtname[80];						/* Name of text-file      */
char scrname[80];						/* Name of screen-file    */
char logname[80];						/* Name of log-file       */

char *options;							/* Passed options       */
HLPHDR hlphdr;							/* Header of help-file   */

int glbref_cnt;							/* Number of external references */
int warnings;							/* Number of warnings issued     */
int errors;								/* Number of errors              */

/*------------- Some flags for control ----------*/
unsigned char log_flag = FALSE;
unsigned char txt_flag = FALSE;
unsigned char scr_flag = FALSE;

char msg[512];							/* Buffer for messages   */
char bold_on[80];
char bold_off[80];
char form_feed[80];

/*-------------- Some messages ------------------*/
#if !defined(ENABLE_NLS) && defined(NLS_LANG_GERMAN) && NLS_LANG_GERMAN
#define no_ram_msg      "\n*** Nicht genÅgend Speicherplatz"
#define ill_opt_msg     "\nIllegales Optionszeichen '%c'!\n"
#define log_opn_err     "\n\n*** Kann Logdatei %s nicht îffnen! ***\n"
#define hlp_nf_msg      "\n\n*** Help-Datei %s nicht gefunden! ***\n"
#define no_hf_msg       "\n\n*** Die Datei %s ist keine Help-Datei! ***\n"
#define hdr_err_msg     "\n\n*** Grîûe der HLPHDR-Struktur falsch! ***\n"
#define rd_sens_msg     "\n\tLese sensitive Suchworttabelle..."
#define rd_caps_msg     "\n\tLese capsensitive Suchworttabelle..."
#define rd_scr_msg      "\n\tLese Screen-Tabelle..."
#define rd_scr_err      "\n\n*** Kann ScreenTabelle nicht lesen! ***\n"
#define rd_str_msg      "\n\tLese String-Tabelle..."
#define rd_str_err      "\n\n*** Kann StringTabelle nicht lesen! ***\n"
#define rd_idx_msg      "\n\tLese Index-Screen..."
#define rd_idx_err      "\n\nKann Index nicht verarbeiten! ***\n"
#define set_attr_msg    "\n\tSetze Namensattribute..."
#define link_msg        "\n\tBearbeite \\link-Verweise..."
#define decomp_msg      "\n\tRekompiliere Screens..."
#define decomp_err      "\n\nKann Helpdatei nicht recompilieren!\n\n"
#define file_creat_err  "\n\nKann Datei %s nicht erzeugen!\n\n"
#define final_msg       "\n\n%d Fehler. %d Warnungen. %d Verweise in andere HELP-Dateien."
#define scr_cnt_msg     "\t%d Screens gefunden."
#define idx_warn_msg    "\n\n*** Mehr als 27 EintrÑge im Index! ***\n"
#define wr_nt_msg       "\n\tSchreibe Namenstabelle..."
#define wr_lk_msg       "\n\tSchreibe Link Tabelle..."
#define lk_head_msg     "\n\n%s\n\t\t\tLink Tabelle\n%s\n"
#define lk_cnt_msg      "\n%d Link-Verweise gefunden."
#define ill_code_msg    "\n\n*** UnzulÑssiger ScreenCode 0x%X! ***\n"
#define abort_msg       "\n******* Programm wird abgebrochen ********"
#define glb_ref_msg     "\n\t*** WARNUNG: Globaler Verweis <%s>"
#define nt_head_msg     "\n\n%s\n\t\t\tNamenstabelle\n%s\n"
#define name_cnt_msg    "\n%d Namen gefunden."
#define opt_msg         "Optionen (L=Log, S=SCR-Datei,T=TXT-Datei): "
#define maketxt_msg     "\n\tErzeuge Text-Datei..."
#define hlp_rc1			"\n                        HELPFILE RECOMPILER  Ver. 1.0 "
#define hlp_rc2         "\n                        =============================\n"
#define hlp_rc3         "\n                                (c) Volker Reichel\n\n"
#define option_msg      "Optionen: %s\n\n"
#define start_string_msg "\tStart String Tabelle\t: %ld (0x%lx)\n"
#define length_msg      "\tLÑnge\t\t\t\t\t: %ld (0x%lx)\n"
#define most_msg        "\tdie hÑufigsten Zeichen\t: %s\n"
#define start_sens_msg  "\tStart sensitive Tabelle\t: %ld (0x%lx)\n"
#define sens_words_msg  "\tAnz. sens. Worte\t: %ld (0x%lx)\n"
#define start_cap_msg   "\tStart capsens. Tabelle\t: %ld (0x%lx)\n"
#define cap_words_msg   "\tAnz. capsens Worte\t: %ld (0x%lx)\n"
#define keytable_msg    "        Position   Code  Suchwort\n"
#define capstab_msg     "\n\n%s\n\t\tCapsensitive Tabelle\n%s\n"
#define senstab_msg     "\n\n%s\n\t\tSensitive Tabelle\n%s\n"
#define from_msg        " von 0x%X"
#define ready_msg       "\nFertig."
#else
#define no_ram_msg      _("\n*** Insufficient memory")
#define ill_opt_msg     _("\nIllegal option character '%c'!\n")
#define log_opn_err     _("\n\n*** Cannot open log-file %s! ***\n")
#define hlp_nf_msg      _("\n\n*** Help-file %s not found! ***\n")
#define no_hf_msg       _("\n\n*** The file %s is not a help-file! ***\n")
#define hdr_err_msg     _("\n\n*** Size of HLPHDR-structure incorrect! ***\n")
#define rd_sens_msg     _("\n\tReading sensitive search-word table...")
#define rd_caps_msg     _("\n\tReading capsensitive search-word table...")
#define rd_scr_msg      _("\n\tReading screen-table...")
#define rd_scr_err      _("\n\n*** Cannot read screen-table! ***\n")
#define rd_str_msg      _("\n\tReading string-table...")
#define rd_str_err      _("\n\n*** Cannot read string-table! ***\n")
#define rd_idx_msg      _("\n\tReading index-screen...")
#define rd_idx_err      _("\n\nCannot process index! ***\n")
#define set_attr_msg    _("\n\tSetting name-atributes...")
#define link_msg        _("\n\tRevising \\link-cross-references...")
#define decomp_msg      _("\n\tDecompiling screens...")
#define decomp_err      _("\n\nCannot decompile help-file!\n\n")
#define file_creat_err  _("\n\nCannot create file %s!\n\n")
#define final_msg       _("\n\n%d Errors. %d Warnings. %d Cross-references to other HELP-files.")
#define scr_cnt_msg     _("\t%d screens found.")
#define idx_warn_msg    _("\n\n*** More than 27 entries in index! ***\n")
#define wr_nt_msg       _("\n\tWriting name-table...")
#define wr_lk_msg       _("\n\tWriting link-table...")
#define lk_head_msg     _("\n\n%s\n\t\t\tLink-table\n%s\n")
#define lk_cnt_msg      _("\n%d link cross-references found.")
#define ill_code_msg    _("\n\n*** Inadmissible screen-code 0x%X! ***\n")
#define abort_msg       _("\n******* Program will be aborted! *******")
#define glb_ref_msg     _("\n\t*** WARNING: Global cross-reference <%s>")
#define nt_head_msg     _("\n\n%s\n\t\t\tName-table\n%s\n")
#define name_cnt_msg    _("\n%d names found.")
#define opt_msg         _("Options (L=Log, S=SCR-file,T=TXT-file): ")
#define maketxt_msg     _("\n\tCreating text-file...")
#define hlp_rc1			_("\n                        HELPFILE DECOMPILER  Ver. 1.0 ")
#define hlp_rc2         _("\n                        =============================\n")
#define hlp_rc3         _("\n                                (c) Volker Reichel\n\n")
#define option_msg      _("Options: %s\n\n")
#define start_string_msg _("\tStart string-table\t: %ld (0x%lx)\n")
#define length_msg      _("\tLength\t\t\t\t\t: %ld (0x%lx)\n")
#define most_msg        _("\tThe most frequent characters\t: %s\n")
#define start_sens_msg  _("\tStart sensitive table\t: %ld (0x%lx)\n")
#define sens_words_msg  _("\tNumber of sensitive words\t: %ld (0x%lx)\n")
#define start_cap_msg   _("\tStart capsens. table\t: %ld (0x%lx)\n")
#define cap_words_msg   _("\tNumber of capsens words\t: %ld (0x%lx)\n")
#define keytable_msg    _("        Position   Code  Search-word\n")
#define capstab_msg     _("\n\n%s\n\t\tCapsensitive table\n%s\n")
#define senstab_msg     _("\n\n%s\n\t\tSensitive table\n%s\n")
#define from_msg        _(" from 0x%X")
#define ready_msg       _("\nFinished!")
#endif

/*----- 'get_nibble()' gets its data from here ------*/
unsigned char *curr_coded_text;
unsigned char must_read = TRUE;					/* For get_nibble    */

/*-----------------------------------------------------------*/
/*-------------- Prototypes ---------------------------------*/
/*-----------------------------------------------------------*/
void strfill(char *s, char c, int cnt);
void trans_bstr(char *s, unsigned char *bstr);
void read_info(void);
unsigned char get_nibble(void);
unsigned char get_byte(void);
long decode(int index, char *txtbuf);
void wr_header(void);
char *get_keyword(SRCHKEY_ENTRY *keytable, int i);
int read_header(void);
int read_screen_table(void);
int read_string_table(void);
int read_Index(void);
int read_Link(void);
int is_helpfile(void);
int get_name(char *pos, char *name);
_UWORD screen_index(_UWORD scr_code);
int is_dir_screen(long offset);
int rd_sidx_names(SUB_IDX_ENTRY subidx_code);
void ins_name(NAME_ENTRY **namelist, int *name_cnt, char *sname, _UWORD code, unsigned char attr, _UWORD lnk_idx);
int find_name(NAME_ENTRY *namelist, char *sname, NAME_ENTRY **pelem);
void corr_attrs(NAME_ENTRY *namelist);
void setup_namearr(NAME_ENTRY *namelist);
void order_nametable(NAME_ENTRY *namelist);
int write_names(NAME_ENTRY *namelist);
void wr_nametable(void);
void wr_linktable(void);
void transform(char *source, long length, char *d);
int decompile(void);
int make_txtfile(void);
void init_rc(void);
void get_options(void);
void wr_options(void);
void open_log(void);

/*--------- Some general routines --------------*/
void strfill(char *s, char c, int cnt)
{
	while (cnt-- > 0)
		*s++ = c;
	*s = EOS;
}


/*--------------------------------------------------------*/
/*  strin:                                                */
/*--------------------------------------------------------*/
/*  Tests whether character 'c' is present in string 'm'. */
/*--------------------------------------------------------*/
static int strin(char c, const char *m)
{
	while (*m && *m != c)
		m++;
	return (*m && *m == c);
}

/*----------------------------------------------------*/
/*  strint:                                           */
/*----------------------------------------------------*/
/*  Converts the string beginning at 's' into an      */
/*  integer value.                                    */
/*  If the first character of 's' is a '$', then a    */
/*  hex number will be expected. Otherwise a decimal  */
/*  number.                                           */
/*  After the call '*lp' points to the first          */
/*  character that does not belong to a number.       */
/*----------------------------------------------------*/
static int strint(const char *s, const char **lp)
{
	int value;
	int base = 10;

	value = 0;
	if (*s == '$')
	{
		base = 16;
		s++;
	}
	while (*s)
	{
		*lp = s;
		if (isdigit(*s))
		{
			value *= base;
			value += *s - '0';
		} else if (base == 16 && isxdigit(*s))
		{
			value *= 16;
			value += *s - 'A' + 10;
		} else
			break;
		s++;
	}
	return (value);
}


static void wr_msg(const char *s, unsigned char device)
{
	if (device & TO_SCREEN)
	{
		printf("%s", s);
	}
	if (log_flag && (device & TO_LOG))
		fprintf(logfile, "%s", s);
}


void trans_bstr(char *s, unsigned char * bstr)
{
	static const char *numbers = "0123456789ABCDEF";

	while (*bstr)
		if ((*bstr < 0x20) || (*bstr > 0x7F))
		{
			*s++ = '\\';
			*s++ = numbers[*bstr >> 4];
			*s++ = numbers[*bstr++ & 0x0F];
		} else
			*s++ = (char) *bstr++;
	*s = EOS;
}


void init_rc(void)
{
	char *p;

	glbref_cnt = 0;
	screen_cnt = 0;
	warnings = 0;
	errors = 0;
	p = strchr(filename, '.');
	if (p)
		*p = EOS;						/* Delete extensions */
	sprintf(hlpname, "%s.HLP", filename);
	sprintf(logname, "%s.LOG", filename);
	sprintf(txtname, "%s.TXT", filename);
	sprintf(scrname, "%s.SCR", filename);
}


void get_options(void)
{
	char *s;

	s = options;
	while (*s)
	{
		switch (*s++)
		{
		case 'L':
		case 'l':
			log_flag = TRUE;
			break;
		case 'T':
		case 't':
			txt_flag = TRUE;
			break;
		case 'S':
		case 's':
			scr_flag = TRUE;
			break;
		case '-':
		case ' ':
		case '\t':
			break;
		default:
			sprintf(msg, ill_opt_msg, *(s - 1));
			wr_msg(msg, TO_ALL);
			break;
		}
	}
	log_flag = log_flag || !(txt_flag || scr_flag);
}


/*--------------------------------------------------------*/
/*  read_info:                                            */
/*--------------------------------------------------------*/
/*  Reads from the file HELP_RC.INF the control sequences */
/*  to be used to adapt the output.                       */
/*  'bold_on', 'bold_off', 'form_feed' are used by        */
/*  'make_txtfile'.                                       */
/*  File format:                                          */
/*    Comments are preceded by '*'. They begin in the     */
/*    first column and finish at the end of the line.     */
/*    First comes the character string to switch on bold  */
/*    printing, followed by that for switching it off.    */
/*    At the end one also has to specify the character    */
/*    string to be used when changing screens.            */
/*    The character string is to be input as pairs of     */
/*    hex numbers, separated by commas. It finishes at    */
/*    the end of the line.                                */
/*--------------------------------------------------------*/
void read_info(void)
{
	FILE *info_file;
	int cnt = 0;
	char *s;
	const char *sp;
	const char *lp;

	char line[180];

	info_file = fopen("HELP_RC.INF", "r");
	if (info_file == NULL)
	{
		strcpy(bold_on, BOLD_ON);
		strcpy(bold_off, BOLD_OFF);
		strcpy(form_feed, FORM_FEED);
		return;
	}

  /*--------- File present ------------*/
	while (!feof(info_file) && (cnt < 3))
	{
		fgets(line, 80, info_file);
		if (*line && !strin(*line, "\n \t*"))
		{
			switch (cnt)
			{
			case 0:
				s = bold_on;
				break;
			case 1:
				s = bold_off;
				break;
			case 2:
				s = form_feed;
				break;
			}
			cnt++;
			lp = sp = line;
			while (*sp && strin(*sp, " \t,$"))
			{
				while (*sp && strin(*sp, " \t,"))
					sp++;
				if (*sp)
				{
					*s++ = strint(sp, &lp);
					sp = lp;
				}
			}							/* while */
			*s = '\0';
		}								/* if */
	}									/* while */
	fclose(info_file);
}



void wr_nametable(void)
{
	char bar[81];

	wr_msg(wr_nt_msg, TO_SCREEN);
	strfill(bar, '=', 80);
	fprintf(logfile, nt_head_msg, bar, bar);
	write_names(namelist);
	fprintf(logfile, name_cnt_msg, name_cnt);
}


void wr_linktable(void)
{
	char bar[81];

	wr_msg(wr_lk_msg, TO_SCREEN);
	strfill(bar, '=', 80);
	fprintf(logfile, lk_head_msg, bar, bar);
	write_names(link_list);
	fprintf(logfile, lk_cnt_msg, link_cnt);
}


void wr_options(void)
{
	sprintf(msg, option_msg, options);
	wr_msg(msg, TO_ALL);
}


void open_log(void)
{
	if (!logfile)
	{
		logfile = fopen(logname, "w");
		if (!logfile)
		{
			printf(log_opn_err, logname);
			errors++;
			log_flag = FALSE;
		} /* Log-file cannot be created */
		else
		{
			setvbuf(logfile, NULL, _IOFBF, 32 * 1024L);
			fprintf(logfile, "%s%s%s%s", hlp_rc1, __DATE__, hlp_rc2, hlp_rc3);
		}
	}									/* Logfile nicht geîffnet */
}


/*----------------------------------------------*/
/*  get_nibble:                                 */
/*----------------------------------------------*/
/*  Reads the next half-byte from the input.    */
/*  'curr_coded_text' points to the next byte.  */
/*----------------------------------------------*/
unsigned char get_nibble(void)
{
	static unsigned char byte_read;

	unsigned char nibble;

	if (must_read)
	{
		byte_read = *curr_coded_text++;
		nibble = byte_read >> 4;
		must_read = FALSE;
	} else
	{
		nibble = byte_read & 0x0F;
		must_read = TRUE;
	}
	return (nibble);
}


/*----------------------------------------------*/
/*  get_byte:                                   */
/*----------------------------------------------*/
/*  Reads the next two nibbles from the input   */
/*  and returns them as a byte.                 */
/*----------------------------------------------*/
unsigned char get_byte(void)
{
	unsigned char byte;

	byte = get_nibble();
	byte <<= 4;
	byte += get_nibble();
	return (byte);
}


void wr_header(void)
{
	char char_string[50];

	trans_bstr(char_string, (unsigned char *) hlphdr.char_table);
	fprintf(logfile, _("\nHeader of Helpfile %s\n\n"), hlpname);
	fprintf(logfile, _("\tScreens\t\t\t\t\t: %4ld\n"), hlphdr.scr_tab_size >> 2);
	fprintf(logfile, start_string_msg, hlphdr.str_offset, hlphdr.str_offset);
	fprintf(logfile, length_msg, hlphdr.str_size, hlphdr.str_size);
	fprintf(logfile, most_msg, char_string);
	fprintf(logfile, start_sens_msg, hlphdr.sens_offset, hlphdr.sens_offset);
	fprintf(logfile, length_msg, hlphdr.sens_size, hlphdr.sens_size);
	fprintf(logfile, sens_words_msg, hlphdr.sens_cnt, hlphdr.sens_cnt);
	fprintf(logfile, start_cap_msg, hlphdr.caps_offset, hlphdr.caps_offset);
	fprintf(logfile, length_msg, hlphdr.caps_size, hlphdr.caps_size);
	fprintf(logfile, cap_words_msg, hlphdr.caps_cnt, hlphdr.caps_cnt);
}


static int read_key_table(SRCHKEY_ENTRY ** ptable, int which)
{
	long offset;
	long size;

	*ptable = NULL;
	if (which == SENS_TABLE)
	{
		offset = hlphdr.sens_offset;
		size = hlphdr.sens_size;
		wr_msg(rd_sens_msg, TO_SCREEN);
	} else if (which == CAP_TABLE)
	{
		offset = hlphdr.caps_offset;
		size = hlphdr.caps_size;
		wr_msg(rd_caps_msg, TO_SCREEN);
	} else
	{
		abort();
	}

	if (size != 0)
	{
		fseek(hlpfile, offset, SEEK_SET);
		*ptable = (SRCHKEY_ENTRY *) g_malloc(size);
		if (*ptable != NULL)
			fread(*ptable, 1, size, hlpfile);
	}
	return (*ptable != NULL);
}


/*---------------------------------------------*/
/*  read_coded:                                */
/*---------------------------------------------*/
/*  Reads in the 'Index' screen.               */
/*---------------------------------------------*/
static int read_coded(int index, unsigned char *coded_text)
{
	long code_length,
	 bytes_read;

	code_length = screen_table[index + 1] - screen_table[index];
	fseek(hlpfile, screen_table[index], SEEK_SET);
	bytes_read = fread(coded_text, 1, code_length, hlpfile);
	return (bytes_read == code_length);
}


static void wr_keytable(SRCHKEY_ENTRY *table, int cnt)
{
	int i;

	fprintf(logfile, keytable_msg);
	for (i = 0; i < cnt; i++)
		fprintf(logfile, "           0x%08lx   %04x  \"%s\"\n", table[i].pos, table[i].code, get_keyword(table, i));
}



static void wr_keytables(void)
{
	char bar[81];

	if (hlphdr.caps_cnt > 0)
	{
		strfill(bar, 'c', 80);
		fprintf(logfile, capstab_msg, bar, bar);
		wr_keytable(c_key_table, (int) hlphdr.caps_cnt);
	}

	if (hlphdr.sens_cnt > 0)
	{
		strfill(bar, 's', 80);
		fprintf(logfile, senstab_msg, bar, bar);
		wr_keytable(key_table, (int) hlphdr.sens_cnt);
	}
}


/*----------------------------------------------*/
/*  get_keyword:                                */
/*----------------------------------------------*/
/*  Gets the 'i'th keyword from the search-word */
/*  table. ('i' starts from 0)                  */
/*----------------------------------------------*/
char *get_keyword(SRCHKEY_ENTRY *keytable, int i)
{
	return ((char *) (&keytable[i]) + keytable[i].pos);
}


/*----------------------------------------------*/
/*  corr_attrs:                                 */
/*----------------------------------------------*/
/*  Corrects the assumption that all names are  */
/*  screen-names. For this the attribute that   */
/*  corresponds to its affiliation to the       */
/*  search-word tables is set.                  */
/*----------------------------------------------*/
void corr_attrs(NAME_ENTRY *namelist)
{
	int i;
	char *search_name;
	NAME_ENTRY *elem;

	wr_msg(set_attr_msg, TO_SCREEN);
  /*----- First the sensitive names ------*/
	for (i = 0; i < hlphdr.sens_cnt; i++)
	{
		search_name = get_keyword(key_table, i);
		if (find_name(namelist, search_name, &elem))
			elem->name_attr = SENSITIVE;
	}

  /*----- Now for the capsensitive names */
	for (i = 0; i < hlphdr.caps_cnt; i++)
	{
		search_name = get_keyword(c_key_table, i);
		if (find_name(namelist, search_name, &elem))
			elem->name_attr = CAP_SENS;
	}
}


/*--------------------------------------------------*/
/*  decode:                                         */
/*--------------------------------------------------*/
/*  Decodes the screen given by ScreenTable[index]. */
/*  'plain_text' must point to a sufficiently       */
/*  large storage area.                             */
/*  The return is the length of the decoded screen. */
/*--------------------------------------------------*/
long decode(int index, char *plain_text)
{
	static unsigned char first_call = TRUE;
	static unsigned char *code_buffer = NULL;
	unsigned char nibble;
	_UWORD idx;
	_UWORD str_len;
	_ULONG offset;
	char *p;
	long size = 0L;

	if (first_call)
	{
		code_buffer = g_new(unsigned char, MAXCODEDSIZE);
		first_call = FALSE;
	}

	/*------------- Read the screen -------*/
	if (!read_coded(index, code_buffer))
		return (0L);

	curr_coded_text = code_buffer;
	must_read = TRUE;					/* No byte read yet */

	/*------------ Now also decode it -----------*/
	while (TRUE)
	{
		nibble = get_nibble();
		if (nibble == CHAR_DIR)
		{
			*plain_text++ = (char) get_byte();
			size++;
		} else if (nibble < CHAR_DIR)
		{
			*plain_text++ = hlphdr.char_table[nibble];
			size++;
		} else if (nibble == STR_TABLE)
		{
			*plain_text++ = (char) CR;
			*plain_text++ = (char) LF;
			size += 2;
		} else if (nibble < STR_TABLE)
		{
			idx = get_byte() << 4;
			idx += get_nibble();
			str_len = (_UWORD) (((long *) string_tab)[idx + 1] - ((long *) string_tab)[idx]);
			offset = ((long *) string_tab)[idx];
			p = (char *) string_tab + offset;
			size += str_len;
			while (str_len-- > 0)
			{
				*plain_text++ = (char) (*p ^ 0xA3);
				p++;
			}
		} else
		{
			*plain_text = EOS;
			break;
		}
	}
	return (size);
}


/*----------------------------------------------*/
/*  screen_index:                               */
/*----------------------------------------------*/
/*  Calculates from a reference code the index  */
/*  to the screen-table.                        */
/*----------------------------------------------*/
_UWORD screen_index(_UWORD scr_code)
{
	_UWORD index;

	if ((scr_code & 0x0004) > 0x0004)
	{
		sprintf(msg, ill_code_msg, scr_code);
		wr_msg(msg, TO_ALL);
		wr_msg(abort_msg, TO_ALL);
		errors++;
		exit(1);
	}
	index = (((scr_code & 0x7FF8) >> 1) - 4) >> 2;
	return (index);
}


/*----------------------------------------------*/
/*  get_name:                                   */
/*----------------------------------------------*/
/*  Returns in 'name' the string starting from  */
/*  'pos' and finishing at the next ESC_CHR.    */
/*  In addition the total length of the string  */
/*  is returned.                                */
/*----------------------------------------------*/
int get_name(char *pos, char *name)
{
	char *s;

	s = name;
	while (*pos != ESC_CHR)
		*s++ = *pos++;
	*s = '\0';
	return ((int) (s - name + 1));
}


/*-----------------------------------------------*/
/*  find_name:                                   */
/*-----------------------------------------------*/
/*  Searches in the name-list 'namelist' for the */
/*  name 'sname' and if successfull returns a    */
/*  pointer '*pelem' to the found entry.         */
/*-----------------------------------------------*/
int find_name(NAME_ENTRY * namelist, char *sname, NAME_ENTRY ** pelem)
{
	unsigned char found = FALSE;

	while ((namelist != NULL) && !found)
	{
		found = strcmp(namelist->name, sname) == 0;
		*pelem = namelist;
		namelist = namelist->next;
	}

	return (found);
}


/*----------------------------------------------*/
/*  find_code:                                  */
/*----------------------------------------------*/
/*  Searches in the name-table for the cross-   */
/*  reference 'code' code.                      */
/*----------------------------------------------*/
static int find_code(_UWORD search_code, NAME_ENTRY ** pelem)
{
	int i;

	for (i = 0; i < name_cnt; i++)
		if (name_array[i]->scr_code == search_code)
		{
			*pelem = name_array[i];
			return (TRUE);
		}
	return (FALSE);
}


int write_names(NAME_ENTRY * namelist)
{
	static const char *const attr_str[ATTR_CNT] = { "SCREEN_NAME ",
		"CAPSENSITIVE",
		" SENSITIVE  ",
		"   LINK     ",
	};
	int i = 0;

	fprintf(logfile, "Name\t\t\t\tAttribute    Code     ScreenOffset\n");

	while (namelist != NULL)
	{
		fprintf(logfile, "<%-32.32s> %s 0x%X", namelist->name, attr_str[namelist->name_attr], namelist->scr_code);
		if (namelist->name_attr == LINK)
			fprintf(logfile, from_msg, namelist->link_index);
		else
			fprintf(logfile, " = 0x%X", screen_index(namelist->scr_code));
		fprintf(logfile, "\n");
		namelist = namelist->next;
		i++;
	}
	return (i);
}


/*** ---------------------------------------------------------------------- ***/

#if !defined(g_strdup) && !defined(HAVE_GLIB)
char *g_strdup(const char *str)
{
	char *dst;
	
	if (str == NULL)
		return NULL;
	dst = g_new(char, strlen(str) + 1);
	if (dst == NULL)
		return NULL;
	return strcpy(dst, str);
}
#endif

/*-----------------------------------------------*/
/* ins_name:                                     */
/*-----------------------------------------------*/
/*  Inserts the name 'sname' with its attributes */
/*  code 'attr' and 'lnk_idx' into the name-list */
/*  '*namelist'. During this the number of       */
/*  insertions will be counted in '*name_cnt'.   */
/*-----------------------------------------------*/
void ins_name(NAME_ENTRY ** namelist, int *name_cnt, char *sname, _UWORD code, unsigned char attr, _UWORD lnk_idx)
{
	NAME_ENTRY *newentry;

	newentry = g_new(NAME_ENTRY, 1);
	if (!newentry)
	{
		sprintf(msg, "%s\n", no_ram_msg);
		wr_msg(msg, TO_ALL);
		wr_msg(abort_msg, TO_ALL);
		errors++;
		exit(1);
	}
	/* Insert at start of list */
	newentry->next = *namelist;
	newentry->name_attr = attr;
	newentry->scr_code = code;
	newentry->name = g_strdup(sname);
	if (attr == LINK)
		newentry->link_index = lnk_idx;
	*namelist = newentry;
	(*name_cnt)++;
}


/*-----------------------------------------------*/
/*  attr_cmp:                                    */
/*-----------------------------------------------*/
/* Compares two elements of the name-list for    */
/* their attribute-value.                        */
/*-----------------------------------------------*/
static int attr_cmp(const void *e1, const void *e2)
{
	const NAME_ENTRY *elem1 = *(const NAME_ENTRY *const *) e1;
	const NAME_ENTRY *elem2 = *(const NAME_ENTRY *const *) e2;
	_ULONG val1, val2;
	_UWORD idx1, idx2;

	idx1 = screen_index(elem1->scr_code);
	idx2 = screen_index(elem2->scr_code);

	val1 = (idx1 << 4) + elem1->name_attr;
	val2 = (idx2 << 4) + elem2->name_attr;

	return ((val1 < val2) ? -1 : (val1 > val2) ? 1 : 0);
}


/*-----------------------------------------*/
/*  setup_namearr:                         */
/*-----------------------------------------*/
/*  Creates a dynamic array and saves the  */
/*  names-list to it.                      */
/*-----------------------------------------*/
void setup_namearr(NAME_ENTRY * namelist)
{
	int arr_idx;

	name_array = g_new(NAME_ENTRY *, name_cnt);
	if (!name_array)
	{
		sprintf(msg, "\n%s\n", no_ram_msg);
		wr_msg(msg, TO_ALL);
		errors++;
		exit(1);
	}

	arr_idx = 0;
	while ((arr_idx < name_cnt) && (namelist != NULL))
	{
		name_array[arr_idx++] = namelist;
		namelist = namelist->next;
	}
}


/*-------------------------------------------------*/
/*  transform:                                     */
/*-------------------------------------------------*/
/*  Transforms the source so that it can be output */
/*-------------------------------------------------*/
void transform(char *source, long length, char *d)
{
	char *s,
	*limit;

	NAME_ENTRY *elem;

	char name[80];

	_UWORD code;

	unsigned char global = FALSE;				/* Global reference */

	s = source;
	limit = source + length;
	while (s < limit)
		switch (*s)
		{
		case ESC_CHR:
			code = (*(unsigned char *) (s + 1)) << 8;
			code += *(unsigned char *) (s + 2);
			s += 3;						/* Point to name */
			s += get_name(s, name);
			if (code == 0xFFFF)
			{
				sprintf(msg, glb_ref_msg, name);
				wr_msg(msg, TO_ALL);
				warnings++;
				glbref_cnt++;
				global = TRUE;
			}
			if (find_name(link_list, name, &elem) || global)
			{
				if (!global)
					find_code(elem->scr_code, &elem);
				strcpy(d, "\\link(\"");
				d += 7;
				if (global)
				{
					strcpy(d, "%%GLOBAL%%");
					d += 10;
					global = FALSE;
				} else
				{
					strcpy(d, elem->name);
					d += strlen(elem->name);
				}
				strcpy(d, "\")");
				d += 2;
				strcpy(d, name);
				d += strlen(name);
				strcpy(d, "\\#");
				d += 2;
			} else
			{
				strcpy(d, "\\#");
				d += 2;
				strcpy(d, name);
				d += strlen(name);
				strcpy(d, "\\#");
				d += 2;
			}
			break;

		case CR:
			*d++ = '\n';
			s += 2;						/* Jump over LF */
			break;

		case BACKSLASH:				/* Must be doubled up */
			*d++ = *s++;
			*d++ = '\\';
			break;
		default:
			*d++ = *s++;
			break;
		}

	*d = '\0';
}


/*----------------------------------------------*/
/*  decompile:                                  */
/*----------------------------------------------*/
/*  Re-creates a readable text from a HLP-file. */
/*----------------------------------------------*/
int decompile(void)
{
	int i = 0;
	_UWORD last_code;
	unsigned char new_screen = TRUE;
	char *result;
	char *textbuffer;
	long textlength;

	wr_msg(decomp_msg, TO_SCREEN);
	result = g_new(char, TXTBUFSIZE);
	if (!result)
	{
		sprintf(msg, "%s", no_ram_msg);
		wr_msg(msg, TO_ALL);
		errors++;
		return (FALSE);
	}

	textbuffer = g_new(char, MAXCODEDSIZE);
	if (!textbuffer)
	{
		sprintf(msg, "%s", no_ram_msg);
		wr_msg(msg, TO_ALL);
		errors++;
		return (FALSE);
	}

  /*----- Sort by attributes -----*/
	setup_namearr(namelist);
	qsort(name_array, name_cnt, sizeof(NAME_ENTRY *), attr_cmp);

	scrfile = fopen(scrname, "w");
	if (!scrfile)
	{
		sprintf(msg, file_creat_err, scrname);
		wr_msg(msg, TO_ALL);
		errors++;
		return (FALSE);
	}

	setvbuf(scrfile, NULL, _IOFBF, 32 * 1024L);

	last_code = name_array[0]->scr_code;
	while (i < name_cnt)
	{
		while ((i < name_cnt) && (name_array[i]->scr_code == last_code))
		{
			if (new_screen)
			{
				fprintf(scrfile, "\n\nscreen( ");
				new_screen = FALSE;
			} else
				fprintf(scrfile, ",\n\t\t");

			switch (name_array[i]->name_attr)
			{
			case SCR_NAME:
				fprintf(scrfile, "\"%s\"", name_array[i]->name);
				break;
			case SENSITIVE:
				fprintf(scrfile, "sensitive(\"%s\")", name_array[i]->name);
				break;
			case CAP_SENS:
				fprintf(scrfile, "capsensitive(\"%s\")", name_array[i]->name);
				break;
			}
			i++;
		}
		fprintf(scrfile, " )\n");
		textlength = decode(screen_index(last_code), textbuffer);
		transform(textbuffer, textlength, result);
		fputs(result, scrfile);
		fputs("\n\\end", scrfile);
		/*fprintf(scrfile,"%s\n\\end",result); */

		if (i < name_cnt)
			last_code = name_array[i]->scr_code;
		new_screen = TRUE;
	}									/* while */
	fprintf(scrfile, "\n");
	fclose(scrfile);
	return (TRUE);
}


/*----------------------------------------------*/
/*  make_txtfile:                               */
/*----------------------------------------------*/
/*  Lists all screens in order.                 */
/*  'bold_on' or 'bold_off' determine how       */
/*  cross-references are emphasised.            */
/*  'form_feed' determines how screens are to   */
/*  be separated.                               */
/*----------------------------------------------*/
int make_txtfile(void)
{
	int index;

	char *textbuffer,
	*tp,
	*limit;

	long size;

	wr_msg(maketxt_msg, TO_SCREEN);
	txtfile = fopen(txtname, "w");
	if (!txtfile)
	{
		sprintf(msg, file_creat_err, txtname);
		wr_msg(msg, TO_ALL);
		errors++;
		return (FALSE);
	}

	setvbuf(txtfile, NULL, _IOFBF, 32 * 1024L);

	textbuffer = g_new(char, TXTBUFSIZE);
	if (!textbuffer)
	{
		wr_msg(no_ram_msg, TO_ALL);
		errors++;
		return (FALSE);
	}

	for (index = 0; index < screen_cnt; index++)
	{
		if (!is_dir_screen(screen_table[index]))
		{
			size = decode(index, textbuffer);
			tp = textbuffer;
			limit = textbuffer + size;
			while (tp < limit)
			{
				switch (*tp)
				{
				case ESC_CHR:
					fputs(bold_on, txtfile);
					tp += 3;			/* Skip over cross-reference */
					while (*tp != ESC_CHR)
						fputc(*tp++, txtfile);
					fputs(bold_off, txtfile);
					tp++;
					break;
				case CR:
					tp += 2;			/* Skip over LF */
					fputc('\n', txtfile);
					break;
				default:
					fputc(*tp++, txtfile);
					break;
				}
			}
			fputs(form_feed, txtfile);
		}
	}

	g_free(textbuffer);
	fclose(txtfile);

	return (TRUE);
}


/*------------------------------------------------*/
/*  rd_sidx_names:                                */
/*------------------------------------------------*/
/* Enters all the names present in this sub-index */
/* screen into the name-table. The assumption     */
/* that they are all screen-names will be         */
/* corrected later.                               */
/*------------------------------------------------*/
int rd_sidx_names(SUB_IDX_ENTRY subidx_code)
{
	static char *plain_text = NULL;

	long size;

	char *pos;

	_UWORD scr_index;					/* Index in screen-table */

	_UWORD screen_code;					/* that belongs to the name */

	char screen_name[80];

	static unsigned char first_call = TRUE;

	if (first_call)
	{
		plain_text = g_new(char, MAXCODEDSIZE);
		first_call = FALSE;
	}
	if (!plain_text)
		return (FALSE);

	scr_index = screen_index(subidx_code);
	if (!screen_done[scr_index])
	{
		size = decode(scr_index, plain_text);

		/* Work through every entry of the IV */
		pos = plain_text;
		while (pos < plain_text + size)
		{
			if (*pos == ESC_CHR)
			{
				pos++;
				screen_code = *(unsigned char *) pos++ << 8;
				screen_code += *(unsigned char *) pos++;
				pos += get_name(pos, screen_name);
				ins_name(&namelist, &name_cnt, screen_name, screen_code, SCR_NAME, 0);
			} else
				pos++;
		}								/* while */
		screen_done[scr_index] = TRUE;
	}									/* if */
	return (TRUE);
}


/*----------------------------------------------*/
/*  is_dir_screen:                              */
/*----------------------------------------------*/
/* Determines whether the offset belongs to a   */
/* screen.                                      */
/*----------------------------------------------*/
int is_dir_screen(long offset)
{
	int i;

	for (i = 0; i < INDEX_CNT; i++)
		if (screen_table[screen_index(subidx_scrs[i])] == offset)
			return (TRUE);
	return (FALSE);
}


/*----------------------------------------------*/
/*  read_Link:                                  */
/*----------------------------------------------*/
/* Searches all screens except the Copyright-,  */
/* Index- and all Subindex-screens for the      */
/* cross-references they contain. If during     */
/* this a cross-reference is found that is not  */
/* contained in the name-table, then it is      */
/* certain that this reference has been         */
/* generated via a '\link' instruction, since   */
/* '\link' instructions do not produce entries  */
/* in the search-word tables. The reference     */
/* will be included in the link-list.           */
/*----------------------------------------------*/
int read_Link(void)
{
	int i;

	long size;

	char *pos;

	char name[80];

	_UWORD to_code;

	NAME_ENTRY *elem;

	static char *plain_text = NULL;

	static unsigned char first_call = TRUE;

	char *limit;

	wr_msg(link_msg, TO_SCREEN);
	if (first_call)
	{
		plain_text = g_new(char, TXTBUFSIZE);
		first_call = FALSE;
	}

	if (!plain_text)
		return (FALSE);

  /*--- Screen 0 Copyright screen 1 Index -----*/
	for (i = 2; i < (hlphdr.scr_tab_size >> 2) - 1; i++)
	{
	/*------ Is it a directory screen?---*/
		if (!is_dir_screen(screen_table[i]))
		{
	  /*------ Fetch page and decode it ---*/
			size = decode(i, plain_text);
	  /*----- Work through every name -----*/
	  /*----- entry of a screen -----------*/
			pos = plain_text;
			limit = plain_text + size;
			while (pos < limit)
			{
				if (*pos == ESC_CHR)
				{
					pos++;
					to_code = *(unsigned char *) pos++ << 8;
					to_code += *(unsigned char *) pos++;
					pos += get_name(pos, name);
					if (!find_name(namelist, name, &elem) && !find_name(link_list, name, &elem))
					{
						ins_name(&link_list, &link_cnt, name, to_code, LINK, i);
					}
				} /* if ESC_CHR */
				else
					pos++;
			}							/* while */
		}								/* if */
	}									/* for */
	return (TRUE);
}


/*----------------------------------------------*/
/*  read_Index:                                 */
/*----------------------------------------------*/
/* An Index-screen will be read and the screens */
/* belonging to the letters A to Z and the      */
/* 'Miscellaneous' entry will be determined.    */
/* Following this all names present on the Sub- */
/* index screens will be read in.               */
/*----------------------------------------------*/
int read_Index(void)
{

	char *plain_idx_text;				/* Decoded Index */

	long size;							/* Its length */

	char *limit;

	int sub_idx = 0;					/* Entry being worked on */

	int i;

	char *pos;

	char dummy[80];

	_UWORD screen_code;

	wr_msg(rd_idx_msg, TO_SCREEN);
	plain_idx_text = g_new(char, 0x1000L);
	if (!plain_idx_text)
		return (FALSE);

	screen_done[INDEX_SCR] = TRUE;
	size = decode(INDEX_SCR, plain_idx_text);

	/* Work through every entry of the Index */
	pos = plain_idx_text;
	limit = plain_idx_text + size;
	while (pos < limit)
	{
		if (*pos == ESC_CHR)
		{
			pos++;
			screen_code = *(unsigned char *) pos++ << 8;
			screen_code += *(unsigned char *) pos++;
			if (sub_idx >= INDEX_CNT)
			{
				wr_msg(idx_warn_msg, TO_ALL);
				warnings++;
			} else
				subidx_scrs[sub_idx] = screen_code;
			sub_idx++;
			pos += get_name(pos, dummy);
		} else
			pos++;
	}

	/* Now work through every Sub-index */
	for (i = 0; i < INDEX_CNT; i++)
		if (!rd_sidx_names(subidx_scrs[i]))
			return (FALSE);

	g_free(plain_idx_text);
	return (TRUE);
}


/*----------------------------------------------*/
/*  is_helpfile:                                */
/*----------------------------------------------*/
/* Are we dealing with an HC2.0 file?           */
/*----------------------------------------------*/
int is_helpfile(void)
{
	char buffer[4];

	fseek(hlpfile, 0x54L, SEEK_SET);
	fread(buffer, 1, 4L, hlpfile);
	return (!strncmp(buffer, HC_VERS, strlen(HC_VERS)));
}


/*------------------------------------------*/
/*  read_header:                            */
/*------------------------------------------*/
/*  Reads the description block from the    */
/*  help-file.                              */
/*------------------------------------------*/
int read_header(void)
{
	fseek(hlpfile, 0x58L, SEEK_SET);
	return (fread(&hlphdr, 1, sizeof(HLPHDR), hlpfile) == HEADER_SIZE);
}


/*-------------------------------------------*/
/*  read_screen_table:                       */
/*-------------------------------------------*/
/* Reads the table with the screen offsets   */
/* from the help-file.                       */
/*-------------------------------------------*/
int read_screen_table(void)
{
	long bytes_read;
	int i;

	wr_msg(rd_scr_msg, TO_SCREEN);
	fseek(hlpfile, 0x88L, SEEK_SET);
	screen_table = (long *) g_malloc(hlphdr.scr_tab_size);
	if (!screen_table)
	{
		sprintf(msg, "%s", no_ram_msg);
		wr_msg(msg, TO_ALL);
	}
	screen_cnt = (int) hlphdr.scr_tab_size >> 2;
	screen_done = g_new(unsigned char, screen_cnt);
	if (!screen_done)
	{
		sprintf(msg, "\n%s\n", no_ram_msg);
		wr_msg(msg, TO_ALL);
	}


	for (i = 0; i < screen_cnt; i++)
	{
		screen_done[i] = FALSE;
	}

	bytes_read = fread(screen_table, 1, hlphdr.scr_tab_size, hlpfile);
	return ((bytes_read == hlphdr.scr_tab_size) && screen_table && screen_done);
}


/*---------------------------------------------*/
/*  read_string_table:                         */
/*---------------------------------------------*/
/*  Reads in the table with the code-strings.  */
/*---------------------------------------------*/
int read_string_table(void)
{
	long bytes_read;

	wr_msg(rd_str_msg, TO_SCREEN);
	string_tab = g_new(unsigned char, hlphdr.str_size);
	if (!string_tab)
	{
		sprintf(msg, "%s", no_ram_msg);
		wr_msg(msg, TO_ALL);
	}
	fseek(hlpfile, hlphdr.str_offset, SEEK_SET);
	bytes_read = fread(string_tab, 1, hlphdr.str_size, hlpfile);
	return ((bytes_read == hlphdr.str_size) && string_tab);
}


int main(int argc, char *argv[])
{
	char buf[40];

	printf("%s%s%s", hlp_rc1, hlp_rc2, hlp_rc3);

	read_info();
	if (argc > 2)
	{
		options = argv[1];
		strcpy(filename, argv[2]);
	} else
	{
		printf("Name of the HELP-file: ");
		fgets(filename, (int)sizeof(filename), stdin);
		printf(opt_msg);
		fgets(buf, (int)sizeof(buf), stdin);
		options = buf;
	}
	init_rc();
	get_options();
	if (log_flag)
		open_log();
	wr_options();

	hlpfile = fopen(hlpname, "rb");
	if (!hlpfile)
	{
		printf(hlp_nf_msg, hlpname);
		errors++;
		goto end;
	}
	setvbuf(hlpfile, NULL, _IOFBF, 4 * 1024L);

	if (!is_helpfile())
	{
		printf(no_hf_msg, hlpname);
		errors++;
		goto end;
	}

	if (!read_header())
	{
		printf(hdr_err_msg);
		errors++;
		goto end;
	}
	if (log_flag)
		wr_header();

	read_key_table(&key_table, SENS_TABLE);
	read_key_table(&c_key_table, CAP_TABLE);
	if (log_flag)
		wr_keytables();

	if (!read_screen_table())
	{
		wr_msg(rd_scr_err, TO_ALL);
		errors++;
		goto end;
	}
	sprintf(msg, scr_cnt_msg, screen_cnt);
	wr_msg(msg, TO_SCREEN);

	if (!read_string_table())
	{
		wr_msg(rd_str_err, TO_ALL);
		errors++;
		goto end;
	}

	if (!read_Index())
	{
		wr_msg(rd_idx_err, TO_ALL);
		errors++;
		goto end;
	}

	corr_attrs(namelist);
	if (log_flag)
		wr_nametable();

	read_Link();
	if (log_flag)
		wr_linktable();

	if (scr_flag)
	{
		if (!decompile())
		{
			wr_msg(decomp_err, TO_ALL);
			errors++;
			goto end;
		}								/* if */
	}
	/* if */
	if (txt_flag)
	{
		if (!make_txtfile())
		{
			sprintf(msg, file_creat_err, txtname);
			wr_msg(msg, TO_ALL);
			errors++;
			goto end;
		}
	}

  end:

	sprintf(msg, final_msg, errors, warnings, glbref_cnt);
	wr_msg(msg, TO_ALL);
	fclose(hlpfile);
	if (log_flag)
		fclose(logfile);
	puts(ready_msg);
	return errors == 0 ? 0 : 1;
}
