#include "file.h"
#include "vec_mat_errors.h"
#include <fstream>

using namespace std;

vme_int file_unit_id;
vme_int FILE_ERR_CANT_OPEN_FILE;

void file_init() {
	file_unit_id = vme_register_unit("File");
	FILE_ERR_CANT_OPEN_FILE = vme_register_error_type(file_unit_id, "Can't open file");
}

char* file_read(const char* file_name) {
	ifstream fin(file_name);
	if (!fin) {
		push_error_info(FILE_ERR_CANT_OPEN_FILE, file_name);
		return nullptr;
	}
	fin.seekg(0, fin.end);
	int file_size = fin.tellg();
	fin.seekg(0, fin.beg);

	char *s_source = new char[file_size + 1];
	memset(s_source, 0, file_size);
	fin.read(s_source, file_size);

	return s_source;
}
