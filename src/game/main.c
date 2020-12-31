/*	For the interactive python interpreter	*/
#include <Python.h>

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include "structdef_game.h"
#include "main.h"
#include "animate.h"
#include "level.h"
#include "audio.h"
#include "clock.h"
#include "pause.h"
#include "object_logic.h"
#include <libconfig.h>
#include <dlfcn.h>
#include "config.h"
#if USE_OPENGL
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include "opengl_funcs.h"
#else
#endif
#include "backend_funcs.h"
#define FIXME_SWITCH 0

int main() {


	/*	Initialise the basic game state struct	*/

	struct level_struct level = {0};
	struct player_struct player = {0};
	struct audio_struct audio = {0};
	struct time_struct timing = {0};
	struct program_struct program = {0};
	struct graphics_struct master_graphics = {
		.width = NATIVE_RES_X * ZOOM_MULT,
		.height = NATIVE_RES_Y * ZOOM_MULT,
		.debug_anchors = &program.debug.show_anchors,
		.debug_containers = &program.debug.show_containers,
		.debug_test_render_list_robustness = &program.debug.test_render_list_robustness,
		.graphics.master_graphics = &master_graphics,
	};

#if FIXME_SWITCH
	struct menu_stage_struct pause_stage = {0};
#endif
	
	if (graphics_init(&master_graphics)) {
		return 1;
	}

	if (audio_init(&audio) == R_FAILURE) {
		return R_FAILURE;
	}

    //    pthread_t soundeffects;
    //    int sdl_rc = pthread_create(&soundeffects, NULL, playsound, (void*)NULL);
    //    if (sdl_rc) {
    //            printf("ya dun goofed. return code is %d\n.", sdl_rc);
    //            exit(-1);
    //    }

	struct status_struct status = {
		.level = &level,
		.player = &player,
		.audio = &audio,
		.timing = &timing,
		.master_graphics = &master_graphics,
		.program = &program
	};
	timing.fpsanim = 30;
	timing.fpsglobal = 60;
	timing_init(&timing);
	program.debug.show_anchors = 0;
	program.debug.show_containers = 0;
	program.debug.test_render_list_robustness = 0;

	//TODO: Put this and other SDL stuff into functions
	//texture_t screen_texture = 0;
	//master_graphics.graphics.tex_target_ptr = &screen_texture;
	
	/* NOTE: I've moved timing out of its own thread into the main thread.
	 * frametimer() is no longer used */
	//pthread_t framethread;
	//int rc = pthread_create(&framethread, NULL, frametimer, (void *)&timing);
	//if (rc) {
	//	printf("ya dun goofed. return code is %d\n.", rc);
	//	exit(-1);
	//}

	program.python_interpreter_enable = 0;
	PyObject    *pModule ; 
	if (program.python_interpreter_enable) {

		printf("Python interpreter enabled! Initialising...\n");

		/*	Initialise the Python interpreter	*/
		Py_Initialize() ;

		char        python_funcname[] = "while_ipython" ;
		char		python_dirname[] = "scripts";
		char		python_filename[] = "pythonhelper";

		/*	Don't start the python interpreter until prompted	*/
		program.python_interpreter_activate = 0;

		/*	Add current directory to path of C-API python interpreter	*/
		PyRun_SimpleString("import sys");
		PyRun_SimpleString("sys.path.append('scripts')");
		PyRun_SimpleString("import os");
		
		/* Get Python code/module */
		pModule = PyImport_ImportModule(python_filename);
		if (pModule) {
			/* Get the function and check if its callable function */   
			program.python_helper_function = PyObject_GetAttrString(pModule, python_funcname);

			/* Build the input arguments */
			program.status_python_capsule = PyCapsule_New(&status, NULL, NULL);

			program.python_helper_function_generator = 
					PyObject_CallFunction(status.program->python_helper_function,
							"O", status.program->status_python_capsule);
			PyObject_CallMethod(status.program->python_helper_function_generator,
					"send", "s", NULL);
			printf("Made generator that returned %p\n", 
					program.python_helper_function_generator);
			if (program.python_helper_function_generator == NULL) {
				PyErr_Print();
			}
		} else {
			char cwd[100];
			fprintf (stderr,"Couldn't load the python module: %s/%s/%s\n", getcwd(cwd,sizeof(cwd)), python_dirname, python_filename) ;
			PyErr_Print();
		}
	}

	/* Set up hooks */

	/* Create list of functions to call in level */
	enum return_codes_e rc = hooks_setup(&program);
	if (rc!=R_SUCCESS) {
		fprintf(stderr,"Failed to setup the module hooks, sorry :(\n");
		return R_FAILURE;
	}

	enum  return_codes_e returncode = R_SUCCESS;//startscreen(master_graphics.window, master_graphics.renderer, &status);

	/*	The Main Game Loop */
	while (1) {

		audio_func(&audio);

		/* Call functions of per-frame plugins */
		
		struct hooks_list_struct *frame_hook = program.hooks.frame;
		while (frame_hook) {
			frame_hook->hookfunc(&status);
			frame_hook = frame_hook->next;
		}


		if ( returncode == R_SUCCESS ) {
			returncode = level_init(status);
		}
		else if ( returncode == R_FAILURE ) {
			return R_FAILURE;
		}
#if FIXME_SWITCH
		else if ( returncode == R_STARTSCREEN ) {
			returncode = startscreen(master_graphics.window, master_graphics.renderer, &status);
		}
		else if ( returncode == R_PAUSE_LEVEL) {
			returncode = pausefunc(master_graphics.renderer, &pause_stage,
					//*status.level->stage.graphics.tex_target_ptr, 
					&status);
		}
#endif
		else if ( returncode == R_LOOP_LEVEL) {
			returncode = level_loop(status);
		}
		else if ( returncode == R_RESTART_LEVEL) {
			returncode = level_init(status);
		}
		else if ( returncode == R_QUIT_TO_DESKTOP) {
			break;
		}
		else {
			break;
		}

		process_object_logics(master_graphics.graphics.object_list_stack);

		//render_process(master_graphics.graphics.object_list_stack, 
		//		&master_graphics.graphics, &master_graphics, 
		//		timing.currentbeat);

		SDL_Delay(wait_to_present(&timing));
		present_screen(timing, &master_graphics);
		update_time(&timing);

	}

	/* Finish Suspended Python Functions */
	if (status.program->python_interpreter_enable) {
		if (status.program->python_helper_function && 
				PyCallable_Check(status.program->python_helper_function)) {
			PyObject *pResult = PyObject_CallMethod(
					status.program->python_helper_function_generator,
					"send", "i", 1);
			if (!pResult) {
				//PyErr_Print();
			}
		} else {
			printf ("Some error with the function\n") ;
		}

		/* Release python resources. */
		if (program.python_helper_function_generator)
			Py_DECREF(program.python_helper_function_generator);
		if (program.python_helper_function)
			Py_DECREF(program.python_helper_function);
		if (pModule)
			Py_DECREF(pModule);

		/*Release the interpreter */
		Py_Finalize() ;
	}

	graphics_deinit(&master_graphics);
	if (master_graphics.font) {
		TTF_CloseFont(master_graphics.font);
	}

	printf("Game Over.\n");
	printf("(After %d frames)\n", timing.framecount);
	//pthread_mutex_lock(&quit_mutex);
	//quitgame = 1;
	//pthread_mutex_unlock(&quit_mutex);
	//pthread_join(framethread, NULL);
	return 0;
}

