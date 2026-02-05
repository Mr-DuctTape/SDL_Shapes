#include "SDL_Shapes.h"
#include <iostream>
#include <map>
#include <Windows.h>

SDL_Texture* SDL_Shapes::circleTexture = nullptr;
SDL_Texture* SDL_Shapes::rectangleTexture = nullptr;

namespace SDL_Shapes
{
	static std::vector<SDL_Vertex> shapeVerticies;
	static std::vector<int> shapeIndices;

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

	// Helper functions
	namespace 
	{
		SDL_Texture* SDL_CreateCircleTexture(SDL_Renderer* renderer)
		{
			SDL_Color before{ 0,0,0,0 };
			SDL_GetRenderDrawColor(renderer, &before.r, &before.g, &before.b, &before.a);

			//Create the texture
			SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR32, SDL_TEXTUREACCESS_TARGET, Render_Resolution_Size, Render_Resolution_Size);
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

			SDL_SetRenderTarget(renderer, nullptr);
			SDL_SetRenderDrawColor(renderer, before.r, before.g, before.b, before.a);

			return texture;
		}
		SDL_Texture* SDL_CreateRectangleTexture(SDL_Renderer* renderer)
		{

			SDL_Color before{ 0,0,0,0 };
			SDL_GetRenderDrawColor(renderer, &before.r, &before.g, &before.b, &before.a);

			SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR32, SDL_TEXTUREACCESS_TARGET, Render_Resolution_Size, Render_Resolution_Size);
			SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);

			SDL_SetRenderTarget(renderer, texture);
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

			for (int x = 0; x < Render_Resolution_Size; x++)
			{
				for (int y = 0; y < Render_Resolution_Size; y++)
				{
					SDL_RenderPoint(renderer, x, y);
				}
			}

			SDL_SetRenderTarget(renderer, nullptr);
			SDL_SetRenderDrawColor(renderer, before.r, before.g, before.b, before.a);

			return texture;
		}
		void SDL_CreateTriangleTexture()
		{

		}
		void SDL_CreateAllTextures()
		{

		}
	}

	//Helper variables
	namespace
	{
		static std::vector<SDL_FPoint> previousCirclePositions;
		static std::vector<SDL_FPoint> previousRectanglePositions;
	}

	void SDL_DrawAllShapes(SDL_Renderer* renderer, std::vector<SDL_Circle>* circles, std::vector<SDL_Rectangle>* rectangles)
	{  //Keep track of previous positions, can definetly optimize this cause it copies
		static bool Initialzed_Geometry = false;

		if (!circleTexture)
		{
			circleTexture = SDL_CreateCircleTexture(renderer);
		}

		if (!rectangleTexture)
		{
			rectangleTexture = SDL_CreateRectangleTexture(renderer);
		}

		if (!Initialzed_Geometry)
		{
			//Reserve extra space for verticies and indicies
			shapeVerticies.reserve(400000 * 4);
			shapeIndices.reserve(400000 * 6);
		}

		// Rezise Vertices and indicies
		if (circles && rectangles)
		{
			if (shapeVerticies.size() != (circles->size() * 4) + (rectangles->size() * 4))
			{
				shapeVerticies.resize((circles->size() * 4) + (rectangles->size() * 4));
				shapeIndices.resize((circles->size() * 6) + (rectangles->size() * 6));
			}

			if (previousCirclePositions.size() != circles->size()) //Resize positions
				previousCirclePositions.resize(circles->size());

			if (previousRectanglePositions.size() != rectangles->size()) //Resize positions
				previousRectanglePositions.resize(rectangles->size());
		}
		else if (circles && !rectangles)
		{
			if (shapeVerticies.size() != (circles->size() * 4))
			{
				shapeVerticies.resize((circles->size() * 4));
				shapeIndices.resize((circles->size() * 6));
			}

			if (previousCirclePositions.size() != circles->size()) //Resize positions
				previousCirclePositions.resize(circles->size());
		}
		else if (!circles && rectangles)
		{
			if (shapeVerticies.size() != (rectangles->size() * 4))
			{
				shapeVerticies.resize((rectangles->size() * 4));
				shapeIndices.resize((rectangles->size() * 6));
			}

			if (previousRectanglePositions.size() != rectangles->size()) //Resize positions
				previousRectanglePositions.resize(rectangles->size());
		}

		if (circles)
		{
			for (size_t i = 0; i < circles->size(); i++)
			{
				SDL_Circle& circle = circles->at(i);

				if (Initialzed_Geometry && circle.positionX == previousCirclePositions[i].x && circle.positionY == previousCirclePositions[i].y)
					continue;

				float cX = circle.positionX;
				float cY = circle.positionY;
				float radius = circle.radius;

				previousCirclePositions[i].x = circle.positionX;
				previousCirclePositions[i].y = circle.positionY;

				SDL_FColor color{ circle.color.r / 255.0f, circle.color.g / 255.0f, circle.color.b / 255.0f, circle.color.a / 255.0f };

				SDL_Vertex& v0 = shapeVerticies[i * 4 + 0];
				SDL_Vertex& v1 = shapeVerticies[i * 4 + 1];
				SDL_Vertex& v2 = shapeVerticies[i * 4 + 2];
				SDL_Vertex& v3 = shapeVerticies[i * 4 + 3];

				// Each circle is rendered as a quad (4 vertices).
				// A quad is made of 2 triangles, and each triangle needs 3 indices ->6 indices total.
				//
				// Indices are just numbers that reference vertices.
				// Vertices are points in space and do not form triangles on their own.
				//
				// i * 4 = starting vertex index for this circle
				// i * 6 = starting index position in the index buffer

				// Triangle 1: v0, v1, v2
				shapeIndices[i * 6 + 0] = i * 4 + 0;
				shapeIndices[i * 6 + 1] = i * 4 + 1;
				shapeIndices[i * 6 + 2] = i * 4 + 2;

				// Triangle 2: v2, v3, v0
				shapeIndices[i * 6 + 3] = i * 4 + 2;
				shapeIndices[i * 6 + 4] = i * 4 + 3;
				shapeIndices[i * 6 + 5] = i * 4 + 0;

				v0.position = { cX - radius , cY - radius }; // Top left 
				v0.color = color;
				v0.tex_coord = { 0,0 }; // 0,0 is top left of texture

				v1.position = { cX + radius , cY - radius }; // Top right
				v1.color = color;
				v1.tex_coord = { 1,0 };

				v2.position = { cX + radius, cY + radius }; // Bottom right
				v2.color = color;
				v2.tex_coord = { 1,1 };

				v3.position = { cX - radius, cY + radius }; // Bottom left
				v3.tex_coord = { 0,1 };
				v3.color = color;
			}
		}

		if (rectangles)
		{
			const size_t IndiceOffset = circles ? circles->size() * 6 : 0;
			const size_t VerticeOffset = circles ? circles->size() * 4 : 0;

			size_t rectanglePosIndex = 0;

			for (size_t i = 0; i < rectangles->size(); i++)
			{
				SDL_Rectangle& Rect = rectangles->at(i);

				if (Initialzed_Geometry && Rect.positionX == previousRectanglePositions[rectanglePosIndex].x && Rect.positionY == previousRectanglePositions[rectanglePosIndex].y)
					continue;

				float cX = Rect.positionX;
				float cY = Rect.positionY;
				float rW = Rect.width;
				float rH = Rect.height;

				previousRectanglePositions[rectanglePosIndex].x = Rect.positionX;
				previousRectanglePositions[rectanglePosIndex].y = Rect.positionY;

				SDL_FColor color{ Rect.color.r / 255.0f, Rect.color.g / 255.0f, Rect.color.b / 255.0f, Rect.color.a / 255.0f };

				SDL_Vertex& v0 = shapeVerticies[VerticeOffset + i * 4 + 0];
				SDL_Vertex& v1 = shapeVerticies[VerticeOffset + i * 4 + 1];
				SDL_Vertex& v2 = shapeVerticies[VerticeOffset + i * 4 + 2];
				SDL_Vertex& v3 = shapeVerticies[VerticeOffset + i * 4 + 3];

				// Each Rectangle is rendered as a quad (4 vertices).
				// A quad is made of 2 triangles, and each triangle needs 3 indices ->6 indices total.
				//
				// Indices are just numbers that reference vertices.
				// Vertices are points in space and do not form triangles on their own.
				//
				// i * 4 = starting vertex index for this circle
				// i * 6 = starting index position in the index buffer

				// Triangle 1: v0, v1, v2
				shapeIndices[IndiceOffset + i * 6 + 0] = VerticeOffset + i * 4 + 0;
				shapeIndices[IndiceOffset + i * 6 + 1] = VerticeOffset + i * 4 + 1;
				shapeIndices[IndiceOffset + i * 6 + 2] = VerticeOffset + i * 4 + 2;

				// Triangle 2: v2, v3, v0
				shapeIndices[IndiceOffset + i * 6 + 3] = VerticeOffset + i * 4 + 2;
				shapeIndices[IndiceOffset + i * 6 + 4] = VerticeOffset + i * 4 + 3;
				shapeIndices[IndiceOffset + i * 6 + 5] = VerticeOffset + i * 4 + 0;

				v0.position = { cX - rW , cY - rH }; // Top left 
				v0.color = color;
				v0.tex_coord = { 0,0 }; // 0,0 is top left of texture

				v1.position = { cX + rW , cY - rH }; // Top right
				v1.color = color;
				v1.tex_coord = { 1,0 };

				v2.position = { cX + rW, cY + rH }; // Bottom right
				v2.color = color;
				v2.tex_coord = { 1,1 };

				v3.position = { cX - rW, cY + rH }; // Bottom left
				v3.tex_coord = { 0,1 };
				v3.color = color;

				rectanglePosIndex++;
			}
		}

		SDL_RenderGeometry(renderer, circleTexture, shapeVerticies.data(), shapeVerticies.size(), shapeIndices.data(), shapeIndices.size());
		Initialzed_Geometry = true;
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
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

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