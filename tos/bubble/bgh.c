#define  __BGH_IMPLEMENTATION__

#include "hv_defs.h"
#include "tos/bubble.h"
#include "tos/bgh.h"
#include "tos/mem.h"



#define BGH_MAGIC 0x23424748L

#define IS_SPACE(a)			(((a) == ' ') || ((a) == '\t'))
#define IS_EOL(a)			(((a) == '\n') || ((a) == '\r'))
#define IS_NUMBER(a)		(((a) >= '0') && ((a) <= '9'))

#define BGH_LAST (BGH_MORE + 1)

typedef struct bgh_object BGH_object;
struct bgh_object
{
	short index;
	BGH_object *next;
	char *help_string;
};

typedef struct bgh_group BGH_group;
struct bgh_group
{
	short index;
	BGH_group *next;
	BGH_object *first;
	char *help_string;
};

struct bgh_head
{
	BGH_group *section[BGH_LAST];
};

static char *gl_help_string;

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static char *Goto_Lineend(char *ptr)
{
	while (*ptr && !IS_EOL(*ptr))
		ptr++;
	return ptr;
}

/*** ---------------------------------------------------------------------- ***/

static char *Skip_Lineend(char *ptr)
{
	while (*ptr && IS_EOL(*ptr))
		ptr++;
	return ptr;
}

/*** ---------------------------------------------------------------------- ***/

static char *Skip_Spaces(char *ptr)
{
	while (*ptr && IS_SPACE(*ptr))
		ptr++;
	return ptr;
}

/*** ---------------------------------------------------------------------- ***/

static char *Skip_Number(char *ptr)
{
	while (*ptr && IS_NUMBER(*ptr))
		ptr++;
	return ptr;
}

/*** ---------------------------------------------------------------------- ***/

BGH_head *BGH_load(const char *Name)
{
	long ret;
	long len;
	short handle;
	short not_good = FALSE;
	short i;
	BGH_head *head;
	BGH_group *group = NULL, *new_group;
	BGH_object *obj = NULL, *new_obj;
	char *read, *write = NULL;

	ret = Fopen(Name, O_RDONLY);
	if (ret < 0)
		return NULL;

	handle = (short) ret;
	len = Fseek(0, handle, 2);
	Fseek(0, handle, 0);

	if (len < (long)sizeof(BGH_head))
		return NULL;

	head = (BGH_head *) g_malloc(2 * len + 2);
	if (head)
	{
		/* align to even address */
		read = (char *) (((long) head + len + 1) & ~1L);

		if (Fread(handle, len, read) == len)
		{
			if (*(long *) read == BGH_MAGIC)
			{
				read[len] = 0;
				write = (char *) head + sizeof(BGH_head);
				for (i = 0; i < BGH_LAST; i++)
					head->section[i] = NULL;

				while (*read)
				{
					read = Goto_Lineend(read);	/* find end of line */
					read = Skip_Lineend(read);	/* goto start of next line */

					if (*read)
					{
						if (*read == '#')
						{
							read++;
							read = Skip_Spaces(read);

							if (IS_NUMBER(*read))	/* found an object */
							{
								if (group)
								{
									new_obj = (BGH_object *) write;
									write += sizeof(BGH_object);

									new_obj->next = NULL;
									new_obj->help_string = NULL;
									new_obj->index = atoi(read);

									read = Skip_Number(read);
									read = Skip_Spaces(read);

									if (obj && *read == '^')
									{
										/* reference to previous object */
										new_obj->help_string = obj->help_string;
									} else if (!IS_EOL(*read))
									{
										/* ordinary help string follows */
										new_obj->help_string = write;
										while (*read && !IS_EOL(*read))
											*write++ = *read++;
										*write++ = 0;

										/* align to even address */
										if ((long) write & 1)
											write++;
									}

									if (group->first)
									{
										obj = group->first;
										while (obj->next)
											obj = obj->next;
										obj->next = new_obj;
									} else
									{
										group->first = new_obj;
									}
									obj = new_obj;
								}
							} else
							{
								const char *ptr = "N/A";
								bgh_type sect = BGH_DIAL;

								i = 0;
								switch (*read)
								{
								case 'd':
								case 'D':
									i = 4;
									ptr = "dial";
									sect = BGH_DIAL;
									break;
								case 'a':
								case 'A':
									i = 5;
									ptr = "alert";
									sect = BGH_ALERT;
									break;
								case 'u':
								case 'U':
									i = 4;
									ptr = "user";
									sect = BGH_USER;
									break;
								case 'm':
								case 'M':
									i = 4;
									ptr = "more";
									sect = BGH_MORE;
									break;
								}
								/* group found? */
								if (i && strnicmp(read, ptr, i) == 0)
								{
									read += i;
									read = Skip_Spaces(read);

									new_group = (BGH_group *) write;
									write += sizeof(BGH_group);

									new_group->next = NULL;
									new_group->first = NULL;
									new_group->index = atoi(read);
									read = Skip_Number(read);
									if (IS_SPACE(*read))
									{
										read = Skip_Spaces(read);
										new_group->help_string = write;
										while (*read && !IS_EOL(*read))
											*write++ = *read++;
										*write++ = 0;

										/* align to even address */
										if ((long) write & 1)
											write++;
									} else
									{
										new_group->help_string = NULL;
									}
									
									if (head->section[sect])
									{
										group = head->section[sect];
										while (group->next)
											group = group->next;
										group->next = new_group;
									} else
									{
										head->section[sect] = new_group;
									}
									group = new_group;
									obj = NULL;
								}
							}
						}
					}
				}
			} else
			{
				not_good = TRUE;
			}
		} else
		{
			not_good = TRUE;
		}
		
		if (not_good)
		{
			g_free(head);
			head = NULL;
		}
	}
	Fclose(handle);

	if (head)
	{
		head = (BGH_head *)g_realloc(head, write - (char *)head);

		/*
		 * resolve references to "More" section #0
		 */
		if (head->section[BGH_MORE])
		{
			BGH_group *ref_group = head->section[BGH_MORE];

			while (ref_group && ref_group->index != 0)
				ref_group = ref_group->next;

			if (ref_group)
			{
				BGH_object *ref_obj;
				_WORD sect;
				
				for (sect = 0; sect < BGH_LAST; sect++)
				{
					for (group = head->section[sect]; group; group = group->next)
					{
						for (obj = group->first; obj; obj = obj->next)
						{
							read = obj->help_string;
							
							if (read && *read == '>')
							{
								read++;
								i = atoi(read);
								ref_obj = ref_group->first;
								while (ref_obj && ref_obj->index != i)
									ref_obj = ref_obj->next;
				
								if (ref_obj)
									obj->help_string = ref_obj->help_string;
							}
						}
					}
				}
			}
		}
		
#if 0 /* debug output */
		{
			_WORD sect;
			
			for (sect = 0; sect < BGH_LAST; sect++)
			{
				for (group = head->section[sect]; group; group = group->next)
				{
					nf_debugprintf("section %d(%d): %s\n", sect, group->index, printnull(group->help_string));
					for (obj = group->first; obj; obj = obj->next)
					{
						nf_debugprintf("  %d: %s\n", obj->index, printnull(obj->help_string));
					}
				}
			}
		}
#endif
	}
	
	return head;
}

