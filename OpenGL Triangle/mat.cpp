#include "mat.h"
#include "vec_mat_errors.h"
#include <iostream>
#include <string>

#define hnd2mat(x) ((mat_t*)(x))
#define mat_assert(x, ret) if (!x || !hnd2mat(x)->data) { push_error_info(MAT_ERR_NULL_IN_DATA, "In argument: "#x); return ret;}\
					  if (!mat_validate_size(hnd2mat(x)->rows, hnd2mat(x)->cols)) { push_error(MAT_ERR_BAD_SIZE); return ret;}
#define get_elem(mat_name, row, col) ((mat_name)->data[(row)*((mat_name)->cols)+(col)])
#define mat_create_out(cols_name, rows_name) mathnd out_hnd = mat_create((rows_name), (cols_name)); mat_t* out = hnd2mat(out_hnd)

struct mat_t {
	mat_elem_t* data;
	int rows;
	int cols;
};

typedef mat_elem_t(*on_elem_pfunc)(mat_elem_t, mat_elem_t);

int mat_unit_id;
int MAT_ERR_OK = 0;
int MAT_ERR_BAD_SIZE;
int MAT_ERR_DIFF_SIZE;
int MAT_ERR_NULL_IN_DATA;
int MAT_ERR_OUT_OF_BOUNDS;
int MAT_ERR_DIV_BY_ZERO;
int MAT_ERR_MAT_IS_SINGULAR;

void mat_init(){
	mat_unit_id = vme_register_unit("Matrix");
	MAT_ERR_BAD_SIZE = vme_register_error_type(mat_unit_id, "Invalide size of matrix");
	MAT_ERR_DIFF_SIZE = vme_register_error_type(mat_unit_id, "Different size between two matrix");
	MAT_ERR_NULL_IN_DATA = vme_register_error_type(mat_unit_id, "Null pointer in input data");
	MAT_ERR_OUT_OF_BOUNDS = vme_register_error_type(mat_unit_id, "Out of bounds of matrix coordinates");
	MAT_ERR_DIV_BY_ZERO = vme_register_error_type(mat_unit_id, "Divide by zero");
	MAT_ERR_MAT_IS_SINGULAR = vme_register_error_type(mat_unit_id, "Matrix is singular");
}

inline bool mat_validate_size(int rows, int cols){
	return !(rows < 1 || cols < 1 || rows > MAT_MAX_ROWS || cols > MAT_MAX_COLS);
}

void swap(mat_elem_t** a, mat_elem_t **b){
	if (!a || !b || !*a || !*b)
		return;

	mat_elem_t *t = *a;
	*a = *b;
	*b = t;
}

void swap_rows(mat_t* a, int row_id1, int row_id2){
	mat_elem_t *t = new mat_elem_t[a->rows];
	memcpy(t, &a->data[a->cols*row_id1], sizeof(mat_elem_t)*a->cols);
	memcpy(&a->data[a->cols*row_id1], &a->data[a->cols*row_id2], sizeof(mat_elem_t)*a->cols);
	memcpy(&a->data[a->cols*row_id2], t, sizeof(mat_elem_t)*a->cols);
}

mathnd mat_create(int rows, int cols){
	if (!mat_validate_size(rows, cols)) {
		push_error(MAT_ERR_BAD_SIZE);
		return nullptr;
	}

	mat_t* mat = new mat_t;
	memset(mat, 0, sizeof(mat_t));

	mat->data = new mat_elem_t[rows*cols];
	memset(mat->data, 0, sizeof(mat_elem_t)*rows*cols);

	mat->cols = cols;
	mat->rows = rows;

	return (mathnd)mat;
}


mathnd mat_rotate_mat2(mat_elem_t angle){
	mat_create_out(2, 2);

	mat_elem_t cos_a = cos(angle);
	mat_elem_t sin_a = sin(angle);

	mat_set_elem(out_hnd, 0, 0, cos_a);
	mat_set_elem(out_hnd, 0, 1, sin_a);
	mat_set_elem(out_hnd, 1, 0, -sin_a);
	mat_set_elem(out_hnd, 1, 1, cos_a);

	return out_hnd;
}

