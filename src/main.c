#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include "../linmath.h/linmath.h"

static void opengl_error(GLenum err, const char *file, int line)
{
	switch (err) {
		case GL_INVALID_ENUM:                  fprintf(stderr, "[warning] OpenGL error: INVALID_ENUM %s:%d\n", file, line); break;
		case GL_INVALID_VALUE:                 fprintf(stderr, "[warning] OpenGL error: INVALID_VALUE %s:%d\n", file, line); break;
		case GL_INVALID_OPERATION:             fprintf(stderr, "[warning] OpenGL error: INVALID_OPERATION %s:%d\n", file, line); break;
		case GL_STACK_OVERFLOW:                fprintf(stderr, "[warning] OpenGL error: STACK_OVERFLOW %s:%d\n", file, line); break;
		case GL_STACK_UNDERFLOW:               fprintf(stderr, "[warning] OpenGL error: STACK_UNDERFLOW %s:%d\n", file, line); break;
		case GL_OUT_OF_MEMORY:                 fprintf(stderr, "[warning] OpenGL error: OUT_OF_MEMORY %s:%d\n", file, line); break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: fprintf(stderr, "[warning] OpenGL error: INVALID_FRAMEBUFFER_OPERATION %s:%d\n", file, line); break;
		default: break;
	}
}
void opengl_debug(const char *file, int line)
{
	GLenum err = glGetError();

	if (err != GL_NO_ERROR)
		opengl_error(err, file, line);
}

struct shader {
	const char *name;
	GLenum type;
	const char *src;
	size_t len;
};

int win_width, win_height;

static void framebuffer_size_callback(GLFWwindow *handle, int width, int height)
{
	(void) handle;
	glViewport(0, 0, width, height);
	win_width = width;
	win_height = height;
}

#define glUniform(loc, x) _Generic((x), \
	GLfloat: glUniform1f, \
	GLint: glUniform1i, \
	GLuint: glUniform1ui \
	)(loc, x)

GLuint shader_program_create(const char *name, struct shader *shaders, size_t num_shaders)
{
	GLuint prog_id = glCreateProgram();
	GLuint shader_ids[num_shaders];

	for (size_t i = 0; i < num_shaders; i++) {
		GLint len = shaders[i].len;

		shader_ids[i] = glCreateShader(shaders[i].type);
		glShaderSource(shader_ids[i], 1, &shaders[i].src, &len);
		glCompileShader(shader_ids[i]);

		GLint compile_success;
		glGetShaderiv(shader_ids[i], GL_COMPILE_STATUS, &compile_success);

		if (!compile_success) {
			char error[BUFSIZ];
			glGetShaderInfoLog(shader_ids[i], BUFSIZ, NULL, error);
			fprintf(stderr, "[error] failed to compile %s shader: %s", shaders[i].name, error);
			exit(EXIT_FAILURE);
		}

		glAttachShader(prog_id, shader_ids[i]);
	}

	glLinkProgram(prog_id);

	for (size_t i = 0; i < num_shaders; i++)
		glDeleteShader(shader_ids[i]);

	GLint link_success;
	glGetProgramiv(prog_id, GL_LINK_STATUS, &link_success);
	if (!link_success) {
		char error[BUFSIZ];
		glGetProgramInfoLog(prog_id, BUFSIZ, NULL, error);
		fprintf(stderr, "[error] failed to link %s shader program: %s\n", name, error);
		exit(EXIT_FAILURE);
	}

	return prog_id;
}

#define GL_DEBUG opengl_debug(__FILE__, __LINE__);

#include "hacks.h"

VERTEX_DEF(vertex,
	(T_FLOAT, pos, 4),
	(T_FLOAT, texCoord, 2)
)

int main()
{
	if (!glfwInit()) {
		fprintf(stderr, "[error] failed to initialize GLFW\n");
		exit(EXIT_FAILURE);
	}

	//FST(FLOAT) x = 0.0;
	//MEOW(int) a = 0;
	//int a = MEW();
	//int a = CALL(_1, PAIR(1,2));
	//int b = CALL(_2, FLOAT);

	atexit(glfwTerminate);

	glfwWindowHint(GLFW_SAMPLES, 8);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	win_width = 1025;
	win_height = 750;

	GLFWwindow *window = glfwCreateWindow(win_width, win_height, "Juxtapos", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "[error] failed to create window\n");
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "[error] failed to initialize GLEW\n");
		exit(EXIT_FAILURE);
	}

	glViewport(0, 0, win_width, win_height);
	glfwSetFramebufferSizeCallback(window, &framebuffer_size_callback);

	// dimensions of the image
	int tex_w = 480, tex_h = 480;
	GLuint wood_texture;
	glGenTextures(1, &wood_texture);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, wood_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, wood_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	/*
	int work_grp_cnt[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);
	printf("max global (total) work group counts x:%i y:%i z:%i\n",
  		work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);

	int work_grp_inv;
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
	printf("max local work group invocations %i\n", work_grp_inv); */

	const char wood_shader_src[] = {
#include "wood.glsl.h"
	};

	GLuint wood_shader = shader_program_create("wood",  (struct shader []) {
		{ "wood", GL_COMPUTE_SHADER, wood_shader_src, sizeof wood_shader_src },
	}, 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, wood_texture);
	glUseProgram(wood_shader);
	glDispatchCompute(tex_w, tex_h, 1);

	const char fragment_shader_src[] = {
#include "fragment.glsl.h"
	};
	const char vertex_shader_src[] = {
#include "vertex.glsl.h"
	};

	GLuint main_shader = shader_program_create("main", (struct shader []) {
		{ "vertex", GL_VERTEX_SHADER, vertex_shader_src, sizeof vertex_shader_src },
		{ "fragment", GL_FRAGMENT_SHADER, fragment_shader_src, sizeof fragment_shader_src },
	}, 2);

	GLint loc_model = glGetUniformLocation(main_shader, "model");
	GLint loc_viewProj = glGetUniformLocation(main_shader, "viewProj");

	/*struct vertex cube[3][2][2][2];
	for (int i = 0; i < 3; i++)
		// for those, i noticed i can replace them with a single loop and use bit manipulation
		for (int a = 0; a <= 1; a++)
		for (int b = 0; b <= 1; b++)
		for (int c = 0; c <= 1; c++) {
			struct vertex *v = &cube[i][a][b][c];
			// this simply turned into a loop over k
			v->pos[(i+0)%3] = a * 2.0 - 1.0;
			v->pos[(i+1)%3] = b * 2.0 - 1.0;
			v->pos[(i+2)%3] = c * 2.0 - 1.0;
		}*/

