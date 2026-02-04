#include <SDL3/SDL.h>
#include "SDL_Shapes.h"
#include <Windows.h>
#include <iostream>

int main()
{
	SDL_Renderer* renderer = nullptr;
	SDL_Window* window = nullptr;

	SDL_CreateWindowAndRenderer("2D SDL Shapes Library", 1280, 720, SDL_WINDOW_RESIZABLE, &window, &renderer);

	SDL_Color color{ 255, 0, 255, 255 };


	int total = 55000;
	int perRow = 300;
	double spacing = 10.0;

	srand(time(0));
	std::vector<SDL_Shapes::SDL_Circle> circles;

	for (int i = 0; i < total; i++)
	{
		int row = i / perRow;
		int col = i % perRow;

		double x = col * spacing;
		double y = row * spacing;

		int r = rand() % 255;
		int g = rand() % 255;
		int b = rand() % 255;

		int a = 255;
		SDL_Color color{ r,g,b,a };

		circles.emplace_back(x, y, 4, color);
	}


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

		for(auto& c : circles)
			SDL_Shapes::SDL_DrawShape(renderer, c);
		
		SDL_RenderPresent(renderer);
	}

	SDL_Shapes::SDL_CleanTextureCache();

	return 0;
}