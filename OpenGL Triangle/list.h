typedef void(*destruct_func)(void**);

typedef void* nodehnd;


nodehnd list_add_node_next(nodehnd node, void* data, destruct_func destroy);
nodehnd list_add_node_prev(nodehnd node, void* data, destruct_func destroy);
nodehnd list_get_next_node(nodehnd node);
nodehnd list_get_prev_node(nodehnd node);
nodehnd list_node_get_data(nodehnd node);
nodehnd* list_node_get_data_ptr(nodehnd node);
void list_node_set_ptr_to_this_from_prev(nodehnd hnd, nodehnd* ptr);
void list_node_set_ptr_to_this_from_next(nodehnd hnd, nodehnd* ptr);
void list_node_set_data(nodehnd node, void* data);
void list_del_node(nodehnd node);
destruct_func list_node_get_destroy(nodehnd node);
void list_node_set_destroy(nodehnd node, destruct_func destroy);