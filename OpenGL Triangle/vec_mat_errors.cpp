#include "vec_mat_errors.h"
#include <string>
#include <iostream>
#define ERR_STACK_SIZE 20
#define CALL_STACK_SIZE 30

using namespace std;

struct error_type_t {
	char *info;
	char *unit_name;
	vme_int err_code;
	vme_int unit_id;
};

struct error_t {
	vme_int type;
	bool not_empty;
	int line;
	char* func_name;
	char* info;
};

struct unit_t {
	error_t *errors[ERR_STACK_SIZE];
	bool error_appear;
	int errors_count;
	int last_err_pos;
	char* name;
};

struct call_stack_elem_t{
	vme_int unit_id;
	char *func_name;
};

typedef unit_t* unit_hnd;

error_t err_stack[ERR_STACK_SIZE];
error_t *curr_err = 0;
unit_t *units;
error_type_t *error_types;
int unit_count = 0;
int error_type_count = 0;

bool error_appear = false;

bool already_init = false;

void vme_init(){
	if (already_init)
		return;
	memset(err_stack, 0, sizeof(error_t)*ERR_STACK_SIZE);
}

void clear_err(error_t* error){
	unit_t* curr_unit = &units[error_types[error->type].unit_id];
	curr_unit->errors_count--;
	delete error->func_name;
	delete error->info;
	memset(error, 0, sizeof(error_t));
}

void next_err(){
	if (curr_err < &err_stack[0] + ERR_STACK_SIZE - 1 && curr_err)
		curr_err++;
	else 
		curr_err = &err_stack[0];
	clear_err(curr_err);
}

vme_int vme_register_unit(char* unit_name){
	unit_t* t_units = new unit_t[unit_count + 1];
	memcpy(t_units, units, sizeof(unit_t)*(unit_count));
	delete(units);
	units = t_units;
	memset(&units[unit_count], 0, sizeof(unit_t));
	int unit_name_len = strlen(unit_name);
	units[unit_count].name = new char[unit_name_len + 1];
	strcpy_s(units[unit_count].name, unit_name_len + 1, unit_name);
	return unit_count++;
}

vme_int vme_register_error_type(vme_int unit_id, char* error_info){
	if (unit_id < 0 || unit_id > unit_count)
		return VME_UNIT_IS_UNREGISTERED;

	unit_t* curr_unit = &units[unit_id];

	error_type_t *t_errors = new error_type_t[error_type_count + 1];
	memcpy(t_errors, error_types, sizeof(error_type_t)*error_type_count);
	delete(error_types);
	error_types = t_errors;

	error_type_t* curr_err_type = &error_types[error_type_count];
	curr_err_type->err_code = error_type_count;
	curr_err_type->unit_id = unit_id;
	curr_err_type->unit_name = curr_unit->name;
	int error_info_len = strlen(error_info);
	curr_err_type->info = new char[error_info_len + 1];
	strcpy_s(curr_err_type->info, error_info_len + 1, error_info);

	return error_type_count++;
}

vme_error vme_push_error(vme_int error_type_id, char* func_name, int line, char* error_info){
	if (error_type_id < 0 || error_type_id > error_type_count)
		return VME_ERROR_TYPE_IS_UNREGISTERED;

	next_err();

	if (func_name) {
		int func_name_len = strlen(func_name);
		curr_err->func_name = new char[func_name_len + 1];
		strcpy_s(curr_err->func_name, func_name_len + 1, func_name);
	}

	if (error_info) {
		int error_info_len = strlen(error_info);
		curr_err->info = new char[error_info_len + 1];
		strcpy_s(curr_err->info, error_info_len + 1, error_info);
	}

	curr_err->type = error_type_id;
	curr_err->line = line;
	curr_err->not_empty = true;

	unit_t *unit = &units[error_types[curr_err->type].unit_id];

	unit->errors_count++;
	unit->last_err_pos = (unit->last_err_pos + 1) % ERR_STACK_SIZE;
	unit->errors[unit->last_err_pos] = curr_err;
	unit->error_appear = error_appear = true;
}

void vme_push_call_stack(vme_int unit_id, char* func_name){

}

errhnd vme_get_last_err(){
	return curr_err;
}

errhnd vme_get_last_err(vme_int unit_id){
	if (unit_id < 0 || unit_id > unit_count)
		return nullptr;
	return units[unit_id].errors[units[unit_id].last_err_pos];
}

void vme_print_error(errhnd error){
	error_t* err = (error_t*)error;

	cout << "Error in unit: " << error_types[err->type].unit_name << ". " << "In function: " << err->func_name << ", " << "On line: " << err->line << ", ";
	cout << error_types[err->type].info << ", ";
	if (err->info)
		cout << err->info;
	cout << endl;
}

void vme_print_errors(){
	error_t *err = curr_err;
	for (int i = 0; i < ERR_STACK_SIZE; i++){
		if (!err->not_empty)
			break;
		vme_print_error(err);
		if (err == &err_stack[0])
			err = &err_stack[0] + ERR_STACK_SIZE-1;
		else
			err--;
	}
}

void vme_print_errors(vme_int unit_id){
	if (unit_id < 0 || unit_id >= unit_count) {
		cout << "Unit id: " << unit_id << ' ' << "is unregistered";
		return;
	}

	unit_t* unit = &units[unit_id];
	error_t *err = unit->errors[unit->last_err_pos];

	for (int i = 0; i < unit->errors_count; i++){
		err = unit->errors[(unit->last_err_pos - i) % unit->errors_count];
		vme_print_error(err);
	}
}

bool vme_error_appear(){
	return error_appear;
}

bool vme_error_appear(vme_int unit_id){
	if (unit_id < 0 || unit_id >= unit_count)
		return true;
	return units[unit_id].error_appear;
}

void vme_set_no_errors(){
	error_appear = false;
}

void vme_set_no_errors(vme_int unit_id){
	if (unit_id < 0 || unit_id >= unit_count)
		return;
	units[unit_id].error_appear = false;
}

void vme_set_error_info(errhnd hnd, char* info){
	int info_len = strlen(info);
	char *t_info = new char[info_len + 1];
	strcpy_s(t_info, info_len + 1, info);
	((error_t*)(hnd))->info = t_info;
}
