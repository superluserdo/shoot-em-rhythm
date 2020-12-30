#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <assert.h>
#include "structdef_engine.h"
#include "opengl_funcs.h"

//Screen dimension constants
unsigned int texture_shader_simple = 0;
unsigned int texture_shader_white = 0;
unsigned int texture_shader_glow_behind = 0;
unsigned int texture_shader_glow = 0;
struct intrect default_viewport = {0, 0, 640, 640};

void get_texture_size(unsigned int texture, int *w, int *h) {
	int miplevel = 0;
	glBindTexture(GL_TEXTURE_2D, texture);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, h);
}

void change_render_target(struct glrenderer *target_renderer) {
	struct framebuffer *target = target_renderer->framebuffer;
	int framebuffer;
	struct intrect viewport;
	if (target) {
		framebuffer = target->framebuffer;
		viewport = target->viewport;
	} else {
		framebuffer = 0;
		viewport = default_viewport;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glViewport(
			viewport.x,
			viewport.y,
			viewport.w,
			viewport.h);
	//printf("framebuffer %d\n"
	//		"viewport: %d, %d, %d, %d\n",
	//		framebuffer,
	//		viewport.x,
	//		viewport.y,
	//		viewport.w,
	//		viewport.h);
}

void texture_shader_glow_uniforms(unsigned int shader) {
	/* Make sure glUseProgram(shader) is called first! */
	int time_loc = glGetUniformLocation(shader, "time");
	int r_disc_loc = glGetUniformLocation(shader, "r_disc");
	int r_rays_loc = glGetUniformLocation(shader, "r_rays");
	float time = (float)(SDL_GetTicks() )/500;
	//if (time > 1) {
	//	time = 2.0 - time;
	//}
	glUniform1f(time_loc, time);
	glUniform1f(r_disc_loc, 0.58);
	glUniform1f(r_rays_loc, 1.0);
}

unsigned int create_texture_shader(int n_v_snippets, const char **v_snippets, 
		int n_f_snippets, const char **f_snippets) {
	unsigned int vertex, fragment;

	/* Vertex shader */
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, n_v_snippets, v_snippets, NULL);
	glCompileShader(vertex);
	checkCompileErrors(vertex, "VERTEX");
	/* Fragment Shader */
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, n_f_snippets, f_snippets, NULL);
	glCompileShader(fragment);
	checkCompileErrors(fragment, "FRAGMENT");


	/* Full shader */
	unsigned int texture_shader = glCreateProgram();
	glAttachShader(texture_shader, vertex);
	glAttachShader(texture_shader, fragment);


	glLinkProgram(texture_shader);
	checkCompileErrors(texture_shader, "PROGRAM");

	glDeleteShader(vertex);
	glDeleteShader(fragment);
	return texture_shader;
}

int texture_from_path(char *path, struct glrenderer *renderer_UNUSED, unsigned int *texture) {
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture); 
     // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // set texture filtering parameters (GL_NEAREST or GL_LINEAR)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	SDL_Surface* surface = IMG_Load(path);
	if (!surface) {
		printf("no surface\n");
		return 1;
	}
	int Mode1 = GL_RGB;
	if(surface->format->BytesPerPixel == 4) {
		Mode1 = GL_RGBA;
		printf("RGBA surface\n");
	} else {
		printf("RGB surface\n");
	}
	 
	glTexImage2D(GL_TEXTURE_2D, 0, Mode1, surface->w, surface->h, 0, Mode1, GL_UNSIGNED_BYTE, surface->pixels);
    glBindTexture(GL_TEXTURE_2D, 0); //default
	return 0;
}

unsigned int create_texture(struct glrenderer *renderer_unused, int w, int h) {
	unsigned int texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	  
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return texture;
}

