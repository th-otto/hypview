#ifndef _MODEL_H
#define _MODEL_H

#define NUM_FEATS 7480
#define NUM_LANGS 97
#define NUM_STATES 9118

extern unsigned int const langid_tk_nextmove[NUM_STATES][256];
extern unsigned int const langid_tk_output_c[NUM_STATES];
extern unsigned int const langid_tk_output_s[NUM_STATES];
extern unsigned int const langid_tk_output[];
extern double const langid_nb_pc[NUM_LANGS];
extern double const langid_nb_ptc[NUM_FEATS * NUM_LANGS];
extern const char *const langid_nb_classes[NUM_LANGS];

#endif
