#ifndef MAT_H
#define MAT_H

#define MAT_MAX_ROWS 10
#define MAT_MAX_COLS 10

enum mat_axis{
	MAT_X,
	MAT_Y,
	MAT_Z
};

typedef float mat_elem_t;

typedef void* mathnd;

void mat_init();
mathnd mat_create(int rows, int cols);
mathnd mat_create_e(int n);
mathnd mat_rotate_mat2(mat_elem_t angle);
mathnd mat_rotate_mat3(mat_elem_t angle, mat_axis axis);
mathnd mat_rotate_mat3(mat_elem_t angle_x, mat_elem_t angle_y, mat_elem_t angle_z);
mathnd mat_rotate_mat4(mat_elem_t angle, mat_axis axis);
mathnd mat_rotate_mat4(mat_elem_t angle_x, mat_elem_t angle_y, mat_elem_t angle_z);
mathnd mat_translate(mat_elem_t x, mat_elem_t y, mat_elem_t z);
mathnd mat_scale(mat_elem_t s);
mathnd mat_scale(mat_elem_t x, mat_elem_t y, mat_elem_t z);
mathnd mat_orthographic_projection(mat_elem_t left, mat_elem_t right, mat_elem_t bottom, mat_elem_t top, mat_elem_t near_val, mat_elem_t far_val);
mathnd mat_perspective_projection(mat_elem_t fov, mat_elem_t ratio, mat_elem_t z_near, mat_elem_t z_far);
void mat_destroy(mathnd*);  //
void mat_gc_unregist(mathnd hnd);
mathnd mat_add(mathnd a, mathnd b);
mathnd mat_sub(mathnd a, mathnd b);
mathnd mat_mul(mathnd a, mathnd b);
mathnd mat_muls(mathnd a, mathnd b);
mathnd mat_mul(mathnd a, mat_elem_t);
mathnd mat_create_minor(mathnd a, int col, int row);
//mat_err_code mat_mulv();
mat_elem_t mat_det(mathnd hnd);  //Определитель
mathnd mat_invert(mathnd hnd);    //Инвертирование 
mathnd mat_transpose(mathnd hnd); //Транспонирование
int mat_cols(mathnd hnd);
int mat_rows(mathnd hnd);
void mat_set_elem(mathnd hnd, int row, int col, mat_elem_t);
void mat_set_elems(mathnd hnd, mat_elem_t* elems);
mat_elem_t mat_get_elem(mathnd hnd, int row, int col);
mat_elem_t* mat_get_elems(mathnd hnd);
bool mat_equal(mathnd a, mathnd b);
mathnd mat_copy(mathnd hnd);
void mat_copy(mathnd dst, mathnd src);
void mat_print(mathnd hnd);
#endif