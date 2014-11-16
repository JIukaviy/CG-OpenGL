#include "list.h"

#define start_garbage_collect(id) gc_id_t gc_garbage_id##id = gc_start_collect();
#define erase_collected_garbage(id) gc_erase_collected(gc_garbage_id##id);

typedef nodehnd gc_id_t;

void gc_init();
gc_id_t gc_start_collect();
void gc_pause_collect();
void gc_resume_collect();
void gc_erase_collected(gc_id_t gc_id);
gc_id_t gc_push_garbage(void* g, destroy_func destroy);
void gc_clear_garbage();
void gc_clear_all_garbage();
void gc_unregist(gc_id_t id);
