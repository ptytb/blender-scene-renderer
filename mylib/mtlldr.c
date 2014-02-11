#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

#include "mtlldr.h"
#include "filemap.h"
#include "texldr.h"
#include "strfeed.h"

#ifdef WIN32
#include "win32.h"
#endif

const mtl_t material_white = {
	.name = "white",
	.amb = { .r = 0.0f, .g = 0.0f, .b = 0.0f },
	.dif = { .r = 1.0f, .g = 1.0f, .b = 1.0f },
	.spec = { .r = 0.0f, .g = 0.0f, .b = 0.0f },
	.emis = { .r = 0.0f, .g = 0.0f, .b = 0.0f },
	.shininess = 0.0f,
	.alpha = 1.0f,
	.map_amb = -1,
	.map_dif = -1,
	.map_spec = -1
};

const char *_mtl_glob_tex_path = NULL;

void mtl_glob_tex_path(const char *p)
{
	_mtl_glob_tex_path = p;
}

void mtl_load(const char *model_filename, const char *filename,
		svec_t *mtlv)
{
	file_info_t fi;
	char *p, *p_end, legit_name[NAME_MAX], *slash;
	enum Mtl_Line line;
	texldr_t tl;
	mtl_t *m = NULL; /* Active material */
	int fo_err;

	/* Copy model path */
	p = legit_name;
	slash = NULL;

	while ( (*p = *model_filename) != '\0' ) {
		if (*p == '/' || *p == '\\')
			slash = p;
		++p;
		++model_filename;
	}
	p = (slash == NULL) ? legit_name : slash + 1;
	str_cpyfeed(p, filename, NAME_MAX);

	fo_err = file_open(legit_name, &fi);
	if (fo_err)
		return;

	tex_init(&tl, _mtl_glob_tex_path);

	p = fi.fp_in;
	p_end = fi.fp_in + fi.l_in;

	while (p < p_end)
	{
		line = ML_UNKNOWN;

		if (*p == '\n') {
			++p;
			continue;
		} else if (*p == '#') {
			line = ML_COMMENT;
		} else if ( * (unsigned int *) p == 0x6D77656E) { /* "newm"*/
			line = ML_NEW;
		} else if ( * (unsigned short *) p == 0x614B) { /* "Ka" */
			line = ML_AMBIENT;
		} else if ( * (unsigned short *) p == 0x644B) { /* "Kd" */
			line = ML_DIFFUSE;
		} else if ( * (unsigned short *) p == 0x734B) { /* "Ks" */
			line = ML_SPECULAR;
		} else if ( * (unsigned short *) p == 0x734E) { /* "Ns" */
			line = ML_SHININESS;
		} else if ( * (unsigned short *) p == 0x654B) { /* "Ke" */
			line = ML_EMISSION;
		} else if ( * (unsigned short *) p == 0x2064 || /* "d " */
				* (unsigned short *) p == 0x7254) { /* "Tr" */
			line = ML_ALPHA;
		} else if (* (unsigned int *) p == 0x5F70616D) { /* "map_" */
			if (* (unsigned short *) (p + 4) == 0x614B) { /* "map_Ka" */
				line = ML_MAP_AMBIENT;
			} else if (* (unsigned short *) (p + 4) == 0x644B) { /* "map_Kd" */
				line = ML_MAP_DIFFUSE;
			} else if (* (unsigned short *) (p + 4) == 0x734B) { /* "map_Ks" */
				line = ML_MAP_SPECULAR;
			}
		}

		switch (line)
		{
			case ML_NEW:
				p += 7;
				m = svec_put(mtlv);
				*m = material_white;
				str_cpyfeed(m->name, p, MTL_MAX_NAME);
//				printf("new mat: %s\n", m->name);
				break;

			case ML_AMBIENT:
				p += 3;
				sscanf(p, "%f %f %f",
						&m->amb.r,
						&m->amb.g,
						&m->amb.b);
				break;

			case ML_DIFFUSE:
				p += 3;
				sscanf(p, "%f %f %f",
						&m->dif.r,
						&m->dif.g,
						&m->dif.b);
				break;
			
			case ML_SPECULAR:
				p += 3;
				sscanf(p, "%f %f %f",
						&m->spec.r,
						&m->spec.g,
						&m->spec.b);
				break;

			case ML_SHININESS:
				p += 3;
				sscanf(p, "%f", &m->shininess);
				break;

			case ML_EMISSION:
				p += 3;
				sscanf(p, "%f %f %f",
						&m->emis.r,
						&m->emis.g,
						&m->emis.b);
				break;

			case ML_ALPHA:
				p += 2;
				sscanf(p, "%f", &m->alpha);
				break;

			case ML_MAP_AMBIENT:
				p += 7;
				m->map_amb = tex_queue(&tl, p);
				break;

			case ML_MAP_DIFFUSE:
				p += 7;
				m->map_dif = tex_queue(&tl, p);
				break;

			case ML_MAP_SPECULAR:
				p += 7;
				m->map_spec = tex_queue(&tl, p);
				break;
			default:
				break;
		}

		while (p < p_end && *p != '\n')
			++p;
	}

	file_close(&fi);
	tex_load(&tl);
	tex_free(&tl);
}

mtl_t *mtl_get_by_name(svec_t *mtlv, const char *name)
{
	svec_iter_t i;
	mtl_t *m;

	svec_iter_init(&i, mtlv);

	while ( (m = svec_iter_next(&i)) ) {
		if (str_cmpfeed(m->name, name))
			return m;
	}

	return NULL;
}

void mtl_dump(svec_t *mtlv)
{
	int i, elems;
	mtl_t *m;

	elems = mtlv->elems;

	for (i = 0; i < elems; ++i) {
		m = svec_get(mtlv, i);

		printf("name: %s\namb: %f %f %f, dif: %f %f %f"
				", spec: %f %f %f\nshin: %f, emis: %f %f %f, alpha: %f\n"
				"tex id : map amb: %d, map dif: %d, map spec: %d\n\n",
				m->name,
				m->amb.r, m->amb.g, m->amb.b,
				m->dif.r, m->amb.g, m->amb.b,
				m->spec.r, m->spec.g, m->spec.b,
				m->shininess, m->emis.r, m->emis.g, m->emis.b,
				m->alpha, m->map_amb, m->map_dif, m->map_spec);
	}
}


