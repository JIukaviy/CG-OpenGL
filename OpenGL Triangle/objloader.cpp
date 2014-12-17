#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "vec.h"
#include "gc.h"
#include "objloader.h"
#include "vec_mat_errors.h"
#include <set>

using namespace std;

#define obj_assert(x) if (!x) {push_error_info(OBJLOADER_ERR_NULL_IN_DATA, "In argument "#x); return;}

vme_int objloader_unit_id;
vme_int OBJLOADER_ERR_NULL_IN_DATA;
vme_int OBJLOADER_ERR_CANT_OPEN_FILE;
vme_int OBJLOADER_ERR_INVALIDATE_FILE;

struct vec3 {
	GLfloat x;
	GLfloat y;
	GLfloat z;
};

struct vec2 {
	GLfloat x;
	GLfloat y;
};

struct vertex {
	int v;
	int vn;
	int vt;
	vechnd vtan;
};

void objlodaer_init() {
	objloader_unit_id = vme_register_unit("OBJ_LOADER");
	OBJLOADER_ERR_NULL_IN_DATA = vme_register_error_type(objloader_unit_id, "Null in input data");
	OBJLOADER_ERR_CANT_OPEN_FILE = vme_register_error_type(objloader_unit_id, "Can't open file");
	OBJLOADER_ERR_INVALIDATE_FILE = vme_register_error_type(objloader_unit_id, "Invalidate file");
}

void objlodaer_load_file(const char* file_name,
						 GLfloat** a_vertices, int* vert_size,
						 GLfloat** a_normals, int* normals_size,
						 GLushort** a_elements, int* elements_size) {
	obj_assert(file_name);
	obj_assert(a_vertices);
	obj_assert(vert_size);
	obj_assert(a_normals);
	obj_assert(normals_size);
	obj_assert(a_elements);
	obj_assert(elements_size);

	ifstream fin(file_name);
	if (!fin) {
		push_error_info(OBJLOADER_ERR_CANT_OPEN_FILE, file_name);
		return;
	}

	struct vec3 {
		GLfloat x;
		GLfloat y;
		GLfloat z;
	};

	struct polygon {
		int v;
		int vn;
	};

	vector<vec3> vertices;
	vector<vec3> normals;
	vector<vec3> tex;
	vector<GLushort> elements;

	string line;
	while (getline(fin, line)) {
		if (line.substr(0, 2) == "v ") {
			istringstream s(line.substr(2));
			vec3 v; 
			s >> v.x >> v.y >> v.z;
			vertices.push_back(v);
		} else if (line.substr(0, 3) == "vt ") {
			istringstream s(line.substr(3));
			vec3 v;
			s >> v.x >> v.y >> v.z;
			normals.push_back(v);
		} /*else if (line.substr(0, 2) == "f ") {
			istringstream s(line.substr(2));
			GLushort v1, v2, v3, vn1, vn2, vn3;
			s >> v1; s.ignore(2);
			s >> vn1;
			s >> v2; s.ignore(2);
			s >> vn2;
			s >> v3; s.ignore(2);
			s >> vn3;
			v1--; vn1--; v2--; vn2--; v3--; vn3--;
			elements.push_back(v1); elements.push_back(v2); elements.push_back(v3);
			}*/ 
		else if (line.substr(0, 2) == "f ") {
			istringstream s(line.substr(2));
			GLushort v1, v2, v3;
			s >> v1 >> v2 >> v3;
			v1--; v2--; v3--;
			elements.push_back(v1); elements.push_back(v2); elements.push_back(v3);
		}
		else if (line[0] == '#') { /* ignoring this line */ } else { /* ignoring this line */ }
	}

#define vec2vechnd(vec) vec_create3((vec_elem_t*)&(vec))
#define vechnd2vec3(hnd) *((vec3*)vec_get_elems(hnd))

	vector<GLfloat> nb_seen;
	normals.resize(vertices.size());
	nb_seen.resize(vertices.size(), 0);
	for (int i = 0; i < elements.size(); i += 3) {
		GLushort ia = elements[i];
		GLushort ib = elements[i + 1];
		GLushort ic = elements[i + 2];
		start_garbage_collect(1);
		vechnd normal = vec_normalize(vec_cross(vec_sub(vec2vechnd(vertices[ib]), vec2vechnd(vertices[ia])),
												vec_sub(vec2vechnd(vertices[ic]), vec2vechnd(vertices[ia]))));
		normals[ia] = normals[ib] = normals[ic] = *((vec3*)vec_get_elems(normal));

		int v[3];  v[0] = ia;  v[1] = ib;  v[2] = ic;
		for (int j = 0; j < 3; j++) {
			GLushort cur_v = v[j];
			nb_seen[cur_v]++;
			if (nb_seen[cur_v] == 1) {
				normals[cur_v] = vechnd2vec3(normal);
			} else {
				// average
				vechnd vec = vec_create3((vec_elem_t*)&(normals[cur_v]));
				vec = vec_mul(vec, (1.0 - 1.0 / nb_seen[cur_v]));
				vec = vec_add(vec, vec_div(normal, nb_seen[cur_v]));
				normals[cur_v] = vechnd2vec3(vec_normalize(vec));
			}
		}

		erase_collected_garbage(1);
	}

	*vert_size = vertices.size() * 3;
	*normals_size = normals.size() * 3;
	*elements_size = elements.size();

	GLfloat* t_vertices;
	GLfloat* t_normals;
	GLushort* t_elements;
	int x = 0;
	t_vertices = new GLfloat[vertices.size() * 3];
	for (int i = 0; i < vertices.size(); i++) {
		t_vertices[x++] = vertices[i].x;
		t_vertices[x++] = vertices[i].y;
		t_vertices[x++] = vertices[i].z;
	}
	x = 0;
	t_normals = new GLfloat[normals.size() * 3];
	for (int i = 0; i < normals.size(); i++) {
		t_normals[x++] = normals[i].x;
		t_normals[x++] = normals[i].y;
		t_normals[x++] = normals[i].z;
	}

	t_elements = new GLushort[elements.size()];
	for (int i = 0; i < elements.size(); i++)
		t_elements[i] = elements[i];

	*a_vertices = t_vertices;
	*a_normals = t_normals;
	*a_elements = t_elements;
}