struct glrenderer new_glrenderer(int own_framebuffer, 
		struct intrect viewport) {

	unsigned int VAO, VBO, EBO;
	unsigned int framebuffer = 0;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 
			(void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

	/* FRAMEBUFFER STUFF */
	unsigned int fb_texture = 0;
	struct framebuffer *fb_struct = NULL;
	if (own_framebuffer) {

		fb_texture = create_texture(NULL, viewport.w, viewport.h);
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);    

		fb_struct = calloc(sizeof(*fb_struct), 1);
		*fb_struct = (struct framebuffer) {
			.framebuffer = framebuffer,
			.viewport = viewport,
			.texture = fb_texture,
		};

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb_texture, 0);  // attach to currently bound framebuffer object

		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			printf("ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0); /* default */

	}

	struct glrenderer renderer = {
		.VAO = VAO,
		.VBO = VBO,
		.EBO = EBO,
		.framebuffer = fb_struct,
	};
	return renderer;
}

void max_min_n(int n_nums, float *nums, float *max, float *min) {
	if (n_nums == 1) {
		*max = nums[0];
		*min = nums[0];
		return;
	} else if (n_nums == 2) {
		float num1 = nums[0];
		float num2 = nums[1];
		*max = (num1 > num2) ? num1 : num2;
		*min = (num1 < num2) ? num1 : num2;
		return;
	}
	/* Bisect list */
	int n_nums_1 = n_nums/2;
	float max_1, min_1;
	max_min_n(n_nums_1, &nums[0], &max_1, &min_1);

	int n_nums_2 = n_nums - n_nums_1;
	float max_2, min_2;
	max_min_n(n_nums_2, &nums[n_nums_1-1], &max_2, &min_2);

	*max = (max_1 > max_2) ? max_1 : max_2;
	*min = (min_1 < min_2) ? min_1 : min_2;
}

struct globject *add_border_vertices(struct globject *object, struct globject_list *object_list,
		unsigned int texture_shader,
		float frac_up, float frac_left, float frac_down, float frac_right) {

	assert(object->n_vertices == 4);
	float x_max_old, x_min_old, y_max_old, y_min_old;

	max_min_n(object->n_vertices,
			(float []){
			object->vertices[0],
			object->vertices[4],
			object->vertices[8],
			object->vertices[12],
			},
			&x_max_old, &x_min_old);

	max_min_n(object->n_vertices,
			(float []){
			object->vertices[1],
			object->vertices[5],
			object->vertices[9],
			object->vertices[13],
			},
			&y_max_old, &y_min_old);

	float w_prev = x_max_old - x_min_old;
	float h_prev = y_max_old - y_min_old;

	/* New bounding box vertices */
	float x_max_new, x_min_new, y_max_new, y_min_new;
	x_min_new = x_min_old - frac_left * w_prev;
	y_min_new = y_min_old - frac_down * h_prev;
	x_max_new = x_max_old + frac_right * w_prev;
	y_max_new = y_max_old + frac_up * h_prev;

	float xs[4] = {x_min_new, x_min_old, x_max_old, x_max_new};
	float ys[4] = {y_min_new, y_min_old, y_max_old, y_max_new};
	float tex_xs[4] = {-frac_left, 0, 1, 1+frac_right};
	float tex_ys[4] = {-frac_down, 0, 1, 1+frac_up};

    float *vertices = calloc(sizeof(*vertices),4*4*4);
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			vertices[4*(i*4+j)] = xs[i];
			vertices[4*(i*4+j)+1] = ys[j];
			vertices[4*(i*4+j)+2] = tex_xs[i];
			vertices[4*(i*4+j)+3] = tex_ys[3-j];
		}
	}

