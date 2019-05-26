#include "aes.h"
#include "gempd.h"
#include "debug.h"
#include "gem_rsc.h"
#include "crysbind.h"

/*
 * Crystal function op code
 */
#define OP_CODE pb->control[0]
#define IN_LEN  pb->control[1]
#define OUT_LEN pb->control[2]
#define AIN_LEN pb->control[3]
#define AOUT_LEN pb->control[4]

#define RET_CODE int_out[0]


#undef CONF_WITH_PCGEM
#define CONF_WITH_PCGEM 0


#ifdef ENABLE_KDEBUG
static void aestrace(const char *message)
{
	char appname[AP_NAMELEN+1];
	const char *src = rlr->p_name;
	char *dest = appname;

	while (dest < &appname[AP_NAMELEN] && *src != ' ')
		*dest++ = *src++;
	*dest++ = '\0';

	KINFO(("AES: %s: %s\n", appname, message));
}
#else
#define aestrace(a)
#endif


#define AES_PARAMS(opcode,nintin,nintout,naddrin,naddrout) \
	if (IN_LEN < nintin || \
		OUT_LEN < nintout || \
		AIN_LEN < naddrin || \
		AOUT_LEN < naddrout) { \
		KINFO(("AES(%d): wrong #parameters\n", opcode)); \
	}


