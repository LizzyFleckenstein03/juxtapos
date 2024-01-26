project('juxtapos', 'c', default_options: ['warning_level=2'])

add_project_arguments('-DGLEW_NO_GLU', language: 'c')

gl = dependency('gl')
glfw = dependency('glfw3')
glew = dependency('glew')

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
		]),
	],
	dependencies: [glfw, glew],
	install: true,
)

run_target('run', command: juxtapos)
