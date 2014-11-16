#ifndef MESH_H
#define MESH_H

#include "objloader.h"
#include "shader_var.h"

typedef void* meshhnd;

meshhnd mesh_create(float* vertexes, int vertex_len, int* elems, int elems_len, float* normals, int normals_len);
void mesh_destroy(meshhnd* mesh);

#endif