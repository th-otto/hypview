/*
 * Implementation of the Language Identification method of Lui & Baldwin 2011
 * in pure C, based largely on langid.py, using the sparse set structures suggested
 * by Dawid Weiss.
 *
 * Marco Lui <saffsd@gmail.com>, July 2014
 */

#include "hypdefs.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include "liblangid.h"
#include "model.h"

/* Return a pointer to a LanguageIdentifier based on the in-built default model
 */
LanguageIdentifier *langid_get_default_identifier(void)
{
	LanguageIdentifier *lid;

	if ((lid = g_new0(LanguageIdentifier, 1)) == NULL)
		return NULL;

	lid->sv = langid_alloc_set(NUM_STATES);
	lid->fv = langid_alloc_set(NUM_FEATS);
	if (lid->sv == NULL || lid->fv == NULL)
	{
		langid_destroy_identifier(lid);
		return NULL;
	}

	lid->num_feats = NUM_FEATS;
	lid->num_langs = NUM_LANGS;
	lid->num_states = NUM_STATES;
	lid->tk_nextmove = &tk_nextmove;
	lid->tk_output_c = &tk_output_c;
	lid->tk_output_s = &tk_output_s;
	lid->tk_output = &tk_output;
	lid->nb_pc = &nb_pc;
	lid->nb_ptc = &nb_ptc;
	lid->nb_classes = &nb_classes;

	return lid;
}

void langid_destroy_identifier(LanguageIdentifier *lid)
{
	if (lid != NULL)
	{
		langid_free_set(lid->sv);
		langid_free_set(lid->fv);
		g_free(lid);
	}
}

/* 
 * Convert a text stream into a feature vector. The feature vector counts
 * how many times each sequence is seen.
 */
static void text_to_fv(LanguageIdentifier *lid, const char *text, size_t textlen, Set *sv, Set *fv)
{
	size_t i;
	unsigned int j, m, s = 0;

	langid_clear_set(sv);
	langid_clear_set(fv);

	for (i = 0; i < textlen; i++)
	{
		s = (*lid->tk_nextmove)[s][(unsigned char) text[i]];
		langid_add_set(sv, s, 1);
	}

	/* convert the SV into the FV */
	for (i = 0; i < sv->members; i++)
	{
		m = sv->dense[i];
		for (j = 0; j < (*lid->tk_output_c)[m]; j++)
		{
			langid_add_set(fv, (*lid->tk_output)[(*lid->tk_output_s)[m] + j], sv->counts[i]);
		}
	}
}


static void fv_to_logprob(LanguageIdentifier *lid, Set *fv, double logprob[])
{
	unsigned i, j, m;
	const double *nb_ptc_p;

	/* Initialize using prior */
	for (i = 0; i < lid->num_langs; i++)
	{
		logprob[i] = (*lid->nb_pc)[i];
	}

	/* Compute posterior for each class */
	for (i = 0; i < fv->members; i++)
	{
		m = fv->dense[i];
		/* NUM_FEATS * NUM_LANGS */
		nb_ptc_p = &(*lid->nb_ptc)[m * lid->num_langs];
		for (j = 0; j < lid->num_langs; j++)
		{
			logprob[j] += fv->counts[i] * (*nb_ptc_p);
			nb_ptc_p += 1;
		}
	}
}


static int logprob_to_pred(LanguageIdentifier *lid, const double logprob[])
{
	int m = 0;
	unsigned int i;

	for (i = 1; i < lid->num_langs; i++)
	{
		if (logprob[m] < logprob[i])
			m = i;
	}

	return m;
}


const char *langid_identify(LanguageIdentifier *lid, const char *text, size_t textlen, double *prob)
{
	double lp[lid->num_langs];
	int pred;
	
	if (lid == NULL)
		return NULL;
	text_to_fv(lid, text, textlen, lid->sv, lid->fv);
	fv_to_logprob(lid, lid->fv, lp);
	pred = logprob_to_pred(lid, lp);

#if defined(DEBUG)
	{
		int i;
		fprintf(stderr, "pred lang: %s logprob: %lf\n", (*lid->nb_classes)[pred], lp[pred]);
		for (i = 0; i < lid->num_langs; i++)
		{
			fprintf(stderr, "  lang: %s logprob: %lf\n", (*lid->nb_classes)[i], lp[i]);
		}
	}
#endif
#if 0
	{
		double maxval;
		double norm_prob = 0;
		maxval = lp[pred];
		{
			int i;
			for (i = 0; i < lid->num_langs; i++)
				if (fabs(lp[i]) > fabs(maxval))
					maxval = lp[i];
			for (i = 0; i < lid->num_langs; i++)
			{
				norm_prob += 1 / exp(lp[i] - maxval);
			}
		}
		*prob = norm_prob;
	}
#else
	*prob = lp[pred];
#endif
	return (*lid->nb_classes)[pred];
}