void objlodaer_load_file(const char* file_name,
						 GLfloat** a_vertices, 
						 GLfloat** a_normals,
						 GLushort** a_elements,
						 GLfloat** a_texture, 
						 GLfloat** a_tangent,
						 int* vert_size,
						 int* tex_size,
						 int* elems_size
						 ) {
	obj_assert(file_name);

	ifstream fin(file_name);
	if (!fin) {
		push_error_info(OBJLOADER_ERR_CANT_OPEN_FILE, file_name);
		return;
	}

	struct face {
		int v;
		int vn;
		int vt;
	};

	vector<vec3> vertices;
	vector<vec3> normals;
	vector<vec2> texcoords;
	vector<int> indices;
	vector<vertex> ivertices;

	string line;
	int indpos = 0;
	start_garbage_collect(1);
	while (getline(fin, line)) {
		if (line.substr(0, 2) == "v ") {
			istringstream s(line.substr(2));
			vec3 v;
			s >> v.x >> v.y >> v.z;
			vertices.push_back(v);
		} else if (line.substr(0, 3) == "vn ") {
			istringstream s(line.substr(3));
			vec3 v;
			s >> v.x >> v.y >> v.z;
			normals.push_back(v);
		} else if (line.substr(0, 3) == "vt ") {
			istringstream s(line.substr(3));
			vec2 v;
			s >> v.x >> v.y;
			texcoords.push_back(v);
		} else if (line.substr(0, 2) == "f ") {
			istringstream s(line.substr(2));
			for (int i = 0; i < 3; i++) {
				vertex vert;	char c; 
				s >> vert.v >> c >> vert.vt >> c >> vert.vn;
				vert.v--; vert.vt--; vert.vn--;
				vert.vtan = vec_create(3);

				int pos = -1;
				for (int i = 0; i < ivertices.size(); i++)
					if (memcmp(&ivertices[i], &vert, sizeof(vertex)) == 0) {
						pos = i;
						break;
					}
				
				if (pos < 0) {
					ivertices.push_back(vert); 
					pos = indpos;
				}

				indices.push_back(pos);
				indpos++;
			}
		} else if (line[0] == '#') { /* ignoring this line */ } else { /* ignoring this line */ }
	}

	int pos = 0;

	*vert_size = ivertices.size() * 3;
	*tex_size = ivertices.size() * 2;
	*elems_size = indices.size();

	GLfloat* t_vertices;
	GLfloat* t_normals;
	GLfloat* t_texcoords;
	GLfloat* t_tangents;
	GLushort* t_elements;
	int x = 0;

	for (int i = 0; i < ivertices.size(); i += 3) {
		start_garbage_collect(1);
		vertex vtx0 = ivertices[i];
		vertex vtx1 = ivertices[i + 1];
		vertex vtx2 = ivertices[i + 2];

		vechnd v0 = vec2vechnd(vertices[vtx0.v]);
		vechnd v1 = vec2vechnd(vertices[vtx1.v]);
		vechnd v2 = vec2vechnd(vertices[vtx2.v]);

		float DeltaU1 = texcoords[vtx1.vt].x - texcoords[vtx0.vt].x;
		float DeltaV1 = texcoords[vtx1.vt].y - texcoords[vtx0.vt].y;

		float DeltaU2 = texcoords[vtx2.vt].x - texcoords[vtx0.vt].x;
		float DeltaV2 = texcoords[vtx2.vt].y - texcoords[vtx0.vt].y;

		float f = 1.0 / (DeltaU1 * DeltaV2 - DeltaU2 * DeltaV1);

		vechnd Edge1 = vec_mul(vec_sub(v1, v0), DeltaV2);
		vechnd Edge2 = vec_mul(vec_sub(v2, v0), DeltaV1);

		vechnd tangent = vec_mul(vec_sub(Edge1, Edge2), f);

		vec_copy(ivertices[i].vtan, vec_add(ivertices[i].vtan, tangent));
		vec_copy(ivertices[i + 1].vtan, vec_add(ivertices[i + 1].vtan, tangent));
		vec_copy(ivertices[i + 2].vtan, vec_add(ivertices[i + 2].vtan, tangent));

		erase_collected_garbage(1);
	}

	t_vertices = new GLfloat[*vert_size];
	for (int i = 0; i < ivertices.size(); i++) 
		memcpy(&t_vertices[i*3], &vertices[ivertices[i].v], sizeof(float) * 3);

	t_normals = new GLfloat[*vert_size];
	for (int i = 0; i < ivertices.size(); i++)
		memcpy(&t_normals[i*3], &normals[ivertices[i].vn], sizeof(float) * 3);

	t_texcoords = new GLfloat[*tex_size];
	for (int i = 0; i < ivertices.size(); i++)
		memcpy(&t_texcoords[i*2], &texcoords[ivertices[i].vt], sizeof(float) * 2);

	t_tangents = new GLfloat[*vert_size];
	for (int i = 0; i < ivertices.size(); i++) {
		start_garbage_collect(1);
		memcpy(&t_tangents[i*3], vec_get_elems(vec_normalize(ivertices[i].vtan)), sizeof(float) * 3);
		erase_collected_garbage(1);
	}

	t_elements = new GLushort[indices.size()];
	for (int i = 0; i < indices.size(); i++)
		t_elements[i] = indices[i];

	*a_vertices = t_vertices;
	*a_normals = t_normals;
	*a_elements = t_elements;
	*a_texture = t_texcoords;
	*a_tangent = t_tangents;

	erase_collected_garbage(1);
}

#undef vec2vechnd
#undef vechnd2vec3