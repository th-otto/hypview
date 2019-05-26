/*  	GEMFSLIB.C	5/14/84 - 07/16/85	Lee Lorenzen		*/
/*	merge High C vers. w. 2.2 		8/21/87		mdf	*/

/*
 *       Copyright 1999, Caldera Thin Clients, Inc.                      
 *       This software is licenced under the GNU Public License.         
 *       Please see LICENSE.TXT for further information.                 
 *                                                                       
 *                  Historical Copyright
 *	-------------------------------------------------------------
 *	GEM Application Environment Services		  Version 3.0
 *	Serial No.  XXXX-0000-654321		  All Rights Reserved
 *	Copyright (C) 1987			Digital Research Inc.
 *	-------------------------------------------------------------
 */

#include "aes.h"
#include "gemlib.h"
#include "gemdos.h"
#include "dos.h"
#include "gem_rsc.h"


#define NM_NAMES (F9NAME - F1NAME + 1)
#define NAME_OFFSET F1NAME
#define NM_DRIVES (FSLSTDRV-FS1STDRV+1)
#define DRIVE_OFFSET FS1STDRV
#define LPATH	128

#define LEN_FSNAME (LEN_ZFNAME+1)	/* includes leading flag byte & trailing nul */

typedef struct fstruct
{
	char snames[LEN_FSNAME];
} FSTRUCT;

uint32_t gl_bvdisk;
uint32_t gl_bvhard;

static GRECT gl_rfs;
static OBJECT *ad_fstree;
static char *ad_fpath;
static char *ad_title;
static char *ad_select;
static FSTRUCT *ad_fsnames;
static _WORD fs_first;					/* first enter the file selector */
static _UWORD fs_topptr;
static _UWORD fs_count;
static int32_t fs_fnum;					/* max file allowed */
static DTA *ad_fsdta;

static char const wildstr[] = "*.*";
static char const wslstr[] = "\\*.*";

static char fsname[40];
static char fcopy[40];
static char *pathcopy;					/* path copy    */
static _WORD defdrv;

typedef struct pathstruct
{
	char pxname[LPATH];
} PATHSTRUCT;

static PATHSTRUCT *pxpath;


#if (AESVERSION >= 0x330)
static void FXWait(void);
static void FXSelect(OBJECT *tree, _WORD obj);
static void FXDeselect(OBJECT *tree, _WORD obj);
#endif




/*
 *	Routine to back off the end of a file string.
 */
static char *fs_back(char *pstr)
{
	char *pend;
	_WORD i;

	i = strlen(pstr);					/* find the end of string   */
	pend = pstr + i;

	while (*pend != '\\' && pend != pstr)
	{
		pend--;
		if ((*pend == ':') && (pend == pstr + 1))
			break;
	}
	/* if a : then insert a backslash for fsel dir line */
	if (*pend == ':')
	{
		pend++;
		memmove(pend + 1, pend, i + 1);
		*pend = '\\';
	}

	return pend;
}


/*
 * do the fs_sset and ob_draw
 */
static void fs_draw(_WORD index, char *path, char **addr1)
{
	_WORD dummy;
	
	fs_sset(ad_fstree, index, path, addr1, &dummy);
	ob_draw(ad_fstree, index, MAX_DEPTH);
}


/* sort files using shell sort from page 108 of K&R C Prog. Lang. */
static void r_sort(FSTRUCT *buffer, _WORD count)
{
	_WORD gap, i, j;
	char tmp[LEN_FSNAME];

	for (gap = count / 2; gap > 0; gap /= 2)
	{
		for (i = gap; i < count; i++)
		{
			for (j = i - gap; j >= 0; j -= gap)
			{
				if (strchk(buffer[j].snames, buffer[j + gap].snames) <= 0)
					break;

				strcpy(tmp, buffer[j].snames);
				strcpy(buffer[j].snames, buffer[j + gap].snames);
				strcpy(buffer[j + gap].snames, tmp);
			}
		}
	}
}


/*
 * Read files into the buffer
 * The buffer size will always be NM_NAMES or more
 * for easy coding and redraw the count will return
 * the actual number of files
 */
