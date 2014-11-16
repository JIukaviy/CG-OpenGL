#include "light.h"
#include "gc.h"
#include "vec_mat_errors.h"
#include "file.h"
#include <string>

#define hnd2light(x, type) ((type*)(x))
#define new_hnd2light(x, name, type) type* name = hnd2light(x, type);
#define assert(x, error, ret) if (!(x)) {push_error_info(error, "In argument "#x); return ret;}
#define obj_assert(x, ret) assert(x, LIGHT_ERR_NULL_IN_DATA, ret);
#define data_assert(x, ret) assert(x, LIGHT_ERR_NULL_IN_DATA, ret);
#define type_assert(x, type_name, ret) if (hnd2light(x, light_base_t)->type != type_name) {push_error_info(LIGHT_ERR_WRONG_TYPE, "Type of object "#x##" must be: "#type_name); return ret;} 
#define light_assert(x, type_name, ret) obj_assert(x, ret); type_assert(x, type_name, ret);

struct light_base_t {
	LIGHT_TYPE type;
	vechnd color;
	float multiply;
	gc_id_t gc_id;
};

struct light_directional_t {
	light_base_t base;
	vechnd direction;
};

struct light_omni_t {
	light_base_t base;
	vechnd pos;
	light_attenuation_t attenuation;
};

struct light_spot_t {
	light_base_t base;
	vechnd pos;
	vechnd direction;
	float cut_off;
	light_attenuation_t attenuation;
};

vme_int light_unit_id;
vme_int LIGHT_ERR_OK = 0;
vme_int LIGHT_ERR_NULL_IN_DATA;
vme_int LIGHT_INVALIDATE_DATA;
vme_int LIGHT_ERR_WRONG_TYPE;

void light_destroy(lighthnd* light);
void light_destroy_directional(lighthnd light);
void light_destroy_omni(lighthnd light);
void light_destroy_spot(lighthnd light);

typedef void(*light_destroy_func)(void*);

light_destroy_func destroys[] = { light_destroy_directional, light_destroy_omni, light_destroy_spot };
const char* shader_names[] = { "shaders/directional_light.dat", "shaders/omni_light.dat", "shaders/spot_light.dat" };

void light_init() {
	light_unit_id = vme_register_unit("LIGHT");
	LIGHT_ERR_NULL_IN_DATA = vme_register_error_type(light_unit_id, "Null pointer in input data");
	LIGHT_ERR_WRONG_TYPE = vme_register_error_type(light_unit_id, "Wrong type of object");
	LIGHT_INVALIDATE_DATA = vme_register_error_type(light_unit_id, "Invalidate input data");
}

light_attenuation_t light_attenuation(float constant, float linear, float quadratic) {
	light_attenuation_t a;
	a.constant = constant;
	a.linear = linear;
	a.quadratic = quadratic;
	return a;
}

char* light_get_shader(LIGHT_TYPE type) {
	data_assert(((int)type >= 0 && (int)type < LIGHT_TYPE_COUNT), nullptr);
	return file_read(shader_names[type]);
}

//--------------BASE LIGHT---------------

void light_create_base(light_base_t* base, vechnd color, float multiply, LIGHT_TYPE type) {
	obj_assert(base);
	base->color = vec_copy(color);
	vec_gc_unregist(base->color);
	base->multiply = multiply;
	base->type = type;
	base->gc_id = gc_push_garbage(base, light_destroy);
}

void light_destroy(lighthnd* light) {
	if (!light || !*light) return;
	new_hnd2light(*light, t, light_base_t);
	gc_unregist(t->gc_id);
	vec_destroy(&t->color);
	destroys[t->type](*light);
	
	delete *light;
	*light = nullptr;
}

void light_set_color(lighthnd light, vechnd color) {
	obj_assert(light);
	vec_copy(hnd2light(light, light_base_t)->color, color);
}

vechnd light_get_color(lighthnd light) {
	obj_assert(light, nullptr);
	return vec_copy(hnd2light(light, light_base_t)->color);
}

void light_set_multiply(lighthnd light, float multiply) {
	obj_assert(light);
	hnd2light(light, light_base_t)->multiply = multiply;
}

float light_get_multiply(lighthnd light) {
	obj_assert(light, NAN);
	return hnd2light(light, light_base_t)->multiply;
}

char* light_get_shader(lighthnd light) {
	obj_assert(light, nullptr);
	new_hnd2light(light, t, light_base_t);

	return light_get_shader(t->type);
}

//--------------------DIRECTIONAL LIGHT----------------------

lighthnd light_create_directional(vechnd color, float multiply, vechnd direction) {
	obj_assert(color, nullptr);
	obj_assert(direction, nullptr);

	light_directional_t* new_light = new light_directional_t;
	light_create_base(&new_light->base, color, multiply, LIGHT_DIRECTIONAL);

	new_light->direction = vec_copy(direction);
	vec_gc_unregist(new_light->direction);

	return new_light;
}

