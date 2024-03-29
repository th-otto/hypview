/*************************************************/
/*                                               */
/*  H E L P F I L E - D E C O M P I L E R  V1.0  */
/*                                               */
/*  Author: Volker Reichel                       */
/*     B�hlstra�e 8                              */
/*     7507 Pfinztal 2                           */
/*                                               */
/*  Last changes: 31.01.1992                     */
/*************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#undef HAVE_GLIB
#undef HAVE_GTK
#undef ENABLE_NLS

#include "hypdefs.h"
#include <portab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include "help_rc.h"
#undef fclose

/*-------- VARIABLES: ---------------------------*/
/*-------- For management of name-tables --------*/
static NAME_ENTRY *namelist = NULL;			/* Names found      */
static int name_cnt = 0;					/* Number of names  */
static NAME_ENTRY **name_array;				/* As table         */
static NAME_ENTRY *link_list = NULL;		/* Link names found */
static int link_cnt = 0;					/* Number of them   */

/*--------- For managing the subindexes ---------*/
static SUB_IDX_ENTRY subidx_scrs[INDEX_CNT];

/*--------- The search-word tables --------------*/
static SRCHKEY_ENTRY *key_table = NULL;
static SRCHKEY_ENTRY *c_key_table = NULL;

/*---------- The screen-table -------------------*/
static int32_t *screen_table;			/* File-offsets of screens  */
static int screen_cnt = 0;				/* Number of screens        */
static unsigned char *screen_done;		/* 'Done' marking           */

/*---------- The string-table -------------------*/
unsigned char *string_tab;				/* Coded strings     */

/*---------- File-streams -----------------------*/
static FILE *hlpfile = NULL;			/* Input help-file     */
static FILE *txtfile = NULL;			/* Text output-file    */
static FILE *scrfile = NULL;			/* Screen output-file  */
static FILE *logfile = NULL;			/* Log-file            */

static char filename[80];
static char hlpname[80+4];				/* Name of help-file      */
static char txtname[80+4];				/* Name of text-file      */
static char scrname[80+4];				/* Name of screen-file    */
static char logname[80+4];				/* Name of log-file       */

static char *options;					/* Passed options       */
static HLPHDR hlphdr;					/* Header of help-file   */

static int glbref_cnt;					/* Number of external references */
static int warnings;					/* Number of warnings issued     */
static int errors;						/* Number of errors              */

/*------------- Some flags for control ----------*/
static unsigned char log_flag = FALSE;
static unsigned char txt_flag = FALSE;
static unsigned char scr_flag = FALSE;

static char bold_on[80];
static char bold_off[80];
static char form_feed[80];

/*-------------- Some messages ------------------*/
#if !defined(ENABLE_NLS) && defined(NLS_LANG_GERMAN) && NLS_LANG_GERMAN
#define ill_opt_msg     "\nIllegales Optionszeichen '%c'!\n"
#define log_opn_err     "\n\n*** Kann Logdatei %s nicht �ffnen! ***\n"
#define hlp_nf_msg      "\n\n*** Help-Datei %s nicht gefunden! ***\n"
#define no_hf_msg       "\n\n*** Die Datei %s ist keine Help-Datei! ***\n"
#define hdr_err_msg     "\n\n*** Gr��e der HLPHDR-Struktur falsch! ***\n"
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
#define idx_warn_msg    "\n\n*** Mehr als 27 Eintr�ge im Index! ***\n"
#define wr_nt_msg       "\n\tSchreibe Namenstabelle..."
#define wr_lk_msg       "\n\tSchreibe Link Tabelle..."
#define lk_head_msg     "\n\n%s\n\t\t\tLink Tabelle\n%s\n"
#define lk_cnt_msg      "\n%d Link-Verweise gefunden."
#define ill_code_msg    "\n\n*** Unzul�ssiger ScreenCode 0x%X! ***\n"
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
#define length_msg      "\tL�nge\t\t\t\t\t: %ld (0x%lx)\n"
#define most_msg        "\tdie h�ufigsten Zeichen\t: %s\n"
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
#define start_string_msg _("    Start string-table           : %ld (0x%lx)\n")
#define length_msg      _("    Length                       : %ld (0x%lx)\n")
#define most_msg        _("    The most frequent characters : %s\n")
#define start_sens_msg  _("    Start sensitive table        : %ld (0x%lx)\n")
#define sens_words_msg  _("    Number of sensitive words    : %ld (0x%lx)\n")
#define start_cap_msg   _("    Start capsens. table         : %ld (0x%lx)\n")
#define cap_words_msg   _("    Number of capsens words      : %ld (0x%lx)\n")
#define keytable_msg    _("        Position     Code  Search-word\n")
#define capstab_msg     _("\n\n%s\n        Capsensitive table\n%s\n")
#define senstab_msg     _("\n\n%s\n        Sensitive table\n%s\n")
#define from_msg        _(" from 0x%X")
#define ready_msg       _("\nFinished!")
#endif

