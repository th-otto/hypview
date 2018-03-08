/*
 * compiler versions:
 * Version 3 introduced @limage
 * Version 6 introduced charsets, and possibility of UTF-8 encoding
 */
#define HCP_COMPILER_VERSION 6

/*
 * evaluated by "@if VERSION"
 */
#define HCP_COMPILER_VERSION_STRING "6"

/*
 * "filename" that can be specified to indicate
 * the output filename should be created based
 * on the input filename.
 */
#define HCP_OUTPUT_WILDCARD "~"

#ifdef __HCP_OPTS_H__
gboolean hcp_compile(const char *filename, hcp_opts *opts);
#endif
