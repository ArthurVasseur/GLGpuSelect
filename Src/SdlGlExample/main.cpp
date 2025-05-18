/* rectangles_opengl.c ... */

/*
 * This example creates an SDL window and an OpenGL-backed renderer,
 * then draws some rectangles to it every frame using the SDL 2D Render API.
 *
 * This code is public domain. Feel free to use it for any purpose!
 */

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <vector>
#include <Windows.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

 /* We will use this renderer to draw into this window every frame. */
static SDL_Window* window = nullptr;


#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
	int* breakOnValueChange = (int*)(reinterpret_cast<intptr_t>(NtCurrentTeb()) + 0x68);
	SDL_SetAppMetadata("Example OpenGL Renderer Rectangles", "1.0", "com.example.renderer-rectangles");
	auto r = SDL_SetHint(SDL_HINT_LOGGING, "*=verbose");

	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
	

	/* Prefer the OpenGL render driver for the 2D API */
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");

	/* Create an OpenGL-compatible window */
	window = SDL_CreateWindow(
		"examples/renderer/rectangles",
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		SDL_WINDOW_OPENGL
	);
	if (!window) {
		SDL_Log("Couldn't create window: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	auto context1 = SDL_GL_CreateContext(window);
	auto context2 = SDL_GL_CreateContext(window);

	auto func = SDL_GL_GetProcAddress("wglShareLists");
	if (!window) {
		SDL_Log("Couldn't create context: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
	if (event->type == SDL_EVENT_QUIT) {
		return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
	}
	return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void* appstate)
{
	
	return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
	/* SDL cleans up window/renderer for us */
}
