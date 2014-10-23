#include "vec.h"
#include "vec_mat_errors.h"
#include "gc.h"
#include <string>
#include <iostream>

#define hnd2vec(x) ((vec_t*)(x))
#define vec_assert_1(x, ret) if (!x || !hnd2vec(x)->data) { push_error_info(VEC_ERR_NULL_IN_DATA, "In argument: "#x); return ret;}\
						if (hnd2vec(x)->size < 1 || hnd2vec(x)->size > VEC_MAX_SIZE) {push_error(VEC_ERR_BAD_SIZE); return ret;}
#define vec_assert_2(x, y, ret) vec_assert_1(x, ret); vec_assert_1(y, ret); if (hnd2vec(x)->size != hnd2vec(y)->size) {push_error(VEC_ERR_DIFF_SIZE); return ret;}
#define vec_create_out(size_name) vechnd out_hnd = vec_create(size_name); vec_t* out = hnd2vec(out_hnd);

struct vec_t {
	vec_elem_t* data;
	int size;
	gc_id_t gc_id;
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

	newvec->gc_id = gc_push_garbage(newvec, vec_destroy);

	return (vechnd*)newvec;
}

vechnd vec_create3(vec_elem_t x, vec_elem_t y, vec_elem_t z){
	vec_create_out(3);

	vec_set_elem(out_hnd, 0, x);
	vec_set_elem(out_hnd, 1, y);
	vec_set_elem(out_hnd, 2, z);

	return out_hnd;
}

vechnd vec_create4(vec_elem_t x, vec_elem_t y, vec_elem_t z, vec_elem_t w){
	vec_create_out(4);

	vec_set_elem(out_hnd, 0, x);
	vec_set_elem(out_hnd, 1, y);
	vec_set_elem(out_hnd, 2, z);
	vec_set_elem(out_hnd, 3, w);

	return out_hnd;
}

vechnd vec_create4(vechnd vec3, vec_elem_t w){
	vec_create_out(4);
	if (hnd2vec(vec3)->size != 3) {
		push_error_info(VEC_ERR_BAD_SIZE, "Size of vector not equal 3");
		return nullptr;
	}
	memcpy(out->data, hnd2vec(vec3)->data, sizeof(vec_elem_t) * 3);
	vec_set_elem(out_hnd, 3, w);

	return out_hnd;
}

void vec_destroy(vechnd* hnd){
	vec_t* t = hnd2vec(*hnd);
	if (!hnd || !*hnd)
		return;

	gc_unregist(t->gc_id);

	delete t->data;
	delete *hnd;
	*hnd = nullptr;
}

vechnd vec_on_elem(vechnd a, vechnd b, on_elem_pfunc func){
	vec_t* t = hnd2vec(a);
	vec_t* g = hnd2vec(b);

	vec_assert_2(a, b, nullptr);

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

	vec_assert_1(a, nullptr);

	vec_create_out(t->size);

	vme_set_no_errors(vec_unit_id);
	for (int i = 0; i < t->size; i++)
		out->data[i] = func(t->data[i], b);

	if (vme_error_appear(vec_unit_id))
		vec_destroy(&out_hnd);

	return out_hnd;
}