static _WORD r_files(char *path, char *select, uint32_t *count, char *filename)
{
	_WORD i;
	int32_t j;
	int32_t k;
	_WORD ret;
	char *chrptr;
	FSTRUCT *fsnames;
	_WORD drvid;

	fsnames = ad_fsnames;

	*filename = 0;						/* no file name     */

	/* uppercase the drive path */
	if (*(path + 1) == ':')
	{
		*path = aes_toupper(*path);
		drvid = (_WORD) (*path - 'A');
	} else
	{
		drvid = defdrv;
	}
	
	/* the drive present ?  */
	k = 1;
	k = k << drvid;
	j = isdrive();						/* get the drive map    */

	if (!(k & j))						/* drive not there  */
		return FALSE;

	dos_sdrv(drvid);					/* set the default drive    */
	/* take out the wild string stuff   */

	chrptr = fs_back(path);				/* get the directory    */
	if (*chrptr == '\\')				/* path exists, point at filename */
		chrptr++;

	if ((int)strlen(chrptr) > 12)			/* 9/5/90       */
		chrptr[12] = 0;

	strcpy(filename, chrptr);			/* save the file name   */
	strcpy(chrptr, wildstr);			/* this is the dir  */
	dos_setdta(ad_fsdta);
	/* look for all sub dir */
	if ((ret = dos_sfirst(path, 0x37)) != 0)
	{									/* error        */
		if (ret != E_NMFIL && ret != E_FILNF)		/* it is not no files   */
		{
			strcpy(chrptr, filename);	/* then return      */
			return FALSE;
		}
	}

	if (!fs_first)
		*select = 0;					/* clean up selection filed */
	else
		fs_first = FALSE;				/* don't clean up at this   */
	/* time */

	for (i = 0; i < NM_NAMES; i++)
		strcpy(&fsnames[i].snames[0], " ");

	i = 0;
	/* look for directory */
	while (ret && ((_UWORD) (i) < fs_fnum))
	{
		if (ad_fsdta->d_attrib & (F_HIDDEN | F_SYSTEM))
			goto rfile2;

		if (ad_fsdta->d_attrib & F_SUBDIR)	/* if subdirectory  */
		{
			if (ad_fsdta->d_fname[0] != '.')
				fsnames[i].snames[0] = 7;
			else
				goto rfile2;
		} else
		{
			if (wildcmp(filename, ad_fsdta->d_fname))
				fsnames[i].snames[0] = 0x20;
			else
				goto rfile2;
		}

		fmt_str(ad_fsdta->d_fname, &fsnames[i++].snames[1]);
	  rfile2:
		ret = dos_snext();
	}

	if (i)
		r_sort(fsnames, i);

	strcpy(chrptr, filename);			/* restore file name    */

	*count = i;

	return TRUE;
}


/*
 * show files and update the scroll bar
 */
static void r_sfiles(_WORD index)
{
	_WORD label, i;
	_WORD h;
	OBJECT *tree;
	char *addr;
	_WORD dummy;
	
	label = F1NAME;
	tree = ad_fstree;

	for (i = index; i < (index + NM_NAMES); i++)
	{
		tree[label].ob_state = OS_NORMAL;
		fs_sset(ad_fstree, label, " ", &addr, &dummy);
		fs_draw(label++, ad_fsnames[i].snames, &addr);
	}

	h = tree[FSVSLID].ob_height;

	h = h * fs_topptr;
	if (fs_count != 0)
		h = h / fs_count;
	
	tree[FSVELEV].ob_y = h;

	ob_draw(tree, FSVSLID, MAX_DEPTH);	/* erase the old slide bar */
}



/*
 * read in a directory
 */
static _WORD r_dir(char *path, char *select, uint32_t *count)
{
	OBJECT *tree;
	int h;
	char *addr;
	_WORD status;
	char filename[16];

	set_mouse_to_hourglass();

	if (!r_files(path, select, count, filename))
	{
		/* if failed */
		fm_error(2);
		status = FALSE;
		goto r_exit;
	}

	fs_count = *count;
	fs_draw(FSDIRECT, path, &addr);
	fs_draw(FTITLE, filename, &addr);
	fs_draw(FSSELECT, select, &ad_select);

	tree = ad_fstree;
	fs_topptr = 0;						/* reset top pointer */

	h = tree[FSVSLID].ob_height;
#if AES3D
	h += ADJ3DPIX << 1;
#endif
	if (*count > NM_NAMES)
	{
		h = (h * NM_NAMES) / *count;
	}

#if AES3D
	if (!h)								/* min size */
	{
		h = 1;
	} else
	{
		if (h > (ADJ3DPIX << 1))
			h -= (ADJ3DPIX << 1);
	}
#endif
	tree[FSVELEV].ob_y = 0;			/* move it to the top     */
	tree[FSVELEV].ob_height = (_UWORD) h;	/* height of the elevator */
	r_sfiles(0);						/* show form the top      */
	status = TRUE;

  r_exit:
	gr_mouse(M_PREV, NULL);
	return status;
}


