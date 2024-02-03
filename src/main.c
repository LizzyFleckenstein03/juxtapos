#define _GNU_SOURCE
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../linmath.h/linmath.h"
#include "../stb/stb_image.h"

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


void APIENTRY gl_debug_message_callback(GLenum source, GLenum type, GLuint id,
							GLenum severity, GLsizei length,
							const GLchar *msg, const void *data)
{
	char* _source;
	char* _type;
	char* _severity;

	switch (source) {
		case GL_DEBUG_SOURCE_API:
		_source = "API";
		break;

		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		_source = "WINDOW SYSTEM";
		break;

		case GL_DEBUG_SOURCE_SHADER_COMPILER:
		_source = "SHADER COMPILER";
		break;

		case GL_DEBUG_SOURCE_THIRD_PARTY:
		_source = "THIRD PARTY";
		break;

		case GL_DEBUG_SOURCE_APPLICATION:
		_source = "APPLICATION";
		break;

		case GL_DEBUG_SOURCE_OTHER:
		_source = "UNKNOWN";
		break;

		default:
		_source = "UNKNOWN";
		break;
	}

	switch (type) {
		case GL_DEBUG_TYPE_ERROR:
		_type = "ERROR";
		break;

		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		_type = "DEPRECATED BEHAVIOR";
		break;

		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		_type = "UDEFINED BEHAVIOR";
		break;

		case GL_DEBUG_TYPE_PORTABILITY:
		_type = "PORTABILITY";
		break;

		case GL_DEBUG_TYPE_PERFORMANCE:
		_type = "PERFORMANCE";
		break;

		case GL_DEBUG_TYPE_OTHER:
		_type = "OTHER";
		break;

		case GL_DEBUG_TYPE_MARKER:
		_type = "MARKER";
		break;

		default:
		_type = "UNKNOWN";
		break;
	}

	switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH:
		_severity = "HIGH";
		break;

		case GL_DEBUG_SEVERITY_MEDIUM:
		_severity = "MEDIUM";
		break;

		case GL_DEBUG_SEVERITY_LOW:
		_severity = "LOW";
		break;

		case GL_DEBUG_SEVERITY_NOTIFICATION:
		_severity = "NOTIFICATION";
		break;

		default:
		_severity = "UNKNOWN";
		break;
	}

	printf("%d: %s of %s severity, raised from %s: %s\n",
			id, _type, _severity, _source, msg);
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
bool resized = false;

static void framebuffer_size_callback(GLFWwindow *handle, int width, int height)
{
	(void) handle;
	win_width = width;
	win_height = height;
	resized = true;
}

#define glUniform(loc, x) _Generic((x), \
	GLfloat: glUniform1f, \
	GLint: glUniform1i, \
	GLuint: glUniform1ui \
	)(loc, x)

#define GL_DEBUG opengl_debug(__FILE__, __LINE__);

GLuint texture_upload(unsigned int width, unsigned int height, unsigned char *data, GLenum format)
{
	GLuint txo;
	glGenTextures(1, &txo); GL_DEBUG
	glBindTexture(GL_TEXTURE_2D, txo); GL_DEBUG

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); GL_DEBUG
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); GL_DEBUG
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); GL_DEBUG
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); GL_DEBUG

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data); GL_DEBUG
	glGenerateMipmap(GL_TEXTURE_2D); GL_DEBUG
	return txo;
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
			fprintf(stderr, "[error] failed to compile %s %s shader: %s", name, shaders[i].name, error);
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

#define COMPUTE_SHADER(name, compute_src) \
	GLuint name ## _shader = shader_program_create(#name, (struct shader []) { \
		{ "compute", GL_COMPUTE_SHADER, compute_src, sizeof compute_src }, \
	}, 1);

#define RENDER_SHADER(name, vertex_src, fragment_src) \
	GLuint name ## _shader = shader_program_create(#name, (struct shader []) { \
		{ "vertex", GL_VERTEX_SHADER, vertex_src, sizeof vertex_src }, \
		{ "fragment", GL_FRAGMENT_SHADER, fragment_src, sizeof fragment_src }, \
	}, 2);

