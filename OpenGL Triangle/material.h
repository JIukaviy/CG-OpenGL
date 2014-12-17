#ifndef MTL_H
#define MTL_H

#include "vec.h"
#include <GL/glew.h>
#include <GL/freeglut.h>

#define MTL_EFFECTS_COUNT 2
#define MTL_DIFFUSE_SHADER_NAME "shaders/diffuse.f.glsl"
#define MTL_SPECULAR_SHADER_NAME "shaders/specular.f.glsl"

enum MTL_EFFECT_TYPE {
	MTL_DIFFUSE,
	MTL_SPECULAR
};

typedef void* mtlhnd;
typedef void* efcthnd;

void mtl_init();

mtlhnd mtl_create();
void mtl_add_effect(mtlhnd mtl, efcthnd effect);
void mtl_destroy(mtlhnd* hnd);
void mtl_draw(mtlhnd hnd, int size);
efcthnd* mtl_get_efcts(mtlhnd mtl);
int mtl_get_size(mtlhnd mtl);

efcthnd efct_create_diffuse(vechnd color);
efcthnd efct_create_specular(vechnd color, float shinines);
GLuint efct_get_program(efcthnd hnd);
void efct_destroy(efcthnd* effect);

#endif