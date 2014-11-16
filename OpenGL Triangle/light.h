#ifndef LIGHT_H
#define LIGHT_H
#include "vec.h"
#include "mat.h"

typedef void* lighthnd;

#define LIGHT_TYPE_COUNT 3

enum LIGHT_TYPE {
	LIGHT_DIRECTIONAL,
	LIGHT_OMNI,
	LIGHT_SPOT
};

struct light_attenuation_t {
	float constant;
	float linear;
	float quadratic;
};

void light_init();

lighthnd light_create_directional(vechnd color, float multiply, vechnd direction);
lighthnd light_create_omni(vechnd color, float multiply, vechnd pos, light_attenuation_t attenuation);
lighthnd light_create_spot(vechnd color, float multiply, vechnd pos, vechnd direction, float cutoff, light_attenuation_t attenuation);
char* light_get_shader(LIGHT_TYPE type);
void light_destroy(lighthnd* light);


#endif