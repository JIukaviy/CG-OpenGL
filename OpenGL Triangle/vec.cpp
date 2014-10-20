#include "vec.h"
#include "vec_mat_errors.h"
#include <string>
#include <iostream>

#define hnd2vec(x) ((vec_t*)(x))
#define vec_assert_1(x) if (!x || !hnd2vec(x)->data) { push_error_info(VEC_ERR_NULL_IN_DATA, "In argument: "#x); return nullptr;	}\
						if (hnd2vec(x)->size < 1 || hnd2vec(x)->size > VEC_MAX_SIZE) {	 push_error(VEC_ERR_BAD_SIZE); return nullptr;}
#define vec_assert_2(x, y) if (vec_validate((x), (y))) return nullptr;
#define vec_create_out(size_name) vechnd out_hnd = vec_create(size_name); vec_t* out = hnd2vec(out_hnd);

struct vec_t {
	vec_elem_t* data;
	int size;
};

typedef vec_elem_t(*on_elem_pfunc)(vec_elem_t a, vec_elem_t b);

vme_int vec_unit_id;
vme_int VEC_ERR_OK = 0;
vme_int VEC_ERR_DIFF_SIZE;
vme_int VEC_ERR_NULL_IN_DATA;
vme_int VEC_ERR_BAD_SIZE;
vme_int VEC_ERR_DIV_BY_ZERO;
vme_int VEC_ERR_OUT_OF_BOUNDS;

void vec_init(){
	vec_unit_id = vme_register_unit("Vectors");
	VEC_ERR_DIFF_SIZE = vme_register_error_type(vec_unit_id, "Different size between two vectors");
	VEC_ERR_NULL_IN_DATA = vme_register_error_type(vec_unit_id, "Null pointer in input data");
	VEC_ERR_BAD_SIZE = vme_register_error_type(vec_unit_id, "Invalide size of vector");
	VEC_ERR_DIV_BY_ZERO = vme_register_error_type(vec_unit_id, "Divide by zero");
	VEC_ERR_OUT_OF_BOUNDS = vme_register_error_type(vec_unit_id, "Out of bounds of vector coordinates");
}


 vme_int vec_validate(vechnd a){
	vec_t* t = hnd2vec(a);

	if (!t || !t->data) {
		push_error(VEC_ERR_NULL_IN_DATA);
		return VEC_ERR_NULL_IN_DATA;
	}

	if (t->size < 1 || t->size > VEC_MAX_SIZE) {
		push_error(VEC_ERR_BAD_SIZE);
		return VEC_ERR_BAD_SIZE;
	}

	return 0;
}

vme_int vec_validate(vechnd a, vechnd b){
	vec_t* t = hnd2vec(a);
	vec_t* g = hnd2vec(b);

	if (vme_int err_code = vec_validate(a)) {
		vme_set_error_info(vme_get_last_err(vec_unit_id), "In first argument");
		return err_code;
	}
	if (vme_int err_code = vec_validate(b)) {
		vme_set_error_info(vme_get_last_err(vec_unit_id), "In second argument");
		return err_code;
	}
	if (t->size != g->size) {
		push_error(VEC_ERR_DIFF_SIZE);
		return VEC_ERR_DIFF_SIZE;
	}

	return 0;
}

vechnd vec_create(int size){
	if (size < 1 || size > VEC_MAX_SIZE) {
		push_error(VEC_ERR_BAD_SIZE);
		return nullptr;
	}

	vec_t* newvec = new vec_t;
	memset(newvec, 0, sizeof(vec_t));

	newvec->data = new vec_elem_t[size];
	memset(newvec->data, 0, sizeof(vec_elem_t)*size);

	newvec->size = size;

	return (vechnd*)newvec;
}

void vec_destroy(vechnd* a){
	if (!a || vec_validate(*a))
		return;

	delete hnd2vec(*a)->data;

	delete *a;
	*a = nullptr;
}

vechnd vec_on_elem(vechnd a, vechnd b, on_elem_pfunc func){
	vec_t* t = hnd2vec(a);
	vec_t* g = hnd2vec(b);

	vec_assert_2(a, b);

	vec_create_out(t->size);	

	vme_set_no_errors(vec_unit_id);
	for (int i = 0; i < t->size; i++)
		out->data[i] = func(t->data[i], g->data[i]);

	if (vme_error_appear(vec_unit_id))
		vec_destroy(&out_hnd);

	return out_hnd;
}

vechnd vec_on_elem(vechnd a, vec_elem_t b, on_elem_pfunc func){
	vec_t* t = hnd2vec(a);

	vec_assert_1(a);

	vec_create_out(t->size);

	vme_set_no_errors(vec_unit_id);
	for (int i = 0; i < t->size; i++)
		out->data[i] = func(t->data[i], b);

	if (vme_error_appear(vec_unit_id))
		vec_destroy(&out_hnd);

	return out_hnd;
}

vechnd vec_cross(vechnd a, vechnd b){
	vec_t* t = hnd2vec(a);
	vec_t* g = hnd2vec(b);

	vec_assert_2(a, b);

	if (t->size != 3 || g->size != 3) {
		push_error(VEC_ERR_BAD_SIZE);
		return nullptr;
	}

	vec_create_out(t->size);

	vec_elem_t x = t->data[1] * g->data[2] - t->data[2] * g->data[1];
	vec_elem_t y = t->data[2] * g->data[0] - t->data[0] * g->data[2];
	vec_elem_t z = t->data[0] * g->data[1] - t->data[1] * g->data[0];

	out->data[0] = x;
	out->data[1] = y;
	out->data[2] = z;

	return out_hnd;
}