static _WORD crysbind(AESPB *pb)
{
	AES_GLOBAL *pglobal = (AES_GLOBAL *)pb->global;
	_WORD opcode = OP_CODE;
	const _WORD *int_in = pb->intin;
	_WORD *int_out = pb->intout;
	void **addr_in = pb->addrin;
	_BOOL unsupported = FALSE;
	_WORD ret = TRUE;
	_LONG timeval;
	intptr_t lbuparm;
	
	switch (opcode)
	{
	/* Application Manager */
	case APPL_INIT:
		aestrace("appl_init()");
		AES_PARAMS(10,0,1,0,0);
        /* reset dispatcher count to let the app run a while */
        if (rlr == NULL)
        	aes_init();
        ret = ap_init(pglobal);
        break;

	case APPL_READ:
		aestrace("appl_read()");
		AES_PARAMS(11,2,1,1,0);
#if NYI
		ret = ap_rdwr(AQRD, AP_RWID, AP_LENGTH, (_WORD *)AP_PBUFF);
#endif
		break;

	case APPL_WRITE:
		aestrace("appl_write()");
		AES_PARAMS(12,2,1,1,0);
#if NYI
		ret = ap_rdwr(AQWRT, AP_RWID, AP_LENGTH, (_WORD *)AP_PBUFF);
#endif
		break;

	case APPL_FIND:
		aestrace("appl_find()");
		AES_PARAMS(13,0,1,1,0);
#if NYI
		ret = ap_find((const char *)AP_PNAME);
#endif
		break;

	case APPL_TPLAY:
		aestrace("appl_tplay()");
		AES_PARAMS(14,2,1,1,0);
#if NYI
		ap_tplay((const uint32_t *)AP_TBUFFER, AP_TLENGTH, AP_TSCALE);
#endif
		break;

	case APPL_TRECORD:
		aestrace("appl_trecord()");
		AES_PARAMS(15,1,1,1,0);
#if NYI
		ret = ap_trecd((uint32_t *)AP_TBUFFER, AP_TLENGTH);
#endif
		break;

	case APPL_BVSET:
		aestrace("appl_bvset()");
		AES_PARAMS(16,2,1,0,0);
#if CONF_WITH_PCGEM
		gl_bvdisk = HW(AP_BVDISK);
		gl_bvhard = HW(AP_BVHARD);
#endif
		break;

	case APPL_SEARCH:
	/* case APPL_BVEXT: */
		/* distinguish between appl_search() and appl_xbvset() */
		if (IN_LEN == 3)
		{
			aestrace("appl_search()");
			AES_PARAMS(18,1,3,1,0);
			unsupported = TRUE;
		} else
		{
			aestrace("appl_xbvset()");
			AES_PARAMS(18,1,0,0,0);
#if CONF_WITH_PCGEM
			switch (int_in[0])
			{
			case 0:
				int_out[1] = gl_bvdisk;
				int_out[2] = gl_bvdisk >> 16;
				int_out[3] = gl_bvhard;
				int_out[4] = gl_bvhard >> 16;
				break;
			case 1:
				gl_bvdisk = (_ULONG)(uintptr_t)addr_in[0];
				gl_bvhard = (_ULONG)(uintptr_t)addr_in[1];
				break;
			}
#else
			unsupported = TRUE;
#endif
		}
		break;

	case APPL_YIELD:
		aestrace("appl_yield()");
		AES_PARAMS(17,0,1,0,0);
		dsptch();
		break;

	case APPL_EXIT:
		aestrace("appl_exit()");
		AES_PARAMS(19,0,1,0,0);
		ap_exit();
		aes_exit();
		break;

	case APPL_CONTROL:
		aestrace("appl_control()");
		AES_PARAMS(129,2,1,1,0);
		unsupported = TRUE;
		break;

	case APPL_GETINFO:
		if (AIN_LEN >= 4)
		{
			aestrace("appl_getinfo_str()");
			AES_PARAMS(130,1,1,4,0);
		} else
		{
			aestrace("appl_getinfo()");
			AES_PARAMS(130,1,5,0,0);
		}
		unsupported = TRUE;
		break;


	/* Event Manager */
	case EVNT_KEYBD:
		aestrace("evnt_keybd()");
		AES_PARAMS(20,0,1,0,0);
#if NYI
		ret = ev_keybd();
#endif
		break;

	case EVNT_BUTTON:
		aestrace("evnt_button()");
		AES_PARAMS(21,3,5,0,0);
#if NYI
		ret = ev_button(B_CLICKS, B_MASK, B_STATE, &EV_MX);
#endif
		break;

	case EVNT_MOUSE:
		aestrace("evnt_mouse()");
		AES_PARAMS(22,5,5,0,0);
#if NYI
		ret = ev_mouse((const MOBLK *)&MO_FLAGS, &EV_MX);
#endif
		break;

	case EVNT_MESAG:
		aestrace("evnt_mesag()");
		AES_PARAMS(23,0,1,1,0);
#if NYI
		ret = ev_mesag((_WORD *)ME_PBUFF);
#endif
		break;

	case EVNT_TIMER:
		aestrace("evnt_timer()");
		AES_PARAMS(24,2,1,0,0);
#if NYI
		ev_timer(MAKE_ULONG(T_HICOUNT, T_LOCOUNT));
#endif
		break;

	case EVNT_MULTI:
		aestrace("evnt_multi()");
		AES_PARAMS(25,16,7,1,0);
		timeval = 0;
		if (MU_FLAGS & MU_TIMER)
			timeval = MAKE_ULONG(MT_HICOUNT, MT_LOCOUNT);
		lbuparm = combine_cms(MB_CLICKS, MB_MASK, MB_STATE);
#if NYI
		ret = ev_multi(MU_FLAGS, (const MOBLK *)&MMO1_FLAGS, (const MOBLK *)&MMO2_FLAGS, timeval, lbuparm, (_WORD *)MME_PBUFF, &EV_MX);
#endif
		break;

	case EVNT_DCLICK:
		aestrace("evnt_dclick()");
		AES_PARAMS(26,2,1,0,0);
		ret = ev_dclick(EV_DCRATE, EV_DCSETIT);
		break;


	/* Menu Manager */
	case MENU_BAR:
		aestrace("menu_bar()");
		AES_PARAMS(30,1,1,1,0);
		mn_bar((OBJECT *)MM_ITREE, SHOW_IT, rlr->p_pid);
		break;

	case MENU_ICHECK:
		aestrace("menu_icheck()");
		AES_PARAMS(31,2,1,1,0);
		do_chg((OBJECT *)MM_ITREE, ITEM_NUM, OS_CHECKED, CHECK_IT, FALSE, FALSE);
		break;

	case MENU_IENABLE:
		aestrace("menu_ienable()");
		AES_PARAMS(32,2,1,1,0);
		do_chg((OBJECT *)MM_ITREE, (ITEM_NUM & 0x7FFF), OS_DISABLED, !ENABLE_IT, ((ITEM_NUM & 0x8000) != 0x0), FALSE);
		break;

	case MENU_TNORMAL:
		aestrace("menu_tnormal()");
		AES_PARAMS(33,2,1,1,0);
		do_chg((OBJECT *)MM_ITREE, TITLE_NUM, OS_SELECTED, !NORMAL_IT, TRUE, TRUE);
		break;

	case MENU_TEXT:
		aestrace("menu_text()");
		AES_PARAMS(34,1,1,2,0);
		mn_text((OBJECT *)MM_ITREE, ITEM_NUM, (const char *)MM_PTEXT);
		break;

	case MENU_REGISTER:
		aestrace("menu_register()");
		AES_PARAMS(35,1,1,1,0);
		ret = mn_register(MM_PID, (char *)MM_PSTR);
		break;

	case MENU_POPUP:
		/* distinguish between menu_unregister() and menu_popup() */
		if (IN_LEN == 1)
		{
			aestrace("menu_unregister()");
			AES_PARAMS(36,1,1,0,0);
#if CONF_WITH_PCGEM
			mn_unregister(MM_PID);
#else
			unsupported = TRUE;
#endif
		} else
		{
			aestrace("menu_popup()");
			AES_PARAMS(36,2,1,2,0);
			unsupported = TRUE;
		}
		break;

	case MENU_CLICK:
		/* distinguish between menu_click() and menu_attach() */
		/*
		 * although menu_click() is PC-GEM only, it's always
		 * enabled because the desktop uses it.
		 */
		if (AIN_LEN == 0)
		{
			aestrace("menu_click()");
			AES_PARAMS(37,2,1,0,0);
			if (MN_SETIT)
				gl_mnclick = MN_CLICK;
			ret = gl_mnclick;
		} else
		{
			aestrace("menu_attach()");
			AES_PARAMS(37,2,1,2,0);
			unsupported = TRUE;
		}
		break;

	case MENU_ISTART:
		aestrace("menu_istart()");
		AES_PARAMS(38,3,1,1,0);
		unsupported = TRUE;
		break;

	case MENU_SETTINGS:
		aestrace("menu_settings()");
		AES_PARAMS(39,1,1,1,0);
		unsupported = TRUE;
		break;


	/* Object Manager */
	case OBJC_ADD:
		aestrace("objc_add()");
		AES_PARAMS(40,2,1,1,0);
		ob_add((OBJECT *)OB_TREE, OB_PARENT, OB_CHILD);
		break;

	case OBJC_DELETE:
		aestrace("objc_delete()");
		AES_PARAMS(41,1,1,1,0);
		ob_delete((OBJECT *)OB_TREE, OB_DELOB);
		break;

	case OBJC_DRAW:
		aestrace("objc_draw()");
		AES_PARAMS(42,6,1,1,0);
		gsx_sclip((const GRECT *)&OB_XCLIP);
		ob_draw((OBJECT *)OB_TREE, OB_DRAWOB, OB_DEPTH);
		break;

	case OBJC_FIND:
		aestrace("objc_find()");
		AES_PARAMS(43,4,1,1,0);
		ret = ob_find((OBJECT *)OB_TREE, OB_STARTOB, OB_DEPTH, OB_MX, OB_MY);
		break;

	case OBJC_OFFSET:
		aestrace("objc_offset()");
		AES_PARAMS(44,1,3,1,0);
		if (gl_aes3d)
			ob_gclip((OBJECT *)OB_TREE, OB_OBJ, &OB_XOFF, &OB_YOFF, &OB_GX, &OB_GY, &OB_GW, &OB_GH);
		else
			ob_offset((OBJECT *)OB_TREE, OB_OBJ, &OB_XOFF, &OB_YOFF);
		break;

	case OBJC_ORDER:
		aestrace("objc_order()");
		AES_PARAMS(45,2,1,1,0);
		ob_order((OBJECT *)OB_TREE, OB_OBJ, OB_NEWPOS);
		break;

	case OBJC_EDIT:
		aestrace("objc_edit()");
		AES_PARAMS(46,4,2,1,0);
		OB_ODX = OB_IDX;
		ret = ob_edit((OBJECT *)OB_TREE, OB_OBJ, OB_CHAR, &OB_ODX, OB_KIND);
		break;

	case OBJC_CHANGE:
		aestrace("objc_change()");
		AES_PARAMS(47,8,1,1,0);
		gsx_sclip((const GRECT *)&OB_XCLIP);
		ob_change((OBJECT *)OB_TREE, OB_DRAWOB, OB_NEWSTATE, OB_REDRAW);
		break;

	case OBJC_SYSVAR:
		aestrace("objc_sysvar()");
		AES_PARAMS(48,4,3,0,0);
		if (gl_aes3d)
			ret = ob_sysvar(OB_MODE, OB_WHICH, OB_I1, OB_I2, &OB_O1, &OB_O2);
		else
			ret = FALSE;
		break;

	case OBJC_XFIND:
		aestrace("objc_xfind()");
		AES_PARAMS(49,4,1,1,0);
		ret = ob_find((OBJECT *)OB_TREE, OB_STARTOB, OB_DEPTH, OB_MX, OB_MY);
		break;
	

	/* Form Manager */
	case FORM_DO:
		aestrace("form_do()");
		AES_PARAMS(50,1,1,1,0);
#if NYI
		ret = fm_do((OBJECT *)FM_FORM, FM_START);
#endif
		break;	

	case FORM_DIAL:
		aestrace("form_dial()");
		AES_PARAMS(51,9,1,0,0);
#if NYI
		fm_dial(FM_TYPE, (const GRECT *)&FM_IX, (const GRECT *)&FM_X);
#endif
		break;

	case FORM_ALERT:
		aestrace("form_alert()");
		AES_PARAMS(52,1,1,1,0);
		ret = fm_alert(FM_DEFBUT, (const char *)FM_ASTRING, int_in[1]);
		break;

	case FORM_ERROR:
		aestrace("form_error()");
		AES_PARAMS(53,1,1,0,0);
		ret = fm_error(FM_ERRNUM);
		break;

	case FORM_CENTER:
		aestrace("form_center()");
		AES_PARAMS(54,0,5,1,0);
		ob_center((OBJECT *)FM_FORM, (GRECT *)&FM_XC);
		break;

	case FORM_KEYBD:
		aestrace("form_keybd()");
		AES_PARAMS(55,3,3,1,0);
		gsx_sclip(&gl_rfull);
		FM_OCHAR = FM_ICHAR;
		FM_ONXTOB = FM_INXTOB;
		ret = fm_keybd((OBJECT *)FM_FORM, FM_OBJ, &FM_OCHAR, &FM_ONXTOB);
		break;

	case FORM_BUTTON:
		aestrace("form_button()");
		AES_PARAMS(56,2,2,1,0);
		gsx_sclip(&gl_rfull);
		ret = fm_button((OBJECT *)FM_FORM, FM_OBJ, FM_CLKS, &FM_ONXTOB);
		break;
	

	/* Graphics Manager */
	case GRAF_RUBBOX:
		aestrace("graf_rubberbox()");
		AES_PARAMS(70,4,3,0,0);
		gr_rubbox(GR_I1, GR_I2, GR_I3, GR_I4, &GR_O1, &GR_O2);
		break;

	case GRAF_DRAGBOX:
		aestrace("graf_dragbox()");
		AES_PARAMS(71,8,3,0,0);
		gr_dragbox(GR_I1, GR_I2, GR_I3, GR_I4, (const GRECT *)&GR_I5, &GR_O1, &GR_O2);
		break;

	case GRAF_MBOX:
		aestrace("graf_movebox()");
		AES_PARAMS(72,6,1,0,0);
		gr_movebox(GR_I1, GR_I2, GR_I3, GR_I4, GR_I5, GR_I6);
		break;

	case GRAF_GROWBOX:
		aestrace("graf_growbox()");
		AES_PARAMS(73,8,1,0,0);
		gr_growbox((const GRECT *)&GR_I1, (const GRECT *)&GR_I5);
		break;

	case GRAF_SHRINKBOX:
		aestrace("graf_shrinkbox()");
		AES_PARAMS(74,8,1,0,0);
		gr_shrinkbox((const GRECT *)&GR_I1, (const GRECT *)&GR_I5);
		break;

	case GRAF_WATCHBOX:
		aestrace("graf_watchbox()");
		AES_PARAMS(75,4,1,1,0);
		ret = gr_watchbox((OBJECT *)GR_TREE, GR_OBJ, GR_INSTATE, GR_OUTSTATE);
		break;

	case GRAF_SLIDEBOX:
		aestrace("graf_slidebox()");
		AES_PARAMS(76,3,1,1,0);
		ret = gr_slidebox((OBJECT *)GR_TREE, GR_PARENT, GR_OBJ, GR_ISVERT);
		break;

	case GRAF_HANDLE:
/*
 * AES #77 - graf_handle - Obtain the VDI handle of the AES workstation. 
 */
		aestrace("graf_handle()");
		AES_PARAMS(77,0,5,0,0);
		GR_WCHAR = gl_wchar;
		GR_HCHAR = gl_hchar;
		GR_WBOX = gl_wbox;
		GR_HBOX = gl_hbox;
		ret = gl_handle;
		break;

	case GRAF_MOUSE:
		aestrace("graf_mouse()");
		AES_PARAMS(78,1,1,1,0);
		ctlmouse(FALSE);
		gr_mouse(GR_MNUMBER, (MFORM *)GR_MADDR);
		ctlmouse(TRUE);
		break;

	case GRAF_MKSTATE:
		aestrace("graf_mkstate()");
		AES_PARAMS(79,0,5,0,0);
		ret = gr_mkstate(&GR_MX, &GR_MY, &GR_MSTATE, &GR_KSTATE);
		break;
	

	/* Scrap Manager */
	case SCRP_READ:
		aestrace("scrap_read()");
		AES_PARAMS(80,0,1,1,0);
#if NYI
		ret = sc_read((char *)SC_PATH);
#endif
		break;

	case SCRP_WRITE:
		aestrace("scrap_write()");
		AES_PARAMS(81,0,1,1,0);
#if NYI
		ret = sc_write((const char *)SC_PATH);
#endif
		break;

	case SCRP_CLEAR:
		aestrace("scrap_clear()");
		AES_PARAMS(82,0,1,0,0);
#if CONF_WITH_PCGEM
		ret = sc_clear();
#endif
		break;
	

	/* File Selector Manager */
	case FSEL_INPUT:
		aestrace("fs_input()");
		AES_PARAMS(90,0,2,2,0);
		ret = fs_input((char *)FS_IPATH, (char *)FS_ISEL, &FS_BUTTON, rs_str(ITEMSLCT));
		break;

	case FSEL_EXINPUT:
		if (AIN_LEN >= 4)
		{
			aestrace("fsel_boxinput()");
		} else
		{
			aestrace("fsel_exinput()");
		}
		AES_PARAMS(91,0,2,3,0);
		ret = fs_input((char *)FS_IPATH, (char *)FS_ISEL, &FS_BUTTON, (const char *)FS_ILABEL);
		break;


	/* Window Manager */
	case 99:
		aestrace("wind_draw()");
		AES_PARAMS(99,2,1,0,0);
		unsupported = TRUE;
		break;

	case WIND_CREATE:
		if (OUT_LEN >= 5)
		{
			aestrace("wind_xcreate()");
		} else
		{
			aestrace("wind_create()");
		}
		AES_PARAMS(100,5,1,0,0);
		ret = wm_create(WM_KIND, (const GRECT *)&WM_WX);
		break;

	case WIND_OPEN:
		aestrace("wind_open()");
		AES_PARAMS(101,5,1,0,0);
		wm_open(WM_HANDLE, (const GRECT *)&WM_WX);
		break;

	case WIND_CLOSE:
		aestrace("wind_close()");
		AES_PARAMS(102,1,1,0,0);
		wm_close(WM_HANDLE);
		break;

	case WIND_DELETE:
		aestrace("wind_delete()");
		AES_PARAMS(103,1,1,0,0);
		wm_delete(WM_HANDLE);
		break;

	case WIND_GET:
		aestrace("wind_get()");
		AES_PARAMS(104,2,5,0,0);
		ret = wm_get(WM_HANDLE, WM_WFIELD, &WM_OX, &WM_IX);
		break;

	case WIND_SET:
		aestrace("wind_set()");
		AES_PARAMS(105,6,1,0,0);
		ret = wm_set(WM_HANDLE, WM_WFIELD, &WM_OX);
		break;

	case WIND_FIND:
		aestrace("wind_find()");
		AES_PARAMS(106,2,1,0,0);
		ret = wm_find(WM_MX, WM_MY);
		break;

	case WIND_UPDATE:
		aestrace("wind_update()");
		AES_PARAMS(107,1,1,0,0);
#if NYI
		ret = wm_update(WM_BEGUP);
#endif
		break;

	case WIND_CALC:
		aestrace("wind_calc()");
		AES_PARAMS(108,6,5,0,0);
		ret = wm_calc(WM_WCTYPE, WM_WCKIND, (const GRECT *)&WM_WCIX, (GRECT *)&WM_WCOX);
		break;

	case WIND_NEW:
		aestrace("wind_new()");
		AES_PARAMS(109,0,0,0,0);
		ret = wm_new();
		break;


	/* Resource Manager */
	case RSRC_LOAD:
		aestrace("rsrc_load()");
		AES_PARAMS(110,0,1,1,0);
		ret = rs_load(pglobal, (const char *)RS_PFNAME);
		break;

	case RSRC_FREE:
		aestrace("rsrc_free()");
		AES_PARAMS(111,0,1,0,0);
		ret = rs_free(pglobal);
		break;

	case RSRC_GADDR:
		aestrace("rsrc_gaddr()");
		AES_PARAMS(112,2,1,0,1);
		ret = rs_gaddr(pglobal, RS_TYPE, RS_INDEX, &pb->addrout[0]);
		break;

	case RSRC_SADDR:
		aestrace("rsrc_saddr()");
		AES_PARAMS(113,2,1,1,0);
		ret = rs_saddr(pglobal, RS_TYPE, RS_INDEX, RS_INADDR);
		break;

	case RSRC_OBFIX:
		aestrace("rsrc_obfix()");
		AES_PARAMS(114,1,1,1,0);
		rs_obfix((OBJECT *)RS_TREE, RS_OBJ);
		break;

	case RSRC_RCFIX:
		aestrace("rsrc_rcfix()");
		AES_PARAMS(115,0,1,1,0);
		unsupported = TRUE;
		break;
	

	/* Shell Manager */
	case SHEL_READ:
		aestrace("shel_read()");
		AES_PARAMS(120,0,1,2,0);
#if NYI
		ret = sh_read((char *)SH_PCMD, (char *)SH_PTAIL);
#endif
		break;

	case SHEL_WRITE:
		aestrace("shel_write()");
		AES_PARAMS(121,3,1,2,0);
#if NYI
		ret = sh_write(SH_DOEX, SH_ISGR, SH_ISCR, (char *)SH_PCMD, (char *)SH_PTAIL);
#endif
		break;

	case SHEL_GET:
		aestrace("shel_get()");
		AES_PARAMS(122,1,1,1,0);
#if NYI
		ret = sh_get((char *)SH_PBUFFER, SH_LEN);
#endif
		break;

	case SHEL_PUT:
		aestrace("shel_put()");
		AES_PARAMS(123,1,1,1,0);
#if NYI
		ret = sh_put((const char *)SH_PDATA, SH_LEN);
#endif
		break;

	case SHEL_FIND:
		aestrace("shel_find()");
		AES_PARAMS(124,0,1,1,0);
#if NYI
		ret = sh_find((char *)SH_PATH);
#endif
		break;

	case SHEL_ENVRN:
		aestrace("shel_envrn()");
		AES_PARAMS(125,0,1,2,0);
#if NYI
		ret = sh_envrn((char **)SH_PATH, (char *)SH_SRCH);
#endif
		break;

	case SHEL_RDEF:
		aestrace("shel_rdef()");
		AES_PARAMS(126,0,1,2,0);
#if CONF_WITH_PCGEM
		sh_rdef((char *)SH_LPCMD, (char *)SH_LPDIR);
#else
		unsupported = TRUE;
#endif
		break;

	case SHEL_WDEF:
		aestrace("shel_wdef()");
		AES_PARAMS(127,0,1,2,0);
#if CONF_WITH_PCGEM
		sh_wdef((const char *)SH_LPCMD, (const char *)SH_LPDIR);
#else
		unsupported = TRUE;
#endif
		break;

	case SHEL_HELP:
		aestrace("shel_help()");
		AES_PARAMS(128,1,1,2,0);
		unsupported = TRUE;
		break;

	default:
		unsupported = TRUE;
		break;
	}
	
	if (unsupported)
	{
		KINFO(("Bad AES function %d\n", opcode));
		if (opcode != 0)	/* Ignore zero since some PRGs are this call */
			fm_show(ALNOFUNC, 1, opcode);
		ret = -1;
	}

	RET_CODE = ret;
		
	UNUSED(lbuparm);
	UNUSED(timeval);
	
	return ret;
}


_WORD aestrap(AESPB *pb)
{
	return crysbind(pb);
}
