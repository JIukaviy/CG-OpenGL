#include "light.h"
#include "gc.h"
#include "vec_mat_errors.h"
#include "file.h"
#include "shader_var.h"
#include <string>

#define hnd2light(x, type) ((type*)(x))
#define new_hnd2light(x, name, type) type* name = hnd2light(x, type);
#define assert(x, error, ret) if (!(x)) {push_error_info(error, "In argument "#x); return ret;}
#define obj_assert(x, ret) assert(x, LIGHT_ERR_NULL_IN_DATA, ret);
#define data_assert(x, ret) assert(x, LIGHT_ERR_NULL_IN_DATA, ret);
#define type_assert(x, type_name, ret) if (hnd2light(x, light_base_t)->type != type_name) {push_error_info(LIGHT_ERR_WRONG_TYPE, "Type of object "#x##" must be: "#type_name); return ret;} 
#define light_assert(x, type_name, ret) obj_assert(x, ret); type_assert(x, type_name, ret);

struct light_var_attenuation_t {
	unifhnd constant;
	unifhnd linear;
	unifhnd quadratic;
};

struct light_shader_t {
	GLuint program;
	const char* v_shader_name;
	const char* f_shader_name;
};

struct light_base_t {
	LIGHT_TYPE type;
	unifhnd color;
	unifhnd multiply;
};

struct light_directional_t {
	light_base_t base;
	unifhnd direction;
};

struct light_omni_t {
	light_base_t base;
	unifhnd pos;
	light_var_attenuation_t attenuation;
};

struct light_spot_t {
	light_base_t base;
	unifhnd pos;
	unifhnd direction;
	unifhnd cut_off;
	light_var_attenuation_t attenuation;
};

vme_int light_unit_id;
vme_int LIGHT_ERR_OK = 0;
vme_int LIGHT_ERR_NULL_IN_DATA;
vme_int LIGHT_INVALIDATE_DATA;
vme_int LIGHT_ERR_WRONG_TYPE;

typedef void(*light_destroy_func)(void*);
typedef void(*light_push_var_data_func)(lighthnd);

void light_destroy(lighthnd* light);
void light_destroy_directional(lighthnd light);
void light_destroy_omni(lighthnd light);
void light_destroy_spot(lighthnd light);
light_destroy_func destroys[] = { light_destroy_directional, light_destroy_omni, light_destroy_spot };

void light_push_data_directional(lighthnd light);
void light_push_data_omni(lighthnd light);
void light_push_data_spot(lighthnd light);
light_push_var_data_func light_push_var_datas[] = { light_push_data_directional, light_push_data_omni, light_push_data_spot };

light_shader_t light_shaders[] = { { 0, SHVR_VERTEX_SHADER_NAME, LIGHT_DIRECTIONAL_SHADER_NAME }, 
									{ 0, SHVR_VERTEX_SHADER_NAME, LIGHT_OMNI_SHADER_NAME }, 
									{ 0, SHVR_VERTEX_SHADER_NAME, LIGHT_SPOT_SHADER_NAME } };

void light_init() {
	light_unit_id = vme_register_unit("Light");
	LIGHT_ERR_NULL_IN_DATA = vme_register_error_type(light_unit_id, "Null pointer in input data");
	LIGHT_ERR_WRONG_TYPE = vme_register_error_type(light_unit_id, "Wrong type of object");
	LIGHT_INVALIDATE_DATA = vme_register_error_type(light_unit_id, "Invalidate input data");

	for (int i = 0; i < LIGHT_TYPE_COUNT; i++) {
		light_shaders[i].program = shvr_create_program(light_shaders[i].v_shader_name, light_shaders[i].f_shader_name);
		if (vme_error_appear())
			return;
	}
}

light_attenuation_t light_atten(float constant, float linear, float quadratic) {
	light_attenuation_t t;
	t.constant = constant;
	t.linear = linear;
	t.quadratic = quadratic;
	return t;
}

light_attenuation_t light_atten(light_var_attenuation_t light_var) {
	light_attenuation_t t;
	t.constant = *(float*)unif_get_data(light_var.constant);
	t.linear = *(float*)unif_get_data(light_var.linear);
	t.quadratic = *(float*)unif_get_data(light_var.quadratic);
	return t;
}