mathnd mat_rotate_mat3(mat_elem_t angle, mat_axis axis){
	mat_create_out(3, 3);

	mat_elem_t cos_a = cos(angle);
	mat_elem_t sin_a = sin(angle);

	switch (axis){
		case MAT_X: {
			mat_set_elem(out_hnd, 0, 0, 1);
			mat_set_elem(out_hnd, 1, 1, cos_a);
			mat_set_elem(out_hnd, 1, 2, -sin_a);
			mat_set_elem(out_hnd, 2, 1, sin_a);
			mat_set_elem(out_hnd, 2, 2, cos_a);
			break;
		} case MAT_Y: {
			mat_set_elem(out_hnd, 0, 0, cos_a);
			mat_set_elem(out_hnd, 0, 2, sin_a);
			mat_set_elem(out_hnd, 1, 1, 1);
			mat_set_elem(out_hnd, 2, 0, -sin_a);
			mat_set_elem(out_hnd, 2, 2, cos_a);
			break;
		} case MAT_Z: {
			mat_set_elem(out_hnd, 0, 0, cos_a);
			mat_set_elem(out_hnd, 0, 1, -sin_a);
			mat_set_elem(out_hnd, 1, 0, sin_a);
			mat_set_elem(out_hnd, 1, 1, cos_a);
			mat_set_elem(out_hnd, 2, 2, 1);
			break;
		}
	} 

	return out_hnd;
}

mathnd mat_rotate_mat4(mat_elem_t angle, mat_axis axis){
	mat_create_out(4, 4);
	mathnd rot_mat3hnd = mat_rotate_mat3(angle, axis);
	mat_t* rot_mat3 = hnd2mat(rot_mat3hnd);
	memcpy(out->data, rot_mat3->data, sizeof(mat_elem_t) * 3);
	memcpy(&get_elem(out, 1, 0), &get_elem(rot_mat3, 1, 0), sizeof(mat_elem_t) * 3);
	memcpy(&get_elem(out, 2, 0), &get_elem(rot_mat3, 2, 0), sizeof(mat_elem_t) * 3);
	mat_set_elem(out_hnd, 3, 3, 1);
	mat_destroy(&rot_mat3hnd);

	return out_hnd;
}

void mat_destroy(mathnd* hnd){
	if (!hnd) {
		//push_error(MAT_ERR_NULL_IN_DATA);
		return;
	}

	mat_t* t = hnd2mat(*hnd);

	delete t->data;
	delete *hnd;
	*hnd = nullptr;
}

mathnd mat_on_elem(mathnd a, mathnd b, on_elem_pfunc func){
	mat_assert(a, nullptr);
	mat_assert(b, nullptr);

	mat_t* t = hnd2mat(a);
	mat_t* g = hnd2mat(b);

	if (t->cols != g->cols || t->rows != g->rows) {
		push_error(MAT_ERR_DIFF_SIZE);
		return nullptr;
	}

	mat_create_out(t->cols, t->rows);

	vme_set_no_errors(mat_unit_id);
	for (int i = 0; i < t->rows; i++)
		for (int j = 0; j < t->cols; j++)
			get_elem(out, i, j) = func(get_elem(t, i, j), get_elem(g, i, j));

	if (vme_error_appear(mat_unit_id))
		mat_destroy(&out_hnd);

	return out_hnd;
}

mathnd mat_on_elem(mathnd a, mat_elem_t b, on_elem_pfunc func){
	mat_assert(a, nullptr);

	mat_t* t = hnd2mat(a);

	mat_create_out(t->cols, t->rows);

	vme_set_no_errors(mat_unit_id);
	for (int i = 0; i < t->rows; i++)
		for (int j = 0; j < t->cols; j++)
			get_elem(out, i, j) = func(get_elem(t, i, j), b);

	if (vme_error_appear(mat_unit_id))
		mat_destroy(&out_hnd);

	return out_hnd;
}

mat_elem_t mat_on_elem_add(mat_elem_t a, mat_elem_t b){
	return a + b;
}

