/*	For the interactive python interpreter	*/
#include <Python.h>

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include "structdef.h"
#include "main.h"
#include "level.h"
//#include "music.h"
#include "audio.h"
#include "clock.h"
#include <libconfig.h>
#include <dlfcn.h>

int main() {


	SDL_Window *win = NULL;
	SDL_Renderer *renderer = NULL;
	// Initialize SDL.
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		return 1;

	/*	Initialise the basic game state struct	*/

	struct level_struct level = {0};
	struct player_struct player = {0};
	struct audio_struct audio = {0};
	struct time_struct timing = {0};
	struct graphics_struct graphics = {0};
	struct program_struct program = {0};
	
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
		.graphics = &graphics,
		.program = &program
	};
	timing.fpsanim = 30;
	timing.fpsglobal = 60;
	timing_init(&timing);
	graphics.width = NATIVE_RES_X * ZOOM_MULT;
	graphics.height = NATIVE_RES_Y * ZOOM_MULT;
	program.debug.show_anchors = 1;
	program.debug.show_containers = 1;
	graphics.debug_anchors = &program.debug.show_anchors;
	graphics.debug_containers = &program.debug.show_containers;
	graphics.debug_test_render_list_robustness = &program.debug.test_render_list_robustness;

	/* NOTE: I've moved timing out of its own thread into the main thread.
	 * frametimer() is no longer used */
	//pthread_t framethread;
	//int rc = pthread_create(&framethread, NULL, frametimer, (void *)&timing);
	//if (rc) {
	//	printf("ya dun goofed. return code is %d\n.", rc);
	//	exit(-1);
	//}

	win = SDL_CreateWindow("TOM'S SUPER COOL GAME", 100, 100, graphics.width, graphics.height, SDL_WINDOW_RESIZABLE);
	graphics.window = win;
	renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		fprintf(stderr,"SDL_CreateRenderer failed. Error message: '%s\n'", SDL_GetError());
		
		/* Print driver info if renderer creation fails */
		SDL_RendererInfo info;
		int num = SDL_GetNumRenderDrivers();
		printf("There are %d usable render drivers\n", num);
		printf("Driver  SDL_RendererFlags\n");
		for (int i = 0; i < num; i++) {
	   	if (SDL_GetRenderDriverInfo(i,&info) == 0)
	   	printf("%s  %d\n", info.name, info.flags&2);
		}
	}
	graphics.renderer = renderer;

	program.python_interpreter_enable = 0;
	PyObject    *pModule ; 
	PyObject    *pDict ;
	PyObject    *python_func;
	PyObject *generator;
	if (program.python_interpreter_enable) {

		printf("Python interpreter enabled! Initialising...\n");

		/*	Initialise the Python interpreter	*/
		Py_Initialize() ;

		int         iSize = 0 ;
		char        python_funcname[] = "while_ipython" ;
		char		python_filename[] = "pythonhelper";

		/*	Don't start the python interpreter until prompted	*/
		program.python_interpreter_activate = 0;

		/*	Add current directory to path of C-API python interpreter	*/
		PyRun_SimpleString("import sys");
		PyRun_SimpleString("sys.path.append('.')");
		PyRun_SimpleString("import os");
		
		/* Get Python code/module */
		pModule = PyImport_ImportModule(python_filename);
		if (NULL != pModule)
		{
			/* Get the function and check if its callable function */   
			program.python_helper_function = PyObject_GetAttrString(pModule, python_funcname);

			/* Build the input arguments */
			program.status_python_capsule = PyCapsule_New(&status, NULL, NULL);

		} else {
			fprintf (stderr,"Couldnt load the python module %s\n", python_filename) ;
		}
		program.python_helper_function_generator = PyObject_CallFunction(status.program->python_helper_function,"O", status.program->status_python_capsule);
		PyObject_CallMethod(status.program->python_helper_function_generator,"send", "s", NULL);
		printf("Made generator that returned %p\n", program.python_helper_function_generator);
	}

	/* Set up hooks */

	enum return_codes_e rc = hooks_setup(&program);
	if (rc!=R_SUCCESS) {
		fprintf(stderr,"Failed to setup the module hooks, sorry :(\n");
		return R_FAILURE;
	}

	/* Create list of functions to call in level */

	void *(*hookfuncs[])(struct status_struct *status) = {funcptr, funcptr};

	/* ---------------------------------------------------------------------- */

	enum  return_codes_e returncode = startscreen(win, renderer, &status);

	/*	The Main Game Loop */
	//	NOTE: The stack smashing error seemed to occur when the loop just kept looping because it wasn'tr intercepting a particular return code!
	while (1) {

		query_resize(&graphics);

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
		else if ( returncode == R_STARTSCREEN ) {
			returncode = startscreen(win, renderer, &status);
		}
		else if ( returncode == R_PAUSE_LEVEL) {
			returncode = pausefunc(renderer, graphics.texTarget, level.currentlevel, &status);
		}
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

	}

	/* Finish Suspended Python Functions */
	if (status.program->python_interpreter_enable) {
		if (status.program->python_helper_function && PyCallable_Check(status.program->python_helper_function)) {
			PyObject *pResult = PyObject_CallMethod(status.program->python_helper_function_generator,"send", "i", 1);
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

	/* Release SDL resources. */
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(win);
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

	const char *cfg_path = "modules.cfg";
	config_t cfg;
	config_init(&cfg);

	/* Read the file. If there is an error, report it and exit. */
	if(! config_read_file(&cfg, cfg_path))
	{
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
		config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
		return(R_FAILURE);
	}


	config_setting_t *modules_list_setting = config_lookup(&cfg, "modules_list");
	if (modules_list_setting == NULL) {
		fprintf(stderr,"Error looking up setting for 'modules_list'\n");
		return(R_FAILURE);
	}
	int num_modules = config_setting_length(modules_list_setting); 
	printf("Module files to load: %d\n", num_modules);
	for (int i = 0; i < num_modules; i++) {
		config_setting_t *module_setting = config_setting_get_elem(modules_list_setting, i);
		config_setting_t *path_setting = config_setting_get_member(module_setting, "path");
		const char *path = config_setting_get_string(path_setting);
		if (!path) {
			fprintf(stderr,"Could not find valid module file path in entry %d of file %s\n", i, cfg_path);
			return(R_FAILURE);
		}

		config_setting_t *functions_setting = config_setting_get_member(module_setting, "functions");
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
		   return R_FAILURE;
		}

		for (int j = 0; j < num_functions; j++) {
			config_setting_t *function_setting = config_setting_get_elem(functions_setting, j);
			if (!function_setting) {
				fprintf(stderr,"Error looking up setting for function %d in module %d\n", j, i);
				return(R_FAILURE);
			}
			config_setting_t *name_setting = config_setting_get_member(function_setting, "name");
			if (!name_setting) {
				fprintf(stderr,"Error looking up setting for 'name' in function %d in module %d\n", j, i);
				return(R_FAILURE);
			}
			const char *func_name = config_setting_get_string(name_setting);
			if (!func_name) {
				fprintf(stderr,"Error getting value for 'name' in function %d in module %d\n", j, i);
				return(R_FAILURE);
			}

			config_setting_t *exec_location_setting = config_setting_get_member(function_setting, "exec_location");
			if (!exec_location_setting) {
				fprintf(stderr,"Error looking up setting for 'exec_location' in function %d in module %d\n", j, i);
				return(R_FAILURE);
			}
			enum hook_type_e hook_type = config_setting_get_int(exec_location_setting);
			if (hook_type < 0 || hook_type >= NUM_HOOK_LOCATIONS) {
				fprintf(stderr,"Attempting to store hook in execution location that doesn't exist.\n");
				return R_FAILURE;
			}

			/* Finally, after all that we can store the info about the hooks */

			char *error;
			
			funcptr = dlsym(lib_handle, func_name);
			if ((error = dlerror()) != NULL) 
			{
			   fprintf(stderr, "%s\n", error);
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

	return R_SUCCESS;
}

void query_resize(struct graphics_struct *graphics) {

	/* Get (potentially updated) window dimensions */
	int w, h;
	SDL_GetWindowSize(graphics->window, &w, &h);

	if ((w != graphics->width) || (h != graphics->height)) {
		graphics->width = w;
		graphics->height = h;

		/* Resize texTarget to new texture */
		SDL_Texture *texTarget_new = SDL_CreateTexture(graphics->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, graphics->width, graphics->height);
		SDL_Texture *target_prev = SDL_GetRenderTarget(graphics->renderer);
		SDL_SetRenderTarget(graphics->renderer, texTarget_new);

		SDL_RenderClear(graphics->renderer);
		SDL_RenderCopy(graphics->renderer, graphics->texTarget, NULL, NULL);

		SDL_SetRenderTarget(graphics->renderer, target_prev);
		SDL_DestroyTexture(graphics->texTarget);
		graphics->texTarget = texTarget_new;
	}
}