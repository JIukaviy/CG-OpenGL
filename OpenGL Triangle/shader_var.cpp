#include "shader_var.h"
#include "vec_mat_errors.h"
#include "list.h"
#include "gc.h"
#include "file.h"
#include <string>
#include <sstream>
#include <iostream>

using namespace std;

typedef void(*unif_set_var_func)(unifhnd, void*);
typedef void*(*unif_var_create_func)();
typedef void(*unif_push_var_data_func)(GLuint, void*);

struct attribute_t {
	SHVR_TYPE type;
	GLuint buffer_location;
	GLuint var_location;
	char* name;
};

struct uniform_t {
	SHVR_TYPE type;
	SHVR_VAR_TYPE var_type;
	GLuint var_location;
	void* data;
	char* name;
};

vme_int shvr_unit_id;
vme_int SHVR_ERR_NULL_IN_DATA;
vme_int SHVR_ERR_INVALIDE_DATA;
vme_int SHVR_ERR_WRONG_TYPE;
vme_int SHVR_ERR_WRONG_PREFIX_TYPE;
vme_int SHVR_ERR_CANT_CREATE_PROGRAM;
vme_int SHVR_ERR_CANT_BIND_ATTRIBUTE;
vme_int SHVR_ERR_CANT_BIND_UNIFORM;

#define hnd2type(x, type) ((type*)(x))
#define new_hnd2type(x, name, type) type* name = hnd2type(x, type);
#define new_hnd2attr(x, name) new_hnd2type(x, name, attribute_t);
#define new_hnd2unif(x, name) new_hnd2type(x, name, uniform_t);
#define assert(x, error, ret) if (!(x)) {push_error_info(error, "In argument "#x); return ret;}
#define obj_assert(x, ret) assert(x, SHVR_ERR_NULL_IN_DATA, ret);
#define data_assert(x, ret) assert(x, SHVR_ERR_INVALIDE_DATA, ret);
#define type_assert(x, ret) assert((int)(x) >= 0 && (int)(x) < SHVR_VAR_TYPE_COUNT, SHVR_ERR_WRONG_TYPE, ret);
#define type_prefix_assert(x, ret) assert((int)(x) >= 0 && (int)(x) < SHVR_TYPE_COUNT, SHVR_ERR_WRONG_PREFIX_TYPE, ret);

//typedef shvrhnd(void* data, char* name, SHVR_TYPE type_prefix);

void unif_destroy_float(void** data);
destroy_func unif_var_destroys[] = { unif_destroy_float, vec_destroy, vec_destroy, mat_destroy, mat_destroy };

void* unif_var_create_float();
void* unif_var_create_vec3();
void* unif_var_create_vec4();
void* unif_var_create_mat3();
void* unif_var_create_mat4();
unif_var_create_func unif_var_creates[] = { unif_var_create_float, unif_var_create_vec3, unif_var_create_vec4, unif_var_create_mat3, unif_var_create_mat4 };

void unif_push_var_data_float(GLuint location, void* data);
void unif_push_var_data_vec3(GLuint location, void* data);
void unif_push_var_data_vec4(GLuint location, void* data);
void unif_push_var_data_mat3(GLuint location, void* data);
void unif_push_var_data_mat4(GLuint location, void* data);
unif_push_var_data_func unif_push_var_datas[] = { unif_push_var_data_float, unif_push_var_data_vec3, unif_push_var_data_vec4, unif_push_var_data_mat3, unif_push_var_data_mat4 };

void unif_set_data_float(unifhnd hnd, void* ptr);
void unif_set_data_vec(unifhnd hnd, void* ptr);
void unif_set_data_mat(unifhnd hnd, void* ptr);
unif_set_var_func unif_set_vars[] = { unif_set_data_float, unif_set_data_vec, unif_set_data_vec, unif_set_data_mat, unif_set_data_mat };

