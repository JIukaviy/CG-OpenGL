#include "material.h"
#include "shader_var.h"
#include "vec_mat_errors.h"
#include "list.h"

typedef efcthnd(*efct_copy_func)(efcthnd);

struct effect_t {
	GLuint program;
	const char* v_shader_name;
	const char* f_shader_name;
	MTL_EFFECT_TYPE type;
};

struct effect_diffuse_t {
	MTL_EFFECT_TYPE type;
	unifhnd color;
};

struct effect_specular_t {
	MTL_EFFECT_TYPE type;
	unifhnd color;
	unifhnd shinines;
};

struct material_t {
	efcthnd* effects;
	int size;
};

vme_int mtl_unit_id;
vme_int MTL_ERR_NULL_IN_DATA;
vme_int MTL_ERR_INVALIDE_DATA;

#define hnd2type(x, type) ((type*)(x));
#define new_hnd2type(x, name, type) type* name = hnd2type(x, type);
#define hnd2mtl(x) hnd2type(x, material_t);
#define hnd2efct(x) hnd2type(x, effect_t);
#define new_hnd2mtl(x, name) new_hnd2type(x, name, material_t);
#define new_hnd2efct(x, name) new_hnd2type(x, name, effect_t);
#define assert(x, error, ret) if (!(x)) {push_error_info(error, "In argument "#x); return ret;}
#define obj_assert(x, ret) assert(x, MTL_ERR_NULL_IN_DATA, ret);
#define data_assert(x, ret) assert(x, SHVR_ERR_INVALIDE_DATA, ret);

effect_t effect_types[] = { { 0, SHVR_VERTEX_SHADER_NAME, MTL_DIFFUSE_SHADER_NAME }, { 0, SHVR_VERTEX_SHADER_NAME, MTL_SPECULAR_SHADER_NAME } };

//efct_copy_func efct_copy_funcs[] = {efct_copy_diffuse};

void mtl_init() {
	mtl_unit_id = vme_register_unit("Materials");
	MTL_ERR_NULL_IN_DATA = vme_register_error_type(mtl_unit_id, "Null pointer in input data");

	for (int i = 0; i < MTL_EFFECTS_COUNT; i++) {
		effect_types[i].program = shvr_create_program(effect_types[i].v_shader_name, effect_types[i].f_shader_name);
		if (vme_error_appear()) {
			vme_print_errors();
			return;
		}
	}
}

void efct_destroy_diffuse(efcthnd* effect);
void efct_destroy_specular(efcthnd* effect);

destroy_func efct_destroys[] = { efct_destroy_diffuse, efct_destroy_specular };

//-------------------MATERIAL------------------

mtlhnd mtl_create() {
	material_t* new_mtl = new material_t;
	memset(new_mtl, 0, sizeof(material_t));
	return new_mtl;
}

void mtl_add_effect(mtlhnd mtl, efcthnd effect) {
	obj_assert(effect);
	obj_assert(mtl);
	new_hnd2mtl(mtl, m);
	new_hnd2efct(effect, e);

	efcthnd* effects = new efcthnd[m->size + 1];
	memcpy(effects, m->effects, m->size*sizeof(efcthnd));

	delete m->effects;
	m->effects = effects;
	m->effects[m->size] = effect;
	m->size++;
}
/*
mtlhnd mtl_copy(mtlhnd mtl) {
	obj_assert(mtl);
	new_hnd2mtl(mtl, m);

	mtlhnd new_mtl = mtl_create();
	new_hnd2mtl(mtl, nm);

	nm->size = m->size;
	//nm->effects = new effect_t[nm->size];

	for (int i = 0; i < nm->size; i++) {
		nm->effects[i] = new effect_t;

	}
}*/

efcthnd* mtl_get_efcts(mtlhnd mtl) {
	obj_assert(mtl, nullptr);
	new_hnd2mtl(mtl, t);
	return t->effects;
}

int mtl_get_size(mtlhnd mtl) {
	obj_assert(mtl, -1);
	new_hnd2mtl(mtl, t);
	return t->size;
}

void mtl_draw(mtlhnd hnd, int size) {
	obj_assert(hnd);
	new_hnd2mtl(hnd, t);

	for (int i = 0; t->size; i++) {
		glUseProgram(efct_get_program(t->effects[i]));
		glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_SHORT, 0);
	}
}

void mtl_destroy(mtlhnd* hnd) {
	obj_assert(hnd);
	obj_assert(*hnd);
	new_hnd2mtl(*hnd, t);
	
	for (int i = 0; i < t->size; i++) 
		efct_destroy(&t->effects[i]);

	delete t->effects;
	delete *hnd;
	*hnd = nullptr;
}

//------------------EFFECTS------------------------

void efct_destroy(efcthnd* hnd) {
	obj_assert(hnd);
	obj_assert(*hnd);
	efct_destroys[*(int*)hnd](hnd);
}

GLuint efct_get_program(efcthnd hnd) {
	obj_assert(hnd, -1);
	new_hnd2efct(hnd, t);
	return t->program;
}

//------------------DIFFUSE_EFFECT-----------------

efcthnd efct_create_diffuse(vechnd color) {
	obj_assert(color, nullptr);
	effect_diffuse_t* new_effect = new effect_diffuse_t;
	new_effect->type = MTL_DIFFUSE;
	new_effect->color = unif_create("color", SHVR_VEC3);
	unif_set_data(new_effect->color, color);

	return new_effect;
}

void efct_destroy_diffuse(efcthnd* effect) {
	obj_assert(effect);
	obj_assert(*effect);

	new_hnd2type(*effect, t, effect_diffuse_t);
	unif_destroy(&t->color);

	delete t;
	*effect = nullptr;
}

efcthnd efct_copy_diffuse(efcthnd effect) {
	obj_assert(effect, nullptr);
	new_hnd2type(effect, t, effect_diffuse_t);
}

void efct_refresh_var_locations_diffuse(efcthnd efct) {
	obj_assert(efct);
	new_hnd2type(efct, t, effect_diffuse_t);

	unif_refresh_location(t->color, effect_types[t->type].program);
}

//-----------------SPECULAR_EFFECT----------------

efcthnd efct_create_specular(vechnd color, float shinines) {
	obj_assert(color, nullptr);
	effect_specular_t* new_effect = new effect_specular_t;
	new_effect->type = MTL_DIFFUSE;

	new_effect->color = unif_create("color", SHVR_VEC3);
	unif_set_data(new_effect->color, color);

	new_effect->shinines = unif_create("shininess", SHVR_FLOAT);
	unif_set_data(new_effect->color, &shinines);

	return new_effect;
}

void efct_destroy_specular(efcthnd* effect) {
	obj_assert(effect);
	obj_assert(*effect);

	new_hnd2type(*effect, t, effect_specular_t);
	unif_destroy(&t->color);
	unif_destroy(&t->shinines);

	delete t;
	*effect = nullptr;
}


void efct_refresh_var_locations_specular(efcthnd efct) {
	obj_assert(efct);
	new_hnd2type(efct, t, effect_specular_t);

	unif_refresh_location(t->color, effect_types[t->type].program);
	unif_refresh_location(t->shinines, effect_types[t->type].program);
}