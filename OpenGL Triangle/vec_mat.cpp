#include "vec_mat.h"
#include "vec_mat_errors.h"

vme_int unit_id;
vme_int VM_NULL_IN_DATA;
vme_int VM_DIFF_SIZE;
vme_int VM_BAD_SIZE;

#define obj_assert(x, ret) if (!x) {push_error_info(VM_NULL_IN_DATA, "In argument "#x); return ret;}

void vm_init(){
	unit_id = vme_register_unit("Vertex and Matrices");
	VM_NULL_IN_DATA = vme_register_error_type(unit_id, "Null pointer in input data");
	VM_DIFF_SIZE = vme_register_error_type(unit_id, "Different size between two objects");
	VM_BAD_SIZE = vme_register_error_type(unit_id, "Bad size of object");
	vec_init();
	mat_init();
}

vechnd vm_mat2vec(mathnd mat){
	obj_assert(mat, nullptr);

	if (mat_rows(mat) != 1) {
		push_error_info(VM_BAD_SIZE, "Amount of rows in input matrix must be equal 1");
		return nullptr;
	}

	vechnd out_vec = vec_create(mat_cols(mat));

	mat_elem_t *elems = mat_get_elems(mat);

	vec_set_elems(out_vec, elems);

	return out_vec;
}

mathnd vm_vec2mat(vechnd vec){
	obj_assert(vec, nullptr);

	mathnd out_mat = mat_create(1, vec_size(vec));
	vec_elem_t *elems = vec_get_elems(vec);
	mat_set_elems(out_mat, elems);

	return out_mat;
}

vechnd vm_mat_vec_mul(mathnd mat, vechnd vec){
	obj_assert(mat, nullptr);
	obj_assert(vec, nullptr);

	if (mat_cols(mat) != vec_size(vec)) {
		push_error(VM_DIFF_SIZE);
		return nullptr;
	}

	mathnd temp_mat = vm_vec2mat(vec);
	mathnd res_mat = mat_mul(mat, temp_mat);
	vechnd out_vec = vm_mat2vec(res_mat);

	mat_destroy(&temp_mat);
	mat_destroy(&res_mat);

	return out_vec;
}

mathnd vm_mat_translate(vechnd vec){
	obj_assert(vec, nullptr);

	if (vec_size(vec) != 3)
		push_error_info(VM_BAD_SIZE, "Vectors for translate matrix must be size 3");
	
	return mat_translate(vec_get_elem(vec, 0), vec_get_elem(vec, 1), vec_get_elem(vec, 2));
}

mathnd vm_mat_look_at(vechnd cam_pos, vechnd cam_target, vechnd cam_up){
	obj_assert(cam_pos, nullptr);
	obj_assert(cam_target, nullptr);
	obj_assert(cam_up, nullptr);

	vechnd vecs[3];

	vechnd vec_t_x = vec_sub(cam_target, cam_pos);
	vechnd vec_x = vec_normalize(vec_t_x);
	vec_destroy(&vec_t_x);

	vechnd vec_t_z = vec_cross(vec_x, cam_up);
	vechnd vec_z = vec_normalize(vec_t_z);
	vec_destroy(&vec_t_z);

	vechnd vec_t_y = vec_cross(vec_x, vec_z);
	vechnd vec_y = vec_normalize(vec_t_y);
	vec_destroy(&vec_t_y);

	mathnd out_mat = mat_create(4, 4);

	vecs[0] = vec_x;
	vecs[1] = vec_y;
	vecs[2] = vec_z;

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			mat_set_elem(out_mat, i, j, vec_get_elem(vecs[i], j));

	for (int i = 0; i < 3; i++)
		mat_set_elem(out_mat, 3, i, vec_get_elem(cam_pos, i));

	mat_set_elem(out_mat, 3, 3, 1);

	for (int i = 0; i < 3; i++)
		vec_destroy(&vecs[i]);

	return out_mat;
}