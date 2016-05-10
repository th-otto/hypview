#ifndef _LANGID_H
#define _LANGID_H

#include "sparse.h"

/* Structure containing all the state required to
 * implement a language identifier
 */
typedef struct {
    unsigned int num_feats;
    unsigned int num_langs;
    unsigned int num_states;

    unsigned int const (*tk_nextmove)[][256];
    unsigned int const (*tk_output_c)[];
    unsigned int const (*tk_output_s)[];
    unsigned int const (*tk_output)[];

    double const (*nb_pc)[];
    double const (*nb_ptc)[];

    const char *const (*nb_classes)[];

    /* sparsesets for counting states and features. these are
     * part of LanguageIdentifier as the clear operation on them
     * is much less costly than allocating them from scratch
     */
	Set *sv, *fv;
} LanguageIdentifier;

extern LanguageIdentifier *langid_get_default_identifier(void);
extern LanguageIdentifier *langid_load_identifier(char*);
extern void langid_destroy_identifier(LanguageIdentifier*);
extern const char *langid_identify(LanguageIdentifier*, const char *, size_t, double *);

#endif
