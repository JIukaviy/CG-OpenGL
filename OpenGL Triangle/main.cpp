#define GLEW_STATIC
#include <GL\glew.h>
#include <GL\freeglut.h>
#include "gc.h"
#include "vec_mat.h"
#include "vec_mat_errors.h"
#include "objloader.h"
#include <iostream>
#include <fstream>

using namespace std;

int screen_width = 800, screen_height = 600;

bool keys[256];

GLuint program;
GLint attribute_coord3d;
GLint attribute_v_color;
GLint attribute_v_normal;
GLint uniform_fade;
GLint uniform_mvp;
GLint uniform_rotate;
GLuint vbo_obj_vertices;
GLuint vbo_obj_normals;
GLuint vbo_obj_colors;
GLuint ibo_obj_elements;
GLuint vao_id;

const int MAX_FILE_SIZE = 500;

mathnd rotate_mat;

void print_log(GLuint object){
	GLint log_length = 0;
	if (glIsShader(object))
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
	else if (glIsProgram(object))
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
	else {
		cout << "printlog: Not a shader or a program" << endl;
		return;
	}

	char* log = new char[log_length];

	if (glIsShader(object))
		glGetShaderInfoLog(object, log_length, NULL, log);
	else if (glIsProgram(object))
		glGetProgramInfoLog(object, log_length, NULL, log);

	cout << log << endl;
	free(log);
}

int create_shader(char* file_name, GLenum shader_type) {
	GLuint res;
	GLint compile_ok = GL_FALSE;

	ifstream fin(file_name);
	if (!fin.is_open()) {
		cout << "Can't open file: " << file_name << endl;
		return 0;
	}
	fin.seekg(0, fin.end);
	int file_size = fin.tellg();
	fin.seekg(0, fin.beg);

	char *s_source = new char[file_size + 1];
	memset(s_source, 0, file_size);
	fin.read(s_source, file_size);
	
	res = glCreateShader(shader_type);
	glShaderSource(res, 1, &s_source, NULL);
	glCompileShader(res);
	glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);

	if (!compile_ok) {
		cout << "Error in shader: " << file_name << endl;
		print_log(res);
		return 0;
	}

	return res;
}

int get_attrib_location(const char* attrib_name, GLint* attrib) {
	*attrib = glGetAttribLocation(program, attrib_name);
	if (*attrib == -1) {
		cout << "Could not bind attribute " << attrib_name << endl;
		return 1;
	}
	return 0;
}

int get_uniform_location(const char* uniform_name, GLint* uniform) {
	*uniform = glGetUniformLocation(program, uniform_name);
	if (*uniform == -1) {
		cout << "Could not bind uniform " << uniform_name << endl;
		return 1;
	}
	return 0;
}

int init_resources() {
	GLint link_ok = GL_FALSE;
	GLint compile_ok = GL_FALSE;
	GLchar *info_buffer;
	info_buffer = new GLchar[200];

	memset(&keys, 0, sizeof(keys));

	// Компилируем вершинный шейдер
	
	GLuint vs;
	
	if (!(vs = create_shader("vertex_shader.dat", GL_VERTEX_SHADER)))
		return 1;

	//Компилируем фрагментный шейдер

	GLuint fs;
	if (!(fs = create_shader("fragment_shader.dat", GL_FRAGMENT_SHADER)))
		return 1;

	//Создаем программу и линкуем шейдеры

	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);

	if (!link_ok) {
		print_log(vs);
		return 1;
	}
	glUseProgram(program);

	//Создание VBO

	GLfloat* obj_vertices;
	GLfloat* obj_normals;
	GLushort* obj_elements;
	int vert_size;
	int norm_size;
	int elem_size;

	objlodaer_load_file("models/teapot.obj", &obj_vertices, &vert_size, &obj_normals, &norm_size, &obj_elements, &elem_size);

	if (vme_error_appear()) {
		vme_print_errors();
		return 1;
	}

	glGenBuffers(1, &vbo_obj_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_obj_vertices);
	glBufferData(GL_ARRAY_BUFFER, vert_size*sizeof(GLfloat), obj_vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &vbo_obj_normals);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_obj_normals);
	glBufferData(GL_ARRAY_BUFFER, norm_size*sizeof(GLfloat), obj_normals, GL_STATIC_DRAW);

	//Создание IBO

	glGenBuffers(1, &ibo_obj_elements);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_obj_elements);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, elem_size*sizeof(GLushort), obj_elements, GL_STATIC_DRAW);

	delete obj_vertices;
	delete obj_normals;
	delete obj_elements;

	//Создание VAO
	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);

	//Получение указателя на переменные

	if (get_attrib_location("coord3d", &attribute_coord3d)) return 1;
	if (get_attrib_location("v_normal", &attribute_v_normal)) return 1;
	if (get_uniform_location("mvp", &uniform_mvp)) return 1;
	if (get_uniform_location("rotate", &uniform_rotate)) return 1;

	return 0;
}

