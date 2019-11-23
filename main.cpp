#include "8080.h"


int main(int argc, char* argv[])
{
	sdlHandle();
	SDL_Event e;
	bool quit = false;

	reg* state = init();
	const char* file1 = "invaders.txt";
	loadmem(state, file1, 0);
	state->pc = 0;
	
	

	while (!quit)
	{
		emulate(state);
		uint16_t mem = 0x2400;
		uint8_t *byte = &state->memory[mem];
		int it = 8;
		SDL_FreeSurface(screenSurface);

		for(int x = 0; x <= screenWidth; x++)
		{
			
			for(int y = 0; y <= screenHeight; y++)
			{
				if(*byte & (1 << (it-1)) == (1 << (it-1)))
				{
					SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
					SDL_RenderDrawPoint(renderer, x, y);
				}
				else
				{
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
					SDL_RenderDrawPoint(renderer, x, y);
				}
				
				
				
				it--;
				if(it == 0)
				{
					mem++;
					it = 8;
				}
			}
		}
		SDL_RenderPresent(renderer);
		
		
		
		
		
		
		
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
		}
	}
	return 1;

}

/*
SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
SDL_RenderDrawPoint(renderer, 100, 100);
SDL_RenderPresent(renderer);
*/