void shvr_init() {
	shvr_unit_id = vme_register_unit("Shader Var");
	SHVR_ERR_NULL_IN_DATA = vme_register_error_type(shvr_unit_id, "Null pointer in input data");
	SHVR_ERR_INVALIDE_DATA = vme_register_error_type(shvr_unit_id, "Invalide input data");
	SHVR_ERR_WRONG_TYPE = vme_register_error_type(shvr_unit_id, "Wrong type");
	SHVR_ERR_WRONG_PREFIX_TYPE = vme_register_error_type(shvr_unit_id, "Wrong prefix type");
	SHVR_ERR_CANT_CREATE_PROGRAM = vme_register_error_type(shvr_unit_id, "Can't create program");
	SHVR_ERR_CANT_BIND_ATTRIBUTE = vme_register_error_type(shvr_unit_id, "Could not bind attribute");
	SHVR_ERR_CANT_BIND_UNIFORM = vme_register_error_type(shvr_unit_id, "Could not bind uniform");
}

char* get_log(GLuint object) {
	GLint log_length = 0;
	if (glIsShader(object))
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
	else if (glIsProgram(object))
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
	else
		return "printlog: Not a shader or a program";

	char* log = new char[log_length];

	if (glIsShader(object))
		glGetShaderInfoLog(object, log_length, NULL, log);
	else if (glIsProgram(object))
		glGetProgramInfoLog(object, log_length, NULL, log);

	gc_push_garbage(log, gc_sample_destroy);
	return log;
}

int shvr_create_shader(const char* file_name, GLenum shader_type) {
	GLuint res;
	GLint compile_ok = GL_FALSE;

	char* s_source = file_read(file_name);

	if (vme_error_appear())
		return 0;

	res = glCreateShader(shader_type);
	glShaderSource(res, 1, &s_source, NULL);
	glCompileShader(res);
	glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);

	delete s_source;

	if (!compile_ok) {
		start_garbage_collect(1);
		push_error_info(SHVR_ERR_CANT_CREATE_PROGRAM, get_log(res));
		erase_collected_garbage(1);
		return 0;
	}

	return res;
}

GLuint shvr_create_program(const char* vertex_shader_name, const char* fragment_shader_name) {
	GLint link_ok = GL_FALSE;
	GLuint vs;
	vs = shvr_create_shader(vertex_shader_name, GL_VERTEX_SHADER);

	GLuint fs;
	fs = shvr_create_shader(fragment_shader_name, GL_FRAGMENT_SHADER);

	if (vme_error_appear())
		return 0;

	GLuint program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);

	if (!link_ok) {
		start_garbage_collect(1);
		push_error_info(shvr_unit_id, get_log(program));
		erase_collected_garbage(1);
		return 0;
	}

	return program;
}

GLuint shvr_get_attrib_location(GLuint program, const char* attrib_name) {
	GLuint attrib = glGetAttribLocation(program, attrib_name);
	if (attrib == -1)
		push_error_info(SHVR_ERR_CANT_BIND_ATTRIBUTE, attrib_name);
	return attrib;
}

GLuint shvr_get_uniform_location(GLuint program, const char* attrib_name) {
	GLuint uniform = glGetUniformLocation(program, attrib_name);
	if (uniform == -1)
		push_error_info(SHVR_ERR_CANT_BIND_UNIFORM, attrib_name);
	return uniform;
}

//-----------------SHVR_ATTRIBUTE------------

attrhnd attrib_create(char* name, void* data, int data_size) {
	obj_assert(name, nullptr);
	obj_assert(data, nullptr);
	attribute_t* new_attrib = new attribute_t;
	new_attrib->type = SHVR_ATTRIBUTE;
	int name_len = strlen(name);
	new_attrib->name = new char[name_len + 1];
	memcpy(new_attrib->name, name, name_len + 1);
	glGenBuffers(1, &new_attrib->buffer_location);
	glBindBuffer(GL_ARRAY_BUFFER, new_attrib->buffer_location);
	glBufferData(GL_ARRAY_BUFFER, data_size, data, GL_STATIC_DRAW);
}

void attrib_destroy(attrhnd* hnd) {
	obj_assert(hnd);
	obj_assert(*hnd);
	new_hnd2attr(*hnd, t);
	glDeleteBuffers(1, &t->buffer_location);
	delete t->name;
	delete *hnd;
	*hnd = nullptr;
}

void attrib_refresh_location(attrhnd hnd, GLuint program) {
	obj_assert(hnd);
	new_hnd2attr(hnd, t);
	t->var_location = shvr_get_attrib_location(program, t->name);
}

