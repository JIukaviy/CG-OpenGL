#include "object.h"
#include "vec_mat_errors.h"
#include "gc.h"

struct object_t {
	meshhnd mesh;
	mtlhnd material;
	mathnd model;
	vechnd scale;
	vechnd pos;
	vechnd rotate;
};

vme_int obj_unit_id;
vme_int OBJ_ERR_NULL_IN_DATA;

#define hnd2obj(x) ((object_t*)(x))
#define new_hnd2obj(x, name) object_t* name = hnd2obj(x)
#define obj_assert(x, ret) if (!x) {push_error_info(OBJ_ERR_NULL_IN_DATA, "In argument "#x); return ret;}

void obj_init() {
	obj_unit_id = vme_register_unit("Object");
	OBJ_ERR_NULL_IN_DATA = vme_register_error_type(obj_unit_id, "Null pointer in input data");
}

objhnd obj_create() {
	object_t* new_obj = new object_t;
	memset(new_obj, 0, sizeof(object_t));

	gc_pause_collect();

	new_obj->model = mat_create_e(3);
	new_obj->scale = vec_create3(1);
	new_obj->rotate = vec_create(3);
	new_obj->pos = vec_create(3);

	gc_resume_collect();

	return new_obj;
}

void obj_destroy(objhnd* obj) {
	if (!obj || *obj)
		return;

	new_hnd2obj(*obj, t);

	mesh_destroy(&t->mesh);
}

void obj_set_mesh(objhnd obj, meshhnd mesh) {
	obj_assert(obj);
	obj_assert(mesh);
	
	hnd2obj(obj)->mesh = mesh;
}

meshhnd obj_get_mesh(objhnd obj) {
	obj_assert(obj, nullptr);
	return hnd2obj(obj)->mesh;
}

void obj_set_material(objhnd obj, mtlhnd mtl) {
	obj_assert(obj);
	obj_assert(mtl);

	hnd2obj(obj)->material = mtl;
}

mtlhnd obj_get_material(objhnd obj) {
	obj_assert(obj, nullptr);
	return hnd2obj(obj)->material;
}

void obj_set_position(objhnd obj, vechnd pos) {
	obj_assert(obj);
	obj_assert(pos);
	new_hnd2obj(obj, t);

	start_garbage_collect(1);
		mat_copy(t->model, mat_mul(vm_mat_translate(vec_sub(pos, t->pos)), t->model));
	erase_collected_garbage(1);

	vec_copy(t->pos, pos);
}

void obj_set_rotation(objhnd obj, vechnd rotation) {
	obj_assert(obj);
	obj_assert(rotation);
	new_hnd2obj(obj, t);

	start_garbage_collect(1);
		mat_copy(t->model, mat_mul(vm_mat_rotate_mat4(vec_sub(rotation, t->rotate)), t->rotate));
	erase_collected_garbage(1);

	vec_copy(t->rotate, rotation);
}

void obj_set_scale(objhnd obj, vechnd scale) {
	obj_assert(obj);
	obj_assert(scale);
	new_hnd2obj(obj, t);

	start_garbage_collect(1);
		mat_copy(t->model, mat_mul(vm_mat_scale(vec_sub(scale, t->scale)), t->scale));
	erase_collected_garbage(1);

	vec_copy(t->scale, scale);
}