#define INDICES(XMIN,YMIN,XMAX,YMAX) \
	XMAX*4+YMIN, XMAX*4+YMAX, XMIN*4+YMIN,\
	XMAX*4+YMAX, XMIN*4+YMAX, XMIN*4+YMIN

    unsigned int *indices = calloc(sizeof(*indices),4*4*3*2);
    unsigned int indices_stack[8*3*2] = {
		INDICES(0,0,1,1),
		INDICES(1,0,2,1),
		INDICES(2,0,3,1),

		INDICES(0,1,1,2),
		//INDICES(1,1,2,2), //Skip this middle one
		INDICES(2,1,3,2),

		INDICES(0,2,1,3),
		INDICES(1,2,2,3),
		INDICES(2,2,3,3),
	};

	memcpy(indices, indices_stack, sizeof(indices_stack));

	struct globject *object_new = new_object(object_list);
	object_new->vertices = vertices;
	object_new->n_vertices = 4*4;
	object_new->indices = indices;
	object_new->n_indices = 8*6;
	object_new->texture_shader = texture_shader;

	insert_object_after(object_list, object_new, object);

	return object_new;
}

void update_quad_vertices_sdl(struct floatrect rect, struct globject *object) {
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
		// Have to load textures "upside down" since SDL and openGL
		// have opposite y conventions
        /*
		x positions             y positions              texture coords (SDL)
		*/
        -1.0+2*(rect.x+rect.w), 1.0-2*(rect.y+rect.h),   1.0f, 1.0f, // bottom right
        -1.0+2*(rect.x+rect.w), 1.0-2*(rect.y),          1.0f, 0.0f, // top right
        -1.0+2*(rect.x),        1.0-2*(rect.y),          0.0f, 0.0f, // top left
        -1.0+2*(rect.x),        1.0-2*(rect.y+rect.h),   0.0f, 1.0f  // bottom left 
        // 0.5f,  0.5f,    1.0f, 0.0f, // bottom right
        // 0.5f, -0.5f,    1.0f, 1.0f, // top right
        //-0.5f, -0.5f,    0.0f, 1.0f, // top left
        //-0.5f,  0.5f,    0.0f, 0.0f  // bottom left 
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

	object->n_vertices = 4;
	object->vertices = realloc(object->vertices, sizeof(float) * 4*4);
	object->n_indices = 6;
	object->indices = realloc(object->indices, sizeof(unsigned int) * 3*2);

	memcpy(object->vertices, vertices, sizeof(vertices));
	memcpy(object->indices, indices, sizeof(indices));
}

void update_quad_vertices_opengl(struct floatrect rect, struct globject *object) {
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
		// Have to load textures "upside down" since SDL and openGL
		// have opposite y conventions
        /*
		x positions             y positions              texture coords (opengl)
		*/
        -1.0+2*(rect.x+rect.w), 1.0-2*(rect.y+rect.h),   1.0f, 0.0f, // bottom right
        -1.0+2*(rect.x+rect.w), 1.0-2*(rect.y),          1.0f, 1.0f, // top right
        -1.0+2*(rect.x),        1.0-2*(rect.y),          0.0f, 1.0f, // top left
        -1.0+2*(rect.x),        1.0-2*(rect.y+rect.h),   0.0f, 0.0f  // bottom left 
        // 0.5f,  0.5f,    1.0f, 0.0f, // bottom right
        // 0.5f, -0.5f,    1.0f, 1.0f, // top right
        //-0.5f, -0.5f,    0.0f, 1.0f, // top left
        //-0.5f,  0.5f,    0.0f, 0.0f  // bottom left 
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

	object->n_vertices = 4;
	object->vertices = realloc(object->vertices, sizeof(float) * 4*4);
	object->n_indices = 6;
	object->indices = realloc(object->indices, sizeof(unsigned int) * 3*2);

	memcpy(object->vertices, vertices, sizeof(vertices));
	memcpy(object->indices, indices, sizeof(indices));
}

void insert_object_after(struct globject_list *object_list, struct globject *object, struct globject *target) {
	/* Add object to end of list */
	if (!object_list->head) {
		object_list->head = object;
	}
	if (!target) {
		target = object_list->tail;
	}
	if (target) {
		target->next = object;
	}
	if (target == object_list->tail) {
		object_list->tail = object;
	}
}

struct globject *new_object(struct globject_list *object_list) {
	struct globject *object = calloc(sizeof(*object), 1);

