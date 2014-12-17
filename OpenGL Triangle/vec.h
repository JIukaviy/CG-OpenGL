#ifndef VEC_H
#define VEC_H

#define VEC_MAX_SIZE 5

typedef float vec_elem_t;

typedef void* vechnd;

void vec_init();
vechnd vec_create(int size);
vechnd vec_create3(vec_elem_t x, vec_elem_t y, vec_elem_t z);
vechnd vec_create3(vec_elem_t coord);
vechnd vec_create3(vec_elem_t* elems);
vechnd vec_create4(vec_elem_t x, vec_elem_t y, vec_elem_t z, vec_elem_t w);
void vec_destroy(vechnd*);
void vec_gc_unregist(vechnd hnd);
vechnd vec_convert(vechnd hnd, int size);
vechnd vec_add(vechnd a, vechnd b);
vechnd vec_sub(vechnd a, vechnd b);
vechnd vec_mul(vechnd a, vechnd b);
vechnd vec_cross(vechnd a, vechnd b);
vechnd vec_div(vechnd a, vechnd b);
vechnd vec_add(vechnd a, vec_elem_t b);
vechnd vec_sub(vechnd a, vec_elem_t b);
vechnd vec_mul(vechnd a, vec_elem_t b);
vechnd vec_div(vechnd a, vec_elem_t b);
vechnd vec_inverse(vechnd hnd);
vec_elem_t vec_length(vechnd);
int vec_size(vechnd);
vec_elem_t vec_dot(vechnd a, vechnd b);    //скалярное произведение
vechnd vec_normalize(vechnd in_hnd);
void vec_set_elem(vechnd in_hnd, int id, vec_elem_t);
void vec_set_elems(vechnd in_hnd, vec_elem_t*);
vec_elem_t vec_get_elem(vechnd in_hnd, int id);
vec_elem_t* vec_get_elems(vechnd in_hnd);
vechnd vec_copy(vechnd in_hnd);
void vec_copy(vechnd dst, vechnd src);
bool vec_equal(vechnd a, vechnd b);
void vec_print(vechnd);
#endif