vec_elem_t vec_dot(vechnd a, vechnd b){	
	if (vec_validate(a, b))
		return NAN;

	vec_t* t = hnd2vec(a);
	vec_t* g = hnd2vec(b);

	vec_elem_t out = 0;

	for (int i = 0; i < t->size; i++)
		out += t->data[i] * g->data[i];

	return out;
}


vec_elem_t vec_on_elem_add(vec_elem_t a, vec_elem_t b){
	return a + b;
}

vec_elem_t vec_on_elem_sub(vec_elem_t a, vec_elem_t b){
	return a - b;
}

vec_elem_t vec_on_elem_muls(vec_elem_t a, vec_elem_t b){
	return a * b;
}

vec_elem_t vec_on_elem_div(vec_elem_t a, vec_elem_t b){
	if (b == 0) {
		push_error(VEC_ERR_DIV_BY_ZERO);
		return NAN;
	}

	return a / b;
}

vechnd vec_add(vechnd a, vechnd b){
	return vec_on_elem(a, b, vec_on_elem_add);
}

vechnd vec_sub(vechnd a, vechnd b){
	return vec_on_elem(a, b, vec_on_elem_sub);
}

vechnd vec_mul(vechnd a, vechnd b){
	return vec_on_elem(a, b, vec_on_elem_muls);
}

vechnd vec_div(vechnd a, vechnd b){
	return vec_on_elem(a, b, vec_on_elem_div);
}



vechnd vec_add(vechnd a, vec_elem_t b, vechnd* out_hnd){
	return vec_on_elem(a, b, vec_on_elem_add);
}

vechnd vec_sub(vechnd a, vec_elem_t b, vechnd* out_hnd){
	return vec_on_elem(a, b, vec_on_elem_sub);
}

vechnd vec_mul(vechnd a, vec_elem_t b, vechnd* out_hnd){
	return vec_on_elem(a, b, vec_on_elem_muls);
}

vechnd vec_div(vechnd a, vec_elem_t b, vechnd* out_hnd){
	return vec_on_elem(a, b, vec_on_elem_div);
}

vec_elem_t vec_length(vechnd a){
	if (vec_validate(a))
		return NAN;
	
	vec_t* t = hnd2vec(a);
	vec_elem_t res = 0;

	for (int i = 0; i < t->size; i++)
		res += t->data[i] * t->data[i];

	return sqrt(res);
}

int vec_size(vechnd a){
	if (!a) {
		push_error(VEC_ERR_NULL_IN_DATA);
		return 0;
	}

	return hnd2vec(a)->size;
}

vechnd vec_normalize(vechnd in_hnd){
	vec_assert_1(in_hnd);

	vec_t* t = hnd2vec(in_hnd);
	vechnd out_hnd = vec_copy(in_hnd);
	vec_elem_t len = vec_length(in_hnd);

	for (int i = 0; i < t->size; i++)
		hnd2vec(out_hnd)->data[i] /= len;

	return out_hnd;
}

void vec_set_elem(vechnd a, int id, vec_elem_t val){
	if (vec_validate(a))
		return;

	vec_t* t = hnd2vec(a);

	if (id < 0 || id > t->size - 1) {
		push_error(VEC_ERR_BAD_SIZE);
		return;
	}

	t->data[id] = val;
}

void vec_set_elems(vechnd a, vec_elem_t* elems){
	if (vec_validate(a))
		return;

	if (!elems) {
		push_error(VEC_ERR_NULL_IN_DATA);
		return;
	}

	vec_t *t = hnd2vec(a);

	memcpy(t->data, elems, sizeof(vec_elem_t)*t->size);
}

vec_elem_t vec_get_elem(vechnd a, int id){
	if (vec_validate(a))
		return NAN;

	vec_t* t = hnd2vec(a);

	if (id < 0 || id > t->size - 1) {
		push_error(VEC_ERR_BAD_SIZE);
		return NAN;
	}

	return t->data[id];
}

vec_elem_t* vec_get_elems(vechnd in_hnd){
	vec_assert_1(in_hnd);

	return hnd2vec(in_hnd)->data;
}

vechnd vec_copy(vechnd in_hnd){
	vec_assert_1(in_hnd);

	vec_t* t = hnd2vec(in_hnd);
	vec_create_out(t->size);

	memcpy(hnd2vec(out_hnd)->data, t->data, sizeof(vec_elem_t)*t->size);

	return out_hnd;
}

bool vec_equal(vechnd a, vechnd b){
	if (vec_validate(a, b))
		return false;

	vec_t* t = hnd2vec(a);
	vec_t* g = hnd2vec(b);

	return !memcmp(t->data, g->data, sizeof(vec_elem_t)*t->size);
}

void vec_print(vechnd a){
	if (vec_validate(a))
		return;

	vec_t* t = hnd2vec(a);

	for (int i = 0; i < t->size; i++) {
		std::cout << t->data[i];
		if (i < t->size - 1)
			std::cout << ", ";
	}

	std::cout << std::endl;
}