	*object = (struct globject) {0};

	insert_object_after(object_list, object, NULL);

	return object;
}

struct globject *new_quad(struct floatrect rect, unsigned int texture, unsigned int texture_shader, struct globject_list *object_list, int sdl_coords) {

	struct globject *object = new_object(object_list);

	object->texture = texture;
	object->texture_shader = texture_shader;

	if (sdl_coords) {
		update_quad_vertices_sdl(rect, object);
	} else {
		update_quad_vertices_opengl(rect, object);
	}

	return object;
}

int init_sdl_opengl(struct graphics_struct *master_graphics) {

	printf("This is a test!\n");

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		exit(1);
	}
	//Use OpenGL 3.1 core
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );


	//Create window
	SDL_Window *window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, 
			SDL_WINDOWPOS_UNDEFINED, master_graphics->width, master_graphics->height, 
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE );
	if( window == NULL )
	{
		printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
		exit(1);
	}
	master_graphics->window = window;
	//OpenGL context
	SDL_GLContext gContext;

	//Create context
	gContext = SDL_GL_CreateContext( window );
	if( gContext == NULL )
	{
		printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
		exit(1);
	}
	//Initialize GLEW
	glewExperimental = GL_TRUE; 
	GLenum glewError = glewInit();
	if( glewError != GLEW_OK )
	{
		printf( "Error initializing GLEW! %s\n", glewGetErrorString( glewError ) );
	}

	//Use Vsync
	if( SDL_GL_SetSwapInterval( 1 ) < 0 )
	{
		printf( "Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError() );
	}

	return 0;
}

