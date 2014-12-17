#ifndef OBJECT_H
#define OBJECT_H

#include "vec.h"
#include "mat.h"
#include "mesh.h"
#include "material.h"
#include "vec_mat.h"

typedef void* objhnd;

void obj_init();
objhnd obj_create();
void obj_destroy(objhnd* obj);
void obj_set_material(objhnd obj, mtlhnd mtl);
void obj_set_mesh(objhnd obj, meshhnd mesh);
void obj_set_position(objhnd obj, vechnd pos);
void obj_set_rotation(objhnd obj, vechnd rotation);
void obj_set_scale(objhnd obj, vechnd scale);
meshhnd obj_get_mesh(objhnd obj);
mtlhnd obj_get_material(objhnd obj);

#endif