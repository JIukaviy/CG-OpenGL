enum vme_error{
	VME_OK,
	VME_UNIT_IS_UNREGISTERED,
	VME_ERROR_TYPE_IS_UNREGISTERED,
	VME_NULL_IN_DATA,
	VME_NOT_ENOUGH_MEMORY
};

#define push_error(error_type_id) vme_push_error(error_type_id, __FUNCTION__, __LINE__, 0)
#define push_error_info(error_type_id, error_info) vme_push_error(error_type_id, __FUNCTION__, __LINE__, error_info)

typedef void* errhnd;
typedef int vme_int;

void vme_init();
vme_int vme_register_unit(char* unit_name);
vme_int vme_register_error_type(vme_int unit_id, char* error_info);
vme_error vme_push_error(vme_int error_type_id, char* func_name, int line, char* error_info);
errhnd vme_get_last_err();
errhnd vme_get_last_err(vme_int unit_id);
void vme_print_errors();
void vme_print_errors(vme_int unit_id);
bool vme_error_appear();
bool vme_error_appear(vme_int unit_id);
void vme_set_no_errors();
void vme_set_no_errors(vme_int unit_id);

void vme_set_error_info(errhnd hnd, char* info);