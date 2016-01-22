#ifndef __HYPDOC_H__
#define __HYPDOC_H__

#include "hypdefs.h"

/* Anonymous document type specific functions */
typedef struct _document_ DOCUMENT;


typedef struct {
	long line;              /* Line number */
	long y;
	long offset;            /* Char. offset to beginning of word */
	short x;                /* x coordinate of lower left corner */
} TEXT_POS;

typedef struct
{
	gboolean valid;         /* Flag indicating validity of content */
	TEXT_POS start;         /* Start of block */
	TEXT_POS end;           /* End of block */
} BLOCK;

typedef enum {
	BLK_ASCIISAVE, /* Save current block as ASCII */
	BLK_PRINT       /* Print current block */
} hyp_blockop;


typedef	void (*DOC_PROC1)(DOCUMENT *doc);
typedef	hyp_nodenr (*DOC_GETNODEPROC)(DOCUMENT *doc);
typedef	gboolean (*DOC_GOTOPROC)(DOCUMENT *doc, const char *chapter, hyp_nodenr node);
typedef	long (*DOC_AUTOLOCPROC)(DOCUMENT *doc, long line, const char *search);
typedef	void (*DOC_GETCURSORPROC)(DOCUMENT *doc, int x, int y, TEXT_POS *pos);
typedef	gboolean (*DOC_BLOCKPROC)(DOCUMENT *doc, hyp_blockop op, BLOCK *block, void *param);
typedef	void (*DOC_PREPNODEPROC)(DOCUMENT *doc);

typedef struct _hyp_nav_buttons
{
	unsigned int info : 1;       /* TO_INFO; always enabled */
	unsigned int load : 1;       /* TO_LOAD: always enabled */
	unsigned int back : 1;       /* TO_BACK; disabled when history empty */
	unsigned int history : 1;    /* TO_HISTORY; disabled when history empty */
	unsigned int memory : 1;     /* TO_MEMORY (marks); disabled if form_popup() is not available */
	unsigned int save : 1;       /* TO_SAVE; currently enable only when displaying ASCII */
	unsigned int help : 1;       /* TO_HELP; enabled if file contains help entry */
	unsigned int index : 1;      /* TO_INDEX; enabled if file contains Index entry */
	unsigned int prevphys : 1;   /* TO_PREV_PHYS; disabled if current node has no predecessor */
	unsigned int previous : 1;   /* TO_PREV; disabled if current node has no predecessor */
	unsigned int next : 1;       /* TO_NEXT; disabled if current node has no successor */
	unsigned int nextphys : 1;   /* TO_NEXT_PHYS; disabled if current node has no successor */
	unsigned int first : 1;      /* TO_FIRST; disabled if already on first text page */
	unsigned int last : 1;       /* TO_LAST; disabled if already on last text page */
	unsigned int home : 1;       /* TO_HOME; enabled if current node has parent toc_index */
	unsigned int references : 1; /* TO_REFERENCES; enabled if current node has external references */
	unsigned int ascii : 1;      /* TO_ASCII; enabled if we can save file as ASCII */
	unsigned int searchbox : 1;  /* enabled if search text entry is visible */
	unsigned int menu : 1;       /* context menu */
} hyp_nav_buttons;

struct _document_
{
	DOCUMENT *next;             /* Pointer to next document */
	hyp_filetype type;          /* Document type see F_xy constants */
	char *path;                 /* Full file access path */
	char *window_title;         /* Window title, in utf8 encoding */
	long start_line;            /* First visible line of document */
	long lines;                 /* Number of lines (in window lines) */
	long height;                /* heoght of document (in pixel) */
	long columns;               /* Number of window columns */
	
	/* Toolbar button configuration (bit vector) */
	hyp_nav_buttons buttons;             
	void *data;                 /* File format specific data */
	HYP_NODE *displayed_node;   /* Currently displayed node */
	time_t mtime;    	        /* File modification time and date */
	void /* WINDOW_DATA */ *window;        /* Window associated with this file */
	void /* WINDOW_DATA */ *popup;			/* Currently activ popup window */
	DOC_PROC1 displayProc;      /* Document display function */
	DOC_PROC1 closeProc;        /* Document close function */
	DOC_GOTOPROC gotoNodeProc;  /* Document navigation function */
	DOC_GETNODEPROC getNodeProc;/* Function to determine current node number */
	DOC_AUTOLOCPROC autolocProc;/* Autolocator search function */
#ifdef WITH_GUI_GEM
	char *autolocator;          /* Autolocator search string */
#endif
	int autolocator_dir;        /* Autolocator direction (1 = down, else up) */
	DOC_GETCURSORPROC getCursorProc;/* Cursor position function */
	BLOCK selection;            /* Content of  selection */
	DOC_BLOCKPROC blockProc;    /* Block operation function */
	DOC_PREPNODEPROC prepNode;
};


/*
 *		Block.c
 */
gboolean HypBlockOperations(DOCUMENT *doc, hyp_blockop op, BLOCK *block, void *param);


/*
 *	hypdco.c
 */
hyp_filetype HypLoad(DOCUMENT *doc, int handle, gboolean return_if_ref);
hyp_nodenr HypFindNode(DOCUMENT *doc, const char *chapter);


/*
 *		Search.c
 */
void search_allref(void /* WINDOW_DATA */ *win, const char *string, gboolean no_message);


/*
 *		File.c
 */
hyp_filetype LoadFile(DOCUMENT *doc, int handle, gboolean return_if_ref);
void HypCloseFile(DOCUMENT *doc);
DOCUMENT *HypOpenFile(const char *path, gboolean return_if_ref);
void CheckFiledate(DOCUMENT *doc);
void HypDeleteIfLast(DOCUMENT *doc, HYP_DOCUMENT *hyp);


/*
 *		Display.c
 */
void HypDisplayPage(DOCUMENT *doc);
void HypPrepNode(DOCUMENT *doc);

/*
 *		Ascii.c
 */
hyp_filetype AsciiLoad(DOCUMENT *doc, int handle);


/*
 *		Autoloc.c
 */
char *HypGetTextLine(DOCUMENT *doc, HYP_NODE *node, long line);
long HypAutolocator(DOCUMENT *doc, long line, const char *search);


/*
 *		Cursor.c
 */
void HypGetCursorPosition(DOCUMENT *doc, int x, int y, TEXT_POS *pos);

#endif /* __HYPDOC_H__ */
