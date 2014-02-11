#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>

#include "objldr.h"
#include "filemap.h"
#include "mtlldr.h"
#include "strfeed.h"
#include "numparse.h"

void model_init(model_t *mdl)
{
	svec_init(&mdl->vert, MODEL_INIT_VERTS, sizeof(vert_t));
	svec_init(&mdl->vtex, MODEL_INIT_VTEXS, sizeof(vtex_t));
	svec_init(&mdl->vnor, MODEL_INIT_VNORS, sizeof(vnor_t));
	svec_init(&mdl->face, MODEL_INIT_FACES, sizeof(face_t));
	svec_init(&mdl->mtl, MODEL_INIT_MTLS, sizeof(mtl_t));
	svec_init(&mdl->mtl_use, MODEL_INIT_MTLUS, sizeof(mtl_use_t));
	svec_init(&mdl->group, MODEL_INIT_GROUPS, sizeof(group_t));
}

void model_free(model_t *mdl)
{
	svec_free(&mdl->vert);
	svec_free(&mdl->vtex);
	svec_free(&mdl->vnor);
	svec_free(&mdl->face);
	svec_free(&mdl->mtl);
	svec_free(&mdl->mtl_use);
	svec_free(&mdl->group);
}

void model_vbo_free(model_t *mdl)
{
	svec_free(&mdl->vert);
	svec_free(&mdl->vtex);
	svec_free(&mdl->vnor);
	svec_free(&mdl->face);
}

#ifdef MODEL_FAST_PARSE

static inline void _face_get(char **pp, int *pvalue)
{
	enum Indeces_Mask {		/* NTVNTVNTV */
		FACE_MASK_VTN	= 511,	/* 111111111 */
		FACE_MASK_VT	= 219,	/* 011011011 */
		FACE_MASK_VN	= 365,	/* 101101101 */
		FACE_MASK_V	= 73	/* 001001001 */
	};

	char *p = *pp;
	int value;
	int parsed = 0;
	int mask = FACE_MASK_VTN;

	while (*p == ' ')
		++p;

	while (mask) {

		for (value = 0; isdigit(*p); ++p)
			value = value * 10 + *p - '0';
		*pvalue = value;

		++parsed;

		if (parsed == 1 && *p != '/') {
			mask = FACE_MASK_V;
		}

		if (parsed == 2 && *p != '/') {
			mask = FACE_MASK_VT;
		}

		++p; /* Linefeed \n CAN be skipped here */

		if (*p == '/') {
			if (parsed == 1) {
				mask = FACE_MASK_VN;
				++p;
			} else {
				++p;
			}
			++parsed;
		}

		++pvalue;

		while (!( (mask >>= 1) & 1 ) && mask) {
			*pvalue = 0;
			++pvalue;
		}
	}

	*pp = p;
}

#endif

