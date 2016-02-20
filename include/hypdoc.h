#ifndef __HYPDOC_H__
#define __HYPDOC_H__

#include "hypdefs.h"

/* Anonymous document type specific functions */
typedef struct _document_ DOCUMENT;
typedef struct _window_data_ WINDOW_DATA;

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

#include "hv_ascii.h"


typedef struct _history_  HISTORY;
struct _history_
{
	HISTORY *next;               /* Pointer to next history entry */
	DOCUMENT *doc;               /* Pointer to document */
	long line;                   /* First visible line */
	hyp_nodenr node;             /* Document node (=chapter) number */
	char *title;                 /* history title */
};

typedef	void (*DOC_DISPLAYPROC)(WINDOW_DATA *win);
typedef	void (*DOC_CLOSEPROC)(DOCUMENT *doc);
typedef	hyp_nodenr (*DOC_GETNODEPROC)(WINDOW_DATA *win);
typedef	gboolean (*DOC_GOTOPROC)(WINDOW_DATA *win, const char *chapter, hyp_nodenr node);
typedef	long (*DOC_AUTOLOCPROC)(WINDOW_DATA *win, long line, const char *search, gboolean casesensitive, gboolean wordonly);
typedef	void (*DOC_GETCURSORPROC)(WINDOW_DATA *win, int x, int y, TEXT_POS *pos);
typedef	gboolean (*DOC_BLOCKPROC)(WINDOW_DATA *win, hyp_blockop op, BLOCK *block, void *param);
typedef	void (*DOC_PREPNODEPROC)(WINDOW_DATA *win, HYP_NODE *node);

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
	unsigned int remarker_running : 1;   /* remarker available */
	unsigned int remarker : 1;   /* remarker state */
} hyp_nav_buttons;

struct _document_
{
	hyp_filetype type;          /* Document type see F_xy constants */
	int ref_count;				/* usage count */
	char *path;                 /* Full file access path */
	long start_line;            /* First visible line of document */
	
	/* Toolbar button configuration (bit vector) */
	hyp_nav_buttons buttons;             
	void *data;                 /* File format specific data */
	time_t mtime;    	        /* File modification time and date */
	DOC_DISPLAYPROC displayProc;      /* Document display function */
	DOC_CLOSEPROC closeProc;    /* Document close function */
	DOC_GOTOPROC gotoNodeProc;  /* Document navigation function */
	DOC_GETNODEPROC getNodeProc;/* Function to determine current node number */
	DOC_AUTOLOCPROC autolocProc;/* Autolocator search function */
	int autolocator_dir;        /* Autolocator direction (1 = down, else up) */
	DOC_GETCURSORPROC getCursorProc;/* Cursor position function */
	DOC_BLOCKPROC blockProc;    /* Block operation function */
	DOC_PREPNODEPROC prepNode;
};


/*
 * block.c
 */
gboolean HypBlockOperations(WINDOW_DATA *win, hyp_blockop op, BLOCK *block, void *param);


/*
 *	hypdoc.c
 */
hyp_filetype HypLoad(DOCUMENT *doc, int handle, gboolean return_if_ref);
hyp_nodenr HypFindNode(DOCUMENT *doc, const char *chapter);
DOCUMENT *hypdoc_unref(DOCUMENT *doc);
DOCUMENT *hypdoc_ref(DOCUMENT *doc);
DOCUMENT *hypwin_doc(WINDOW_DATA *win);
HYP_NODE *hypwin_node(WINDOW_DATA *win);
DOCUMENT *HypOpenFile(const char *path, gboolean return_if_ref);
hyp_filetype LoadFile(DOCUMENT *doc, int handle, gboolean return_if_ref);
void HypCloseFile(DOCUMENT *doc);


/*
 * hv_srch.c
 */
WINDOW_DATA *search_allref(WINDOW_DATA *win, const char *string, gboolean no_message);


/*
 * hv_file.c
 */
void CheckFiledate(WINDOW_DATA *win);


/*
 * hv_disp.c
 */
void HypDisplayPage(WINDOW_DATA *win);
void HypPrepNode(WINDOW_DATA *win, HYP_NODE *node);

/*
 * hyp_asc.c
 */
hyp_filetype AsciiLoad(DOCUMENT *doc, int handle);
hyp_filetype AsciiCalcLines(DOCUMENT *doc, FMT_ASCII *ascii);


/*
 * autoloc.c
 */
char *HypGetTextLine(WINDOW_DATA *win, HYP_NODE *node, long line);
long HypAutolocator(WINDOW_DATA *win, long line, const char *search, gboolean casesensitive, gboolean wordonly);


/*
 * hv_curs.c
 */
void HypGetCursorPosition(WINDOW_DATA *win, int x, int y, TEXT_POS *pos);

#endif /* __HYPDOC_H__ */