vechnd vec_cross(vechnd a, vechnd b){
	vec_assert_2(a, b, nullptr);

	vec_t* t = hnd2vec(a);
	vec_t* g = hnd2vec(b);

	if (t->size != 3 || g->size != 3) {
		push_error_info(VEC_ERR_BAD_SIZE, "Cross of two vectors available only for 3 dimensional vectors");
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
	vec_assert_2(a, b, NAN);

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



vechnd vec_add(vechnd a, vec_elem_t b){
	return vec_on_elem(a, b, vec_on_elem_add);
}

vechnd vec_sub(vechnd a, vec_elem_t b){
	return vec_on_elem(a, b, vec_on_elem_sub);
}

vechnd vec_mul(vechnd a, vec_elem_t b){
	return vec_on_elem(a, b, vec_on_elem_muls);
}

vechnd vec_div(vechnd a, vec_elem_t b){
	return vec_on_elem(a, b, vec_on_elem_div);
}

vec_elem_t vec_length(vechnd hnd){
	vec_assert_1(hnd, NAN);
	
	vec_t* t = hnd2vec(hnd);
	vec_elem_t res = 0;

	for (int i = 0; i < t->size; i++)
		res += t->data[i] * t->data[i];

	return sqrt(res);
}

int vec_size(vechnd hnd){
	vec_assert_1(hnd, 0);

	return hnd2vec(hnd)->size;
}

vechnd vec_normalize(vechnd hnd){
	vec_assert_1(hnd, nullptr);

	vec_t* t = hnd2vec(hnd);
	vechnd out_hnd = vec_copy(hnd);
	vec_elem_t len = vec_length(hnd);

	for (int i = 0; i < t->size; i++)
		hnd2vec(out_hnd)->data[i] /= len;

	return out_hnd;
}

vechnd vec_invert(vechnd hnd){
	vec_assert_1(hnd, nullptr);
	vec_t* t = hnd2vec(hnd);
	vec_create_out(t->size);

	for (int i = 0; i < t->size; i++)
		out->data[i] = -t->data[i];

	return out_hnd;
}

void vec_set_elem(vechnd hnd, int id, vec_elem_t val){
	vec_assert_1(hnd);

	vec_t* t = hnd2vec(hnd);

	if (id < 0 || id > t->size - 1) {
		push_error(VEC_ERR_BAD_SIZE);
		return;
	}

	t->data[id] = val;
}

void vec_set_elems(vechnd hnd, vec_elem_t* elems){
	vec_assert_1(hnd);

	if (!elems) {
		push_error(VEC_ERR_NULL_IN_DATA);
		return;
	}

	vec_t *t = hnd2vec(hnd);

	memcpy(t->data, elems, sizeof(vec_elem_t)*t->size);
}

vec_elem_t vec_get_elem(vechnd hnd, int id){
	vec_assert_1(hnd, NAN);

	vec_t* t = hnd2vec(hnd);

	if (id < 0 || id > t->size - 1) {
		push_error(VEC_ERR_BAD_SIZE);
		return NAN;
	}

	return t->data[id];
}

vec_elem_t* vec_get_elems(vechnd hnd){
	vec_assert_1(hnd, nullptr);

	return hnd2vec(hnd)->data;
}

vechnd vec_copy(vechnd hnd){
	vec_assert_1(hnd, nullptr);

	vec_t* t = hnd2vec(hnd);
	vec_create_out(t->size);

	memcpy(hnd2vec(out_hnd)->data, t->data, sizeof(vec_elem_t)*t->size);

	return out_hnd;
}

void vec_copy(vechnd dst, vechnd src){
	vec_assert_2(dst, src);

	vec_t* t = hnd2vec(dst);
	vec_t* g = hnd2vec(src);

	memcpy(t->data, g->data, sizeof(vec_elem_t)*t->size);
}

void vec_unregist_in_gc(vechnd hnd){
	vec_assert_1(hnd);
	vec_t* t = hnd2vec(hnd);
	gc_unregist(t->gc_id);
}

bool vec_equal(vechnd a, vechnd b){
	vec_assert_1(a, b, false);

	vec_t* t = hnd2vec(a);
	vec_t* g = hnd2vec(b);

	return !memcmp(t->data, g->data, sizeof(vec_elem_t)*t->size);
}

void vec_print(vechnd a){
	vec_assert_1(a);

	vec_t* t = hnd2vec(a);

	for (int i = 0; i < t->size; i++) {
		std::cout << t->data[i];
		if (i < t->size - 1)
			std::cout << ", ";
	}

	std::cout << std::endl;
}