void model_load(const char *filename, model_t *mdl)
{
	file_info_t fi;
	char *p, *p_end;
	enum Obj_Line line;
	void *data;
#ifndef MODEL_FAST_PARSE
	int read;
#endif
	
	file_open(filename, &fi);
	p = fi.fp_in;
	p_end = fi.fp_in + fi.l_in;

	while (p < p_end)
	{
		line = OL_UNKNOWN;

		if (*p == '\n') {
			++p;
			continue;
		} else if (*p == '#') {
			line = OL_COMMENT;
		} else if ( * (unsigned short *) p == 0x206F) { /* "o " */
			line = OL_OBJECT;
		} else if ( * (unsigned short *) p == 0x2076) { /* "v " */
			line = OL_VERTEX;
		} else if ( * (unsigned short *) p == 0x7476) { /* "vt" */
			line = OL_TEXTURE;
		} else if ( * (unsigned short *) p == 0x6E76) { /* "vn" */
			line = OL_NORMAL;
		} else if ( * (unsigned short *) p == 0x2066) { /* "f " */
			line = OL_FACE;
		} else if ( * (unsigned int *) p == 0x6C6C746D) { /* "mtll"*/
			line = OL_MATERIAL_LIB;
		} else if ( * (unsigned int *) p == 0x6D657375) { /* "usem"*/
			line = OL_MATERIAL_USE;
		} else if ( * (unsigned short *) p == 0x2067) { /* "g "*/
			line = OL_GROUP;
		}

		switch (line)
		{
			case OL_VERTEX:
				data = svec_put(&mdl->vert);
				p += 2;
#ifdef MODEL_FAST_PARSE
				((vert_t *) data)->x = _atof(&p);
				((vert_t *) data)->y = _atof(&p);
				((vert_t *) data)->z = _atof(&p);
				continue;
#else
				read = sscanf(p, "%f %f %f",
						&((vert_t *) data)->x,
						&((vert_t *) data)->y,
						&((vert_t *) data)->z);
#endif
				break;

			case OL_TEXTURE:
				data = svec_put(&mdl->vtex);
				p += 3;
#ifdef MODEL_FAST_PARSE
				((vtex_t *) data)->x = _atof(&p);
				((vtex_t *) data)->y = _atof(&p);
				continue;
#else
				read = sscanf(p, "%f %f",
						&((vtex_t *) data)->x,
						&((vtex_t *) data)->y);
#endif
				break;

			case OL_NORMAL:
				data = svec_put(&mdl->vnor);
				p += 3;
#ifdef MODEL_FAST_PARSE
				((vnor_t *) data)->x = _atof(&p);
				((vnor_t *) data)->y = _atof(&p);
				((vnor_t *) data)->z = _atof(&p);
				continue;
#else
				read = sscanf(p, "%f %f %f", 
						&((vnor_t *) data)->x,
						&((vnor_t *) data)->y,
						&((vnor_t *) data)->z);
#endif
				break;

			case OL_FACE:
				data = svec_put(&mdl->face);
				p += 2;
#ifdef MODEL_FAST_PARSE
				_face_get(&p, ((face_t *) data)->index);
				continue;
#else
				read = sscanf(p, "%d/%d/%d %d/%d/%d %d/%d/%d", /* v-t-n*/
						&((face_t *) data)->index[0],
						&((face_t *) data)->index[1],
						&((face_t *) data)->index[2],
						&((face_t *) data)->index[3],
						&((face_t *) data)->index[4],
						&((face_t *) data)->index[5],
						&((face_t *) data)->index[6],
						&((face_t *) data)->index[7],
						&((face_t *) data)->index[8]);
				if (read == 9)
					break;
				
				read = sscanf(p, "%d//%d %d//%d %d//%d", /* v-n */
						&((face_t *) data)->index[0],
						&((face_t *) data)->index[2],
						&((face_t *) data)->index[3],
						&((face_t *) data)->index[5],
						&((face_t *) data)->index[6],
						&((face_t *) data)->index[8]);
				if (read == 6) {
					((face_t *) data)->index[1] = 0;
					((face_t *) data)->index[4] = 0;
					((face_t *) data)->index[7] = 0;
					break;
				}

				read = sscanf(p, "%d/%d %d/%d %d/%d", /* v-t */
						&((face_t *) data)->index[0],
						&((face_t *) data)->index[1],
						&((face_t *) data)->index[3],
						&((face_t *) data)->index[4],
						&((face_t *) data)->index[6],
						&((face_t *) data)->index[7]);
				if (read == 6) {
					((face_t *) data)->index[2] = 0;
					((face_t *) data)->index[5] = 0;
					((face_t *) data)->index[8] = 0;
					break;
				}

				read = sscanf(p, "%d %d %d", /* v */
						&((face_t *) data)->index[0],
						&((face_t *) data)->index[3],
						&((face_t *) data)->index[6]);
				((face_t *) data)->index[1] = 0;
				((face_t *) data)->index[2] = 0;
				((face_t *) data)->index[4] = 0;
				((face_t *) data)->index[5] = 0;
				((face_t *) data)->index[7] = 0;
				((face_t *) data)->index[8] = 0;
#endif
				break;

			case OL_MATERIAL_LIB:
				p += 7;
				mtl_load(filename, p, &mdl->mtl);
			//	mtl_dump(&mdl->mtl);
				break;

			case OL_MATERIAL_USE:
				p += 7;
				{
					mtl_use_t *mu;
					mtl_t *material;

					material = mtl_get_by_name(&mdl->mtl, p);
					if (material != NULL) {
						mu = svec_put(&mdl->mtl_use);
						mu->material = material;
						mu->face_id = mdl->face.elems;
					}
				}
				break;

			case OL_GROUP:
				p += 2;
				{
					group_t *g;
					
					g = svec_put(&mdl->group);
					str_cpyfeed(g->name, p, MODEL_GROUP_MAX_NAME);
					g->face_id = mdl->face.elems;
				}
				break;

			default:
				break;
		}

		while (p < p_end && *p != '\n')
			++p;
	}

	file_close(&fi);

	svec_shrink(&mdl->vert);
	svec_shrink(&mdl->vtex);
	svec_shrink(&mdl->vnor);
	svec_shrink(&mdl->face);
	svec_shrink(&mdl->mtl);
	svec_shrink(&mdl->mtl_use);
	svec_shrink(&mdl->group);
}

