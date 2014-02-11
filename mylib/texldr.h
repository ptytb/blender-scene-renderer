/*
 * Texture loader
 *
 * Inital directory must be set
 *
 * Supports .tga textures without compression
 *
 * Texture name length is limited
 *
 */

#pragma once

#include "slabvec.h"

#define TEX_MAX_NAME 32

typedef struct texldr_struct {
	svec_t tex_pending;
	int id_low;
	int id_high;
	const char *path;
} texldr_t;

typedef struct texinfo_struct {
	char filename[TEX_MAX_NAME];
} texinfo_t;

void tex_init(texldr_t *t, const char *path);
void tex_free(texldr_t *t);
/* Returns id */
int tex_queue(texldr_t *t, const char *filename);
/* Load all pending */
void tex_load(texldr_t *t); 
int tex_get_by_filename(texldr_t *t, const char *filename);
void tex_glob_id_low(int id);

extern int _tex_glob_id_low;

