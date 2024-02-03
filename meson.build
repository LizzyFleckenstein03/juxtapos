project('juxtapos', 'c', default_options: ['warning_level=2'])

add_project_arguments('-DGLEW_NO_GLU', language: 'c')

cc = meson.get_compiler('c')

gl = dependency('gl')
glfw = dependency('glfw3')
glew = dependency('glew')
m = cc.find_library('m', required: false)

embed = generator(find_program('python3'),
	output: '@PLAINNAME@.h',
	arguments: [
		'-c',
		'open("@OUTPUT@", "w").write(",".join([str(ord(c)) for c in open("@INPUT@").read()]))',
	],
)

juxtapos = executable('juxtapos',
	sources: [
		'src/main.c',
		embed.process([
			'assets/wood.glsl',
			'assets/vertex.glsl',
			'assets/fragment.glsl',
			'assets/quad_vertex.glsl',
			'assets/quad_fragment.glsl',
		]),
	],
	dependencies: [glfw, glew, m],
	install: true,
)

run_target('run', command: juxtapos)
