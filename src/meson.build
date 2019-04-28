project('shoot-em-rhythm', 'c')
add_global_arguments('-DSOME_TOKEN=value', language : 'c')
cc = meson.get_compiler('c')

#inc = include_directories('./')
inc = include_directories('include')
#
#subdir('include')
#subdir('src')
#subdir('test')

structdef_py = custom_target('structdef_py',
  output : 'structdef.py',
  #command : [comp, '@INPUT@', '@OUTPUT@'],
  command : ['../src/create_structdef_py.sh'],
  install : true,
  install_dir : 'subdir')

sources = files(
	# Game:
	'game/main.c', 
	'game/startscreen.c',
	'game/level.c',
	'game/spawn.c',
	'game/pause.c',
	'game/object_logic.c',
	'game/transform.c',
	# Engine:
	'engine/structure.c',
	'engine/audio.c',
	'engine/clock.c',
	'engine/animate.c',
	'engine/dict.c',
	'engine/helpers.c',
	'engine/rdtsc.c',
)

deps = [
	dependency('SDL2'),
	dependency('SDL2_image'),
	dependency('SDL2_ttf'),
	dependency('SDL2_mixer'),
	dependency('python3'),
	cc.find_library('dl'),
	cc.find_library('m'),
	cc.find_library('config'),
]

executable('shoot-em-rhythm', sources,
	include_directories : inc,
	dependencies : deps,
	install: true,
	install_dir: '../bin/',
)
