#include "list.h"
#include "vec_mat_errors.h"

#define obj_assert(x, ret) if (!x) {push_error_info(LIST_ERR_NULL_IN_DATA, "In argument "#x); return ret;}
#define hnd2node(hnd) ((node_t*)hnd)
#define connect_nodes(node, next, prev) {node_t* t = hnd2node(node);\
								if (t->next)\
								t->next->ptr_to_this_from_##prev = (void**)&newnode->next;\
								if (t->ptr_to_this_from_##next)\
								*t->ptr_to_this_from_##next = newnode;\
								t->next = newnode;\
								t->ptr_to_this_from_##next = (void**)&newnode->prev;\
								newnode->prev = t;\
								newnode->ptr_to_this_from_##prev = (void**)&t->next;\
								newnode->ptr_to_this_from_##next = t->ptr_to_this_from_##next;}\

struct node_t{
	void* data;
	destroy_func destroy;
	node_t* next;
	node_t* prev;
	void** ptr_to_this_from_prev;
	void** ptr_to_this_from_next;
};

vme_int list_unit_id;

vme_int LIST_ERR_NULL_IN_DATA;
vme_int LIST_ERR_UNLINKED;

void list_init(){
	list_unit_id = vme_register_unit("List");
	LIST_ERR_NULL_IN_DATA = vme_register_error_type(list_unit_id, "Null pointer in input data");
}

node_t* create_node(void* data, destroy_func destroy){
	node_t* newnode = new node_t;
	newnode->data = data;
	newnode->destroy = destroy;
	newnode->next = newnode->prev = nullptr;
	newnode->ptr_to_this_from_next = newnode->ptr_to_this_from_prev = nullptr;
	return newnode;
}

nodehnd list_add_node_next(nodehnd node, void* data, destroy_func destroy){
	node_t* newnode = create_node(data, destroy);
	
	if (node) {
		node_t* t = hnd2node(node);

		if (t->next)
			t->next->ptr_to_this_from_prev = (void**)&newnode->next;

		if (t->ptr_to_this_from_next)
			*t->ptr_to_this_from_next = newnode;

		t->next = newnode;
		newnode->ptr_to_this_from_next = t->ptr_to_this_from_next;
		t->ptr_to_this_from_next = (void**)&newnode->prev;
		newnode->prev = t;
		newnode->ptr_to_this_from_prev = (void**)&t->next;
	}
	
	return newnode;
}

nodehnd list_add_node_prev(nodehnd node, void* data, destroy_func destroy){
	node_t* newnode = create_node(data, destroy);

	if (node) {
		node_t* t = hnd2node(node);

		if (t->prev)
			t->prev->ptr_to_this_from_next = (void**)&newnode->prev;

		if (t->ptr_to_this_from_prev)
			*t->ptr_to_this_from_prev = newnode;

		t->prev = newnode;
		t->ptr_to_this_from_prev = (void**)&newnode->next;
		newnode->next = t;
		newnode->ptr_to_this_from_next = (void**)&t->prev;
		newnode->ptr_to_this_from_prev = t->ptr_to_this_from_prev;
	}

	return newnode;
}

void list_del_node(nodehnd node){
	obj_assert(node);
	node_t* t = hnd2node(node);
	node_t* prev_node = t->prev;
	node_t* next_node = t->next;

	if (t->ptr_to_this_from_prev)
		*t->ptr_to_this_from_prev = next_node;

	if (next_node)
		next_node->ptr_to_this_from_prev = t->ptr_to_this_from_prev;

	if (t->ptr_to_this_from_next)
		*t->ptr_to_this_from_next = prev_node;

	if (prev_node)
		prev_node->ptr_to_this_from_next = t->ptr_to_this_from_next;

	if (t->destroy)
		t->destroy(&t->data);

	delete t;
}

void list_node_set_ptr_to_this_from_prev(nodehnd hnd, nodehnd* ptr){
	obj_assert(hnd);
	obj_assert(ptr);
	hnd2node(hnd)->ptr_to_this_from_prev = ptr;
}

void list_node_set_ptr_to_this_from_next(nodehnd hnd, nodehnd* ptr){
	obj_assert(hnd);
	obj_assert(ptr);
	hnd2node(hnd)->ptr_to_this_from_next = ptr;
}

nodehnd list_get_next_node(nodehnd node){
	obj_assert(node, nullptr);
	return hnd2node(node)->next;
}

nodehnd list_get_prev_node(nodehnd node){
	obj_assert(node, nullptr);
	return hnd2node(node)->prev;
}

nodehnd list_node_get_data(nodehnd node){
	obj_assert(node, nullptr);
	return hnd2node(node)->data;
}

nodehnd* list_node_get_data_ptr(nodehnd node){
	obj_assert(node, nullptr);
	return &hnd2node(node)->data;
}

void list_node_set_data(nodehnd node, void* data){
	obj_assert(node);
	hnd2node(node)->data = data;
}

destroy_func list_node_get_destroy(nodehnd node){
	obj_assert(node, nullptr);
	return hnd2node(node)->destroy;
}

void list_node_set_destroy(nodehnd node, destroy_func destroy){
	obj_assert(node);
	hnd2node(node)->destroy = destroy;
}