light_var_attenuation_t light_var_atten(light_attenuation_t light_atten) {
	light_var_attenuation_t t;
	t.constant = unif_create("light_k1", SHVR_FLOAT);
	t.linear = unif_create("light_k2", SHVR_FLOAT);
	t.quadratic = unif_create("light_k3", SHVR_FLOAT);
	return t;
}

void light_set_var_attenuation(light_var_attenuation_t var_atten, light_attenuation_t atten) {
	unif_set_data(var_atten.constant, &atten.constant);
	unif_set_data(var_atten.linear, &atten.linear);
	unif_set_data(var_atten.quadratic, &atten.quadratic);
}

//--------------BASE LIGHT---------------

void light_create_base(light_base_t* base, vechnd color, float multiply, LIGHT_TYPE type) {
	obj_assert(base);
	base->color = unif_create("light_color", color,  SHVR_VEC3);
	base->multiply = unif_create("light_multipy", SHVR_FLOAT);
	base->type = type;
}

void light_destroy(lighthnd* light) {
	if (!light || !*light) return;
	new_hnd2light(*light, t, light_base_t);
	unif_destroy(&t->color);
	destroys[t->type](*light);
	
	delete *light;
	*light = nullptr;
}

void light_set_color(lighthnd light, vechnd color) {
	obj_assert(light);
	unif_set_data(hnd2light(light, light_base_t)->color, color);
}

vechnd light_get_color(lighthnd light) {
	obj_assert(light, nullptr);
	return unif_get_data(hnd2light(light, light_base_t)->color);
}

void light_set_multiply(lighthnd light, float multiply) {
	obj_assert(light);
	unif_set_data(hnd2light(light, light_base_t)->color, &multiply);
}

void* light_get_multiply(lighthnd light) {
	obj_assert(light, nullptr);
	return unif_get_data(hnd2light(light, light_base_t)->multiply);
}

void light_push_var_data(lighthnd light) {
	obj_assert(light);
	new_hnd2light(light, t, light_base_t);
	unif_push_var_data(t->color);
	unif_push_var_data(t->multiply);
	light_push_var_datas[t->type](light);
}

void light_draw(lighthnd light, int size) {
	light_push_var_data(light);
	glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_SHORT, 0);
}

//--------------------DIRECTIONAL LIGHT----------------------

lighthnd light_create_directional(vechnd color, float multiply, vechnd direction) {
	obj_assert(color, nullptr);
	obj_assert(direction, nullptr);

	light_directional_t* new_light = new light_directional_t;
	light_create_base(&new_light->base, color, multiply, LIGHT_DIRECTIONAL);

	new_light->direction = unif_create("light_direction", direction, SHVR_VEC3);

	return new_light;
}

void light_destroy_directional(lighthnd light) {
	if (!light) return;
	type_assert(light, LIGHT_DIRECTIONAL);
	unif_destroy(&hnd2light(light, light_directional_t)->direction);
}

void light_set_direction_directional(lighthnd light, vechnd direction) {
	light_assert(light, LIGHT_DIRECTIONAL);
	unif_set_data(hnd2light(light, light_directional_t)->direction, direction);
}

vechnd light_get_direction_directional(lighthnd light) {
	light_assert(light, LIGHT_DIRECTIONAL, nullptr);
	return unif_get_data(hnd2light(light, light_directional_t)->direction);
}

void light_push_data_directional(lighthnd light) {
	light_assert(light, LIGHT_DIRECTIONAL);
	new_hnd2light(light, t, light_directional_t);
	unif_push_var_data(t->direction);
}

//------------------------OMNI LIGHT----------------------------

lighthnd light_create_omni(vechnd color, float multiply, vechnd pos, light_attenuation_t attenuation) {
	obj_assert(color, nullptr);
	obj_assert(pos, nullptr);

	light_omni_t* new_light = new light_omni_t;
	light_create_base(&new_light->base, color, multiply, LIGHT_OMNI);

	new_light->pos = unif_create("light_pos", SHVR_VEC3);
	new_light->attenuation = light_var_atten(attenuation);

	return new_light;
}

void light_destroy_omni(lighthnd light) {
	if (!light) return;
	type_assert(light, LIGHT_OMNI);
	unif_destroy(&hnd2light(light, light_omni_t)->pos);
}

