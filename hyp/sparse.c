/* Implementation of sparse sets based on
 * http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.30.7319
 *
 * Adapted from Dawid Weiss's implementation at
 * https://github.com/carrotsearch/langid-java/blob/master/langid-java/src/main/java/com/carrotsearch/labs/langid/DoubleLinkedCountingSet.java
 *
 * Marco Lui, July 2014
 */
#include "hypdefs.h"
#include <stdlib.h>
#include "sparse.h"

Set *langid_alloc_set(size_t size)
{
	Set *s;

	if ((s = g_new0(Set, 1)) == NULL)
		return NULL;

	s->members = 0;
	if ((s->sparse = g_new(unsigned int, size)) ==  NULL ||
		(s->dense = g_new(unsigned int, size)) == 0 ||
		(s->counts = g_new(unsigned int, size)) == 0)
	{
		langid_free_set(s);
		s = NULL;
	}
	return s;
}

void langid_free_set(Set *s)
{
	if (s)
	{
		g_free(s->sparse);
		g_free(s->dense);
		g_free(s->counts);
		g_free(s);
	}
}

void langid_clear_set(Set *s)
{
	s->members = 0;
}

#if 0
unsigned get(Set *s, unsigned key)
{
	unsigned index = s->sparse[key];

	if (index < s->members && s->dense[index] == key)
	{
		return s->counts[index];
	} else
	{
		return 0;
	}
}
#endif

void langid_add_set(Set *s, unsigned key, unsigned val)
{
	unsigned index = s->sparse[key];

	if (index < s->members && s->dense[index] == key)
	{
		s->counts[index] += val;
	} else
	{
		index = s->members++;
		s->sparse[key] = index;
		s->dense[index] = key;
		s->counts[index] = val;
	}
}
