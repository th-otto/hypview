#include "vdi.h"
#include "debug.h"
#include "gemdos.h"
#include "ro_mem.h"


#define NYI(_x) static int bm_ ## _x(VWK *v, VDIPB *pb) { UNUSED(v); UNUSED(pb); V("%s[%d] NOT YET IMPLEMENTED", #_x, PV_CONTROL(pb)[6]); return VDI_DONE; }

/******************************************************************************/
/* -------------------------------------------------------------------------- */
/******************************************************************************/

static int bm_v_clrwk(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	int width = (v->width + 15) & ~0x0f;
	long lwidth = width * v->planes;
	long size = lwidth * v->height;
	
	V("v_clrwk[%d]", v->handle);

	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);
	memset(v->bitmap_addr, 0, size);
	return VDI_DONE;
}

/******************************************************************************/

static int bm_v_clsbm(VWK *v, VDIPB *pb)
{
	_WORD *control = PV_CONTROL(pb);
	_WORD handle = v->handle;
	
	V("v_clsbm[%d]", handle);

	V_NINTOUT(pb, 0);
	V_NPTSOUT(pb, 0);

	vdi_release_handle(pb, handle);
	
	vdi_clswk(v);
	
	return VDI_DONE;
}

/* -------------------------------------------------------------------------- */

NYI(v_pline)
NYI(v_bez)
NYI(v_pmarker)
NYI(v_fillarea)
NYI(vr_recfl)
NYI(v_bar)
NYI(v_arc)
NYI(v_pieslice)
NYI(v_circle)
NYI(v_ellipse)
NYI(v_ellarc)
NYI(v_ellpie)
NYI(v_rbox)
NYI(v_rfbox)
NYI(v_gtext)
NYI(v_justified)
NYI(vro_cpyfm)
NYI(vrt_cpyfm)

void vdi_init_bm(VWK *v)
{
	vdi_init_common(v);
	v->drv.v_clsbm = bm_v_clsbm;
	v->drv.v_clrwk = bm_v_clrwk;
	v->drv.v_pline = bm_v_pline;
	v->drv.v_bez = bm_v_bez;
	v->drv.v_pmarker = bm_v_pmarker;
	v->drv.v_fillarea = bm_v_fillarea;
	v->drv.vr_recfl = bm_vr_recfl;
	v->drv.v_bar = bm_v_bar;
	v->drv.v_arc = bm_v_arc;
	v->drv.v_pieslice = bm_v_pieslice;
	v->drv.v_circle = bm_v_circle;
	v->drv.v_ellipse = bm_v_ellipse;
	v->drv.v_ellarc = bm_v_ellarc;
	v->drv.v_ellpie = bm_v_ellpie;
	v->drv.v_rbox = bm_v_rbox;
	v->drv.v_rfbox = bm_v_rfbox;
	v->drv.v_gtext = bm_v_gtext;
	v->drv.v_justified = bm_v_justified;
	v->drv.vro_cpyfm = bm_vro_cpyfm;
	v->drv.vrt_cpyfm = bm_vrt_cpyfm;
}
