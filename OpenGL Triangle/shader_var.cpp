#include <GL/freeglut.h>
#include <GL/glew.h>
#include "shader_var.h"
#include "vec_mat_errors.h"
#include "list.h"
#include <string>
#include <sstream>
#include <iostream>

using namespace std;

typedef void(*shvr_destroy_func)(shvrhnd);
typedef void(*shvr_set_data_func)(shvrhnd, void*);


struct shvr_t {
	char* name;
	SHVR_VAR_TYPE type;
	SHVR_VAR_TYPE_PREFIX type_prefix;
	void* data;
	int data_size;
	GLuint location;
	shvr_get_elems_func get_elems;
};

vme_int shvr_unit_id;
vme_int SHVR_ERR_NULL_IN_DATA;
vme_int SHVR_ERR_INVALIDE_DATA;
vme_int SHVR_ERR_WRONG_TYPE;
vme_int SHVR_ERR_WRONG_PREFIX_TYPE;

#define hnd2var(x) ((shvr_t*)(x))
#define new_hnd2var(x, name) shvr_t* name = hnd2var(x);
#define assert(x, error, ret) if (!(x)) {push_error_info(error, "In argument "#x); return ret;}
#define obj_assert(x, ret) assert(x, SHVR_ERR_NULL_IN_DATA, ret);
#define data_assert(x, ret) assert(x, SHVR_ERR_INVALIDE_DATA, ret);
#define type_assert(x, ret) assert((int)(x) >= 0 && (int)(x) < SHVR_VAR_TYPE_COUNT, SHVR_ERR_WRONG_TYPE, ret);
#define type_prefix_assert(x, ret) assert((int)(x) >= 0 && (int)(x) < SHVR_VAR_TYPE_PREFIX_COUNT, SHVR_ERR_WRONG_PREFIX_TYPE, ret);

//typedef shvrhnd(void* data, char* name, SHVR_VAR_TYPE_PREFIX type_prefix);

char* type_names[] = { "float",  //Порядок важен! Указывать в порядке перечесления SHVR_VAR_TYPE
						"vec3",
						"vec4",
						"mat3",
						"mat4" };

char* type_prefix_names[] = { "", "uniform", "attribute" };

void shvr_destroy_float(void** data);

destroy_func data_destroys[] = { shvr_destroy_float, vec_destroy, vec_destroy, mat_destroy, mat_destroy };

void shvr_set_data_float(shvrhnd hnd, void* ptr);
void shvr_set_data_vec(shvrhnd hnd, void* ptr);
void shvr_set_data_mat(shvrhnd hnd, void* ptr);
shvr_set_data_func shvr_set_datas[] = { shvr_set_data_float, shvr_set_data_vec, shvr_set_data_vec, shvr_set_data_mat, shvr_set_data_mat };


void shvr_init() {
	shvr_unit_id = vme_register_unit("Shader Maker");
	SHVR_ERR_NULL_IN_DATA = vme_register_error_type(shvr_unit_id, "Null pointer in input data");
	SHVR_ERR_INVALIDE_DATA = vme_register_error_type(shvr_unit_id, "Invalide input data");
	SHVR_ERR_WRONG_TYPE = vme_register_error_type(shvr_unit_id, "Wrong type");
	SHVR_ERR_WRONG_PREFIX_TYPE = vme_register_error_type(shvr_unit_id, "Wrong prefix type");
}

//-----------------SHVR_VAR------------------

shvrhnd shvr_create(void* data, int data_size, char* name, SHVR_VAR_TYPE type, SHVR_VAR_TYPE_PREFIX type_prefix) {
	obj_assert(data, nullptr);
	obj_assert(name, nullptr);
	type_assert(type, nullptr);
	type_prefix_assert(type_prefix, nullptr);
	shvr_t* new_var = new shvr_t;
	new_var->type_prefix = type_prefix;
	new_var->type = type;
	new_var->name = new char[strlen(name) + 1];
	memcpy(new_var->name, name, strlen(name) + 1);
	if (type_prefix == SHVR_UNIFORM) 
		shvr_set_datas[type]((shvrhnd)new_var, data);
	else {
		new_var->data = new char[data_size];
		memcpy(new_var->data, data, data_size);
	}
	return new_var;
}

shvrhnd shvr_create_uniform(void* data, char* name, SHVR_VAR_TYPE type) {
	return shvr_create(data, 0, name, type, SHVR_UNIFORM);
}

shvrhnd shvr_create_attribute(void* data, int data_size, char* name, SHVR_VAR_TYPE type) {
	return shvr_create(data, data_size, name, type, SHVR_ATTRIBUTE);
}

void shvr_destroy(shvrhnd* hnd) {
	new_hnd2var(hnd, t);
	delete t->name;
	data_destroys[t->type](&t->data);
	delete *hnd;
	*hnd = nullptr;
}

void shvr_set_data(shvrhnd hnd, void* data) {
	obj_assert(hnd);
	hnd2var(hnd)->data = data;
}

void shvr_push_var_data(shvrhnd hnd) {
	obj_assert(hnd);
	new_hnd2var(hnd, t);
}

char* shvr_get_type_name(shvrhnd hnd) {
	obj_assert(hnd, nullptr);
	return type_names[hnd2var(hnd)->type];
}

char* shvr_get_type_prefix_name(shvrhnd hnd) {
	obj_assert(hnd, nullptr);
	new_hnd2var(hnd, t);
	return type_prefix_names[t->type_prefix];
}

char* shvr_get_name(shvrhnd hnd) {
	obj_assert(hnd, nullptr);
	return hnd2var(hnd)->name;
}

//-----------------SHVR_FLOAT----------------

int shvr_get_size_float(void* ptr) {
	return sizeof(float);
}

void shvr_set_data_float(shvrhnd hnd, void* ptr) {
	obj_assert(hnd);
	new_hnd2var(hnd, t);
	t->data = new float;
	memcpy(t->data, ptr, sizeof(float));
}

void shvr_destroy_float(void** data) {
	obj_assert(data);
	obj_assert(*data);
	delete *data;
}

/*void shvr_push_data_float(shvrhnd hnd) {
	obj_assert(hnd);
	new_hnd2var(hnd, t);

	glEnableVertexAttribArray(t->)

	if (t->type_prefix == SHVR_UNIFORM)
		glUniform1f(t->location, *((float*)(t->data)));

	if (t->type_prefix == SHVR_ATTRIBUTE)
		gl
}*/

//-----------------SHVR_VEC-----------------

void* shvr_get_elems_vec(void* ptr) {
	return (void*)vec_get_elems(ptr);
}

void shvr_set_data_vec(shvrhnd hnd, void* ptr) {
	obj_assert(hnd);
	new_hnd2var(hnd, t);
	if (t->data)
		vec_destroy(&t->data);
	t->data = vec_copy(ptr);
	vec_gc_unregist(t->data);
}

//-----------------SHVR_MAT-----------------

void* shvr_get_elems_mat(void* ptr) {
	return (void*)mat_get_elems(ptr);
}

void shvr_set_data_mat(shvrhnd hnd, void* ptr) {
	obj_assert(hnd);
	new_hnd2var(hnd, t);
	t->data = mat_copy(ptr);
}