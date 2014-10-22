#include "gc.h"

#define obj_assert(x, ret) if (!x) {push_error_info(VM_NULL_IN_DATA, "In argument "#x); return ret;}

struct garbage_t{
	void* data;
	destruct_func g_destroy;
	int alive;
	garbage_t* prev;
};

garbage_t* curr_node = 0;

gc_id_t gc_push_garbage(void* g, destruct_func destroy){
	if (!g || !destroy)
		return 0;

	garbage_t* node = new garbage_t;

	node->data = g;
	node->g_destroy = destroy;
	node->prev = curr_node;
	node->alive = true;

	return curr_node = node;
}

void gc_on_destroy(gc_id_t id){
	garbage_t* t = ((garbage_t*)id);
	if (!id || !t->alive)
		return;

	t->alive = false;
}

void gc_clear_garbage(){
	while (curr_node) {
		garbage_t* prev_node = curr_node->prev;
		if (curr_node->alive)
			curr_node->g_destroy(&curr_node->data);
		delete curr_node;
		curr_node = prev_node;
	}
}