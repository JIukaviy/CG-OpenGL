#include "material.h"
#include "shader_var.h"
#include "vec_mat_errors.h"
#include "list.h"

typedef efcthnd(*efct_copy_func)(efcthnd);

struct effect_t {
	GLuint program;
	const char* shader_name;
	MTL_EFFECT_TYPE type;
};

struct effect_diffuse_t {
	effect_t base;
	shvrhnd color;
};

struct effect_specular_t {
	effect_t base;
	shvrhnd color;
	shvrhnd shinines;
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

//efct_copy_func efct_copy_funcs[] = {efct_copy_diffuse};

void mtl_init() {
	mtl_unit_id = vme_register_unit("Materials");
	MTL_ERR_NULL_IN_DATA = vme_register_error_type(mtl_unit_id, "Null pointer in input data");
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

void efct_init_base(efcthnd effect, const char* shader_name) {
	obj_assert(effect);
	new_hnd2efct(effect, t);
	t->shader_name = shader_name;
}

void efct_destroy(efcthnd* effect) {
	obj_assert(effect);
	obj_assert(*effect);
	new_hnd2efct(effect, t);
	delete t->shader_name;
	efct_destroys[t->type](effect);
}

//------------------DIFFUSE_EFFECT-----------------

efcthnd efct_create_diffuse(vechnd color) {
	obj_assert(color, nullptr);
	effect_diffuse_t* new_effect = new effect_diffuse_t;
	efct_init_base((efcthnd)new_effect, "shaders/diffuse_mtl.f.glsl");
	new_effect->color = shvr_create_uniform(color, "color", SHVR_VEC3);

	return new_effect;
}

void efct_destroy_diffuse(efcthnd* effect) {
	obj_assert(effect);
	obj_assert(*effect);

	new_hnd2type(*effect, t, effect_diffuse_t);
	shvr_destroy(&t->color);

	delete t;
	*effect = nullptr;
}

efcthnd efct_copy_diffuse(efcthnd effect) {
	obj_assert(effect, nullptr);
	new_hnd2type(effect, t, effect_diffuse_t);
}

//-----------------SPECULAR_EFFECT----------------

efcthnd efct_create_specular(vechnd color, float shinines) {
	obj_assert(color, nullptr);
	effect_specular_t* new_effect = new effect_specular_t;
	efct_init_base((efcthnd)new_effect, "shaders/specular_mtl.f.glsl");

	new_effect->color = shvr_create_uniform(color, "color", SHVR_VEC3);
	new_effect->shinines = shvr_create_uniform(&shinines, "shininess", SHVR_FLOAT);

	return new_effect;
}

void efct_destroy_specular(efcthnd* effect) {
	obj_assert(effect);
	obj_assert(*effect);

	new_hnd2type(*effect, t, effect_specular_t);
	shvr_destroy(&t->color);

	delete t;
	*effect = nullptr;
}
