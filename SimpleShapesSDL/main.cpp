#include <SDL3/SDL.h>
#include "SDL_Shapes.h"
#include <Windows.h>
#include <iostream>

constexpr int winx = 1280, winy = 720;

int main()
{
	SDL_Renderer* renderer = nullptr;
	SDL_Window* window = nullptr;

	window = SDL_CreateWindow("2D SDL Shapes Library", 1280, 920, SDL_WINDOW_RESIZABLE);

	renderer = SDL_CreateRenderer(window, NULL);

	SDL_Color color{ 255, 25, 255, 255 };

	int total = 200000;
	int perRow = 500;
	double spacing = 2.0;


	SDL_Shapes::SDL_Rectangle trg{ 600, 300, 100,100 };
	SDL_Shapes::SDL_Circle circ{ 400, 200, 100 };

	std::vector<SDL_Shapes::SDL_Rectangle> rectang;

	//rectang.push_back(trg);

	srand(time(0));

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

		SDL_Shapes::SDL_Rectangle c{ x,y, 1, 1 };
		rectang.push_back(c);
	}

	std::cout << rectang.size() << "\n";


	bool running = true;
	SDL_Event event;

	int Render_Resolution_Size = SDL_Shapes::Render_Resolution_Size;

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

		for (auto& tr : rectang)
		{
			tr.rotation++;
		}

		SDL_Shapes::SDL_DrawAllShapes(renderer, nullptr, &rectang, nullptr);

		SDL_RenderPresent(renderer);
	}

	SDL_Shapes::SDL_CleanTextureCache();

	return 0;
}