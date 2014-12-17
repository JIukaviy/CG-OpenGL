#include <GL/glew.h>
#include <GL/freeglut.h>
void objlodaer_init();

void objlodaer_load_file(const char* file_name,
						 GLfloat** a_vertices, int* vert_size,
						 GLfloat** a_normals, int* normals_size,
						 GLushort** a_elements, int* elements_size);

void objlodaer_load_file(const char* file_name,
						 GLfloat** a_vertices,
						 GLfloat** a_normals,
						 GLushort** a_elements,
						 GLfloat** a_texture,
						 GLfloat** a_tangent,
						 int* vert_size,
						 int* tex_size,
						 int* elems_size);