#include "SDL_Shapes.h"
#include <iostream>
#include <map>
#include <Windows.h>

SDL_Texture* SDL_Shapes::circleTexture = nullptr;
SDL_Texture* SDL_Shapes::rectangleTexture = nullptr;

std::vector<SDL_Vertex> SDL_Shapes::circleVerticies;
std::vector<int> SDL_Shapes::circleIndices;

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

	void SDL_DrawAllShapes(SDL_Renderer* renderer, std::vector<SDL_Circle>* circles)
	{
		static std::vector<SDL_Circle> previous = *circles;
		static bool Initialzed_Geometry = false;
		
		if (!Initialzed_Geometry)
		{
			//Reserve extra space for verticies and circles 40MB total
			circles->reserve(200000);
			circleVerticies.reserve(200000 * 4);
			circleIndices.reserve(200000 * 6); 
		}

		// In your draw function, only resize if number of circles increased:
		if (circleVerticies.size() != circles->size() * 4)
		{
			circleVerticies.resize(circles->size() * 4);
			circleIndices.resize(circles->size() * 6);
		}

		for (size_t i = 0; i < circles->size(); i++)
		{
			SDL_Circle& circle = circles->at(i);

			if (Initialzed_Geometry && circle.positionX == previous[i].positionX && circle.positionY == previous[i].positionY)
				continue;

			float cX = circle.positionX;
			float cY = circle.positionY;
			float radius = circle.radius;

			SDL_FColor color{ circle.color.r / 255.0f, circle.color.g / 255.0f, circle.color.b / 255.0f, circle.color.a / 255.0f };

			SDL_Vertex& v0 = circleVerticies[i * 4 + 0];
			SDL_Vertex& v1 = circleVerticies[i * 4 + 1];
			SDL_Vertex& v2 = circleVerticies[i * 4 + 2];
			SDL_Vertex& v3 = circleVerticies[i * 4 + 3];

			circleIndices[i * 6 + 0] = i * 4 + 0;
			circleIndices[i * 6 + 1] = i * 4 + 1;
			circleIndices[i * 6 + 2] = i * 4 + 2;
			circleIndices[i * 6 + 3] = i * 4 + 2;
			circleIndices[i * 6 + 4] = i * 4 + 3;
			circleIndices[i * 6 + 5] = i * 4 + 0;

			v0.position = { cX - radius , cY - radius };
			v0.color = color;
			v0.tex_coord = { 0,0 };

			v1.position = { cX + radius , cY - radius };
			v1.color = color;
			v1.tex_coord = { 1,0 };

			v2.position = { cX + radius, cY + radius };
			v2.color = color;
			v2.tex_coord = { 1,1 };

			v3.position = { cX - radius, cY + radius };
			v3.tex_coord = { 0,1 };
			v3.color = color;
		}

		SDL_RenderGeometry(renderer, circleTexture, circleVerticies.data(), circleVerticies.size(), circleIndices.data(), circleIndices.size());
		Initialzed_Geometry = true;
		previous = *circles;
	}

	void SDL_Shape::RenderShape(SDL_Renderer* renderer)
	{

	}

	bool SDL_Shape::Shape_CompareColors(SDL_Color& other1, SDL_Color& other2)
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
		if (rect.x != positionX || rect.y != positionY)
		{
			rect.x = positionX;
			rect.y = positionY;
		}

		if (rect.h != radius + radius || rect.w != radius + radius)
		{
			rect.h = radius + radius;
			rect.w = radius + radius;
		}

		//Render texture
		if (texture)
		{
			SDL_SetTextureColorMod(texture, color.r, color.g, color.b);
			SDL_RenderTexture(renderer, texture, nullptr, &rect);
			return;
		}
		else if (circleTexture)
		{
			texture = circleTexture;
			SDL_SetTextureColorMod(texture, color.r, color.g, color.b);
			SDL_RenderTexture(renderer, texture, nullptr, &rect);
			return;
		}

		//Creates the texture
		SDL_Color before{ 0,0,0,0 };
		SDL_GetRenderDrawColor(renderer, &before.r, &before.g, &before.b, &before.a);

		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR32, SDL_TEXTUREACCESS_TARGET, Render_Resolution_Size, Render_Resolution_Size);
		SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);

		SDL_SetRenderTarget(renderer, texture);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

		const double radius = (Render_Resolution_Size / 2) - 1;

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


		circleTexture = texture;

		SDL_SetRenderTarget(renderer, nullptr);
		SDL_RenderTexture(renderer, texture, nullptr, &rect);
		SDL_SetRenderDrawColor(renderer, before.r, before.g, before.b, before.a);
	}

	void SDL_Triangle::RenderShape(SDL_Renderer* renderer)
	{

	}

	void SDL_Rectangle::RenderShape(SDL_Renderer* renderer)
	{
		if (rect.x != positionX || rect.y != positionY)
		{
			rect.x = positionX;
			rect.y = positionY;
		}

		if (rect.h != height || rect.w != width)
		{
			rect.h = height;
			rect.w = width;
		}

		//Render texture
		if (texture)
		{
			SDL_SetTextureColorMod(texture, color.r, color.g, color.b);
			SDL_RenderTexture(renderer, texture, nullptr, &rect);
			return;
		}
		else if (rectangleTexture)
		{
			texture = rectangleTexture;
			SDL_SetTextureColorMod(texture, color.r, color.g, color.b);
			SDL_RenderTexture(renderer, texture, nullptr, &rect);
			return;
		}


		SDL_Color before{ 0,0,0,0 };
		SDL_GetRenderDrawColor(renderer, &before.r, &before.g, &before.b, &before.a);

		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR32, SDL_TEXTUREACCESS_TARGET, Render_Resolution_Size, Render_Resolution_Size);
		SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);

		SDL_SetRenderTarget(renderer, texture);
		SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

		for (int x = 0; x < Render_Resolution_Size; x++)
		{
			for (int y = 0; y < Render_Resolution_Size; y++)
			{
				SDL_RenderPoint(renderer, x, y);
			}
		}

		rectangleTexture = texture;

		SDL_SetRenderTarget(renderer, nullptr);
		SDL_RenderTexture(renderer, texture, nullptr, &rect);
		SDL_SetRenderDrawColor(renderer, before.r, before.g, before.b, before.a);
	}

	void SDL_CleanTextureCache()
	{
		if (circleTexture != nullptr)
		{
			std::cout << "Cleaning Circle << " << circleTexture << "\n";
			SDL_DestroyTexture(circleTexture);
			circleTexture = nullptr;
		}

		if (rectangleTexture != nullptr)
		{
			std::cout << "Cleaning Square << " << rectangleTexture << "\n";
			SDL_DestroyTexture(rectangleTexture);
			rectangleTexture = nullptr;
		}
	}
}