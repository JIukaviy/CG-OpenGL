#include "transform.h"
#include "gc.h"
#include "vec_mat_errors.h"
#include <string>

vme_int tsm_unit_id;
vme_int TSM_ERR_NULL_IN_DATA;

void tsm_init() {
	tsm_unit_id = vme_register_unit("Transform");
	TSM_ERR_NULL_IN_DATA = vme_register_error_type(tsm_unit_id, "Null pointer in input data");
}

struct tsm_t {
	vechnd pos;
	vechnd rot;
	vechnd scale;
	mathnd model;
};

#define hnd2obj(x) ((tsm_t*)(x));
#define new_hnd2obj(x, name) tsm_t* name = hnd2obj(x);
#define obj_assert(x, ret) if (!x) {push_error_info(TSM_ERR_NULL_IN_DATA, "In argument "#x); return ret;}

tsmhnd tsm_create() {
	tsm_t* new_tsm = new tsm_t;
	gc_pause_collect();

	new_tsm->pos = vec_create(3);
	new_tsm->rot = vec_create(3);
	new_tsm->scale = vec_create(3);
	new_tsm->model = mat_create_e(4);

	gc_resume_collect();
	return new_tsm;
}

void tsm_destroy(tsmhnd* tsm) {
	obj_assert(tsm);
	obj_assert(*tsm);
	new_hnd2obj(*tsm, t);

	vec_destroy(&t->pos);
	vec_destroy(&t->rot);
	vec_destroy(&t->scale);
	mat_destroy(&t->model);
}

void tsm_set_pos(tsmhnd tsm, vechnd pos) {
	obj_assert(tsm);
	new_hnd2obj(tsm, t);

	start_garbage_collect(1);

	vechnd dvec = vec_sub(pos, t->pos);
	mat_copy(t->model, mat_mul(vm_mat_translate(dvec), t->model));

	erase_collected_garbage(1);
}

void tsm_set_pos(tsmhnd tsm, float x, float y, float z) {
	start_garbage_collect(1);
	tsm_set_pos(tsm, vec_create3(x, y, z));
	erase_collected_garbage(1);
}

void tsm_set_rotate(tsmhnd tsm, vechnd rotate) {

}