typedef void(*destruct_func)(void**);

typedef void* gc_id_t;

gc_id_t gc_push_garbage(void* g, destruct_func destroy);
void gc_clear_garbage();
void gc_on_destroy(gc_id_t id);
