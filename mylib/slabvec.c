#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "slabvec.h"

void svec_init(svec_t *v, int slab_capacity, int elem_size)
{
	v->slab_size = slab_capacity * elem_size +
			sizeof(struct slab_header_struct);
	v->slab_capacity = slab_capacity;
	v->elem_size = elem_size;
	v->elems = 0;
	v->first = malloc(v->slab_size);
	v->last = v->first;

	v->first->next = NULL;
	v->first->elems = 0;
	v->first->capacity = slab_capacity;
}

void svec_free(svec_t *v)
{
	struct slab_header_struct *next, *cur;

	cur = v->first;
	
	while (cur)
	{
		next = cur->next;
		free(cur);
		cur = next;
	}
}

static void svec_expand(svec_t *v)
{
	v->last->next = (struct slab_header_struct *) malloc(v->slab_size);
	assert(v->last->next);
	v->last = v->last->next; 

	v->last->next = NULL;
	v->last->elems = 0;
	v->last->capacity = v->slab_capacity;
}

void svec_add(svec_t *v, void *e)
{
	if (v->last->elems == v->slab_capacity)
		svec_expand(v);
	memcpy((char *) v->last + sizeof(struct slab_header_struct) +
			v->last->elems * v->elem_size, e, v->elem_size);
	++v->elems;
	++v->last->elems;
}

inline void *svec_put(svec_t *v)
{
	void *ptr;

	if (v->last->elems == v->slab_capacity)
		svec_expand(v);

	ptr = (char *) v->last + sizeof(struct slab_header_struct) +
			v->last->elems * v->elem_size;

	++v->elems;
	++v->last->elems;
	return ptr;
}

inline void *svec_get(const svec_t *v, int n)
{
	struct slab_header_struct *slab;

	slab = v->first;
	while (n >= slab->elems) {
		n -= slab->elems;
		slab = slab->next;
		assert(slab);
	}

	return (char *) slab + sizeof(struct slab_header_struct) +
		v->elem_size * n;
}

void svec_shrink(svec_t *v)
{
	void *s;
	struct slab_header_struct *p;
	int is_single;
	void *old;

	old = v->last;
	is_single = (v->first == v->last);

	s = realloc(v->last, v->last->elems * v->elem_size +
			sizeof(struct slab_header_struct));
	
	if (s == NULL)
		return;

	if (s != v->last) {
		v->last = s;
	//	free(old);

		if (!is_single) {
			for (p = v->first; p->next != old; p = p->next)
				;
			p->next = s;
		}
	}

	v->last->capacity = v->last->elems;

	if (is_single)
		v->first = v->last;
}

void svec_iter_init(svec_iter_t *i, const svec_t *v)
{
	i->slab = v->first;
	i->elem_size = v->elem_size;
	i->index = 0;
}

void *svec_iter_next(svec_iter_t *i)
{
	void *ptr;

	if (!i->slab || !i->slab->elems)
		return NULL;

	ptr = (char *) i->slab + sizeof(struct slab_header_struct) +
		i->elem_size * i->index;

	++(i->index);

	if (i->index == i->slab->elems) {
		i->slab = i->slab->next;
		i->index = 0;
	}

	return ptr;
}

void svec_clone(const svec_t *v, svec_t *to)
{
	struct slab_header_struct *slab, **ps;
	int block;

	*to = *v;
	ps = &to->first;

	for (slab = v->first; slab; slab = slab->next) {
		block = ( (slab != v->last)
				? v->slab_size
				: sizeof(struct slab_header_struct) +
				slab->capacity );

		*ps = (struct slab_header_struct *) malloc(block);
		memcpy(*ps, slab, block);		
		ps = &((*ps)->next);
	}

	(*ps)->next = NULL;
}

void *svec_pop(svec_t *v)
{
	struct slab_header_struct *slab;
	
	slab = v->last;

	if (slab->elems == 0) {
		struct slab_header_struct *prev;

		if (v->first == v->last)
			return NULL;

		for (prev = v->first; prev->next != slab;
				prev = prev->next)
			;

		free(slab);
		prev->next = NULL;
		slab = prev;
		v->last = slab;
	}

	--slab->elems;
	--v->elems;

	return (char *) slab + sizeof(struct slab_header_struct) +
		v->elem_size * slab->elems;
}

void *svec_peek(const svec_t *v)
{
	struct slab_header_struct *slab;

	slab = v->last;

	if (slab->elems == 0)
	{
		if (v->first == v->last) {
			return NULL;
		} else {
			struct slab_header_struct *prev;

			for (prev = v->first; prev->next != slab; prev = prev->next)
				;
			slab = prev;
		}
	}

	return (char *) slab + sizeof(struct slab_header_struct) +
		v->elem_size * (slab->elems - 1);
}


