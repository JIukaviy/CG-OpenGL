#ifndef MTL_H
#define MTL_H

#include "vec.h"
#include <GL/freeglut.h>

enum MTL_EFFECT_TYPE {
	MTL_DIFFUSE,
	MTL_SPECULAR
};

typedef void* mtlhnd;
typedef void* efcthnd;

mtlhnd mtl_create();
void mtl_add_effect(mtlhnd mtl, efcthnd effect);
void mtl_destroy(mtlhnd* hnd);

efcthnd efct_create_diffuse(vechnd color);
efcthnd efct_create_specular(vechnd color, float shinines);
void efct_destroy(efcthnd* effect);

#endif