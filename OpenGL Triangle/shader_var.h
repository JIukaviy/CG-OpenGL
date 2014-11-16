#ifndef SHADER_MAKER_Hshm_
#define SHADER_MAKER_H

#include "vec.h"
#include "mat.h"
#include "light.h"
#include "cam.h"

typedef void* shvrhnd;
typedef void* vbohnd;
typedef void*(*shvr_get_elems_func)(void*);

#define SHVR_VAR_TYPE_COUNT 5

enum SHVR_VAR_TYPE {
	SHVR_FLOAT,
	SHVR_VEC3,
	SHVR_VEC4,
	SHVR_MAT3,
	SHVR_MAT4
};

#define SHVR_VAR_TYPE_PREFIX_COUNT 3

enum SHVR_VAR_TYPE_PREFIX {
	SHVR_NONE,
	SHVR_UNIFORM,
	SHVR_ATTRIBUTE
};

shvrhnd shvr_create_uniform(void* data, char* name, SHVR_VAR_TYPE type);
shvrhnd shvr_create_attribute(void* data, int data_size, char* name, SHVR_VAR_TYPE type);
void shvr_destroy(shvrhnd* hnd);

void shvr_set_data(shvrhnd hnd, void* data);

#endif