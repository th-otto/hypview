typedef struct _pdf_obj {
	struct _pdf_obj *next;
	GString *out;
	GString *stream;
	int num;
	size_t fileoffset;
	size_t len;
} PDF_OBJ;


typedef struct _pdf {
	GString *out;
	PDF_OBJ *objects;
	PDF_OBJ **last_obj;
	int obj_num;
} PDF;

PDF *pdf_new(void);
void pdf_delete(PDF *pdf);
PDF_OBJ *pdf_obj_new(PDF *pdf);

void pdf_out_globals(HYP_DOCUMENT *hyp, hcp_opts *opts, PDF *pdf);
gboolean recompile_pdf(HYP_DOCUMENT *hyp, hcp_opts *opts, int argc, const char **argv);