void model_deinterleave(const model_t *mdl, model_d_t *d_mdl)
{
	svec_iter_t face;
	face_t *f;
	vert_t *vert;
	vtex_t *vtex;
	vnor_t *vnor;
	int i;

	d_mdl->elems = 3 * mdl->face.elems;

	vert = d_mdl->vert = (vert_t *) malloc(
			d_mdl->elems * sizeof(vert_t));

	vtex = d_mdl->vtex = (vtex_t *) malloc(
			d_mdl->elems * sizeof(vtex_t));

	vnor = d_mdl->vnor = (vnor_t *) malloc(
			d_mdl->elems * sizeof(vnor_t));

	svec_iter_init(&face, &mdl->face);

	while ( ( f = svec_iter_next(&face) ) )
	{ 
		for (i = 0; i < 3; ++i)
		{
			* (vert) = * (vert_t *) svec_get(&mdl->vert,
					f->index[3 * i] - 1);

			if (f->index[3 * i + 1]) /* Has face vtex coords? */
				* (vtex) = * (vtex_t *) svec_get(&mdl->vtex,
						f->index[3 * i + 1] - 1);

			if (f->index[3 * i + 2]) /* Has face vnor coords? */
				* (vnor) = * (vnor_t *) svec_get(&mdl->vnor,
						f->index[3 * i + 2] - 1);

			++vert;
			++vtex;
			++vnor;
		}
	}
}

void model_d_free(model_d_t *d_mdl)
{
	free(d_mdl->vert);
	free(d_mdl->vtex);
	free(d_mdl->vnor);
}

void model_dump(model_t *mdl)
{
	int i;

	for (i = 0; i < mdl->vert.elems; ++i) {
		vert_t *v = svec_get(&mdl->vert, i);
		printf("%f %f %f\n", v->x, v->y, v->z);
	}

	for (i = 0; i < mdl->vtex.elems; ++i) {
		vtex_t *v = svec_get(&mdl->vtex, i);
		printf("%f %f\n", v->x, v->y);
	}

	for (i = 0; i < mdl->vnor.elems; ++i) {
		vnor_t *v = svec_get(&mdl->vnor, i);
		printf("%f %f %f\n", v->x, v->y, v->z);
	}

	for (i = 0; i < mdl->face.elems; ++i) {
		face_t *v = svec_get(&mdl->face, i);
		printf("%d/%d/%d %d/%d/%d %d/%d/%d\n",
				v->index[0],
				v->index[1],
				v->index[2],
				v->index[3],
				v->index[4],
				v->index[5],
				v->index[6],
				v->index[7],
				v->index[8]);
	}
}