#define UNIFORM_LOC(shader, name) GLint loc_ ## name = glGetUniformLocation(shader, #name);

#define EMBED(...) ((const char []) { __VA_ARGS__ })

#define BITS_FROM(x, i) ((x) & ((~0U) << (i))) // select all bits above and including bit i
#define BITS_TO(x, i) ((x) & ~((~0U) << (i)))  // select all bits below bit i

#define CUBE_VERTICES_POPULATE(array, DIM) \
	for (unsigned int j = 0; j < 1 << DIM; j++) \
	for (unsigned int k = 0; k < DIM; k++) \
		array[j].pos[k] = ((j >> k) & 1) * 2.0 - 1.0;

#define CUBE_VERTICES(name, vertex_type, DIM) \
	vertex_type name[1 << DIM] = { 0 }; \
	CUBE_VERTICES_POPULATE(name, DIM)

#define CUBE_LINE_INDICES(name, DIM) \
	GLuint name[DIM][1 << (DIM-1)][2]; \
	for (unsigned int i = 0; i < DIM; i++) \
	for (unsigned int j = 0; j < 1 << (DIM-1); j++) \
		name[i][j][1] = (indices[i][j][0] = ( \
			BITS_FROM(j, i) << 1 | BITS_TO(j, i) \
		)) | (1 << i);

#define CUBE_FACE_INDICES(name, DIM) \
	GLuint name[(DIM * (DIM-1)) / 2][1 << (DIM-2)][6]; \
	{ \
		unsigned int l = 0; \
		for (unsigned int i = 0; i < DIM; i++) \
		for (unsigned int j = i+1; j < DIM; j++) { \
			for (unsigned int k = 0; k < 1 << (DIM-2); k++) { \
				unsigned int quad[6] = { 0, 1, 2, 2, 3, 1 }; \
				for (unsigned int m = 0; m < 6; m++) { \
					name[l][k][m] = \
						BITS_TO(k, i) | \
						BITS_TO(BITS_FROM(k, i) << 1, j) | \
						BITS_FROM(k << 1, j) << 1 | \
						((quad[m] >> 0) & 1) << i | \
						((quad[m] >> 1) & 1) << j; \
				} \
			} \
			l++; \
		} \
	}

/*
struct tri_sort_arg {
	GLuint *indices;
	struct vertex *vertices;
	mat4x4 model;
	mat4x4 view;
};

float get_depth(struct tri_sort_arg *arg, const GLuint *t)
{
	vec4 world_space = { 0.0 };

	for (int i = 0; i < 3; i++)
		vec4_add(world_space, world_space, arg->vertices[arg->indices[*t * 3 + i]].pos);

	vec4_scale(world_space, world_space, 1.0/4.0);

	vec4 model_space;
	mat4x4_mul_vec4(model_space, arg->model, world_space);
	model_space[4] = model_space[4] * 0.5 + 1.5;
	vec4_scale(model_space, model_space, 1/model_space[4]);

	vec4 ndc_space;
	mat4x4_mul_vec4(ndc_space, arg->view, model_space);
	vec4_scale(ndc_space, ndc_space, 1/ndc_space[4]);

	return ndc_space[3];
}

int tri_sort_compare(const void *v_a, const void *v_b, void *v_arg)
{
	return (int) (get_depth(v_arg, v_b) - get_depth(v_arg, v_a));
}*/

GLuint vertex_array_create(void *vertices, size_t size_vertices, void *indices, size_t size_indices, void configure())
{
	GLuint vao;
	glGenVertexArrays(1, &vao);

	GLuint buffers[2];
	glGenBuffers(2, buffers);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, size_vertices, vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_indices, indices, GL_STATIC_DRAW);

	configure();

	glBindVertexArray(0);
	glDeleteBuffers(2, buffers);

	return vao;
}

#define NUM_INDICES(array) (sizeof(array) / sizeof(GLuint))

#include "hacks.h"

VERTEX_DEF(vertex,
	(T_FLOAT, pos, 4),
	(T_FLOAT, texCoord, 2)
)

VERTEX_DEF(vertex_quad,
	(T_FLOAT, pos, 2)
)