void free_resources() {
	glDeleteProgram(program);
	glDeleteBuffers(1, &vbo_obj_vertices);
	glDeleteBuffers(1, &vbo_obj_colors);
}

void Display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);
	glEnableVertexAttribArray(attribute_coord3d);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_obj_vertices);
	glVertexAttribPointer(attribute_coord3d, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(attribute_v_normal);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_obj_normals);
	glVertexAttribPointer(attribute_v_normal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	int size;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_obj_elements);
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	glDrawElements(GL_TRIANGLES, size / sizeof(GLushort), GL_UNSIGNED_SHORT, 0);

	glDisableVertexAttribArray(attribute_coord3d);
	glDisableVertexAttribArray(attribute_v_normal);

	glutSwapBuffers();
}

void onKeyDown(unsigned char key, int x, int y) {
	keys[key] = true;
}
void onKeyUp(unsigned char key, int x, int y) {
	keys[key] = false;
}

vechnd cam_pos;
vechnd world_cam_target;
float cam_x = 0;
float cam_y = 0;

double prev_time = 0;

void idle(){
	double time = glutGet(GLUT_ELAPSED_TIME);
	double move = sin(time * (2 * 3.145) / 5000);
	double angle = time / 1000;
	glUseProgram(program);

	start_garbage_collect(1);

	mat_elem_t d = 1 * (time - prev_time) / 200;

	if (keys['g'])  cam_x -= d / 5;
	if (keys['j'])  cam_x += d / 5;
	if (keys['y'])  cam_y -= d / 5;
	if (keys['h'])  cam_y += d / 5;

	vechnd cam_up = vec_create3(0, 1, 0);
	vechnd t_cam_pos = vec_copy(cam_pos);
	vechnd cam_target = vec_create(3);

	mathnd rotate_cam = mat_mul(mat_rotate_mat4(cam_y, MAT_X), mat_rotate_mat4(cam_x, MAT_Y));
	vechnd cam_direction = vm_mat_vec_mul(rotate_cam, vec_create4(0, 0, 1, 0));
	cam_direction = vec_normalize(vec_convert(cam_direction, 3));

	vechnd forward_direction = cam_direction;
	vechnd bacward_direction = vec_invert(cam_direction);
	vechnd right_direction = vec_cross(forward_direction, cam_up);
	vechnd left_direction = vec_invert(right_direction);
	vechnd up_direction = vec_invert(cam_up);
	vechnd down_direction = cam_up;
	
	if (keys['w']) 	t_cam_pos = vec_sub(t_cam_pos, vec_mul(forward_direction, d));
	if (keys['s'])	t_cam_pos = vec_sub(t_cam_pos, vec_mul(bacward_direction, d));
	if (keys['q'])	t_cam_pos = vec_sub(t_cam_pos, vec_mul(down_direction, d));
	if (keys['e'])	t_cam_pos = vec_sub(t_cam_pos, vec_mul(up_direction, d));
	if (keys['a'])	t_cam_pos = vec_sub(t_cam_pos, vec_mul(left_direction, d));
	if (keys['d'])	t_cam_pos = vec_sub(t_cam_pos, vec_mul(right_direction, d));

	vec_copy(cam_pos, t_cam_pos);
	vec_copy(cam_up, up_direction);

	vechnd obj_pos = vec_create3(0, 0, -7);

	mathnd scale = mat_scale(1.5);
	mathnd rotate = mat_rotate_mat4(angle, MAT_Y);
	//rotate = mat_mul(rotate, mat_rotate_mat4(angle, MAT_Y));
	//rotate = mat_mul(rotate, mat_rotate_mat4(angle, MAT_X));
	mathnd translate = vm_mat_translate(obj_pos);

	mathnd model = mat_mul(translate, mat_mul(rotate, scale));
	//mathnd view = vm_mat_look_at(cam_pos, vec_pos_anim, cam_up);
	
	mathnd view = mat_mul(rotate_cam, vm_mat_translate(vec_invert(cam_pos)));
	//mathnd view = vm_mat_translate(vec_invert(cam_pos));
	mathnd projection = mat_perspective_projection(45.0, 1.0 * screen_width / screen_height, 0.1, 100.0);
	//mathnd projection = mat_orthographic_projection(-5, 5, -5, 5, -5, 5);
	mathnd mvp = mat_mul(projection, mat_mul(view, model));

	if (vme_error_appear()) {
		vme_print_errors();
		glutIdleFunc(nullptr);
	}
	else {
		glUniformMatrix4fv(uniform_mvp, 1, GL_TRUE, mat_get_elems(mvp));
		glUniformMatrix3fv(uniform_rotate, 1, GL_TRUE, mat_get_elems(mat_create_minor(rotate, 3, 3)));
	}

	erase_collected_garbage(1);

	prev_time = time;

	glutPostRedisplay();
}