enum return_codes_e hooks_setup(struct program_struct *program) {

	/* Module/plugin support via external shared libraries: */

	/* Establish hook locations */
	struct hooks_list_struct *frame_hooks = NULL;

	struct hooks_list_struct *level_init_hooks = NULL;

	struct hooks_list_struct *level_loop_hooks = NULL;

	program->hooks.frame = frame_hooks;
	program->hooks.level_init = level_init_hooks;
	program->hooks.level_loop = level_loop_hooks;

	/* Specify hook execution location by number listed in file: */
	struct hooks_list_struct **hook_locations[3];

	hook_locations[FRAME] = &program->hooks.frame;
	hook_locations[LEVEL_INIT] = &program->hooks.level_init;
	hook_locations[LEVEL_LOOP] = &program->hooks.level_loop;

	/* Search for modules listed in modules.cfg and get them */

	const char *cfg_path = "cfg/modules.cfg";
	config_t cfg;
	config_init(&cfg);

	/* Read the file. If there is an error, report it and exit. */
	if(! config_read_file(&cfg, cfg_path))
	{
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
		config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
		FILEINFO
		return(R_FAILURE);
	}


	config_setting_t *modules_list_setting = config_lookup(&cfg, "modules_list");
	if (modules_list_setting == NULL) {
		fprintf(stderr,"Error looking up setting for 'modules_list'\n");
		FILEINFO
		return(R_FAILURE);
	}
	int num_modules = config_setting_length(modules_list_setting); 
	printf("Module files to load: %d\n", num_modules);
	for (int i = 0; i < num_modules; i++) {
		config_setting_t *module_setting = config_setting_get_elem(
				modules_list_setting, i);
		config_setting_t *path_setting = config_setting_get_member(
				module_setting, "path");
		const char *path = config_setting_get_string(path_setting);
		if (!path) {
			fprintf(stderr,"Could not find valid module file path in entry \
					%d of file %s\n", i, cfg_path);
			return(R_FAILURE);
		}

		config_setting_t *functions_setting = config_setting_get_member(module_setting,
			   	"functions");
		if (!functions_setting) {
			fprintf(stderr,"Error looking up setting for 'functions'\n");
			return(R_FAILURE);
		}
		int num_functions = config_setting_length(functions_setting);

		/* Load .so file */

		void *lib_handle;
		lib_handle = dlopen(path, RTLD_LAZY);
		if (!lib_handle)
		{
		   fprintf(stderr, "%s\n", dlerror());
		   FILEINFO
		   return R_FAILURE;
		}

		for (int j = 0; j < num_functions; j++) {
			config_setting_t *function_setting = config_setting_get_elem(
					functions_setting, j);
			if (!function_setting) {
				fprintf(stderr,"Error looking up setting for function %d\
					   	in module %d\n", j, i);
				return(R_FAILURE);
			}
			config_setting_t *name_setting = config_setting_get_member(
					function_setting, "name");
			if (!name_setting) {
				fprintf(stderr,"Error looking up setting for 'name' in function %d\
					   	in module %d\n", j, i);
				return(R_FAILURE);
			}
			const char *func_name = config_setting_get_string(name_setting);
			if (!func_name) {
				fprintf(stderr,"Error getting value for 'name' in function %d\
					   	in module %d\n", j, i);
				return(R_FAILURE);
			}

			config_setting_t *exec_location_setting = config_setting_get_member(
					function_setting, "exec_location");
			if (!exec_location_setting) {
				fprintf(stderr,"Error looking up setting for 'exec_location' in\
					   	function %d in module %d\n", j, i);
				return(R_FAILURE);
			}
			enum hook_type_e hook_type = config_setting_get_int(
					exec_location_setting);
			if (hook_type < 0 || hook_type >= NUM_HOOK_LOCATIONS) {
				fprintf(stderr,"Attempting to store hook in execution location \
						that doesn't exist.\n");
				return R_FAILURE;
			}

			/* Finally, after all that we can store the info about the hooks */

			char *error;
			
			void *(*funcptr)(struct status_struct *status) = dlsym(lib_handle, func_name);
			if ((error = dlerror()) != NULL) 
			{
			   fprintf(stderr, "%s\n", error);
			   FILEINFO
			   return R_FAILURE;
			} else {

				/* Add hook to end of its corresponding hooks list */
				struct hooks_list_struct **this_hook = hook_locations[hook_type];
				while (*this_hook) {
					*this_hook = (*this_hook)->next;
				}
				(*this_hook) = malloc(sizeof(struct hooks_list_struct));
				(*this_hook)->hookfunc = funcptr;
				(*this_hook)->next = NULL;
			}
		}

	}

	config_destroy(&cfg);
	return R_SUCCESS;
}

