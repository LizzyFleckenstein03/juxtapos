#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

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

static void framebuffer_size_callback(GLFWwindow *handle, int width, int height)
{
	(void) handle;
	glViewport(0, 0, width, height);
}

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

int main()
{
	if (!glfwInit()) {
		fprintf(stderr, "[error] failed to initialize GLFW\n");
		exit(EXIT_FAILURE);
	}

	atexit(glfwTerminate);

	// glfwWindowHint(GLFW_SAMPLES, 8);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	int win_w = 1000;
	int win_h = 1000;

	GLFWwindow *window = glfwCreateWindow(win_w, win_h, "Juxtapos", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "[error] failed to create window\n");
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "[error] failed to initialize GLEW\n");
		exit(EXIT_FAILURE);
	}

	glViewport(0, 0, win_w, win_h);
	glfwSetFramebufferSizeCallback(window, &framebuffer_size_callback);

	// dimensions of the image
	int tex_w = 1000, tex_h = 1000;
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

	GLfloat quad[4][2] = {
		{ -0.5, +0.5 },
		{ -0.5, -0.5 },
		{ +0.5, +0.5 },
		{ +0.5, -0.5 },
	};

	GLuint vao, vbo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof quad, quad, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof *quad, 0);
	glEnableVertexAttribArray(0);

	unsigned int skyblue = 0x87ceeb;
	glClearColor(
		((skyblue >> 16) & 0xff) / 255.0,
		((skyblue >>  8) & 0xff) / 255.0,
		((skyblue >>  0) & 0xff) / 255.0, 1.0);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	while (!glfwWindowShouldClose(window)) {
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// preview wood texture
		glUseProgram(main_shader);
		glBindVertexArray(vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wood_texture);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteProgram(wood_shader);
}
