#include "8080.h"
#include <chrono>

chrono::high_resolution_clock Clock;

int main(int argc, char* argv[])
{
	sdlInit();
	SDL_Event e;
	bool quit = false;
	reg* state = init();
	state->pc = 0;
	
	auto lastInterrupt = Clock.now();
	int interruptnum = 1;


	while (!quit)
	{
		auto time = Clock.now();
		if(chrono::duration_cast<chrono::seconds>(time - lastInterrupt).count() > 5)
		{
			if(state->int_enable)
			{
				lastInterrupt = Clock.now();
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
				SDL_RenderClear(renderer);
				interrupt(state,interruptnum);
				
				if(interruptnum == 2)
				{
					interruptnum-=2;
				}
				interruptnum++;
				
			}
		}
		emulate(state);
		while(SDL_PollEvent(&e) > 0)
		{
			if(e.type == SDL_QUIT)
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