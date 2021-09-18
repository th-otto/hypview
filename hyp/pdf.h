#ifndef __HYPPDF_H__
#define __HYPPDF_H__ 1

#ifdef WITH_PDF

#include <setjmp.h>
#include "hpdf.h"

struct pdf_page {
	HPDF_Page page;
	hyp_nodenr node;
};

typedef struct _pdf {
	HPDF_Doc hpdf;
	hcp_opts *opts;
	jmp_buf error_env;
	HPDF_Font regular_font;
	HPDF_Font bold_font;
	HPDF_Font italic_font;
	HPDF_Font bold_italic_font;
	HPDF_REAL font_size;
	HPDF_REAL line_height;
	HPDF_REAL cell_width;
	size_t num_pages;
	size_t curr_page_num;
	struct pdf_page *pages;
	HPDF_Page *links;
	HPDF_Page page;
	HPDF_REAL page_width;
	HPDF_REAL page_height;
	struct hyp_gfx *hyp_gfx;
} PDF;

PDF *pdf_new(hcp_opts *opts);
void pdf_delete(PDF *pdf);

gboolean recompile_pdf(HYP_DOCUMENT *hyp, hcp_opts *opts, int argc, const char **argv);

#endif /* WITH_PDF */

#endif /* __HYPPDF_H__ */