void mat4x4_rotate_xw(mat4x4 out, float angle)
{
	mat4x4_identity(out);
	out[0][0] = cos(angle);
	out[0][3] = sin(angle);
	out[3][0] = -sin(angle);
	out[3][3] = cos(angle);
}

void mat4x4_rotate_yw(mat4x4 out, float angle)
{
	mat4x4_identity(out);
	out[1][1] = cos(angle);
	out[1][3] = -sin(angle);
	out[3][1] = sin(angle);
	out[3][3] = cos(angle);
}

void mat4x4_rotate_zw(mat4x4 out, float angle)
{
	mat4x4_identity(out);
	out[2][2] = cos(angle);
	out[2][3] = -sin(angle);
	out[3][2] = sin(angle);
	out[3][3] = cos(angle);
}

int main()
{
	if (!glfwInit()) {
		fprintf(stderr, "[error] failed to initialize GLFW\n");
		exit(EXIT_FAILURE);
	}

	atexit(glfwTerminate);

#define SAMPLES 2

#if SAMPLES > 1
	glfwWindowHint(GLFW_SAMPLES, SAMPLES);
#endif
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

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(gl_debug_message_callback, NULL);

	glViewport(0, 0, win_width, win_height);
	glfwSetFramebufferSizeCallback(window, &framebuffer_size_callback);

	const char wood_src[] = {
#include "wood.glsl.h"
	};

	COMPUTE_SHADER(wood, wood_src)

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

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, wood_texture);
	glUseProgram(wood_shader);
	//glDispatchCompute(tex_w, tex_h, 1);
	//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	CUBE_VERTICES(tesseract_vertices, struct vertex, 4)
	CUBE_FACE_INDICES(tesseract_indices, 4)
	GLuint tesseract_vao = vertex_array_create(
		tesseract_vertices, sizeof tesseract_vertices,
		tesseract_indices, sizeof tesseract_indices,
		vertex_configure_vao);

	const char tesseract_vert_src[] = {
#include "vertex.glsl.h"
	};
	const char tesseract_frag_src[] = {
#include "fragment.glsl.h"
	};

	RENDER_SHADER(tesseract, tesseract_vert_src, tesseract_frag_src)
	UNIFORM_LOC(tesseract_shader, model)
	UNIFORM_LOC(tesseract_shader, viewProj)
	UNIFORM_LOC(tesseract_shader, materialTexture)
	UNIFORM_LOC(tesseract_shader, prevDepth)


	CUBE_VERTICES(quad_vertices, struct vertex_quad, 2)
	CUBE_FACE_INDICES(quad_indices, 2)
	GLuint quad_vao = vertex_array_create(
		quad_vertices, sizeof quad_vertices,
		quad_indices, sizeof quad_indices,
		vertex_quad_configure_vao);

	const char quad_vert_src[] = {
#include "quad_vertex.glsl.h"
	};
	const char quad_frag_src[] = {
#include "quad_fragment.glsl.h"
	};

	RENDER_SHADER(quad, quad_vert_src, quad_frag_src)

	/*
	GLuint textures[24];
	for (int i = 0; i < 1; i++) {
		char *file;
		asprintf(&file, "../textures/%d.png", i);

		int width, height, channels;

		unsigned char *data = stbi_load(file, &width, &height, &channels, 0);

		if (!data) {
			fprintf(stderr, "failed to load texture %s\n", file);
			exit(EXIT_FAILURE);
		}

		textures[i] = texture_upload(width, height, data, GL_RGBA);
		stbi_image_free(data);

		free(file);
	}*/

	// config
	unsigned int skyblue = 0x87ceeb;
	glClearColor(
		((skyblue >> 16) & 0xff) / 255.0,
		((skyblue >>  8) & 0xff) / 255.0,
		((skyblue >>  0) & 0xff) / 255.0, 0.0);
	glPointSize(5.0);

#if SAMPLES > 1
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_SAMPLE_SHADING);
	glMinSampleShading(1.0);

	GLfloat value;
	glGetFloatv(GL_MIN_SAMPLE_SHADING_VALUE, &value);
	printf("%f\n", value);