mat_elem_t mat_on_elem_sub(mat_elem_t a, mat_elem_t b){
	return a - b;
}

mat_elem_t mat_on_elem_mul(mat_elem_t a, mat_elem_t b){
	return a * b;
}

mat_elem_t mat_on_elem_div(mat_elem_t a, mat_elem_t b){
	if (b == 0) {
		push_error(MAT_ERR_DIV_BY_ZERO);
		return NAN;
	}

	return a / b;
}

mathnd mat_add(mathnd a, mathnd b){
	return mat_on_elem(a, b, mat_on_elem_add);
}

mathnd mat_sub(mathnd a, mathnd b){
	return mat_on_elem(a, b, mat_on_elem_sub);
}

mathnd mat_mul(mathnd a, mat_elem_t b){
	return mat_on_elem(a, b, mat_on_elem_mul);
}

mathnd mat_muls(mathnd a, mathnd b){
	return mat_on_elem(a, b, mat_on_elem_mul);
}

mathnd mat_div(mathnd a, mat_elem_t b){
	return mat_on_elem(a, b, mat_on_elem_div);
}

mathnd mat_mul(mathnd a, mathnd b){
	mat_assert(a, nullptr);
	mat_assert(b, nullptr);

	mat_t* t = hnd2mat(a);
	mat_t* g = hnd2mat(b);

	if (t->cols != g->rows) {
		push_error(MAT_ERR_DIFF_SIZE);
		return nullptr;
	}

	mat_create_out(t->rows, g->cols);

	for (int i = 0; i < out->rows; i++)
		for (int j = 0; j < out->cols; j++)
			for (int k = 0; k < t->cols; k++)
				get_elem(out, i, j) += get_elem(t, i, k) * get_elem(g, k, j);

	return out_hnd;
}

mathnd mat_create_minor(mathnd a, int col, int row){
	mat_assert(a, nullptr);

	mat_t *t = hnd2mat(a);
	
	mat_create_out(t->cols - 1, t->rows - 1);

	int x = 0, y = 0;
	for (int i = 0; i < t->rows; i++){
		if (i == row) continue;

		for (int j = 0; j < t->cols; j++){
			if (j == col) continue;
			get_elem(out, y, x) = get_elem(t, j, i);
			x++;
		}
		y++;
	}

	return out_hnd;
}

mat_elem_t mat_det(mathnd hnd){
	mat_assert(hnd, NAN);

	mat_t *t = hnd2mat(hnd);

	if (t->cols != t->rows) {
		push_error_info(MAT_ERR_BAD_SIZE, "Calculate the determinant can only be for a square matrix");
		return NAN;
	}

	const mat_elem_t epsilon = 1E-9;

	mat_elem_t det = 1;

	for (int i = 0; i < t->cols; i++){
		int k = i;

		for (int j = i + 1; j < t->cols; j++)
			if (fabs(get_elem(t, j, i)) > fabs(get_elem(t, k, i)))
				k = j;

		if (fabs(get_elem(t, k, i)) < epsilon){
			det = 0;
			break;
		}

		swap_rows(t, i, k);

		if (i != k)
			det = -det;

		det *= get_elem(t, i, i);

		for (int j = i + 1; j < t->cols; j++)
			get_elem(t, i, j) /= get_elem(t, i, i);

		for (int j = 0; j < t->cols; j++)
			if (j != i && fabs(get_elem(t, j, i)) > epsilon)
				for (int k = i + 1; k < t->cols; k++)
					get_elem(t, j, k) -= get_elem(t, i, k) * get_elem(t, j, i);
	}

	return det;
}

