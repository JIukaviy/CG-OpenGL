#ifndef SHADER_MAKER_H
#define SHADER_MAKER_H

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "vec.h"
#include "mat.h"
#include "light.h"
#include "cam.h"

#define SHVR_VERTEX_SHADER_NAME "shaders/shader.v.glsl"

typedef void* attrhnd;
typedef void* unifhnd;
typedef void*(*shvr_get_elems_func)(void*);

#define SHVR_VAR_TYPE_COUNT 5

enum SHVR_VAR_TYPE {
	SHVR_FLOAT,
	SHVR_VEC3,
	SHVR_VEC4,
	SHVR_MAT3,
	SHVR_MAT4
};

#define SHVR_TYPE_COUNT 2

enum SHVR_TYPE {
	SHVR_UNIFORM,
	SHVR_ATTRIBUTE
};
void shvr_init();
int shvr_create_shader(const char* file_name, GLenum shader_type);
GLuint shvr_create_program(const char* vertex_shader_name, const char* fragment_shader_name);
GLuint shvr_get_attrib_location(GLuint program, const char* attrib_name);
GLuint shvr_get_uniform_location(GLuint program, const char* attrib_name);

attrhnd attrib_create(char* name, void* data, int data_size);
unifhnd unif_create(char* name, SHVR_VAR_TYPE type);
unifhnd unif_create(char* name, void* data, SHVR_VAR_TYPE type);
void attrib_destroy(attrhnd* hnd);
void unif_destroy(unifhnd* hnd);

void unif_set_data(unifhnd hnd, void* data);
void* unif_get_data(unifhnd hnd);
void unif_refresh_location(unifhnd hnd, GLuint program);
void unif_push_var_data(unifhnd hnd);

#endif