void initGL(void)
{
	/* Enable transparency */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);

	const char *vShaderCode = {
		"#version 330 core\n"
		"layout (location = 0) in vec2 aPos;\n"
		"layout (location = 1) in vec2 aTexCoord;\n"

		"out vec2 TexCoord;\n"

		"void main()\n"
		"{\n"
		"	gl_Position = vec4(aPos, 0.0, 1.0);\n"
		"	TexCoord = aTexCoord;\n"
		"}"
	};
	//const char *fShaderCode_mix = {
	//	"#version 330 core\n"
	//	"out vec4 FragColor;\n"

	//	"in vec2 TexCoord;\n"

	//	"// texture samplers\n"
	//	"uniform sampler2D texture1;\n"
	//	"uniform sampler2D texture2;\n"

	//	"void main()\n"
	//	"{\n"
	//	"	// linearly interpolate between both textures (80% container, 20% awesomeface)\n"
	//	"	FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);\n"
	//	"}"
	//};

	const char *fShaderCode_animate_behind_texture = {
		"#version 330 core\n"
		"out vec4 FragColor;\n"

		"in vec2 TexCoord;\n"
		"in vec4 gl_FragCoord;"

		"uniform sampler2D screentexture;\n"
		"uniform float time;\n"

		"vec4 shader_animated(vec2 TexCoord, float time);\n"
		"void main()\n"
		"{ \n"
		"	vec4 InColor = texture(screentexture, TexCoord);\n"
		"	if (InColor.w < 1) {\n"
		"		FragColor = shader_animated(TexCoord, time);\n"
		"		if (InColor.w > 0) {\n"
		"			// Must combine both alpha textures properly:\n"
		"			float newalpha = InColor.w + FragColor.w*(1-InColor.w);\n"
		"			FragColor = vec4(\n"
		"				(InColor.xyz*InColor.w + FragColor.xyz*FragColor.w*(1-InColor.w))/newalpha,\n"
		"				newalpha);\n"
		"		}\n"
		"	} else {\n"
		"		FragColor = InColor;\n"
		"	}\n"
		"}"
	};

	const char *fShaderCode_animated = {
		"#version 330 core\n"
		"out vec4 FragColor;\n"

		"in vec2 TexCoord;\n"
		"in vec4 gl_FragCoord;"

		"uniform sampler2D screentexture;\n"
		"uniform float time;\n"

		"vec4 shader_animated(vec2 TexCoord, float time);\n"
		"void main()\n"
		"{ \n"
		"	FragColor = shader_animated(TexCoord, time);\n"
		"}"
	};

	const char *fShaderCode_glow_animated = {

		"uniform float r_disc;\n"
		"uniform float r_rays;\n"

		"vec4 shader_animated(vec2 TexCoord, float time)\n"
		"{ \n"
		"	float dist = sqrt(pow((0.5-TexCoord.x),2) + pow((0.5-TexCoord.y),2));\n"
		"\n"
		"	float edge = r_disc*(1-0.15+0.15*sin(time));// * time*0.5;\n"
		"	float angle = atan((0.5-TexCoord.y) / (0.5-TexCoord.x));\n"
		"	angle = angle * 10 +mod(time*8, 2*3.14159);\n"
		"	float i = dist / edge;\n"
		"	i = (i < 1) ? pow(i,1) : pow(2,-(i-1)*10);\n"
		"	float j = 0.5*(1+sin(angle));\n"
		"	j *= j;\n"
		"	j = (dist > edge) ? j : 0;\n"
		"	j = (dist > r_rays) ? 0 : j;\n"
		"	if (dist > edge && dist < 1) {\n"
		"		float cutoff_mult = 1 - ((dist - edge)/(r_rays - edge));\n"
		"		i *= cutoff_mult;\n"
		"		j *= cutoff_mult;\n"
		"	}\n"
		"	i = (dist > r_rays) ? 0 : i;\n"
		"	float k = i + j;\n"
		"	k = (i+j/2);\n"
		"	return vec4(1, 1, 1, k);\n"
		"	//return vec4(TexCoord.x, TexCoord.y, 0, 1);\n"
		"}"
	};

	const char *fShaderCode_white = {
		"#version 330 core\n"
		"out vec4 FragColor;\n"

		"in vec2 TexCoord;\n"
		"in vec4 gl_FragCoord;"

		"uniform sampler2D screentexture;\n"

		"void main()\n"
		"{ \n"
		"	FragColor = vec4(1, 1, 1, 1);\n"
		"}"
	};

	const char *fShaderCode = {
		"#version 330 core\n"
		"out vec4 FragColor;\n"

		"in vec2 TexCoord;\n"
		"in vec4 gl_FragCoord;"

		"uniform sampler2D screentexture;\n"

		"void main()\n"
		"{ \n"
		"	FragColor = texture(screentexture, TexCoord);\n"
		"	//FragColor = vec4(0);\n"
		"}"
	};
	texture_shader_simple = create_texture_shader(1, &vShaderCode, 1, &fShaderCode);
	texture_shader_white = create_texture_shader(1, &vShaderCode, 1, &fShaderCode_white);
	texture_shader_glow_behind = create_texture_shader(1, &vShaderCode, 2, (const char *[]){fShaderCode_animate_behind_texture, fShaderCode_glow_animated});
	texture_shader_glow = create_texture_shader(1, &vShaderCode, 2, (const char *[]){fShaderCode_animated, fShaderCode_glow_animated});
}

int graphics_init(struct graphics_struct *master_graphics) {
	if (init_sdl_opengl(master_graphics)) {
		return 1;
	}
	if (init_sdl_opengl(master_graphics)) {
		return 1;
	}
	struct glrenderer *fb_renderer = make_renderer(NULL, 0, 
			(float []){0.8f, 0.8f, 0.3f, 1.0f}, 1, 
			(struct intrect){0, 0, master_graphics->width, master_graphics->height});
	
	struct glrenderer *renderer = make_renderer("window.png", fb_renderer->framebuffer, (float []){0.2f, 0.3f, 0.3f, 1.0f}, 0, (struct intrect){0});

	if( !renderer || !fb_renderer)
	{
		printf( "Unable to initialize OpenGL!\n" );
		exit(1);
	}
	master_graphics->renderer = renderer;
	return 0;
}