void light_destroy_directional(lighthnd light) {
	if (!light) return;
	type_assert(light, LIGHT_DIRECTIONAL);
	vec_destroy(&hnd2light(light, light_directional_t)->direction);
}

void light_set_direction_directional(lighthnd light, vechnd direction) {
	light_assert(light, LIGHT_DIRECTIONAL);
	vec_copy(hnd2light(light, light_directional_t)->direction, direction);
}

vechnd light_get_direction_directional(lighthnd light) {
	light_assert(light, LIGHT_DIRECTIONAL, nullptr);
	return vec_copy(hnd2light(light, light_directional_t)->direction);
}

//------------------------OMNI LIGHT----------------------------

lighthnd light_create_omni(vechnd color, float multiply, vechnd pos, light_attenuation_t attenuation) {
	obj_assert(color, nullptr);
	obj_assert(pos, nullptr);

	light_omni_t* new_light = new light_omni_t;
	light_create_base(&new_light->base, color, multiply, LIGHT_OMNI);

	new_light->pos = vec_copy(pos);
	vec_gc_unregist(new_light->pos);

	new_light->attenuation = attenuation;

	return new_light;
}

void light_destroy_omni(lighthnd light) {
	if (!light) return;
	type_assert(light, LIGHT_OMNI);
	vec_destroy(&hnd2light(light, light_omni_t)->pos);
}

void light_set_pos_omni(lighthnd light, vechnd pos) {
	light_assert(light, LIGHT_OMNI);
	vec_copy(hnd2light(light, light_omni_t)->pos, pos);
}

vechnd light_get_pos_omni(lighthnd light) {
	light_assert(light, LIGHT_OMNI, nullptr);
	return vec_copy(hnd2light(light, light_omni_t)->pos);
}

void light_set_attenuation_omni(lighthnd light, light_attenuation_t attenuation) {
	light_assert(light, LIGHT_OMNI);
	hnd2light(light, light_omni_t)->attenuation = attenuation;
}

light_attenuation_t light_get_attenuation_omni(lighthnd light) {
	light_attenuation_t a;
	light_assert(light, LIGHT_OMNI, light_attenuation(NAN, NAN, NAN));
	return hnd2light(light, light_omni_t)->attenuation;
}

//--------------------SPOT LIGHT--------------------------

lighthnd light_create_spot(vechnd color, float multiply, vechnd pos, vechnd direction, float cutoff, light_attenuation_t attenuation) {
	obj_assert(color, nullptr);
	obj_assert(pos, nullptr);

	light_spot_t* new_light = new light_spot_t;
	light_create_base(&new_light->base, color, multiply, LIGHT_SPOT);

	new_light->pos = vec_copy(pos);
	new_light->direction = vec_copy(direction);
	vec_gc_unregist(new_light->pos);
	vec_gc_unregist(new_light->direction);

	new_light->cut_off = cutoff;
	new_light->attenuation = attenuation;

	return new_light;
}

void light_destroy_spot(lighthnd light) {
	if (!light) return;
	type_assert(light, LIGHT_SPOT);
	new_hnd2light(light, t, light_spot_t);
	vec_destroy(&t->pos);
	vec_destroy(&t->direction);
}

void light_set_direction_spot(lighthnd light, vechnd direction) {
	light_assert(light, LIGHT_SPOT);
	vec_copy(hnd2light(light, light_spot_t)->direction, direction);
}

vechnd light_get_direction_spot(lighthnd light) {
	light_assert(light, LIGHT_SPOT, nullptr);
	return vec_copy(hnd2light(light, light_spot_t)->direction);
}

void light_set_pos_spot(lighthnd light, vechnd pos) {
	light_assert(light, LIGHT_SPOT);
	vec_copy(hnd2light(light, light_spot_t)->pos, pos);
}

vechnd light_get_pos_spot(lighthnd light) {
	light_assert(light, LIGHT_SPOT, nullptr);
	return vec_copy(hnd2light(light, light_spot_t)->pos);
}

void light_set_attenuation_spot(lighthnd light, light_attenuation_t attenuation) {
	light_assert(light, LIGHT_SPOT);
	hnd2light(light, light_spot_t)->attenuation = attenuation;
}

light_attenuation_t light_get_attenuation_spot(lighthnd light, light_attenuation_t attenuation) {
	light_assert(light, LIGHT_SPOT, light_attenuation(NAN, NAN, NAN));
	return hnd2light(light, light_spot_t)->attenuation;
}

void light_set_cutoff_spot(lighthnd light, float cutoff) {
	light_assert(light, LIGHT_SPOT);
	hnd2light(light, light_spot_t)->cut_off = cutoff;
}

float light_get_cutoff_spot(lighthnd light) {
	light_assert(light, LIGHT_SPOT, NAN);
	return hnd2light(light, light_spot_t)->cut_off;
}