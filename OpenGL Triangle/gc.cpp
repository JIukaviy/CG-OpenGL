#include "gc.h"
#include "vec_mat_errors.h"

#define obj_assert(x, ret) if (!x) {push_error_info(GC_NULL_IN_DATA, "In argument "#x); return ret;}

nodehnd garbage_stack = 0;

/*void list_destroy(nodehnd* hnd){
	if (!hnd)
		return;
	nodehnd inner_node = list_node_data(*hnd);
	if (inner_node)
		list_del_node(inner_node);
}*/

vme_int gc_unit_id;

vme_int GC_NULL_IN_DATA = -1;
vme_int GC_STACK_CLOSED = -1;
vme_int GC_STACK_DONT_CLOSED = -1;
vme_int GC_STACK_END_REACHED = -1;
vme_int GC_GARBAGE_COLLECT_IS_PAUSED = -1;

bool gc_already_init = false;
bool pause_collect = false;

void gc_init(){
	if (gc_already_init)
		return;
	gc_unit_id = vme_register_unit("Garbage Collection");
	GC_NULL_IN_DATA = vme_register_error_type(gc_unit_id, "Null pointer in data");
	GC_STACK_CLOSED = vme_register_error_type(gc_unit_id, "Collect garbage don't started, use: gc_start_collet");
	GC_STACK_DONT_CLOSED = vme_register_error_type(gc_unit_id, "Inner garbage stack don't closed before the erase stack");
	GC_STACK_END_REACHED = vme_register_error_type(gc_unit_id, "Can't move node, end of stack reached");
	GC_GARBAGE_COLLECT_IS_PAUSED = vme_register_error_type(gc_unit_id, "You must resume collect garbage before erase collected garbage");
	gc_already_init = true;
}

gc_id_t gc_start_collect(){
	garbage_stack = list_add_node_next(garbage_stack, 0, 0);
	list_node_set_ptr_to_this_from_next(garbage_stack, &garbage_stack);
	return garbage_stack;
}

void gc_pause_collect() {
	pause_collect = true;
}

void gc_resume_collect() {
	pause_collect = false;
}

void gc_erase_collected(gc_id_t gc_id){
	obj_assert(gc_id);

	if (pause_collect) {
		push_error(GC_GARBAGE_COLLECT_IS_PAUSED);
		return;
	}

	nodehnd curr_garbage_stack_node = gc_id;

	nodehnd curr_node = list_node_get_data(curr_garbage_stack_node);

	if (list_get_next_node(curr_garbage_stack_node)){
		push_error(GC_STACK_DONT_CLOSED);
		gc_erase_collected(list_get_next_node(curr_garbage_stack_node));
	}

	while (curr_node) {
		nodehnd next_node = list_get_next_node(curr_node);
		void* data = list_node_get_data(curr_node);
		list_node_get_destroy(curr_node)(&data); //деструктор, например mat_destroy, так же вызывает функцию gc_unregist, который в свою очередь, удал€ет node
		curr_node = next_node;					 //вызов gc_unregist в mat_destroy, сделан дл€ того чтобы, если объ€ект удал€етс€ из вне, его можно было так же исключить из реестра.
	}

	garbage_stack = list_get_prev_node(curr_garbage_stack_node);
	list_del_node(curr_garbage_stack_node);
}

gc_id_t gc_push_garbage(void* g, destroy_func destroy){
	if (pause_collect)
		return nullptr;
	obj_assert(g, nullptr);
	obj_assert(destroy, nullptr);

	if (!garbage_stack) {
		push_error(GC_STACK_CLOSED);
		return nullptr;
	}
	nodehnd first_node = list_node_get_data(garbage_stack);
	nodehnd curr_node = list_add_node_prev(first_node, g, destroy);
	list_node_set_data(garbage_stack, curr_node);
	list_node_set_ptr_to_this_from_prev(curr_node, list_node_get_data_ptr(garbage_stack));
	
	return curr_node;
}
/*
void gc_move_to_prev_stack(gc_id_t id){
	obj_assert(id);
	nodehnd node = id;

	if (!garbage_stack || !list_get_prev_node(garbage_stack)) {
		push_error(GC_STACK_END_REACHED);
		return;
	}

	void* data = list_node_get_data(node);
	destroy_func destroy = list_node_get_destroy(node);
	gc_unregist(id);
}
*/
void gc_unregist(gc_id_t id){
	obj_assert(id);

	nodehnd node = id;

	list_node_set_destroy(node, nullptr);
	list_del_node(node);
}

void gc_clear_all_garbage(){
	while (garbage_stack)
		gc_erase_collected(garbage_stack);
}