//-----------------SHVR_UNIFORM------------------

unifhnd unif_create(char* name, SHVR_VAR_TYPE type) {
	obj_assert(name, nullptr);
	type_assert(type, nullptr);
	uniform_t* new_unif = new uniform_t;
	int name_len = strlen(name);
	new_unif->name = new char[name_len + 1];
	memcpy(new_unif->name, name, name_len + 1);
	new_unif->data = unif_var_creates[type]();

}

unifhnd unif_create(char* name, void* data, SHVR_VAR_TYPE type) {
	unifhnd hnd = unif_create(name, type);
	if (vme_error_appear())
		return nullptr;
	unif_set_vars[type](hnd, data);
	return hnd;
}

void unif_destroy(unifhnd* hnd) {
	obj_assert(hnd);
	obj_assert(*hnd);
	new_hnd2unif(*hnd, t);
	unif_var_destroys[t->var_type](&t->data);
	delete t->name;
	delete *hnd;
	*hnd = nullptr;
}

void unif_push_var_data(unifhnd hnd) {
	obj_assert(hnd);
	new_hnd2unif(hnd, t);

	unif_push_var_datas[t->type](t->var_location, t->data);
}

void unif_set_data(unifhnd hnd, void* data) {
	obj_assert(hnd);
	type_assert(hnd);
	unif_set_vars[hnd2type(hnd, uniform_t)->type](hnd, data);
}

void unif_refresh_location(unifhnd hnd, GLuint program) {
	obj_assert(hnd);
	new_hnd2unif(hnd, t);
	t->var_location = shvr_get_uniform_location(program, t->name);
}

void* unif_get_data(unifhnd hnd) {
	obj_assert(hnd, nullptr);
	return hnd2type(hnd, uniform_t)->data;
}

//-----------------SHVR_FLOAT----------------

void* unif_var_create_float() {
	return new float;
}

void unif_set_data_float(unifhnd hnd, void* ptr) {
	obj_assert(hnd);
	obj_assert(ptr);
	new_hnd2unif(hnd, t);
	*((float*)t->data) = *(float*)ptr;
}

void unif_destroy_float(void** data) {
	obj_assert(data);
	obj_assert(*data);
	delete *data;
	*data = nullptr;
}

void unif_push_var_data_float(GLuint location, void* data) {
	obj_assert(data);
	glUniform1f(location, *(float*)data);
}

//-----------------SHVR_VEC-----------------

void* unif_var_create_vec3() {
	gc_pause_collect();
	void* out = vec_create(3);
	gc_resume_collect();
	return out;
}

void unif_push_var_data_vec3(GLuint location, void* data) {
	obj_assert(data);
	glUniform3fv(location, 1, vec_get_elems(data));
}

void* unif_var_create_vec4() {
	gc_pause_collect();
	void* out = vec_create(4);
	gc_resume_collect();
	return out;
}

void unif_push_var_data_vec4(GLuint location, void* data) {
	obj_assert(data);
	glUniform4fv(location, 1, vec_get_elems(data));
}

void unif_set_data_vec(unifhnd hnd, void* ptr) {
	obj_assert(hnd);
	obj_assert(ptr);
	new_hnd2unif(hnd, t);
	vec_copy(t->data, ptr);
}

//-----------------SHVR_MAT-----------------

void* unif_var_create_mat3() {
	gc_pause_collect();
	void* out = mat_create_e(3);
	gc_resume_collect();
	return out;
}

void unif_push_var_data_mat3(GLuint location, void* data) {
	obj_assert(data);
	glUniformMatrix3fv(location, 1, GL_TRUE, mat_get_elems(data));
}

void* unif_var_create_mat4() {
	gc_pause_collect();
	void* out = mat_create_e(4);
	gc_resume_collect();
	return out;
}

void unif_push_var_data_mat4(GLuint location, void* data) {
	obj_assert(data);
	glUniformMatrix4fv(location, 1, GL_TRUE, mat_get_elems(data));
}

void unif_set_data_mat(unifhnd hnd, void* ptr) {
	obj_assert(hnd);
	obj_assert(ptr);
	new_hnd2unif(hnd, t);
	mat_copy(t->data, ptr);
}