inc_engine = include_directories('include')
subdir('include')

sources_engine_list = [
	# Engine:
	'structure.c',
	'audio.c',
	'clock.c',
	'animate.c',
	'dict.c',
	'helpers.c',
	'rdtsc.c',
	]

if get_option('use-opengl')
	sources_engine_list += 'opengl_helpers.c'
endif

sources_engine = files(sources_engine_list)

deps_engine = [
	dependency('SDL2'),
	dependency('SDL2_image'),
	dependency('SDL2_ttf'),
	dependency('SDL2_mixer'),
	dependency('python3'),
	dependency('GL'),
	dependency('GLEW'),
	cc.find_library('dl'),
	cc.find_library('m'),
	cc.find_library('config'),
]