/*
 * AES #90 - fsel_input - File selection input
 * AES #91 - fsel_exinput - File selection extended input
 *
 *	File Selector input routine that takes control of the mouse
 *	and keyboard, searchs and sort the directory, draws the file 
 *	selector, interacts with the user to determine a selection
 *	or change of path, and returns to the application with
 *	the selected path, filename, and exit button.
 *	Add the label parameter
 */
_WORD fs_input(char *pipath, char *pisel, _WORD *pbutton, const char *lstring)
{
	_UWORD i;
	uint32_t j;
	_WORD label;
	_WORD last;
	_WORD ret;
	OBJECT *tree;
	char *addr;
	intptr_t mul;
	DTA *savedta;
	PATHSTRUCT *savepath;
	_WORD botptr;
	_WORD value;
	uint32_t count;
	_WORD xoff, yoff, mx, my, bret;
	char dirbuffer[122];
	char scopy[16];
	char chr;
	_WORD curdrv, savedrv;
	GRECT clip;
	_WORD firstry;
	_WORD dummy;
	
	/*
	 *	Start up the file selector by initializing the fs_tree
	 */
	ad_fstree = aes_rsc_tree[FSELECTR];
	ob_center(ad_fstree, &gl_rfs);

	firstry = TRUE;
	fs_first = TRUE;					/* first enter      */
	last = F1NAME;						/* last selected file   */

	defdrv = dos_gdrv();			/* get the default drive */
	savedrv = defdrv;

	curdrv = defdrv + FS1STDRV;
	/* save for default dr  */
	pxpath = (PATHSTRUCT *)dos_alloc_anyram((int32_t) ((_WORD) LPATH * (_WORD) (NM_DRIVES + 1)));

	if (!pxpath)
		goto bye2;
	/* allocate dta buffer  */
	ad_fsdta = (DTA *)dos_alloc_anyram(0x00000100L);

	if (!ad_fsdta)						/* no buffer, bail out  */
		goto bye;

	/* get all the memory   */
	mul = dos_avail();
	/*  LEN_FSNAMES;    */
	fs_fnum = mul / LEN_FSNAME;

	if (mul == 0 || fs_fnum < NM_NAMES)
	{
		dos_free(ad_fsdta);
bye:
		dos_free(pxpath);
bye2:
		fm_show(NOMEMORY, 1, 0);
		return FALSE;
	} else
	{
		ad_fsnames = (FSTRUCT *)dos_alloc_anyram(mul);
	}

	savepath = pxpath;					/* save the address */
	pathcopy = savepath->pxname;
	pxpath = savepath + 1;

	fm_dial(FMD_START, &gl_rcenter, &gl_rfs);

	tree = ad_fstree;

	/* change the buffer pointer */
	tree[FSDIRECT].ob_spec.tedinfo->te_ptext = dirbuffer;

	fs_sset(tree, FSTITLE, lstring, &addr, &dummy);
	fs_sset(tree, FSDIRECT, "", &ad_fpath, &dummy);
	fs_sset(tree, FTITLE, "", &ad_title, &dummy);
	fs_sset(tree, FSSELECT, "", &ad_select, &dummy);

	/* get the current drive */
	count = isdrive();
	j = 1;
	/* start from A drive set the button    */
	for (ret = 0, i = FS1STDRV; i <= FSLSTDRV; i++, ret++)
	{
		tree[i].ob_state = count & j ? OS_NORMAL : OS_DISABLED;
		j = j << 1;
	}

	label = F1NAME;						/* clean up the files   */

	for (i = 0; i < NM_NAMES; i++)		/* clean up fields  */
	{
		fs_sset(tree, label, " ", &addr, (_WORD *)&addr); /* WTF */
		tree[label++].ob_state = OS_NORMAL;
	}
	/* save the current dta   */
	savedta = dos_getdta();

	gsx_gclip(&clip);					/* get the clipping rect  */
	/* set the new one    */
	gsx_sclip(&gl_rfs);
	/* reset the height */
	tree[FSVELEV].ob_y = 0;
	tree[FSVELEV].ob_height = tree[FSVSLID].ob_height;

	gr_mouse(M_SAVE, NULL);
	set_mouse_to_arrow();

	ob_draw(tree, 0, MAX_DEPTH);		/* draw the box     */

	fmt_str(pisel, scopy);

	strcpy(ad_fpath, pipath);			/* make a copy      */

	pathcopy[0] = defdrv + 'A';			/* Backup path      */
	pathcopy[1] = ':';
	strcpy(&pathcopy[2], wslstr);

	count = 0;
	fs_topptr = 0;
	botptr = 0;

	ret = FSDIRECT;					/* initial action   */
	
	bret = 0; /* quiet compiler */

	do
	{
		value = 1;						/* scroll factor    */

		switch (ret)
		{
		case FSVSLID:
			ob_offset(tree, FSVELEV, &xoff, &yoff);
			value = NM_NAMES;
			if (my <= yoff)
				goto up;
			else /* if (my >= yoff + tree[FSVELEV].ob_height) */
				goto down;
			/* else fall through */

		case FSVELEV:
			fm_own(TRUE);
			value = gr_slidebox(tree, FSVSLID, FSVELEV, TRUE);
			fm_own(FALSE);
			mul = (count - NM_NAMES) * (_UWORD) value;
			mul = mul / 1000;
			value = mul;
			value = fs_topptr - value;
			if (value >= 0)
				goto up;
			value = -value;
			/* fall through */
		case FDNAROW:			/* scroll down  */
		down:
			if (fs_topptr == botptr)
				break;

			if ((fs_topptr + value) <= botptr)
				fs_topptr += value;
			else
				fs_topptr = botptr;

			goto sfiles;

		up:
		case FUPAROW:				/* scroll up    */
			if (!fs_topptr)
				break;

			if ((_WORD) (fs_topptr - value) >= 0)
				fs_topptr -= value;
			else
				fs_topptr = 0;

		sfiles:
			r_sfiles(fs_topptr);
			break;

		case FCLSBOX:					/* close box        */

			*(fs_back(ad_fpath)) = 0;	/* back to last path    */
			strcpy(fs_back(ad_fpath) + 1, ad_title);

			/* fall through     */
		case FSDIRECT:
		rdir:
			if (!*ad_fpath)
				strcpy(ad_fpath, pathcopy);

			strcpy(fcopy, fs_back(ad_fpath));
			/* extension OK ? */
			if ((fcopy[0] == '\\') && (fcopy[1]))
				strcpy(fsname, fcopy);	/* yes          */

			if (fcopy[0] != '\\')		/* any slash ?      */
			{
				fsname[0] = '\\';
				strcpy(fsname + 1, fcopy);
			}

			if (!fcopy[1])				/* if no extension  */
			{
				strcpy(fsname, wslstr);
				strcat(ad_fpath, wildstr);
			}

			if (r_dir(ad_fpath, scopy, &count))
			{
				strcpy(pathcopy, ad_fpath);	/* copy current dir */
				if (count > NM_NAMES)	/* more than 9 items    */
					botptr = count - NM_NAMES;
				else
					botptr = 0;
			} else
			{
		rdir5:
				strcpy(ad_fpath, pathcopy);
				ob_draw(tree, FSDIRECT, MAX_DEPTH);
				if (firstry)
				{
					firstry = FALSE;
					goto rdir;
				}
			}

			firstry = FALSE;

			/* reset the last one   */
			if (curdrv <= FSLSTDRV)
				ob_change(tree, curdrv, OS_NORMAL, TRUE);

			if (*(ad_fpath + 1) == ':')	/* if there a drive */
				defdrv = (_WORD) (*ad_fpath - 'A');

			curdrv = defdrv + FS1STDRV;

			if (curdrv <= FSLSTDRV)
				ob_change(tree, curdrv, OS_SELECTED, TRUE);

			break;

		case F1NAME:
		case F2NAME:
		case F3NAME:
		case F4NAME:
		case F5NAME:
		case F6NAME:
		case F7NAME:
		case F8NAME:
		case F9NAME:
			i = ret - F1NAME;
			addr = &ad_fsnames[i + fs_topptr].snames[1];
			chr = ad_fsnames[i + fs_topptr].snames[0];

			if (chr == 7)				/* is it a directory ?  */
			{
				unfmt_str(addr, fs_back(ad_fpath) + 1);
			  fs1:
				strcat(ad_fpath, fsname);
				goto rdir;
			} else /* must be a file   */ if (chr)
			{							/* clean up the last selected */
				ob_change(tree, last, OS_NORMAL, TRUE);
				strcpy(tree[FSSELECT].ob_spec.tedinfo->te_ptext, addr);
				ob_change(tree, ret, OS_SELECTED, TRUE);
				ob_draw(tree, FSSELECT, MAX_DEPTH);
				last = ret;
				if (bret & 0x8000)		/* double click     */
				{
					ob_change(tree, FSOK, OS_SELECTED, TRUE);
					goto fdone;			/* force to exit    */
				}
			}

			break;

		default:
			curdrv = ret;
			i = ret - FS1STDRV;			/* get the drive */
			if (i >= NM_DRIVES)    /* not for us */
				break;
			*ad_fpath = (char) (i + 'A');	/* stuff into the path */
			*(ad_fpath + 1) = ':';
			if (!dos_gdir(i + 1, ad_fpath + 2))
				goto fs1;
			else
				goto rdir5;
		}								/* end of switch    */

		bret = fm_do(tree, FSSELECT);

		gsx_mxmy(&mx, &my);

		ret = bret & 0x7FFF;

		if (ret == FSCANCEL)
			break;

		if (!streq(ad_fpath, pathcopy))	/*  is dir changed ?  */
		{
			ob_change(tree, ret, OS_NORMAL, TRUE);
			ret = FSDIRECT;			/* force a read again   */
		} else
		{
			if (ret == FSOK)
				break;
		}

	} while (ret != FSCANCEL);

  fdone:

	dos_sdrv(savedrv);
	dos_free(ad_fsdta);
	dos_free(ad_fsnames);
	dos_free(savepath);
	strcpy(pipath, ad_fpath);
	unfmt_str(ad_select, pisel);

	if ((*pbutton = inf_what(tree, FSOK)) < 0)
		*pbutton = FALSE;

	ob_change(tree, ret, OS_NORMAL, FALSE);
	fm_dial(FMD_FINISH, &gl_rcenter, &gl_rfs);
	dos_setdta(savedta);
	gsx_sclip(&clip);
	gr_mouse(M_RESTORE, NULL);

	return TRUE;
}