void light_set_pos_omni(lighthnd light, vechnd pos) {
	light_assert(light, LIGHT_OMNI);
	unif_set_data(hnd2light(light, light_omni_t)->pos, pos);
}

vechnd light_get_pos_omni(lighthnd light) {
	light_assert(light, LIGHT_OMNI, nullptr);
	return unif_get_data(hnd2light(light, light_omni_t)->pos);
}

void light_set_attenuation_omni(lighthnd light, light_attenuation_t attenuation) {
	light_assert(light, LIGHT_OMNI);
	light_set_var_attenuation(hnd2light(light, light_omni_t)->attenuation, attenuation);
}

light_attenuation_t light_get_attenuation_omni(lighthnd light) {
	light_attenuation_t a;
	light_assert(light, LIGHT_OMNI, light_atten(NAN, NAN, NAN));
	return light_atten(hnd2light(light, light_omni_t)->attenuation);
}

void light_push_data_omni(lighthnd light) {
	light_assert(light, LIGHT_OMNI);
	new_hnd2light(light, t, light_omni_t);
	unif_push_var_data(t->pos);
}

//--------------------SPOT LIGHT--------------------------

lighthnd light_create_spot(vechnd color, float multiply, vechnd pos, vechnd direction, float cutoff, light_attenuation_t attenuation) {
	obj_assert(color, nullptr);
	obj_assert(pos, nullptr);

	light_spot_t* new_light = new light_spot_t;
	light_create_base(&new_light->base, color, multiply, LIGHT_SPOT);

	new_light->pos = unif_create("light_pos", pos, SHVR_VEC3);
	new_light->direction = unif_create("light_direction", direction, SHVR_VEC3);

	new_light->cut_off = unif_create("light_cut_off", &cutoff, SHVR_FLOAT);
	new_light->attenuation = light_var_atten(attenuation);

	return new_light;
}

void light_destroy_spot(lighthnd light) {
	if (!light) return;
	type_assert(light, LIGHT_SPOT);
	new_hnd2light(light, t, light_spot_t);
	unif_destroy(&t->pos);
	unif_destroy(&t->direction);
}

void light_set_direction_spot(lighthnd light, vechnd direction) {
	light_assert(light, LIGHT_SPOT);
	unif_set_data(hnd2light(light, light_spot_t)->direction, direction);
}

vechnd light_get_direction_spot(lighthnd light) {
	light_assert(light, LIGHT_SPOT, nullptr);
	return unif_get_data(hnd2light(light, light_spot_t)->direction);
}

void light_set_pos_spot(lighthnd light, vechnd pos) {
	light_assert(light, LIGHT_SPOT);
	unif_set_data(hnd2light(light, light_spot_t)->pos, pos);
}

vechnd light_get_pos_spot(lighthnd light) {
	light_assert(light, LIGHT_SPOT, nullptr);
	return unif_get_data(hnd2light(light, light_spot_t)->pos);
}

void light_set_attenuation_spot(lighthnd light, light_attenuation_t attenuation) {
	light_assert(light, LIGHT_SPOT);
	light_set_var_attenuation(hnd2light(light, light_spot_t)->attenuation, attenuation);
}

light_attenuation_t light_get_attenuation_spot(lighthnd light, light_attenuation_t attenuation) {
	light_assert(light, LIGHT_SPOT, light_atten(NAN, NAN, NAN));
	return light_atten(hnd2light(light, light_spot_t)->attenuation);
}

void light_set_cutoff_spot(lighthnd light, float cutoff) {
	light_assert(light, LIGHT_SPOT);
	unif_set_data(hnd2light(light, light_spot_t)->cut_off, &cutoff);
}

void* light_get_cutoff_spot(lighthnd light) {
	light_assert(light, LIGHT_SPOT, nullptr);
	return unif_get_data(hnd2light(light, light_spot_t)->cut_off);
}

void light_push_data_spot(lighthnd light) {
	light_assert(light, LIGHT_SPOT);
	new_hnd2light(light, t, light_spot_t);
	unif_push_var_data(t->pos);
	unif_push_var_data(t->cut_off);
	unif_push_var_data(t->direction);
}