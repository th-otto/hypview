#ifndef __HYPPDF_H__
#define __HYPPDF_H__ 1

#include <setjmp.h>
#include "hpdf.h"

typedef struct _pdf {
	HPDF_Doc hpdf;
	hcp_opts *opts;
	jmp_buf error_env;
	HPDF_Font font;
} PDF;

PDF *pdf_new(hcp_opts *opts);
void pdf_delete(PDF *pdf);

void pdf_out_globals(HYP_DOCUMENT *hyp, PDF *pdf);
gboolean recompile_pdf(HYP_DOCUMENT *hyp, hcp_opts *opts, int argc, const char **argv);

#endif /* __HYPPDF_H__ */