struct glrenderer *make_renderer(char *texture_path, 
		struct framebuffer *render_target, float clear_colour[4],
		int own_framebuffer, struct intrect viewport) {

	struct glrenderer *renderer = calloc(sizeof(*renderer), 1);
	*renderer = new_glrenderer(own_framebuffer, viewport);
	renderer->render_target = render_target;
	memcpy(renderer->clear_colour, clear_colour, sizeof(float)*4);

	struct globject_list *object_list = &renderer->object_list;

	struct floatrect rect_bg = {.x=0.35, .y=0.35, .w = 0.3, .h=0.3};
	struct floatrect rect = {.x=0.0, .y=0.0, .w = 0.5, .h=0.5};

	/* Get texture */
	if (texture_path) {
		unsigned int texture_hamster;
		texture_from_path(texture_path, NULL, &texture_hamster);
		struct globject *bg_hamster = new_quad(rect_bg, texture_hamster, texture_shader_glow_behind, object_list, 1);
		bg_hamster->uniforms = texture_shader_glow_uniforms;
		struct globject *border = add_border_vertices(bg_hamster, object_list,
			texture_shader_glow, 0.5, 0.5, 0.5, 0.5);
		border->uniforms = texture_shader_glow_uniforms;

		new_quad(rect, texture_hamster, texture_shader_simple, object_list, 1);
	} else {
		struct floatrect quad_rect = {
			.x=0.1, .y=0.1, .w=0.8, .h=0.8
		};
		new_quad(quad_rect, renderer->framebuffer->texture, texture_shader_simple, object_list, 0);
	}

    //float vertices_quad[] = {
    //    // positions     // texture coords
    //     0.8f,  0.8f,    1.0f, 1.0f, // top right
    //     0.8f, -0.8f,    1.0f, 0.0f, // bottom right
    //    -0.8f, -0.8f,    0.0f, 0.0f, // bottom left
    //    -0.8f,  0.8f,    0.0f, 1.0f  // top left 
    //};
    //unsigned int indices_quad[] = {
    //    0, 1, 3, // first triangle
    //    1, 2, 3  // second triangle
    //};

	///* Quad texture to be rendered to the screen */
    //glGenVertexArrays(1, VAO_quad);
    //glGenBuffers(1, VBO_quad);
    //glGenBuffers(1, EBO_quad);

    //glBindVertexArray(*VAO_quad);

    //glBindBuffer(GL_ARRAY_BUFFER, *VBO_quad);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_quad), vertices_quad, GL_STATIC_DRAW);

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBO_quad);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_quad), indices_quad, GL_STATIC_DRAW);

    //// position attribute
    //glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    //glEnableVertexAttribArray(0);
    //// texture coord attribute
    //glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 
	//		(void*)(2 * sizeof(float)));
    //glEnableVertexAttribArray(1);

	return renderer;
}

void clear_render_target(struct graphics_struct *master_graphics) {
	float *clear_colour = master_graphics->renderer->clear_colour;
	glClearColor(
			clear_colour[0],
			clear_colour[1],
			clear_colour[2],
			clear_colour[3]);

	glClear(GL_COLOR_BUFFER_BIT);
}

void render_copy(struct globject *object, struct glrenderer *renderer) {
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * object->n_vertices, object->vertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * object->n_indices, object->indices, GL_STATIC_DRAW);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, object->texture);

	if (renderer->do_wireframe) {
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glUseProgram(texture_shader_white); 
	} else {
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		glUseProgram(object->texture_shader); 
		if (object->uniforms) {
			object->uniforms(object->texture_shader);
		}
	}
	glBindVertexArray(renderer->VAO);
	glDrawElements(GL_TRIANGLES, object->n_indices, GL_UNSIGNED_INT, 0);
}

