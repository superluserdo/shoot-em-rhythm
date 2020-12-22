inc_game = include_directories('include')

sources_game = files(
	# Game:
	'main.c', 
	'startscreen.c',
	'level.c',
	'spawn.c',
	'pause.c',
	'object_logic.c',
	'transform.c',
)

deps_game = [
	dependency('SDL2'),
	dependency('SDL2_image'),
	dependency('SDL2_ttf'),
	dependency('SDL2_mixer'),
	dependency('python3'),
	cc.find_library('python3.9'),
	cc.find_library('dl'),
	cc.find_library('m'),
	cc.find_library('config'),
]

