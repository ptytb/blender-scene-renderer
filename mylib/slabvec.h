#pragma once

struct slab_header_struct {
	struct slab_header_struct *next;
	int elems;
	int capacity;
};

typedef struct svec_struct {
	int slab_size;		/* Avoid size re-calculation */
	int slab_capacity;
	int elem_size;
	int elems;
	struct slab_header_struct *first;
	struct slab_header_struct *last;
} svec_t;

void svec_init(svec_t *v, int slab_capacity, int elem_size);
void svec_free(svec_t *v);
void svec_add(svec_t *v, void *e);
void *svec_put(svec_t *v);
void *svec_get(const svec_t *v, int n);
void svec_shrink(svec_t *v);
void svec_clone(const svec_t *v, svec_t *to);
void *svec_pop(svec_t *v);
void *svec_peek(const svec_t *v);

typedef struct svec_iter_struct {
	struct slab_header_struct *slab;
	int elem_size;
	int index;
} svec_iter_t;

void svec_iter_init(svec_iter_t *i, const svec_t *v);
void *svec_iter_next(svec_iter_t *i);


