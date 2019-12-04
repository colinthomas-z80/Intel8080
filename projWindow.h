#pragma once
SDL_Window *sdlWindow = NULL;
SDL_Surface *screen = NULL;
SDL_Surface *frame = NULL;




SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;

const int screenWidth = 256;
const int screenHeight = 224;

bool sdlInit()
{
	

	//Create Window and Renderer
	sdlWindow = SDL_CreateWindow("INTEL 8080 EMU", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth,
	screenHeight, SDL_WINDOW_SHOWN);

	screen = SDL_GetWindowSurface(sdlWindow);
	
	renderer = SDL_CreateRenderer(sdlWindow, 0, 0);
	texture = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGB332,
	SDL_TEXTUREACCESS_STATIC,screenWidth,screenHeight);

	
	return true;

}