/*
 *	initialise the file selector
 */
void fs_start(void)
{
	OBJECT *tree = aes_rsc_tree[FSELECTR];
	_WORD diff;

	ob_center(tree, &gl_rfs);

	/*
	 * for cosmetic reasons, we make the vertical slider width equal to
	 * the standard box width in the current resolution.  since the FTITLE
	 * object overhangs the vertical slider, we must adjust its width too.
	 */
	diff = tree[SCRLBAR].ob_width - gl_wbox;
	tree[FTITLE].ob_width -= diff;
	tree[SCRLBAR].ob_width = gl_wbox;
	tree[FUPAROW].ob_width = gl_wbox;
	tree[FDNAROW].ob_width = gl_wbox;
	tree[FSVSLID].ob_width = gl_wbox;
	tree[FSVELEV].ob_width = gl_wbox;

#if AES3D
	{
		_WORD x, y, i, j, w;
		OBJECT *obj;
	
		obj = tree;
	
		y = x = 0;
	
		for (i = 0, j = FS1STDRV; i < 8; i++, j++)
		{
			obj[j].ob_x = x;
			obj[j].ob_y = y;
			obj[j + 9].ob_x = obj[j].ob_width + (ADJ3DPIX << 1) + x + 1;
			obj[j + 9].ob_y = y;
			if (j + 18 <= FSLSTDRV)
			{
				obj[j + 18].ob_x = obj[j + 9].ob_width + (ADJ3DPIX << 1) + x + 1;
				obj[j + 18].ob_y = y;
			}
			y += obj[j].ob_height + (ADJ3DPIX << 1) + 1;
		}
	
		/* fix up the remaining of the file selector */
	
		obj[FUPAROW].ob_height -= 1;
		obj[FDNAROW].ob_height -= 1;
		ob_offset(tree, FS1STDRV, &x, &y);
		y -= obj[FSDRIVE].ob_height + 2 + ADJ3DPIX;
		obj[FSDRIVE].ob_y = y;
	
		ob_offset(tree, FCLSBOX, &x, &y);
	
		obj[FILEBOX].ob_x = x - ADJ3DPIX - 1;
		obj[FTITLE].ob_x = x + obj[FCLSBOX].ob_width + ADJ3DPIX;	/* adjust FTITLE */
		obj[FTITLE].ob_y = y - ADJ3DPIX;
		obj[FTITLE].ob_height = obj[FCLSBOX].ob_height + (ADJ3DPIX << 1);
		y = y + obj[FCLSBOX].ob_height + ADJ3DPIX;
		obj[FILEBOX].ob_y = obj[SCRLBAR].ob_y = y;
		obj[FUPAROW].ob_y = ADJ3DPIX;
		y = obj[FUPAROW].ob_height + (ADJ3DPIX << 1);
		obj[FSVELEV].ob_height = obj[FSVSLID].ob_height;
		y += ADJ3DPIX;
		obj[FSVSLID].ob_y = obj[FSVELEV].ob_y = y;
		y += obj[FSVSLID].ob_height + (ADJ3DPIX << 1);
		obj[FDNAROW].ob_y = y;
		y += obj[FDNAROW].ob_height + ADJ3DPIX;
		obj[FILEBOX].ob_height = obj[SCRLBAR].ob_height = y;
		w = obj[FCLSBOX].ob_width + obj[FTITLE].ob_width + (ADJ3DPIX << 2);
		w -= obj[FUPAROW].ob_width + (ADJ3DPIX << 1);
		obj[FILEBOX].ob_width = w;
		obj[SCRLBAR].ob_x = obj[FILEBOX].ob_x + w;
	
		ob_offset(tree, FILEBOX, &x, &y);
		y = obj[FILEBOX].ob_height + y + 6 + ADJ3DPIX;
		obj[FSOK].ob_y = obj[FSCANCEL].ob_y = y;
		y += obj[FSOK].ob_height + ADJ3DPIX + 6;
		obj[0].ob_height = y;
	
		if (gl_ws.ws_ncolors <= G_LWHITE)
		{
			OBSPEC_SET_INTERIORCOL(obj[FCLSBOX].ob_spec, G_WHITE);
			OBSPEC_SET_INTERIORCOL(obj[FUPAROW].ob_spec, G_WHITE);
			OBSPEC_SET_INTERIORCOL(obj[FDNAROW].ob_spec, G_WHITE);
			OBSPEC_SET_INTERIORCOL(obj[FSVELEV].ob_spec, G_WHITE);
		
			for (i = FS1STDRV; i <= FSLSTDRV; i++)
				OBSPEC_SET_INTERIORCOL(obj[i].ob_spec, G_WHITE);
		}
#endif
	}
}


#if (AESVERSION >= 0x330)

static VOID FXWait(void)
{
	do
	{
		dsptch();
	} while (gl_button);
}


static void FXSelect(OBJECT *tree, _WORD obj)
{
	GRECT rect;
	_WORD dummy;

	tree[obj].ob_state |= OS_SELECTED;
	rect = *(GRECT *) & tree[(obj)].ob_x;
	ob_gclip(tree, obj, &dummy, &dummy, &rect.g_x, &rect.g_y, &rect.g_w, &rect.g_h);
	gsx_sclip(&rect);
	ob_draw(tree, obj, MAX_DEPTH);		/* draw the box     */
}


static void FXDeselect(OBJECT *tree, _WORD obj)
{
	GRECT rect;
	_WORD dummy;

	tree[obj].ob_state &= ~OS_SELECTED;
	rect = *(GRECT *) & tree[(obj)].ob_x;
	ob_gclip(tree, obj, &dummy, &dummy, &rect.g_x, &rect.g_y, &rect.g_w, &rect.g_h);
	gsx_sclip(&rect);
	ob_draw(tree, obj, MAX_DEPTH);		/* draw the box     */
}

#endif
