#include <SDL3/SDL.h>
#include "SDL_Shapes.h"

constexpr int winx = 1280, winy = 720;

int main()
{
	SDL_Renderer* renderer = nullptr;
	SDL_Window* window = nullptr;

	window = SDL_CreateWindow("2D SDL Shapes Library", 1280, 920, SDL_WINDOW_RESIZABLE);

	renderer = SDL_CreateRenderer(window, NULL);

	int total = 10000;
	int perRow = 150;
	double spacing = 7.0;

	std::vector<SDL_Shapes::SDL_Triangle> triangles;

	for (int i = 0; i < total; i++)
	{
		int row = i / perRow;
		int col = i % perRow;

		float x = col * spacing;
		float y = row * spacing;

		int r = x;
		int g = y;
		int b = x;

		int a = 255;
		SDL_Color color{ r,g,b,a };

		SDL_Shapes::SDL_Triangle c{ x, y, 5, 5, color };
		triangles.push_back(c);
	}

	bool running = true;
	SDL_Event event;

	SDL_Shapes::SDL_Circle circle{ 300, 300, 50 };
	SDL_Shapes::SDL_Rectangle rectangle{ 500, 300, 100, 100 };
	SDL_Shapes::SDL_Triangle triangle{ 200, 100, 700, 200 };

	float t = 0;

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

		int x;
		int y; 

		SDL_GetWindowSize(window, &x, &y);

		for (auto& t : triangles)
		{
			if (t.positionY > y)
				t.positionY = 0;
			if (t.positionX > x)
				t.positionX = 0;

			t.positionX += rand() % 2;
			t.positionY += rand() % 2;
			t.rotation++;
		}

		rectangle.rotation++;
		triangle.rotation++;

		float r = (sin(t) * 0.5f + 0.5f) * 255.0f;
		float g = (sin(t + 2.094f) * 0.5f + 0.5f) * 255.0f; 
		float b = (sin(t + 4.188f) * 0.5f + 0.5f) * 255.0f; 

		t += 0.01;

		SDL_Color c{
			(Uint8)r,
			(Uint8)g,
			(Uint8)b,
			255
		};

		rectangle.set_color(c);
		circle.set_color(c);
		triangle.set_color(c);

		SDL_Shapes::SDL_DrawShape(renderer, triangle);
		SDL_Shapes::SDL_DrawShape(renderer, rectangle);
		SDL_Shapes::SDL_DrawShape(renderer, circle);

		SDL_Shapes::SDL_DrawAllShapes(renderer, nullptr, nullptr, &triangles);

		SDL_RenderPresent(renderer);
	}

	SDL_Shapes::SDL_CleanTextureCache();

	return 0;
}