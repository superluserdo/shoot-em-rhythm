project('shoot-em-rhythm', 'c')
#add_global_arguments('-DSOME_TOKEN=value', language : 'c')
#add_global_arguments('-Wno-pedantic', language : 'c')
#add_global_arguments('-Wno-all', language : 'c')
cc = meson.get_compiler('c')

conf_data = configuration_data()
conf_data.set10('USE_OPENGL', get_option('use-opengl'))
message('Use opengl: ')
message(get_option('use-opengl'))

subdir('engine')
subdir('game')

executable('shoot-em-rhythm', sources_engine, sources_game,
	include_directories : [inc_engine, inc_game],
	dependencies : [deps_engine, deps_game],
	install: true,
	install_dir: '../bin/',
)

buildscripts_dir = join_paths(meson.source_root(),'../buildscripts')
scripts_dir = join_paths(meson.source_root(),'../scripts')
message('buildscripts_dir is :'+ buildscripts_dir)

cmd = run_command(join_paths(buildscripts_dir, 'get_all_mods.sh'))
message('Modules found:')
mod_sources = cmd.stdout().strip().split('\n')
foreach mod_source : mod_sources
	mod_name = mod_source.split('/')[-1]
	mod_name_stripped = mod_name.strip('.c')
	mod_path = join_paths(meson.source_root(), 'modules', mod_name)
	message(mod_name)
	module = shared_library(mod_name_stripped, mod_path,
	include_directories : inc_engine,
 )
endforeach

path = join_paths(buildscripts_dir, 'create_structdef_py.sh')
message('path is :'+ path)

structdef_py = custom_target('structdef_py',
  input : join_paths(buildscripts_dir, 'create_structdef_py.sh'),
  output : 'structdef.py',
  #command : [comp, '@INPUT@', '@OUTPUT@'],
  command : [join_paths(buildscripts_dir, 'create_structdef_py.sh')],
  install : true,
  install_dir : scripts_dir)

install_subdir(scripts_dir,
  install_dir : 'testdir')