int last_x, last_y, mx, my;

void onMousePress(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		last_x = x;
		last_y = y;
	}
}

void onMouseMotion(int x, int y) {
	mat_destroy(&rotate_mat);
	mx += x - last_x;
	my += y - last_y;
	mathnd mat_x = mat_rotate_mat4((float)mx / -100, MAT_Y);
	mathnd mat_y = mat_rotate_mat4((float)my / 100, MAT_X);
	rotate_mat = mat_mul(mat_x, mat_y);
	mat_destroy(&mat_x);
	mat_destroy(&mat_y);
	last_x = x;
	last_y = y;

	if (vme_error_appear()) {
		vme_print_errors();
		glutMotionFunc(nullptr);
	}
}

void onReshape(int width, int height) {
	screen_width = width;
	screen_height = height;
	glViewport(0, 0, screen_width, screen_height);
}

int main(int argc, char **argv) {
	vm_init();
	vme_init();
	gc_init();
	objlodaer_init();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(140, 140);
	glutInitContextVersion(3, 1);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutCreateWindow("OpenGL Cube");
	
	glClearColor(1.0, 1.0, 1.0, 1.0);
	
	glewExperimental = true;
	GLenum glew_status = glewInit();
	if (glew_status != GLEW_OK)	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
		return EXIT_FAILURE;
	}

	start_garbage_collect(1);

	cam_pos = vec_create(3);
	world_cam_target = vec_create(3);

	if (!init_resources()) {
		glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
		glutDisplayFunc(Display);
		glutReshapeFunc(onReshape);
		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		//glDepthFunc(GL_LESS);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glutIdleFunc(idle);
		glutMouseFunc(onMousePress);
		glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
		glutKeyboardFunc(onKeyDown);
		glutKeyboardUpFunc(onKeyUp);
		//glutMotionFunc(onMouseMotion);
		glutMainLoop();
	}

	erase_collected_garbage(1);
	gc_clear_all_garbage();

	free_resources();
	system("pause");

	return 0;
}