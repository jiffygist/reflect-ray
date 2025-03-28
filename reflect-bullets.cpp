#include <SDL.h>
#include <cmath>

SDL_Window* window;
SDL_Renderer* renderer;
const int FPS = 60;
SDL_Rect world = {0, 0, 60, 80};

struct Trajectory
{
	SDL_FPoint origin;
	float length;
	float angle;

	void draw()
	{
		SDL_FPoint end = {sin(angle) * length, cos(angle) * length};
		SDL_RenderDrawLine(renderer, origin.x, origin.y, origin.x + end.x, origin.y + end.y);
	}
};

void delay_frame(Uint64& cnt_start, Uint64& cnt_end)
{
	float cnt_delta = (float)(cnt_end - cnt_start)/SDL_GetPerformanceFrequency();
	if (cnt_delta < 1.0f/FPS)
	{
		SDL_Delay((1.0f/FPS - cnt_delta)*999);
		cnt_end = SDL_GetPerformanceCounter();
		cnt_delta = (float)(cnt_end - cnt_start)/SDL_GetPerformanceFrequency();
	}
	while (cnt_delta < 1.0f/FPS)
	{
		cnt_end = SDL_GetPerformanceCounter();
		cnt_delta = (float)(cnt_end - cnt_start)/SDL_GetPerformanceFrequency();
	}
}

int main()
{
	Trajectory tr = {{20.0f, 20.0f}, 10.0f, M_PI/4.0f};

	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(600, 800, SDL_WINDOW_RESIZABLE, &window, &renderer);
	SDL_RenderSetLogicalSize(renderer, world.w, world.h);

	Uint64 cnt_end = SDL_GetPerformanceCounter();
	while (1)
	{
		Uint64 cnt_start = cnt_end;

		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				SDL_DestroyWindow(window);
				SDL_DestroyRenderer(renderer);
				SDL_Quit();
				exit(0);
			}
		}
		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
		SDL_RenderFillRect(renderer, &world);
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		tr.draw();
		tr.angle += 0.1;
		SDL_RenderPresent(renderer);

		cnt_end = SDL_GetPerformanceCounter();
		delay_frame(cnt_start, cnt_end);
	}
}