/*#define DIM 3
	struct vertex vertices[DIM][1 << DIM] = { 0 };
	for (int i = 0; i < DIM; i++)
	for (int j = 0; j < 1 << DIM; j++)
	for (int k = 0; k < DIM; k++)
		vertices[i][j].pos[(i+k)%DIM] = ((j >> k) & 1) * 2.0 - 1.0;
*/

	/*GLuint indices[DIM][1 << DIM][2];
	for (int i = 0; i < DIM; i++)
	for (int j = 0; j < 1 << DIM; j++)
		indices[i][j][1] = (indices[i][j][0] = j) ^ (1 << i);*/

#define DIM 0
	struct vertex vertices[1 << DIM] = { 0 };
	for (int j = 0; j < 1 << DIM; j++)
	for (int k = 0; k < DIM; k++)
		vertices[j].pos[k] = ((j >> k) & 1) * 2.0 - 1.0;

#define BITS_FROM(x, i) ((x) & ((~0U) << (i))) // select all bits above and including bit
#define BITS_TO(x, i) ((x) & ~((~0U) << (i)))  // select all bits below bit i

	GLuint indices[DIM][1 << (DIM-1)][2];
	for (unsigned int i = 0; i < DIM; i++)
	for (unsigned int j = 0; j < 1 << (DIM-1); j++)
		indices[i][j][1] = (indices[i][j][0] = (
			BITS_FROM(j, i) << 1 | BITS_TO(j, i)
		)) | (1 << i);

	//for ()

	/*GLuint indices[ CUBE_LEN/4 ][8];
	for (int i = 0; i < CUBE_LEN/4; i++) {
		for (int j = 0; j < 8; j++)
			indices[i][j] = i*4 + (((j+1) % 8) / 2);
	}*/

	/*
	GLuint indices[6][6];
	for (int i = 0; i < 6; i++) {
		GLuint quad[6] = { 0, 1, 2, 2, 3, 1 };
		for (int j = 0; j < 6; j++)
			indices[i][j] = i*4 + quad[j];
	}*/

	/*
	struct vertex_quad quad[4] = {
		{{ -0.5, +0.5, +0.0 }},
		{{ -0.5, -0.5, +0.0 }},
		{{ +0.5, +0.5, +0.0 }},
		{{ +0.5, -0.5, +0.0 }},
	};*/

	GLuint vao, vbo, ebo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);

	vertex_configure_vao();

	unsigned int skyblue = 0x87ceeb;
	glClearColor(
		((skyblue >> 16) & 0xff) / 255.0,
		((skyblue >>  8) & 0xff) / 255.0,
		((skyblue >>  0) & 0xff) / 255.0, 1.0);
	glPointSize(5.0);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glfwSetTime(0.0);
	while (!glfwWindowShouldClose(window)) {
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		// preview wood texture
		glUseProgram(main_shader);

		mat4x4 proj;
		mat4x4_perspective(proj, 90.0, (float) win_width / (float) win_height, 0.1, 100.0);

		vec3 up;
		vec3_norm(up, (vec3) { 0.0, 1.0, -1.0 });

		mat4x4 view;
		mat4x4_look_at(view, (vec3) { 0.0, 2.0, 2.0 }, (vec3) { 0, 0, 0 }, up);

		mat4x4 view_proj;
		mat4x4_mul(view_proj, proj, view);

		mat4x4 id;
		mat4x4_identity(id);
		mat4x4 model;
		mat4x4_rotate_Y(model, id, glfwGetTime() * M_PI * 0.1);

		glUniformMatrix4fv(loc_model, 1, GL_FALSE, &model[0][0]);
		glUniformMatrix4fv(loc_viewProj, 1, GL_FALSE, &view_proj[0][0]);

		glBindVertexArray(vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wood_texture);
		//glDrawArrays(GL_POINTS, 0, sizeof vertices / sizeof(struct vertex));
		glDrawElements(GL_LINES, sizeof indices / sizeof(GLuint), GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteProgram(wood_shader);
}