/*----- 'get_nibble()' gets its data from here ------*/
static unsigned char *code_buffer = NULL;
static char *plain_text;
static unsigned char *curr_coded_text;
static unsigned char must_read = TRUE;					/* For get_nibble    */
static unsigned char byte_read;


#if !defined(__TOS__) && !defined(__atarist__)
# define MUST_SWAP 1
#endif


static int read_int_16(const unsigned char *src)
{
	return (src[0] << 8) | src[1];
}

static int32_t read_int_32(const unsigned char *src)
{
	return ((uint32_t)src[0] << 24) |
	       ((uint32_t)src[1] << 16) |
	       ((uint32_t)src[2] <<  8) |
	       ((uint32_t)src[3]      );
}

/*--------- Some general routines --------------*/
static void strfill(char *s, char c, int cnt)
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
	return *m && *m == c;
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
		if (isdigit((unsigned char)*s))
		{
			value *= base;
			value += *s - '0';
		} else if (base == 16 && isxdigit((unsigned char)*s))
		{
			value *= 16;
			value += *s - 'A' + 10;
		} else
			break;
		s++;
	}
	return value;
}


__attribute__((format(printf, 2, 3)))
static void wr_msg(unsigned char device, const char *s, ...)
{
	va_list args;
	
	if (device & TO_SCREEN)
	{
		va_start(args, s);
		vprintf(s, args);
		va_end(args);
	}
	if (log_flag && (device & TO_LOG))
	{
		va_start(args, s);
		vfprintf(logfile, s, args);
		va_end(args);
	}
}


static void trans_bstr(char *s, const unsigned char *bstr, int len)
{
	static const char *numbers = "0123456789ABCDEF";
	int i;
	
	i = 0;
	while (i < len && *bstr)
	{
		if ((*bstr < 0x20) || (*bstr > 0x7F))
		{
			*s++ = '\\';
			*s++ = numbers[*bstr >> 4];
			*s++ = numbers[*bstr++ & 0x0F];
		} else
		{
			*s++ = (char) *bstr++;
		}
		i++;
	}
	*s = EOS;
}


static void init_rc(void)
{
	char *p;

	glbref_cnt = 0;
	screen_cnt = 0;
	warnings = 0;
	errors = 0;
	strcpy(hlpname, filename);
	p = strrchr(filename, '.');
	if (p)
		*p = EOS;						/* Delete extensions */
	sprintf(logname, "%s.log", filename);
	sprintf(txtname, "%s.txt", filename);
	sprintf(scrname, "%s.scr", filename);
}


static void get_options(void)
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
			wr_msg(TO_ALL, ill_opt_msg, *(s - 1));
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
static void read_info(void)
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
			default:
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
			}
			*s = '\0';
		}
	}
	fclose(info_file);
}



/*----------------------------------------------*/
/*  screen_index:                               */
/*----------------------------------------------*/
/*  Calculates from a reference code the index  */
/*  to the screen-table.                        */
/*----------------------------------------------*/
static uint16_t screen_index(uint16_t scr_code)
{
	uint16_t index;

	if ((scr_code & 0x0004) > 0x0004)
	{
		wr_msg(TO_ALL, ill_code_msg, scr_code);
		wr_msg(TO_ALL, abort_msg);
		errors++;
		exit(1);
	}
	index = (((scr_code & 0x7FF8) >> 1) - 4) >> 2;
	return index;
}