#endif

	GLuint color_buffer;
	glGenTextures(1, &color_buffer);

	GLuint depth_buffers[2];
	glGenTextures(2, depth_buffers);

	GLuint framebuffers[2];
	glGenFramebuffers(2, framebuffers);

	glfwSetTime(0.0);
	float last = 0.0;
	float angle = 0.0;

	resized = true;

	while (!glfwWindowShouldClose(window)) {
		float time = glfwGetTime();
		float delta = time-last;
		last = time;

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);

		if (resized) {
			resized = false;
			glViewport(0, 0, win_width, win_height);

#if SAMPLES > 1
#define TEXTURE_TARGET GL_TEXTURE_2D_MULTISAMPLE
#else
#define TEXTURE_TARGET GL_TEXTURE_2D
#endif

			glBindTexture(TEXTURE_TARGET, color_buffer);
#if SAMPLES > 1
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, SAMPLES, GL_RGBA, win_width, win_height, GL_TRUE);
#else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, win_width, win_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif
			for (int i = 0; i < 2; i++) {
				glBindTexture(TEXTURE_TARGET, depth_buffers[i]);
#if SAMPLES > 1
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, SAMPLES, GL_DEPTH_COMPONENT, win_width, win_height, GL_TRUE);
#else
				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, win_width, win_height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
				//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
#endif

				glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[i]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, TEXTURE_TARGET, color_buffer, 0);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, TEXTURE_TARGET, depth_buffers[i], 0);
			}
		}

		char *tit;
		asprintf(&tit, "%d FPS", (int) (1 / delta));
		glfwSetWindowTitle(window, tit);
		free(tit);

		// preview wood texture

		mat4x4 proj;
		mat4x4_perspective(proj, 90.0, (float) win_width / (float) win_height, 1.0, 4.2);

		vec3 up;
		vec3_norm(up, (vec3) { 0.0, 1.0, -1.0 });

		mat4x4 view;
		mat4x4_look_at(view, (vec3) { 0.0, 1.75, 1.75 }, (vec3) { 0, 0, 0 }, up);

		mat4x4 view_proj;
		mat4x4_mul(view_proj, proj, view);

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			;
		else if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS)
			angle -= delta * M_PI * 0.25;
		else
			angle += delta * M_PI * 0.25;

		/*
		mat4x4 id;
		mat4x4_identity(id);
		mat4x4 model;
		mat4x4_rotate_Z(model, id, angle);
		*/

		mat4x4 model;
		mat4x4_identity(model);

		mat4x4 tmp;

		mat4x4_rotate_yw(tmp, angle);
		mat4x4_mul(model, model, tmp);

		//mat4x4_rotate_yw(tmp, angle);
		//mat4x4_rotate_Y(model, model, angle);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[1]);
		glClearDepth(1.0);
		glClear(GL_DEPTH_BUFFER_BIT);

		for (int i = 0; i < 8; i++) {
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[i % 2]);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_GREATER);
			glDisable(GL_BLEND);
			glClearDepth(0.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(tesseract_shader);

			glUniformMatrix4fv(loc_model, 1, GL_FALSE, &model[0][0]);
			glUniformMatrix4fv(loc_viewProj, 1, GL_FALSE, &view_proj[0][0]);

			//glUniform1i(loc_materialTexture, 0);
			//glActiveTexture(GL_TEXTURE0);
			//glBindTexture(GL_TEXTURE_2D, wood_texture);

			glUniform1i(loc_prevDepth, 1);
			glActiveTexture(GL_TEXTURE0 + 1);
			glBindTexture(TEXTURE_TARGET, depth_buffers[(i + 1) % 2]);

			glBindVertexArray(tesseract_vao);
			glDrawElements(GL_TRIANGLES, NUM_INDICES(tesseract_indices), GL_UNSIGNED_INT, 0);

			// blend result

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glUseProgram(quad_shader);

			glBindVertexArray(quad_vao);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(TEXTURE_TARGET, color_buffer);
			glDrawElements(GL_TRIANGLES, NUM_INDICES(quad_indices), GL_UNSIGNED_INT, 0);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteProgram(wood_shader);
}
