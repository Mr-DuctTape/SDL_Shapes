#include <SDL3/SDL.h>
#include "SDL_Shapes.h"
#include <Windows.h>
#include <iostream>

int main()
{
	SDL_Renderer* renderer = nullptr;
	SDL_Window* window = nullptr;

	window = SDL_CreateWindow("2D SDL Shapes Library", 1280, 720, SDL_WINDOW_RESIZABLE);

	renderer = SDL_CreateRenderer(window, NULL);

	SDL_Color color{ 255, 25, 255, 255 };


	int total = 100000;
	int perRow = 450;
	double spacing = 2.0;
	SDL_Shapes::SDL_Circle circ0{ 600, 300, 50 };
	SDL_Shapes::SDL_DrawShape(renderer, &circ0);

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

		SDL_Shapes::SDL_Circle c{ x, y, 1.0, color };
		circles.push_back(c);
	}

	srand(time(0));


	bool running = true;
	SDL_Event event;

	std::cout << circles.size() << "\n";

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

		for (auto& c : circles)
		{
			c.positionX += rand() % 2;
			c.positionY += rand() % 2;
		}

		SDL_Shapes::SDL_DrawAllShapes(renderer, &circles);
		
		SDL_RenderPresent(renderer);
	}

	SDL_Shapes::SDL_CleanTextureCache();

	return 0;
}