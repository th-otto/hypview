/*********************************************************************
 * LH5 encoder
 *********************************************************************/

#include "hypdefs.h"
#include "hypdebug.h"
#include <limits.h>
#include "lh5int.h"


#define PERCOLATE  1
#undef NIL
#define NIL        0
#define MAX_HASH_VAL (3 * DICSIZ + (DICSIZ / 512 + 1) * UCHAR_MAX)

typedef short node;

#if MAXMATCH <= (UCHAR_MAX + 1)
typedef unsigned char lh5_level_t;
#else
typedef unsigned short lh5_level_t;
#endif

typedef struct {
	short bitcount;
	unsigned short subbitbuf;
	gboolean unpackable;

	unsigned long packedLen;
	unsigned long unpackedLen;
	unsigned long memsize;
	const unsigned char *unpackedMem;
	FILE *outfile;

	int remainder, matchlen;
	node matchpos;

	unsigned int bufsiz;
	unsigned char *buf;
	unsigned int output_pos, output_mask;
	unsigned int cpos;

	node pos, avail;
	
	unsigned char c_len[NC];
	unsigned char pt_len[NPT];

	unsigned short c_freq[2 * NC - 1];
	unsigned short c_code[NC];
	unsigned short p_freq[2 * NP - 1];
	unsigned short pt_code[NPT];

	unsigned short t_freq[2 * NT - 1];

	int heapsize;
	int heap_n;
	short heap[NC + 1];

	unsigned short *freq;
	unsigned short *sortptr;
	unsigned short len_cnt[17];
	unsigned char *len;

	unsigned char *text;
	unsigned char *childcount;
	node *position;
	node *parent;
	node *prev, *next;
	lh5_level_t *level;
	unsigned short *left, *right;
	
	unsigned char text_[DICSIZ * 2 + MAXMATCH];
	unsigned char childcount_[DICSIZ + UCHAR_MAX + 1];
#if PERCOLATE
	node position_[DICSIZ + UCHAR_MAX + 1];
#else
	node position_[DICSIZ];
#endif
	node parent_[DICSIZ * 2];
	node prev_[DICSIZ * 2];
	node next_[MAX_HASH_VAL + 1];
	lh5_level_t level_[DICSIZ + UCHAR_MAX + 1];
	unsigned short left_[2 * NC - 1];
	unsigned short right_[2 * NC - 1];
} lh5_encoder;

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/* Write rightmost n bits of x */
static gboolean putbits(lh5_encoder *lh5, int n, unsigned int x)
{
	if (n < lh5->bitcount)
	{
		lh5->subbitbuf |= x << (lh5->bitcount -= n);
	} else
	{
		if (lh5->packedLen < lh5->unpackedLen)
		{
			if (putc(lh5->subbitbuf | (x >> (n -= lh5->bitcount)), lh5->outfile) == EOF)
				return FALSE;
			lh5->packedLen++;
		} else
		{
			lh5->unpackable = TRUE;
		}
		if (n < CHAR_BIT)
		{
			lh5->subbitbuf = x << (lh5->bitcount = CHAR_BIT - n);
		} else
		{
			if (lh5->packedLen < lh5->unpackedLen)
			{
				if (putc(x >> (n - CHAR_BIT), lh5->outfile) == EOF)
					return FALSE;
				lh5->packedLen++;
			} else
			{
				lh5->unpackable = TRUE;
			}
			lh5->subbitbuf = x << (lh5->bitcount = 2 * CHAR_BIT - n);
		}
	}
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static void count_t_freq(lh5_encoder *lh5)
{
	int i, k, n, count;

	for (i = 0; i < NT; i++)
		lh5->t_freq[i] = 0;
	n = NC;
	while (n > 0 && lh5->c_len[n - 1] == 0)
		n--;
	i = 0;
	while (i < n)
	{
		k = lh5->c_len[i++];
		if (k == 0)
		{
			count = 1;
			while (i < n && lh5->c_len[i] == 0)
			{
				i++;
				count++;
			}
			if (count <= 2)
				lh5->t_freq[0] += count;
			else if (count <= 18)
				lh5->t_freq[1]++;
			else if (count == 19)
			{
				lh5->t_freq[0]++;
				lh5->t_freq[1]++;
			} else
			{
				lh5->t_freq[2]++;
			}
		} else
		{
			lh5->t_freq[k + 2]++;
		}
	}
}

/* ------------------------------------------------------------------------- */

static gboolean write_pt_len(lh5_encoder *lh5, int n, int nbit, int i_special)
{
	int i, k;

	while (n > 0 && lh5->pt_len[n - 1] == 0)
		n--;
	if (putbits(lh5, nbit, n) == FALSE)
		return FALSE;
	i = 0;
	while (i < n)
	{
		k = lh5->pt_len[i++];
		if (k <= 6)
		{
			if (putbits(lh5, 3, k) == FALSE)
				return FALSE;
		} else
		{
			if (putbits(lh5, k - 3, (1U << (k - 3)) - 2) == FALSE)
				return FALSE;
		}
		if (i == i_special)
		{
			while (i < 6 && lh5->pt_len[i] == 0)
				i++;
			if (putbits(lh5, 2, (i - 3) & 3) == FALSE)
				return FALSE;
		}
	}
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean write_c_len(lh5_encoder *lh5)
{
	int i, k, n, count;

	n = NC;
	while (n > 0 && lh5->c_len[n - 1] == 0)
		n--;
	if (putbits(lh5, CBIT, n) == FALSE)
		return FALSE;
	i = 0;
	while (i < n)
	{
		k = lh5->c_len[i++];
		if (k == 0)
		{
			count = 1;
			while (i < n && lh5->c_len[i] == 0)
			{
				i++;
				count++;
			}
			if (count <= 2)
			{
				for (k = 0; k < count; k++)
					if (putbits(lh5, lh5->pt_len[0], lh5->pt_code[0]) == FALSE)
						return FALSE;
			} else if (count <= 18)
			{
				if (putbits(lh5, lh5->pt_len[1], lh5->pt_code[1]) == FALSE)
					return FALSE;
				if (putbits(lh5, 4, count - 3) == FALSE)
					return FALSE;
			} else if (count == 19)
			{
				if (putbits(lh5, lh5->pt_len[0], lh5->pt_code[0]) == FALSE)
					return FALSE;
				if (putbits(lh5, lh5->pt_len[1], lh5->pt_code[1]) == FALSE)
					return FALSE;
				if (putbits(lh5, 4, 15) == FALSE)
					return FALSE;
			} else
			{
				if (putbits(lh5, lh5->pt_len[2], lh5->pt_code[2]) == FALSE)
					return FALSE;
				if (putbits(lh5, CBIT, count - 20) == FALSE)
					return FALSE;
			}
		} else
		{
			if (putbits(lh5, lh5->pt_len[k + 2], lh5->pt_code[k + 2]) == FALSE)
				return FALSE;
		}
	}
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean encode_c(lh5_encoder *lh5, int c)
{
	return putbits(lh5, lh5->c_len[c], lh5->c_code[c]);
}

/* ------------------------------------------------------------------------- */

static gboolean encode_p(lh5_encoder *lh5, unsigned int p)
{
	unsigned int c, q;

	c = 0;
	q = p;
	while (q)
	{
		q >>= 1;
		c++;
	}
	if (putbits(lh5, lh5->pt_len[c], lh5->pt_code[c]) == FALSE)
		return FALSE;
	if (c > 1)
		if (putbits(lh5, c - 1, p & (0xFFFFU >> (17 - c))) == FALSE)
			return FALSE;
	return TRUE;
}

/*****************************************************************************/
/* ------------------------------------------------------------------------- */
/*****************************************************************************/

/***********************************************************
	maketree.c -- make Huffman tree
***********************************************************/

/* priority queue; send i-th entry down heap */
static void downheap(lh5_encoder *lh5, int i)
{
	int j, k;

	k = lh5->heap[i];
	while ((j = 2 * i) <= lh5->heapsize)
	{
		if (j < lh5->heapsize && lh5->freq[lh5->heap[j]] > lh5->freq[lh5->heap[j + 1]])
			j++;
		if (lh5->freq[k] <= lh5->freq[lh5->heap[j]])
			break;
		lh5->heap[i] = lh5->heap[j];
		i = j;
	}
	lh5->heap[i] = k;
}

/* ------------------------------------------------------------------------- */

static void count_len(lh5_encoder *lh5, int i, int depth)			/* call with i = root */
{
	unsigned short *left = lh5->left;
	unsigned short *right = lh5->right;
	
	if (i < lh5->heap_n)
	{
		lh5->len_cnt[(depth < 16) ? depth : 16]++;
	} else
	{
		++depth;
		count_len(lh5, left[i], depth);
		count_len(lh5, right[i], depth);
	}
}

/* ------------------------------------------------------------------------- */

static void make_len(lh5_encoder *lh5, int root)
{
	int i, k;
	unsigned long cum;

	for (i = 0; i <= 16; i++)
		lh5->len_cnt[i] = 0;
	count_len(lh5, root, 0);
	cum = 0;
	for (i = 16; i > 0; i--)
		cum += ((unsigned long)lh5->len_cnt[i]) << (16 - i);
	while (cum != (1UL << 16))
	{
		/* fprintf(stderr, "17"); */
		lh5->len_cnt[16]--;
		for (i = 15; i > 0; i--)
		{
			if (lh5->len_cnt[i] != 0)
			{
				lh5->len_cnt[i]--;
				lh5->len_cnt[i + 1] += 2;
				break;
			}
		}
		cum--;
	}
	for (i = 16; i > 0; i--)
	{
		k = lh5->len_cnt[i];
		while (--k >= 0)
			lh5->len[*(lh5->sortptr)++] = i;
	}
}

/* ------------------------------------------------------------------------- */

static void make_code(lh5_encoder *lh5, int n, unsigned char len[], unsigned short code[])
{
	int i;
	unsigned short start[18];

	start[1] = 0;
	for (i = 1; i <= 16; i++)
		start[i + 1] = (start[i] + lh5->len_cnt[i]) << 1;
	for (i = 0; i < n; i++)
		code[i] = start[len[i]]++;
}

/* ------------------------------------------------------------------------- */

/* make tree, calculate len[], return root */
static int make_tree(lh5_encoder *lh5, int nparm, unsigned short freqparm[], unsigned char lenparm[], unsigned short codeparm[])
{
	int i, j, k, avail;
	unsigned short *left = lh5->left;
	unsigned short *right = lh5->right;

	lh5->heap_n = nparm;
	lh5->freq = freqparm;
	lh5->len = lenparm;
	avail = lh5->heap_n;
	lh5->heapsize = 0;
	lh5->heap[1] = 0;
	for (i = 0; i < lh5->heap_n; i++)
	{
		lh5->len[i] = 0;
		if (lh5->freq[i])
			lh5->heap[++lh5->heapsize] = i;
	}
	if (lh5->heapsize < 2)
	{
		codeparm[lh5->heap[1]] = 0;
		return lh5->heap[1];
	}
	for (i = lh5->heapsize / 2; i >= 1; i--)
		downheap(lh5, i);					/* make priority queue */
	lh5->sortptr = codeparm;
	do
	{									/* while queue has at least two entries */
		i = lh5->heap[1];				/* take out least-freq entry */
		if (i < lh5->heap_n)
			*(lh5->sortptr)++ = i;
		lh5->heap[1] = lh5->heap[lh5->heapsize--];
		downheap(lh5, 1);
		j = lh5->heap[1];				/* next least-freq entry */
		if (j < lh5->heap_n)
			*(lh5->sortptr)++ = j;
		k = avail++;					/* generate new node */
		lh5->freq[k] = lh5->freq[i] + lh5->freq[j];
		lh5->heap[1] = k;
		downheap(lh5, 1);				/* put into queue */
		left[k] = i;
		right[k] = j;
	} while (lh5->heapsize > 1);
	lh5->sortptr = codeparm;
	make_len(lh5, k);
	make_code(lh5, nparm, lenparm, codeparm);
	return k;							/* return root */
}

/* ------------------------------------------------------------------------- */

static gboolean send_block(lh5_encoder *lh5)
{
	unsigned int i, k, flags, root, pos, size;

	root = make_tree(lh5, NC, lh5->c_freq, lh5->c_len, lh5->c_code);
	size = lh5->c_freq[root];
	if (putbits(lh5, 16, size) == FALSE)
		return FALSE;
	if (root >= NC)
	{
		count_t_freq(lh5);
		root = make_tree(lh5, NT, lh5->t_freq, lh5->pt_len, lh5->pt_code);
		if (root >= NT)
		{
			if (write_pt_len(lh5, NT, TBIT, 3) == FALSE)
				return FALSE;
		} else
		{
			if (putbits(lh5, TBIT, 0) == FALSE)
				return FALSE;
			if (putbits(lh5, TBIT, root) == FALSE)
				return FALSE;
		}
		if (write_c_len(lh5) == FALSE)
			return FALSE;
	} else
	{
		if (putbits(lh5, TBIT, 0) == FALSE)
			return FALSE;
		if (putbits(lh5, TBIT, 0) == FALSE)
			return FALSE;
		if (putbits(lh5, CBIT, 0) == FALSE)
			return FALSE;
		if (putbits(lh5, CBIT, root) == FALSE)
			return FALSE;
	}
	root = make_tree(lh5, NP, lh5->p_freq, lh5->pt_len, lh5->pt_code);
	if (root >= NP)
	{
		if (write_pt_len(lh5, NP, PBIT, -1) == FALSE)
			return FALSE;
	} else
	{
		if (putbits(lh5, PBIT, 0) == FALSE)
			return FALSE;
		if (putbits(lh5, PBIT, root) == FALSE)
			return FALSE;
	}
	pos = 0;
	flags = 0;
	for (i = 0; i < size; i++)
	{
		if (i % CHAR_BIT == 0)
			flags = lh5->buf[pos++];
		else
			flags <<= 1;
		if (flags & (1U << (CHAR_BIT - 1)))
		{
			if (encode_c(lh5, lh5->buf[pos++] + (1U << CHAR_BIT)) == FALSE)
				return FALSE;
			k = lh5->buf[pos++] << CHAR_BIT;
			k += lh5->buf[pos++];
			if (encode_p(lh5, k) == FALSE)
				return FALSE;
		} else
		{
			if (encode_c(lh5, lh5->buf[pos++]) == FALSE)
				return FALSE;
		}
		if (lh5->unpackable)
			return TRUE;
	}
	for (i = 0; i < NC; i++)
		lh5->c_freq[i] = 0;
	for (i = 0; i < NP; i++)
		lh5->p_freq[i] = 0;
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean output5(lh5_encoder *lh5, unsigned int c, unsigned int p)
{
	if ((lh5->output_mask >>= 1) == 0)
	{
		lh5->output_mask = 1U << (CHAR_BIT - 1);
		if (lh5->output_pos >= lh5->bufsiz - 3 * CHAR_BIT)
		{
			if (send_block(lh5) == FALSE)
				return FALSE;
			if (lh5->unpackable)
				return TRUE;
			lh5->output_pos = 0;
		}
		lh5->cpos = lh5->output_pos++;
		lh5->buf[lh5->cpos] = 0;
	}
	lh5->buf[lh5->output_pos++] = (unsigned char) c;
	lh5->c_freq[c]++;
	if (c >= (1U << CHAR_BIT))
	{
		lh5->buf[lh5->cpos] |= lh5->output_mask;
		lh5->buf[lh5->output_pos++] = (unsigned char) (p >> CHAR_BIT);
		lh5->buf[lh5->output_pos++] = (unsigned char) p;
		c = 0;
		while (p)
		{
			p >>= 1;
			c++;
		}
		lh5->p_freq[c]++;
	}
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static void init_putbits(lh5_encoder *lh5)
{
	lh5->bitcount = CHAR_BIT;
	lh5->subbitbuf = 0;
}

/* ------------------------------------------------------------------------- */

static gboolean start_huf(lh5_encoder *lh5)
{
	int i;

	if (lh5->bufsiz == 0)
		lh5->bufsiz = 16 * 1024U;
	while ((lh5->buf = (unsigned char *)g_malloc(lh5->bufsiz)) == NULL)
	{
		lh5->bufsiz = (lh5->bufsiz / 10U) * 9U;
		if (lh5->bufsiz < 4 * 1024U)
		{
			return FALSE;
		}
	}
	lh5->buf[0] = 0;
	for (i = 0; i < NC; i++)
		lh5->c_freq[i] = 0;
	for (i = 0; i < NP; i++)
		lh5->p_freq[i] = 0;
	lh5->output_pos = lh5->output_mask = 0;
	init_putbits(lh5);
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static gboolean end_huf(lh5_encoder *lh5)
{
	if (!lh5->unpackable)
	{
		if (send_block(lh5) == FALSE)
			return FALSE;
		if (putbits(lh5, CHAR_BIT - 1, 0) == FALSE)		/* flush remaining bits */
			return FALSE;
	}
	return TRUE;
}

/* ------------------------------------------------------------------------- */

static void init_slide(lh5_encoder *lh5)
{
	node i;
	node *position = lh5->position;
	node *parent = lh5->parent;
	node *next = lh5->next;
	lh5_level_t *level = lh5->level;
	
	for (i = DICSIZ; i <= (node)(DICSIZ + UCHAR_MAX); i++)
	{
		level[i] = 1;
#if PERCOLATE
		position[i] = NIL;				/* sentinel */
#endif
	}
	for (i = DICSIZ; i < (node)(DICSIZ * 2); i++)
		parent[i] = NIL;
	lh5->avail = 1;
	for (i = 1; i < (node)(DICSIZ - 1); i++)
		next[i] = i + 1;
	next[DICSIZ - 1] = NIL;
	for (i = DICSIZ * 2; i <= (node)MAX_HASH_VAL; i++)
		next[i] = NIL;
}

/* ------------------------------------------------------------------------- */

#define HASH(p, c) ((p) + ((c) << (DICBIT - 9)) + DICSIZ * 2)

/* q's child for character c (NIL if not found) */
static node child(lh5_encoder *lh5, node q, unsigned char c)
{
	node r;
	node *parent = lh5->parent;
	node *next = lh5->next;
	
	r = next[HASH(q, c)];
	parent[NIL] = q;					/* sentinel */
	while (parent[r] != q)
		r = next[r];
	return r;
}

/* ------------------------------------------------------------------------- */

/* Let r be q's child for character c. */
static void makechild(lh5_encoder *lh5, node q, unsigned char c, node r)
{
	node h, t;
	unsigned char *childcount = lh5->childcount;
	node *parent = lh5->parent;
	node *prev = lh5->prev;
	node *next = lh5->next;
	
	h = HASH(q, c);
	t = next[h];
	next[h] = r;
	next[r] = t;
	prev[t] = r;
	prev[r] = h;
	parent[r] = q;
	childcount[q]++;
}

/* ------------------------------------------------------------------------- */

static void split(lh5_encoder *lh5, node old)
{
	node newnode, t;
	unsigned char *text = lh5->text;
	unsigned char *childcount = lh5->childcount;
	node *position = lh5->position;
	node *parent = lh5->parent;
	node *prev = lh5->prev;
	node *next = lh5->next;
	lh5_level_t *level = lh5->level;
	
	newnode = lh5->avail;
	lh5->avail = next[newnode];
	childcount[newnode] = 0;
	t = prev[old];
	prev[newnode] = t;
	next[t] = newnode;
	t = next[old];
	next[newnode] = t;
	prev[t] = newnode;
	parent[newnode] = parent[old];
	level[newnode] = lh5->matchlen;
	position[newnode] = lh5->pos;
	makechild(lh5, newnode, text[lh5->matchpos + lh5->matchlen], old);
	makechild(lh5, newnode, text[lh5->pos + lh5->matchlen], lh5->pos);
}

/* ------------------------------------------------------------------------- */

static void insert_node(lh5_encoder *lh5)
{
	node q, r, j, t;
	unsigned char c, *t1, *t2;
	int matchl;
	unsigned char *text = lh5->text;
	node *position = lh5->position;
	node *parent = lh5->parent;
	node *prev = lh5->prev;
	node *next = lh5->next;
	lh5_level_t *level = lh5->level;
	
	matchl = lh5->matchlen;

	if (matchl >= 4)
	{
		matchl--;
		r = (lh5->matchpos + 1) | DICSIZ;
		while ((q = parent[r]) == NIL)
			r = next[r];
		while (level[q] >= matchl)
		{
			r = q;
			q = parent[q];
		}
#if PERCOLATE
		t = q;
		while (position[t] < 0)
		{
			position[t] = lh5->pos;
			t = parent[t];
		}
		if ((unsigned short)t < DICSIZ)
			position[t] = lh5->pos | PERC_FLAG;
#else
		t = q;
		while (t < DICSIZ)
		{
			position[t] = lh5->pos;
			t = parent[t];
		}
#endif
	} else
	{
		q = text[lh5->pos] + DICSIZ;
		c = text[lh5->pos + 1];
		lh5->matchlen = matchl;
		if ((r = child(lh5, q, c)) == NIL)
		{
			makechild(lh5, q, c, lh5->pos);
			matchl = 1;
			return;
		}
		matchl = 2;
	}
	for (;;)
	{
		if ((unsigned short)r >= DICSIZ)
		{
			j = MAXMATCH;
			lh5->matchpos = r;
		} else
		{
			j = level[r];
			lh5->matchpos = position[r] & ~PERC_FLAG;
		}
		if (lh5->matchpos >= lh5->pos)
			lh5->matchpos -= DICSIZ;
		t1 = &text[lh5->pos + matchl];
		t2 = &text[lh5->matchpos + matchl];
		while (matchl < j)
		{
			if (*t1 != *t2)
			{
				lh5->matchlen = matchl;
				split(lh5, r);
				return;
			}
			matchl++;
			t1++;
			t2++;
		}
		if (matchl >= MAXMATCH)
			break;
		position[r] = lh5->pos;
		q = r;
		if ((r = child(lh5, q, *t1)) == NIL)
		{
			lh5->matchlen = matchl;
			makechild(lh5, q, *t1, lh5->pos);
			return;
		}
		matchl++;
	}
	t = prev[r];
	prev[lh5->pos] = t;
	next[t] = lh5->pos;
	t = next[r];
	next[lh5->pos] = t;
	prev[t] = lh5->pos;
	parent[lh5->pos] = q;
	parent[r] = NIL;
	next[r] = lh5->pos;						/* special use of next[] */
	lh5->matchlen = matchl;
}

/* ------------------------------------------------------------------------- */

static void delete_node(lh5_encoder *lh5)
{
#if PERCOLATE
	node q;
#endif
	node r, s, t, u;
	unsigned char *text = lh5->text;
	unsigned char *childcount = lh5->childcount;
	node *position = lh5->position;
	node *parent = lh5->parent;
	node *prev = lh5->prev;
	node *next = lh5->next;
	lh5_level_t *level = lh5->level;
	
	if (parent[lh5->pos] == NIL)
		return;
	r = prev[lh5->pos];
	s = next[lh5->pos];
	next[r] = s;
	prev[s] = r;
	r = parent[lh5->pos];
	parent[lh5->pos] = NIL;
	if ((unsigned short)r >= DICSIZ || --childcount[r] > 1)
		return;
#if PERCOLATE
	t = position[r] & ~PERC_FLAG;
#else
	t = position[r];
#endif
	if (t >= lh5->pos)
		t -= DICSIZ;
#if PERCOLATE
	s = t;
	q = parent[r];
	while ((u = position[q]) & PERC_FLAG)
	{
		u &= ~PERC_FLAG;
		if (u >= lh5->pos)
			u -= DICSIZ;
		if (u > s)
			s = u;
		position[q] = (s | DICSIZ);
		q = parent[q];
	}
	if ((unsigned short)q < DICSIZ)
	{
		if (u >= lh5->pos)
			u -= DICSIZ;
		if (u > s)
			s = u;
		position[q] = s | DICSIZ | PERC_FLAG;
	}
#endif
	s = child(lh5, r, text[t + level[r]]);
	t = prev[s];
	u = next[s];
	next[t] = u;
	prev[u] = t;
	t = prev[r];
	next[t] = s;
	prev[s] = t;
	t = next[r];
	prev[t] = s;
	next[s] = t;
	parent[s] = parent[r];
	parent[r] = NIL;
	next[r] = lh5->avail;
	lh5->avail = r;
}

/* ------------------------------------------------------------------------- */

static void get_next_match(lh5_encoder *lh5)
{
	unsigned int n;
	unsigned char *text = lh5->text;
	
	lh5->remainder--;
	if (++lh5->pos == DICSIZ * 2)
	{
		memmove(&text[0], &text[DICSIZ], DICSIZ + MAXMATCH);
		n = (unsigned int)(lh5->memsize > DICSIZ ? DICSIZ : lh5->memsize);
		memcpy(&text[DICSIZ + MAXMATCH], lh5->unpackedMem, n);
		lh5->remainder += n;
		lh5->unpackedMem += n;
		lh5->memsize -= n;
		lh5->pos = DICSIZ;
	}
	delete_node(lh5);
	insert_node(lh5);
}

/* ------------------------------------------------------------------------- */

gboolean lh5_encode(FILE *outfile, const unsigned char *unpackedMem, unsigned long orgsize, unsigned int bufsize, unsigned long *packedLen)
{
	int lastmatchlen;
	node lastmatchpos;
	unsigned short crc;
	gboolean ret;
	lh5_encoder *lh5;
	fpos_t startpos;
	unsigned char *text;
	
	lh5 = g_new0(lh5_encoder, 1);
	if (lh5 == NULL)
		return FALSE;

	lh5->outfile = outfile;
	lh5->unpackable = FALSE;
	lh5->unpackedLen = lh5->memsize = orgsize;
	lh5->packedLen = 0;
	lh5->unpackedMem = unpackedMem;
	lh5->bufsiz = bufsize;
	
	lh5->text = text = lh5->text_;
	lh5->childcount = lh5->childcount_;
	lh5->position = lh5->position_;
	lh5->parent = lh5->parent_;
	lh5->next = lh5->next_;
	lh5->prev = lh5->prev_;
	lh5->level = lh5->level_;
	lh5->left = lh5->left_;
	lh5->right = lh5->right_;
	
	init_slide(lh5);
	if (start_huf(lh5) == FALSE)
	{
		g_free(lh5);
		return FALSE;
	}

	lh5_make_crctable();
	crc = INIT_CRC;
	crc = lh5_update_crc(lh5->unpackedMem, lh5->memsize, crc);
	
	if (fgetpos(lh5->outfile, &startpos) != 0)
		ret = FALSE;
	
	lh5->remainder = (int)(lh5->memsize > (DICSIZ + MAXMATCH) ? (DICSIZ + MAXMATCH) : lh5->memsize);
	memcpy(&text[DICSIZ], lh5->unpackedMem, lh5->remainder);
	lh5->unpackedMem += lh5->remainder;
	lh5->memsize -= lh5->remainder;
	lh5->matchlen = 0;
	lh5->pos = DICSIZ;
	insert_node(lh5);
	if (lh5->matchlen > lh5->remainder)
		lh5->matchlen = lh5->remainder;
	ret = TRUE;
	while (ret && lh5->remainder > 0 && !lh5->unpackable)
	{
		lastmatchlen = lh5->matchlen;
		lastmatchpos = lh5->matchpos;
		get_next_match(lh5);
		if (lh5->matchlen > lh5->remainder)
			lh5->matchlen = lh5->remainder;
		if (lh5->matchlen > lastmatchlen || lastmatchlen < THRESHOLD)
		{
			ret = output5(lh5, text[lh5->pos - 1], 0);
		} else
		{
			ret = output5(lh5, lastmatchlen + (UCHAR_MAX + 1 - THRESHOLD), (lh5->pos - lastmatchpos - 2) & (DICSIZ - 1));
			if (ret)
			{
				while (--lastmatchlen > 0)
					get_next_match(lh5);
				if (lh5->matchlen > lh5->remainder)
					lh5->matchlen = lh5->remainder;
			}
		}
	}
	if (ret)
		ret = end_huf(lh5);
	if (ret && (lh5->unpackable || lh5->packedLen >= lh5->unpackedLen))
	{
		if (fsetpos(lh5->outfile, &startpos) != 0)
			ret = FALSE;
		if (ret && fwrite(unpackedMem, 1, lh5->unpackedLen, lh5->outfile) != lh5->unpackedLen)
			ret = FALSE;
		lh5->packedLen = lh5->unpackedLen;
	}

	if (packedLen)
		*packedLen = lh5->packedLen;
	
	g_free(lh5->buf);
	g_free(lh5);
	return ret;
}
