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
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;

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

	int numRenderDriver = SDL_GetNumRenderDrivers();
	std::vector<const char*> drivers;
	drivers.resize(numRenderDriver);

	for (int i = 0; i < numRenderDriver; ++i)
	{
		drivers[i] = SDL_GetRenderDriver(i);
	}

	/* Create a renderer explicitly using the "opengl" driver */
	renderer = SDL_CreateRenderer(window, "opengl");
	if (!renderer) {
		SDL_Log("Couldn't create OpenGL renderer: %s", SDL_GetError());
		SDL_DestroyWindow(window);
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
	SDL_FRect rects[16];
	const Uint64 now = SDL_GetTicks();
	int i;

	/* rectangles grow and shrink over time */
	const float direction = ((now % 2000) >= 1000) ? 1.0f : -1.0f;
	const float scale = (((int)(now % 1000) - 500) / 500.0f) * direction;

	/* clear to black */
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);

	/* single red outline rectangle */
	rects[0].x = rects[0].y = 100;
	rects[0].w = rects[0].h = 100 + (100 * scale);
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderRect(renderer, &rects[0]);

	/* three green centered outline rectangles */
	for (i = 0; i < 3; i++) {
		float size = (i + 1) * 50.0f;
		rects[i].w = rects[i].h = size + (size * scale);
		rects[i].x = (WINDOW_WIDTH - rects[i].w) / 2;
		rects[i].y = (WINDOW_HEIGHT - rects[i].h) / 2;
	}
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderRects(renderer, rects, 3);

	/* blue filled rectangle */
	rects[0].x = 400;
	rects[0].y = 50;
	rects[0].w = 100 + (100 * scale);
	rects[0].h = 50 + (50 * scale);
	SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &rects[0]);

	/* white filled bar chart */
	for (i = 0; i < SDL_arraysize(rects); i++) {
		float w = (float)(WINDOW_WIDTH / SDL_arraysize(rects));
		float h = i * 8.0f;
		rects[i].x = i * w;
		rects[i].y = WINDOW_HEIGHT - h;
		rects[i].w = w;
		rects[i].h = h;
	}
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRects(renderer, rects, SDL_arraysize(rects));

	SDL_RenderPresent(renderer);
	return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
	/* SDL cleans up window/renderer for us */
}
