#include "SDL_Shapes.h"
#include <iostream>
#include <map>
#include <Windows.h>

std::vector<SDL_Shapes::CircleKey> SDL_Shapes::circleTextures;
std::vector<SDL_Shapes::RectangleKey> SDL_Shapes::rectangleTextures;

namespace SDL_Shapes
{
	void SDL_DrawShape(SDL_Renderer* renderer, SDL_Shape& shape)
	{
		shape.RenderShape(renderer);
	}

	void SDL_DrawShape(SDL_Renderer* renderer, SDL_Shape* shape)
	{
		if (!shape)
			throw std::runtime_error("Shape parameter pointer is NULL");

		shape->RenderShape(renderer);
	}

	void SDL_Shape::RenderShape(SDL_Renderer* renderer)
	{

	}

	bool SDL_Shape::Shape_CompareColors(SDL_Color other1, SDL_Color other2)
	{
		if (other1.r == other2.r && 
			other1.g == other2.g && 
			other1.b == other2.b && 
			other1.a == other2.a)
			return true;

		return false;
	}

	void SDL_Circle::RenderShape(SDL_Renderer* renderer)
	{
		//Check if texture already exists
		for (auto& c : circleTextures)
		{
			if (Shape_CompareColors(color, c.color) && c.radius == radius)
			{
				texture = c.texture;
				SDL_RenderTexture(renderer, texture, nullptr, &rect);
				return;
			}
		}

		SDL_Color before{0,0,0,0};
		SDL_GetRenderDrawColor(renderer, &before.r, &before.g, &before.b, &before.a);

		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR32, SDL_TEXTUREACCESS_TARGET, radius + radius, radius + radius);

		SDL_SetRenderTarget(renderer, texture);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

		int cX = radius;
		int cY = radius;

		int x = 0;
		int y = -static_cast<int>(radius);

		const int r = radius * radius;

		while (x < -y)
		{
			double yMid = y + 1;

			if ((x * x + yMid * yMid > r))
				y += 1;

			int startX = cX - x;
			int endX = cX + x;

			SDL_RenderLine(renderer, startX, cY + y, endX, cY + y);
			SDL_RenderLine(renderer, startX, cY - y, endX, cY - y);
			SDL_RenderLine(renderer, cX + y, cY + x, cX - y, cY + x);
			SDL_RenderLine(renderer, cX + y, cY - x, cX - y, cY - x);

			x += 1;
		}

		CircleKey key;
		key.color = color;
		key.radius = radius;
		key.texture = texture;

		circleTextures.push_back(key);

		SDL_SetRenderTarget(renderer, nullptr);
		SDL_RenderTexture(renderer, texture, nullptr, &rect);
		SDL_SetRenderDrawColor(renderer, before.r, before.g, before.b, before.a);
	}

	void SDL_Triangle::RenderShape(SDL_Renderer* renderer)
	{

	}

	void SDL_Rectangle::RenderShape(SDL_Renderer* renderer)
	{
		for (auto& r : rectangleTextures)
		{
			if (Shape_CompareColors(color, r.color) && r.height == rect.h && r.width == rect.w)
			{
				texture = r.texture;
				SDL_RenderTexture(renderer, texture, nullptr, &rect);
				return;
			}
		}

		SDL_Color before{ 0,0,0,0 };
		SDL_GetRenderDrawColor(renderer, &before.r, &before.g, &before.b, &before.a);

		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR32, SDL_TEXTUREACCESS_TARGET, rect.w, rect.h);

		SDL_SetRenderTarget(renderer, texture);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

		for (int x = 0; x < rect.w; x++)
		{
			for (int y = 0; y < rect.h; y++)
			{
				SDL_RenderPoint(renderer, x, y);
			}
		}

		RectangleKey key;
		key.color = color;
		key.height = rect.h;
		key.width = rect.w;
		key.texture = texture;

		rectangleTextures.push_back(key);
		SDL_SetRenderTarget(renderer, nullptr);
		SDL_RenderTexture(renderer, texture, nullptr, &rect);
		SDL_SetRenderDrawColor(renderer, before.r, before.g, before.b, before.a);
	}

	void SDL_CleanTextureCache()
	{
		for (auto& c : circleTextures)
		{
			if (c.texture != nullptr)
			{
				std::cout << "Cleaning Circle << " << c.texture << "\n";
				SDL_DestroyTexture(c.texture);
			}
		}

		for (auto& r : rectangleTextures)
		{
			if (r.texture != nullptr)
			{
				std::cout << "Cleaning Square << " << r.texture << "\n";
				SDL_DestroyTexture(r.texture);
			}
		}
	}
}