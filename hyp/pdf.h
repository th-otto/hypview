#ifndef __HYPPDF_H__
#define __HYPPDF_H__ 1

#ifdef WITH_PDF

typedef struct _pdf PDF;
#if defined(__PUREC__) && !defined(__HYP_PDF_IMPLEMENTATION__)
struct _pdf { int dummy; };
#endif

PDF *pdf_new(hcp_opts *opts);
void pdf_delete(PDF *pdf);

gboolean recompile_pdf(HYP_DOCUMENT *hyp, hcp_opts *opts, int argc, const char **argv);

#endif /* WITH_PDF */

#endif /* __HYPPDF_H__ */
