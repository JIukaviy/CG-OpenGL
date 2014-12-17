#ifndef MESH_H
#define MESH_H

#include "objloader.h"
#include "shader_var.h"

typedef void* meshhnd;


void mesh_init();

meshhnd mesh_create(float* vertexes, int vertex_len, int* elems, int elems_len, float* normals, int normals_len);
meshhnd mesh_create(const char* file_name);
void mesh_destroy(meshhnd* mesh);
void mesh_push_data_in_buffer(meshhnd mesh);
int mesh_get_elem_buff_size(meshhnd mesh);

#endif