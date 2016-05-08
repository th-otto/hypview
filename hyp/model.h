#ifndef _MODEL_H
#define _MODEL_H

#define NUM_FEATS 7480
#define NUM_LANGS 97
#define NUM_STATES 9118

extern unsigned int const tk_nextmove[NUM_STATES][256];
extern unsigned int const tk_output_c[NUM_STATES];
extern unsigned int const tk_output_s[NUM_STATES];
extern unsigned int const tk_output[];
extern double const nb_pc[NUM_LANGS];
extern double const nb_ptc[725560];
extern const char *const nb_classes[NUM_LANGS];

#endif
