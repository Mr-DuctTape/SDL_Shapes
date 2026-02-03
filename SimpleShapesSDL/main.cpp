#include <SDL3/SDL.h>
#include "SDL_Shapes.h"
#include <Windows.h>

int main()
{
	SDL_Renderer* renderer = nullptr;
	SDL_Window* window = nullptr;

	SDL_CreateWindowAndRenderer("Test123", 1280, 720, SDL_WINDOW_BORDERLESS, &window, &renderer);

	SDL_Color color{ 255, 220, 200, 255 };
	SDL_Color color2{ 200, 140, 155, 255 };
	SDL_Color color3{ 100, 50, 100, 255 };

	SDL_Shapes::SDL_Circle circle(620.0, 200.0, 100, color);
	SDL_Shapes::SDL_Circle circle1(220.0, 200.0, 200, color3);
	SDL_Shapes::SDL_Circle circle2(920.0, 200.0, 70, color2);

	SDL_Shapes::SDL_Rectangle rectangle(400, 400, 100, 50, color);
	SDL_Shapes::SDL_Rectangle rectangle1(400, 400, 100, 50, color);

	bool running = true;
	SDL_Event event;

	while (running)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
				running = false;
			if (event.type == SDL_EVENT_KEY_DOWN)
			{
				running = false;
			}
		}
		SDL_RenderClear(renderer);

		SDL_Shapes::SDL_DrawShape(renderer, circle);
		SDL_Shapes::SDL_DrawShape(renderer, circle1);
		SDL_Shapes::SDL_DrawShape(renderer, circle2);

		SDL_Shapes::SDL_DrawShape(renderer, rectangle);
		SDL_Shapes::SDL_DrawShape(renderer, rectangle1);

		SDL_RenderPresent(renderer);
	}

	SDL_Shapes::SDL_CleanTextureCache();

	return 0;
}