void render(struct graphics_struct *master_graphics)
{

	struct glrenderer *renderer = master_graphics->renderer;
    glBindVertexArray(renderer->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->EBO);


	change_render_target(master_graphics->renderer);

	clear_render_target(master_graphics);

	/* Face culling */
	//glFrontFace(GL_CCW);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);

	struct globject *object = renderer->object_list.head;
	while (object) {

		render_copy(object, renderer);
		object = object->next;
	}
}

void checkCompileErrors(GLuint shader, const char *type)
{
    // Check shader for errors
	GLint Result = GL_FALSE;
	int InfoLogLength = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0)
    {
        char Error[1000];
        glGetShaderInfoLog(shader, InfoLogLength, NULL, Error);
        printf("%s\n", Error);
		exit(1);
    }
}

//TODO: can prob get rid of these
void printProgramLog( GLuint program )
{
	//Make sure name is shader
	if( glIsProgram( program ) )
	{
		//Program log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;
		
		//Get info string length
		glGetProgramiv( program, GL_INFO_LOG_LENGTH, &maxLength );
		
		//Allocate string
		printf("mallocing %d bytes\n", maxLength);
		char *infoLog = (char *)malloc(maxLength);
		
		//Get info log
		glGetProgramInfoLog( program, maxLength, &infoLogLength, infoLog );
		if( infoLogLength > 0 )
		{
			//Print Log
			printf( "%s\n", infoLog );
		}
	}
	else
	{
		printf( "Name %d is not a program\n", program );
	}
}

void printShaderLog( GLuint shader )
{
	//Make sure name is shader
	if( glIsShader( shader ) )
	{
		//Shader log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;
		
		//Get info string length
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &maxLength );
		
		//Allocate string
		printf("mallocing %d bytes\n", maxLength);
		char *infoLog = (char *)malloc(maxLength);
		
		//Get info log
		glGetShaderInfoLog( shader, maxLength, &infoLogLength, infoLog );
		if( infoLogLength > 0 )
		{
			//Print Log
			printf( "%s\n", infoLog );
		}
	}
	else
	{
		printf( "Name %d is not a shader\n", shader );
	}
}

void query_resize(struct graphics_struct *master_graphics, texture_t *tex_target_ptr) {
	/* Get (potentially updated) window dimensions */
	int w, h;
	SDL_GetWindowSize(master_graphics->window, &w, &h);

	if (((w != master_graphics->width) || (h != master_graphics->height))) {
		//printf("New size!\n"
		//		"orig viewport: w=%d, h=%d\n"
		//		"orig tex: w=%d, h=%d\n"
		//		"new size:  w=%d, h=%d\n",
		//		master_graphics->width,
		//		master_graphics->height,
		//		master_graphics->renderer->framebuffer->viewport.w,
		//		master_graphics->renderer->framebuffer->viewport.h,
		//		w, h);
		master_graphics->width = w;
		master_graphics->height = h;

		default_viewport.w = w;
		default_viewport.h = h;

		/* If rendering to a target (not screen), resize texTarget to new texture */
		glDeleteTextures(1, &master_graphics->renderer->framebuffer->texture);

		unsigned int new_fb_texture;
		glGenTextures(1, &new_fb_texture);
		glBindTexture(GL_TEXTURE_2D, new_fb_texture);
		master_graphics->renderer->framebuffer->texture = new_fb_texture;
		master_graphics->renderer->object_list.head->texture = new_fb_texture;
		  
		master_graphics->renderer->framebuffer->viewport.w = w;
		master_graphics->renderer->framebuffer->viewport.h = h;
		change_render_target(master_graphics->renderer);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, new_fb_texture, 0);  // attach to currently bound framebuffer object
		glBindTexture(GL_TEXTURE_2D, 0);

	}
}

void present_screen(struct time_struct timing, struct graphics_struct *master_graphics) {
	//Update screen
	SDL_GL_SwapWindow( master_graphics->window );
}

void graphics_deinit(struct graphics_struct *master_graphics) {
	//TODO
}
