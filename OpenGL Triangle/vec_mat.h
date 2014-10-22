#include "vec.h"
#include "mat.h"

typedef int vec_mat_err_code;

void vm_init();
vechnd vm_mat_vec_mul(mathnd mat, vechnd vec);
mathnd vm_mat_translate(vechnd vec);
mathnd vm_mat_look_at(vechnd cam_pos, vechnd cam_view, vechnd cam_up);