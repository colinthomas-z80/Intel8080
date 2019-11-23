#pragma once
SDL_Surface *ourImage = NULL;
SDL_Window *sdlWindow = NULL;
SDL_Surface *screenSurface = NULL;
SDL_Renderer *renderer = NULL;

int screenWidth = 256;  // Space invaders specific values
int screenHeight = 224;

bool sdlInit()
{
	//Init SDL
	SDL_Init(SDL_INIT_VIDEO);

	

	//Create Window and Renderer
	sdlWindow = SDL_CreateWindow("INTEL 8080 EMU", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth,
	screenHeight, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(sdlWindow, 0, 0);
	

	screenSurface = SDL_GetWindowSurface(sdlWindow);
	return true;

}

bool sdlHandle()
{
	sdlInit();
	
	return true;
}