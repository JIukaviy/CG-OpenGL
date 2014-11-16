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

#endif