#include "vec_mat.h"
#include "vec_mat_errors.h"

vme_int unit_id;
vme_int VM_NULL_IN_DATA;
vme_int VM_DIFF_SIZE;
vme_int VM_BAD_SIZE;

void vm_init(){
	unit_id = vme_register_unit("Vertex and Matrices");
	VM_NULL_IN_DATA = vme_register_error_type(unit_id, "Null pointer in input data");
	VM_DIFF_SIZE = vme_register_error_type(unit_id, "Different size between two objects");
	VM_BAD_SIZE = vme_register_error_type(unit_id, "Bad size of object");
	vec_init();
	mat_init();
}

vechnd vm_mat2vec(mathnd mat){
	if (!mat) {
		push_error(VM_NULL_IN_DATA);
		return nullptr;
	}

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
	if (!vec) {
		push_error(VM_NULL_IN_DATA);
		return nullptr;
	}

	mathnd out_mat = mat_create(1, vec_size(vec));
	vec_elem_t *elems = vec_get_elems(vec);
	mat_set_elems(out_mat, elems);

	return out_mat;
}

vechnd vm_mat_vec_mul(mathnd mat, vechnd vec){
	if (!mat || !vec) {
		push_error(VM_NULL_IN_DATA);
		return nullptr;
	}

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