static int write_names(NAME_ENTRY *namelist)
{
	static const char *const attr_str[ATTR_CNT] = {
		"SCREEN_NAME ",
		"CAPSENSITIVE",
		" SENSITIVE  ",
		"   LINK     ",
	};
	int i = 0;

	fprintf(logfile, _("Name                               Attribute    Code     ScreenOffset\n"));

	while (namelist != NULL)
	{
		fprintf(logfile, "<%-32.32s> %s 0x%04X", namelist->name, attr_str[namelist->name_attr], namelist->scr_code);
		if (namelist->name_attr == LINK)
			fprintf(logfile, from_msg, namelist->link_index);
		else
			fprintf(logfile, " = 0x%X", screen_index(namelist->scr_code));
		fprintf(logfile, "\n");
		namelist = namelist->next;
		i++;
	}
	return i;
}


static void wr_nametable(void)
{
	char bar[81];

	wr_msg(TO_SCREEN, wr_nt_msg);
	strfill(bar, '=', 80);
	fprintf(logfile, nt_head_msg, bar, bar);
	write_names(namelist);
	fprintf(logfile, name_cnt_msg, name_cnt);
}


static void wr_linktable(void)
{
	char bar[81];

	wr_msg(TO_SCREEN, wr_lk_msg);
	strfill(bar, '=', 80);
	fprintf(logfile, lk_head_msg, bar, bar);
	write_names(link_list);
	fprintf(logfile, lk_cnt_msg, link_cnt);
}


static void wr_options(void)
{
	wr_msg(TO_ALL, option_msg, options);
}


static void open_log(void)
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
	}
}


/*----------------------------------------------*/
/*  get_nibble:                                 */
/*----------------------------------------------*/
/*  Reads the next half-byte from the input.    */
/*  'curr_coded_text' points to the next byte.  */
/*----------------------------------------------*/
static unsigned char get_nibble(void)
{
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
	return nibble;
}


/*----------------------------------------------*/
/*  get_byte:                                   */
/*----------------------------------------------*/
/*  Reads the next two nibbles from the input   */
/*  and returns them as a byte.                 */
/*----------------------------------------------*/
static unsigned char get_byte(void)
{
	unsigned char byte;

	byte = get_nibble();
	byte <<= 4;
	byte += get_nibble();
	return byte;
}


static void wr_header(void)
{
	char char_string[12 * 3 + 1];

	trans_bstr(char_string, hlphdr.char_table, 12);
	fprintf(logfile, _("\nHeader of Helpfile %s\n\n"), hlpname);
	fprintf(logfile, _("    Screens                      : %4ld\n"), (long)hlphdr.scr_tab_size >> 2);
	fprintf(logfile, start_string_msg, (long)hlphdr.str_offset, (long)hlphdr.str_offset);
	fprintf(logfile, length_msg, (long)hlphdr.str_size, (long)hlphdr.str_size);
	fprintf(logfile, most_msg, char_string);
	fprintf(logfile, start_sens_msg, (long)hlphdr.sens_offset, (long)hlphdr.sens_offset);
	fprintf(logfile, length_msg, (long)hlphdr.sens_size, (long)hlphdr.sens_size);
	fprintf(logfile, sens_words_msg, (long)hlphdr.sens_cnt, (long)hlphdr.sens_cnt);
	fprintf(logfile, start_cap_msg, (long)hlphdr.caps_offset, (long)hlphdr.caps_offset);
	fprintf(logfile, length_msg, (long)hlphdr.caps_size, (long)hlphdr.caps_size);
	fprintf(logfile, cap_words_msg, (long)hlphdr.caps_cnt, (long)hlphdr.caps_cnt);
}


static int read_key_table(SRCHKEY_ENTRY **ptable, int which)
{
	int32_t offset;
	int32_t size;
	SRCHKEY_ENTRY *table;
	
	table = NULL;
	if (which == SENS_TABLE)
	{
		offset = hlphdr.sens_offset;
		size = hlphdr.sens_size;
		wr_msg(TO_SCREEN, rd_sens_msg);
	} else if (which == CAP_TABLE)
	{
		offset = hlphdr.caps_offset;
		size = hlphdr.caps_size;
		wr_msg(TO_SCREEN, rd_caps_msg);
	} else
	{
		abort();
	}

	fseek(hlpfile, offset, SEEK_SET);
	if (size != 0)
	{
		table = (SRCHKEY_ENTRY *)g_malloc(size);
		if (table != NULL)
			if ((int32_t)fread(table, 1, size, hlpfile) != size)
				return FALSE;
	}
	*ptable = table;
	return table != NULL;
}