/*** ---------------------------------------------------------------------- ***/

void BGH_free(BGH_head *bgh_handle)
{
	g_free_shared(gl_help_string);
	gl_help_string = NULL;
	g_free(bgh_handle);
}

/*** ---------------------------------------------------------------------- ***/

const char *BGH_gethelpstring(BGH_head *head, bgh_type typ, short groupnr, short idx)
{
	BGH_group *group;
	const char *help_string = NULL;

	if ((short)typ < 0 || typ >= BGH_LAST)
		return NULL;

	group = head->section[typ];
	while (group && group->index != groupnr)
		group = group->next;
	if (group)
	{
		BGH_object *obj = group->first;

		if (idx == -1)
		{
			help_string = group->help_string;
		} else
		{
			while (obj && obj->index != idx)
				obj = obj->next;
			if (obj)
				help_string = obj->help_string;
		}
	}
	return help_string;
}

/*** ---------------------------------------------------------------------- ***/

void BGH_action(BGH_head *bgh_handle, short mx, short my, bgh_type typ, short group, short idx)
{
	BGH_help(mx, my, BGH_gethelpstring(bgh_handle, typ, group, idx));
}

/*** ---------------------------------------------------------------------- ***/

void BGH_help(short mx, short my, const char *helpstring)
{
	_WORD bubble_id;

	if (helpstring)
	{
		bubble_id = appl_locate("BUBBLE", TRUE);
		if (bubble_id < 0)
			bubble_id = appl_locate(getenv( "BUBBLEGEM"), TRUE);

		if (bubble_id >= 0)
		{
			g_free_shared(gl_help_string);
			gl_help_string = g_strdup_shared(helpstring);
			if (gl_help_string)
			{
				/* Protokoll_Handle_Message(bubble_message); */
				Protokoll_Send(bubble_id, BUBBLEGEM_SHOW, mx, my, PROT_NAMEPTR(gl_help_string), 0);
			}
		}
	}
}
