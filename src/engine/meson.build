inc_engine = include_directories('include')

sources_engine = files(
	# Engine:
	'structure.c',
	'audio.c',
	'clock.c',
	'animate.c',
	'dict.c',
	'helpers.c',
	'rdtsc.c',
)

deps_engine = [
	dependency('SDL2'),
	dependency('SDL2_image'),
	dependency('SDL2_ttf'),
	dependency('SDL2_mixer'),
	dependency('python3'),
	cc.find_library('dl'),
	cc.find_library('m'),
	cc.find_library('config'),
]