/*---------------------------------------------*/
/*  read_coded:                                */
/*---------------------------------------------*/
/*  Reads in the 'Index' screen.               */
/*---------------------------------------------*/
static int read_coded(int index, unsigned char *coded_text)
{
	int32_t code_length, bytes_read;

	code_length = screen_table[index + 1] - screen_table[index];
	fseek(hlpfile, screen_table[index], SEEK_SET);
	bytes_read = fread(coded_text, 1, code_length, hlpfile);
	return bytes_read == code_length;
}


/*----------------------------------------------*/
/*  get_keyword:                                */
/*----------------------------------------------*/
/*  Gets the 'i'th keyword from the search-word */
/*  table. ('i' starts from 0)                  */
/*----------------------------------------------*/
static char *get_keyword(SRCHKEY_ENTRY *keytable, int i)
{
	unsigned char *p = (unsigned char *)keytable + i * SIZEOF_SRCHKEY_ENTRY;
	return (char *) (p + read_int_32(p));
}


static void wr_keytable(SRCHKEY_ENTRY *table, int cnt)
{
	int i;

	fprintf(logfile, keytable_msg);
	for (i = 0; i < cnt; i++)
	{
		unsigned char *p = (unsigned char *)table + i * SIZEOF_SRCHKEY_ENTRY;
		fprintf(logfile, "        0x%08lx   %04x  \"%s\"\n", (unsigned long)read_int_32(p), read_int_16(p + 4), get_keyword(table, i));
	}
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


/*-----------------------------------------------*/
/*  find_name:                                   */
/*-----------------------------------------------*/
/*  Searches in the name-list 'namelist' for the */
/*  name 'sname' and if successfull returns a    */
/*  pointer '*pelem' to the found entry.         */
/*-----------------------------------------------*/
static int find_name(NAME_ENTRY *namelist, char *sname, NAME_ENTRY **pelem)
{
	unsigned char found = FALSE;

	while ((namelist != NULL) && !found)
	{
		found = strcmp(namelist->name, sname) == 0;
		*pelem = namelist;
		namelist = namelist->next;
	}

	return found;
}


/*----------------------------------------------*/
/*  corr_attrs:                                 */
/*----------------------------------------------*/
/*  Corrects the assumption that all names are  */
/*  screen-names. For this the attribute that   */
/*  corresponds to its affiliation to the       */
/*  search-word tables is set.                  */
/*----------------------------------------------*/
static void corr_attrs(NAME_ENTRY *namelist)
{
	int i;
	char *search_name;
	NAME_ENTRY *elem = NULL;

	wr_msg(TO_SCREEN, set_attr_msg);
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
static int32_t decode(int index, char *plain_text)
{
	unsigned char nibble;
	uint16_t idx;
	uint16_t str_len;
	uint32_t offset;
	char *p;
	int32_t size = 0;

	/*------------- Read the screen -------*/
	if (!read_coded(index, code_buffer))
		return 0;

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
			offset = read_int_32(string_tab + (idx << 2));
			str_len = (uint16_t) (read_int_32(string_tab + ((idx + 1u) << 2)) - offset);
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
	return size;
}


/*----------------------------------------------*/
/*  get_name:                                   */
/*----------------------------------------------*/
/*  Returns in 'name' the string starting from  */
/*  'pos' and finishing at the next ESC_CHR.    */
/*  In addition the total length of the string  */
/*  is returned.                                */
/*----------------------------------------------*/
static int get_name(char *pos, char *name)
{
	char *s;

	s = name;
	while (*pos != ESC_CHR)
		*s++ = *pos++;
	*s = '\0';
	return (int) (s - name + 1);
}


/*----------------------------------------------*/
/*  find_code:                                  */
/*----------------------------------------------*/
/*  Searches in the name-table for the cross-   */
/*  reference 'code' code.                      */
/*----------------------------------------------*/
static int find_code(uint16_t search_code)
{
	int i;

	for (i = 0; i < name_cnt; i++)
		if (name_array[i]->scr_code == search_code)
		{
			return i;
		}
	return -1;
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
static void ins_name(NAME_ENTRY **namelist, int *name_cnt, char *sname, uint16_t code, unsigned char attr, uint16_t lnk_idx)
{
	NAME_ENTRY *newentry;

	newentry = g_new(NAME_ENTRY, 1);
	if (!newentry)
	{
		wr_msg(TO_ALL, "%s\n", strerror(errno));
		wr_msg(TO_ALL, abort_msg);
		errors++;
		exit(1);
	}
	/* Insert at start of list */
	newentry->next = *namelist;
	newentry->name_attr = attr;
	newentry->scr_code = code;
	newentry->name = g_strdup(sname);
	newentry->link_index = 0;
	if (attr == LINK)
		newentry->link_index = lnk_idx;
	newentry->name_idx = *name_cnt;
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
	uint32_t val1, val2;
	uint16_t idx1, idx2;

	idx1 = screen_index(elem1->scr_code);
	idx2 = screen_index(elem2->scr_code);

	val1 = (idx1 << 4) + elem1->name_attr;
	val2 = (idx2 << 4) + elem2->name_attr;

	if (val1 < val2)
		return -1;
	if (val1 > val2)
		return 1;
	/*
	 * this will keep the order of screen names that refer to the same page
	 */
	return elem1->name_idx < elem2->name_idx ? -1 : 1;
}


/*-----------------------------------------*/
/*  setup_namearr:                         */
/*-----------------------------------------*/
/*  Creates a dynamic array and saves the  */
/*  names-list to it.                      */
/*-----------------------------------------*/
static void setup_namearr(NAME_ENTRY *namelist)
{
	int arr_idx;

	name_array = g_new(NAME_ENTRY *, name_cnt);
	if (!name_array)
	{
		wr_msg(TO_ALL, "\n%s\n", strerror(errno));
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
static void transform(char *source, int32_t length, char *d)
{
	char *s, *limit;
	NAME_ENTRY *elem;
	char name[80];
	uint16_t code;
	int global = FALSE;				/* Global reference */
	int found;
	int same_name;
	
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
				wr_msg(TO_ALL, glb_ref_msg, name);
				warnings++;
				glbref_cnt++;
				global = TRUE;
			}
			found = find_code(code);
			if (found >= 0)
			{
				elem = name_array[found];
				same_name = FALSE;
				while (!same_name && found < name_cnt && name_array[found]->scr_code == code)
				{
					same_name = strcmp(name, name_array[found]->name) == 0;
					found++;
				}
			} else
			{
				same_name = TRUE;
				elem = NULL;
			}
			if (!same_name || global)
			{
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
static int decompile(void)
{
	int i = 0;
	uint16_t last_code;
	unsigned char new_screen = TRUE;
	char *result;
	char *textbuffer;
	int32_t textlength;

	wr_msg(TO_SCREEN, decomp_msg);
	result = g_new(char, TXTBUFSIZE);
	if (!result)
	{
		wr_msg(TO_ALL, "%s", strerror(errno));
		errors++;
		return FALSE;
	}

	textbuffer = g_new(char, MAXCODEDSIZE);
	if (!textbuffer)
	{
		wr_msg(TO_ALL, "%s", strerror(errno));
		errors++;
		return FALSE;
	}

	/*----- Sort by attributes -----*/
	setup_namearr(namelist);
	qsort(name_array, name_cnt, sizeof(NAME_ENTRY *), attr_cmp);

	scrfile = fopen(scrname, "w");
	if (!scrfile)
	{
		wr_msg(TO_ALL, file_creat_err, scrname);
		errors++;
		return FALSE;
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
	}
	fprintf(scrfile, "\n");
	fclose(scrfile);
	
	g_free(textbuffer);
	g_free(result);
	
	return TRUE;
}


/*----------------------------------------------*/
/*  is_dir_screen:                              */
/*----------------------------------------------*/
/* Determines whether the offset belongs to a   */
/* screen.                                      */
/*----------------------------------------------*/
static int is_dir_screen(int32_t offset)
{
	int i;

	for (i = 0; i < INDEX_CNT; i++)
		if (screen_table[screen_index(subidx_scrs[i])] == offset)
			return TRUE;
	return FALSE;
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
static int make_txtfile(void)
{
	int index;
	char *textbuffer, *tp, *limit;
	int32_t size;

	wr_msg(TO_SCREEN, maketxt_msg);
	txtfile = fopen(txtname, "w");
	if (!txtfile)
	{
		wr_msg(TO_ALL, file_creat_err, txtname);
		errors++;
		return FALSE;
	}

	setvbuf(txtfile, NULL, _IOFBF, 32 * 1024L);

	textbuffer = g_new(char, TXTBUFSIZE);
	if (!textbuffer)
	{
		wr_msg(TO_ALL, "%s", strerror(errno));
		errors++;
		return FALSE;
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

	return TRUE;
}


/*------------------------------------------------*/
/*  rd_sidx_names:                                */
/*------------------------------------------------*/
/* Enters all the names present in this sub-index */
/* screen into the name-table. The assumption     */
/* that they are all screen-names will be         */
/* corrected later.                               */
/*------------------------------------------------*/
static int rd_sidx_names(SUB_IDX_ENTRY subidx_code)
{
	int32_t size;
	char *pos;
	uint16_t scr_index;					/* Index in screen-table */
	uint16_t screen_code;					/* that belongs to the name */
	char screen_name[80];

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
			{
				pos++;
			}
		}
		screen_done[scr_index] = TRUE;
	}
	return TRUE;
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
static int read_Link(void)
{
	int i;
	int32_t size;
	char *pos;
	char name[80];
	uint16_t to_code;
	NAME_ENTRY *elem;
	char *limit;

	wr_msg(TO_SCREEN, link_msg);

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
				} else
				{
					pos++;
				}
			}
		}
	}
	return TRUE;
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
static int read_Index(void)
{
	char *plain_idx_text;				/* Decoded Index */
	int32_t size;						/* Its length */
	char *limit;
	int sub_idx = 0;					/* Entry being worked on */
	int i;
	char *pos;
	char dummy[80];
	uint16_t screen_code;

	wr_msg(TO_SCREEN, rd_idx_msg);
	plain_idx_text = g_new(char, 0x1000L);
	if (!plain_idx_text)
		return FALSE;

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
				wr_msg(TO_ALL, idx_warn_msg);
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
			return FALSE;

	g_free(plain_idx_text);
	return TRUE;
}


/*----------------------------------------------*/
/*  is_helpfile:                                */
/*----------------------------------------------*/
/* Are we dealing with an HC2.0 file?           */
/*----------------------------------------------*/
static int is_helpfile(void)
{
	char buffer[4];

	fseek(hlpfile, 0x54L, SEEK_SET);
	fread(buffer, 1, 4L, hlpfile);
	return strncmp(buffer, HC_VERS, strlen(HC_VERS)) == 0;
}


/*------------------------------------------*/
/*  read_header:                            */
/*------------------------------------------*/
/*  Reads the description block from the    */
/*  help-file.                              */
/*------------------------------------------*/
static int read_header(void)
{
	unsigned char hlphdr_raw[HEADER_SIZE];
	fseek(hlpfile, 0x58L, SEEK_SET);
	if (fread(hlphdr_raw, 1, HEADER_SIZE, hlpfile) != HEADER_SIZE)
		return FALSE;
	hlphdr.scr_tab_size = read_int_32(hlphdr_raw + 0);
	hlphdr.str_offset = read_int_32(hlphdr_raw + 4);
	hlphdr.str_size = read_int_32(hlphdr_raw + 8);
	memcpy(hlphdr.char_table, hlphdr_raw + 12, 12);
	hlphdr.caps_offset = read_int_32(hlphdr_raw + 24);
	hlphdr.caps_size = read_int_32(hlphdr_raw + 28);
	hlphdr.caps_cnt = read_int_32(hlphdr_raw + 32);
	hlphdr.sens_offset = read_int_32(hlphdr_raw + 36);
	hlphdr.sens_size = read_int_32(hlphdr_raw + 40);
	hlphdr.sens_cnt = read_int_32(hlphdr_raw + 44);
	return TRUE;
}


/*-------------------------------------------*/
/*  read_screen_table:                       */
/*-------------------------------------------*/
/* Reads the table with the screen offsets   */
/* from the help-file.                       */
/*-------------------------------------------*/
static int read_screen_table(void)
{
	int32_t bytes_read;
	int i;

	wr_msg(TO_SCREEN, rd_scr_msg);
	fseek(hlpfile, 0x88L, SEEK_SET);
	screen_cnt = (int) hlphdr.scr_tab_size >> 2;
	screen_table = g_new(int32_t, screen_cnt);
	if (!screen_table)
	{
		wr_msg(TO_ALL, "%s", strerror(errno));
		return FALSE;
	}
	screen_done = g_new(unsigned char, screen_cnt);
	if (!screen_done)
	{
		wr_msg(TO_ALL, "\n%s\n", strerror(errno));
		return FALSE;
	}

	for (i = 0; i < screen_cnt; i++)
	{
		screen_done[i] = FALSE;
	}

	bytes_read = fread(screen_table, 1, hlphdr.scr_tab_size, hlpfile);
#ifdef MUST_SWAP
	{
		unsigned char *p = (unsigned char *)screen_table;
		for (i = 0; i < screen_cnt; i++)
		{
			screen_table[i] = read_int_32(p + i * 4);
		}
	}
#endif
	return bytes_read == hlphdr.scr_tab_size;
}


/*---------------------------------------------*/
/*  read_string_table:                         */
/*---------------------------------------------*/
/*  Reads in the table with the code-strings.  */
/*---------------------------------------------*/
static int read_string_table(void)
{
	int32_t bytes_read;

	wr_msg(TO_SCREEN, rd_str_msg);
	string_tab = g_new(unsigned char, hlphdr.str_size);
	if (!string_tab)
	{
		wr_msg(TO_ALL, "%s", strerror(errno));
		return FALSE;
	}
	fseek(hlpfile, hlphdr.str_offset, SEEK_SET);
	bytes_read = fread(string_tab, 1, hlphdr.str_size, hlpfile);
	return bytes_read == hlphdr.str_size;
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
		fprintf(stderr, hlp_nf_msg, hlpname);
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

	code_buffer = g_new0(unsigned char, MAXCODEDSIZE);
	plain_text = g_new(char, TXTBUFSIZE);
	if (code_buffer == NULL || plain_text == NULL)
	{
		errors++;
		goto end;
	}

	read_key_table(&key_table, SENS_TABLE);
	read_key_table(&c_key_table, CAP_TABLE);
	if (log_flag)
		wr_keytables();

	if (!read_screen_table())
	{
		wr_msg(TO_ALL, rd_scr_err);
		errors++;
		goto end;
	}
	wr_msg(TO_SCREEN, scr_cnt_msg, screen_cnt);

	if (!read_string_table())
	{
		wr_msg(TO_ALL, rd_str_err);
		errors++;
		goto end;
	}

	if (!read_Index())
	{
		wr_msg(TO_ALL, rd_idx_err);
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
			wr_msg(TO_ALL, decomp_err);
			errors++;
			goto end;
		}
	}
	
	if (txt_flag)
	{
		if (!make_txtfile())
		{
			wr_msg(TO_ALL, file_creat_err, txtname);
			errors++;
			goto end;
		}
	}

  end:

	wr_msg(TO_ALL, final_msg, errors, warnings, glbref_cnt);
	if (hlpfile)
		fclose(hlpfile);
	if (log_flag)
		fclose(logfile);
	
	g_free(screen_table);
	g_free(screen_done);
	g_free(code_buffer);
	g_free(plain_text);
	g_free(name_array);
	g_free(key_table);
	g_free(c_key_table);
	g_free(string_tab);
	
	{
		NAME_ENTRY *ent, *next;
		for (ent = namelist; ent != NULL; ent = next)
		{
			next = ent->next;
			g_free(ent->name);
			g_free(ent);
		}
		for (ent = link_list; ent != NULL; ent = next)
		{
			next = ent->next;
			g_free(ent->name);
			g_free(ent);
		}
	}
	
	puts(ready_msg);
	return errors == 0 ? 0 : 1;
}