mathnd mat_invert(mathnd hnd){
	mat_assert(hnd, nullptr);

	mat_t *t = hnd2mat(hnd);

	if (t->cols != t->rows) {
		push_error_info(MAT_ERR_BAD_SIZE, "Only square matrix can be inverted");
		return nullptr;
	}

	mat_create_out(t->cols, t->rows);

	vme_set_no_errors(mat_unit_id);
	mat_elem_t det = mat_det(hnd);
	if (vme_error_appear(mat_unit_id))
		return nullptr;

	if (det == 0.0) {
		push_error_info(MAT_ERR_MAT_IS_SINGULAR, "Only non singular matrix can be inverted");
		return nullptr;
	}

	for (int i = 0; i < t->cols; i++){
		for (int j = 0; j < t->rows; j++){
			mathnd g = mat_create_minor(hnd, i, j);
			mat_elem_t res = mat_det(hnd);
			get_elem(out, j, i) = (i + j) & 1 ? -res : res;
			mat_destroy(&g);
		}
	}

	mathnd transposed_matrix = mat_transpose(out_hnd);
	mathnd inverted_matrix = mat_div(transposed_matrix, det);

	mat_destroy(&out_hnd);
	mat_destroy(&transposed_matrix);

	if (vme_error_appear(mat_unit_id))
		return nullptr;

	return inverted_matrix;
}

mathnd mat_transpose(mathnd hnd){
	mat_assert(hnd, nullptr);

	mat_t* t = hnd2mat(hnd);
	mat_create_out(t->cols, t->rows);

	for (int i = 0; i < t->rows; i++)
		for (int j = 0; j < t->cols; j++)
			get_elem(out, j, i) = get_elem(t, i, j);

	return (mathnd)out;
}

mathnd mat_copy(mathnd hnd){
	mat_assert(hnd, nullptr);

	mat_t* t = hnd2mat(hnd);
	mat_create_out(t->cols, t->rows);

	memcpy(out->data, t->data, sizeof(mat_elem_t)*t->cols);

	return out_hnd;
}

void mat_set_elem(mathnd hnd, int row, int col, mat_elem_t val){
	mat_assert(hnd);

	mat_t* t = hnd2mat(hnd);

	if (row < 0 || row > t->rows - 1 || col < 0 || col > t->cols - 1) {
		push_error(MAT_ERR_OUT_OF_BOUNDS);
		return;
	}

	get_elem(t, row, col) = val;
}

void mat_set_elems(mathnd hnd, mat_elem_t* elems){
	mat_assert(hnd);

	if (!elems) {
		push_error(MAT_ERR_NULL_IN_DATA);
		return;
	}

	mat_t* t = hnd2mat(hnd);

	memcpy(t->data, elems, sizeof(mat_elem_t)*t->cols*t->rows);
}

mat_elem_t mat_get_elem(mathnd hnd, int row, int col){
	mat_assert(hnd, NAN);

	mat_t* t = hnd2mat(hnd);

	if (row < 0 || row > t->rows - 1 || col < 0 || col > t->cols - 1) {
		push_error(MAT_ERR_OUT_OF_BOUNDS);
		return MAT_ERR_OUT_OF_BOUNDS;
	}

	return get_elem(t, row, col);
}


mat_elem_t* mat_get_elems(mathnd hnd){
	mat_assert(hnd, nullptr);

	mat_t* t = hnd2mat(hnd);

	return hnd2mat(hnd)->data;
}

int mat_cols(mathnd hnd){
	mat_assert(hnd, 0);

	return hnd2mat(hnd)->cols;
}

int mat_rows(mathnd hnd){
	mat_assert(hnd, 0);

	return hnd2mat(hnd)->rows;
}

bool mat_equal(mathnd a, mathnd b){
	mat_assert(a, false);
	mat_assert(b, false);

	mat_t* t = hnd2mat(a);
	mat_t* g = hnd2mat(b);

	if (t->cols != g->cols || t->rows != g->rows) {
		push_error(MAT_ERR_DIFF_SIZE);
		return false;
	}

	return !memcmp(t->data, g->data, t->cols*t->rows);
}

void mat_print(mathnd hnd){
	mat_assert(hnd);

	mat_t* t = hnd2mat(hnd);

	for (int i = 0; i < t->rows; i++) {
		std::cout << '(';
		for (int j = 0; j < t->cols; j++) {
			std::cout << get_elem(t, i, j);
			if (j < t->cols - 1)
				std::cout << ", ";
		}
		std::cout << ')' << std::endl;
	}

	std::cout << std::endl;
}