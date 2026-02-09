#include "SDL_Shapes.h"
#include <iostream>

SDL_Texture* SDL_Shapes::circleTexture = nullptr;
SDL_Texture* SDL_Shapes::rectangleTexture = nullptr;

namespace SDL_Shapes
{
	void SDL_DrawShape(SDL_Renderer* renderer, SDL_Shape& shape)
	{
		shape.RenderShape(renderer);
	}

	void SDL_DrawShape(SDL_Renderer* renderer, SDL_Shape* shape)
	{
		if (shape)
			shape->RenderShape(renderer);
	}

	// Helper functions and variables
	namespace
	{
		constexpr double PI = 3.14159265358979323846;

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
		SDL_Texture* SDL_CreateTriangleTexture(SDL_Renderer* renderer)
		{
			SDL_Color before{ 0,0,0,0 };
			SDL_GetRenderDrawColor(renderer, &before.r, &before.g, &before.b, &before.a);

			SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR32, SDL_TEXTUREACCESS_TARGET, Render_Resolution_Size, Render_Resolution_Size);
			SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

			SDL_SetRenderTarget(renderer, texture);
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

			const double Area = sqrt(Render_Resolution_Size * Render_Resolution_Size + Render_Resolution_Size * Render_Resolution_Size);
			for (int x = 0; x < Render_Resolution_Size; x++)
			{
				for (int y = 0; y < Render_Resolution_Size; y++)
				{
					if (x + y <= Area + (Render_Resolution_Size / 8)) //Render_Resolution / 8 is some padding ignore it, and do not change it
					{
						continue;
					}
					if (x > 0 && y > 0 && x > Render_Resolution_Size / 16 && y > Render_Resolution_Size / 16)
					{
						SDL_RenderPoint(renderer, x - (Render_Resolution_Size / 2), y);
						SDL_RenderPoint(renderer, Render_Resolution_Size - 1 - (x - Render_Resolution_Size / 2), y);
					}
				}
			}

			SDL_SetRenderTarget(renderer, nullptr);
			SDL_SetRenderDrawColor(renderer, before.r, before.g, before.b, before.a);

			return texture;
		}
	}

	//GPU Shape Batching
	namespace
	{

		//Rotations
		static std::vector<double> rectangleRotations;
		static std::vector<double> triangleRotations;

		constexpr int TABLE_SIZE = 360; 
		static float sinTable[TABLE_SIZE];
		static float cosTable[TABLE_SIZE];

		//Positions
		static std::vector<SDL_FPoint> previousCirclePositions;
		static std::vector<SDL_FPoint> previousRectanglePositions;
		static std::vector<SDL_FPoint> trianglePositions;

		//Vertices
		static std::vector<SDL_Vertex> circleVerticies;
		static std::vector<SDL_Vertex> rectangleVerticies;
		static std::vector<SDL_Vertex> triangleVertices;

		//Indices
		static std::vector<int> circleIndices;
		static std::vector<int> rectangleIndices;
		static std::vector<int> triangleIndices;
	}

	void SDL_DrawAllShapes(SDL_Renderer* renderer, std::vector<SDL_Circle>* circles, std::vector<SDL_Rectangle>* rectangles, std::vector<SDL_Triangle>* triangles)
	{  
		static bool Initialzed_Geometry = false;

		if (!circleTexture)
		{
			circleTexture = SDL_CreateCircleTexture(renderer);
		}

		if (!Initialzed_Geometry)
		{
			//Calculate sin and cos tables
			for (size_t i = 0; i < TABLE_SIZE; i++)
			{
				sinTable[i] = sin(static_cast<float>(i * PI / 180.0f));
				cosTable[i] = cos(static_cast<float>(i * PI / 180.0f));
			}
		}

		// Resize Verticies, indices, position and rotation vectors, so that they represent the size of the input vectors
		if (circles || rectangles || triangles)
		{
			if (circles)
			{
				if (circleVerticies.size() != (circles->size() * 4)) // Resize verticies and indicies
				{
					circleVerticies.resize((circles->size() * 4));
					circleIndices.resize((circles->size() * 6));
				}

				if (previousCirclePositions.size() != circles->size()) //Resize positions
					previousCirclePositions.resize(circles->size());
			}
			if (rectangles)
			{
				if (rectangleVerticies.size() != rectangles->size() * 4) // Resize verticies and indicies
				{
					rectangleVerticies.resize((rectangles->size() * 4));
					rectangleIndices.resize((rectangles->size() * 6));
				}

				if (rectangleRotations.size() != rectangles->size())
					rectangleRotations.resize(rectangles->size());

				if (previousRectanglePositions.size() != rectangles->size()) //Resize positions
					previousRectanglePositions.resize(rectangles->size());
			}
			if (triangles)
			{
				if (triangleVertices.size() != triangles->size() * 3) // Resize verticies and indicies
				{
					triangleVertices.resize(triangles->size() * 3);
					triangleIndices.resize(triangles->size() * 3);
				}

				if (triangleRotations.size() != triangles->size()) //Resize positions
					triangleRotations.resize(triangles->size());

				if (trianglePositions.size() != triangles->size()) //Resize positions
					trianglePositions.resize(triangles->size());
			}
		}


		if (circles && circles->size() > 0)
		{
			for (size_t i = 0; i < circles->size(); i++)
			{
				SDL_Circle& circle = (*circles)[i];

				if (Initialzed_Geometry && circle.positionX == previousCirclePositions[i].x && circle.positionY == previousCirclePositions[i].y)
					continue;

				float radius = circle.radius;

				SDL_FPoint vertices[4]
				{
					{-radius, -radius},  //Top left v0
					{radius, -radius},   //Top right v1
					{-radius, radius}, //Bottom left v2
					{radius, radius}   //Bottom right v3
				};

				SDL_FPoint textureCoords[4]
				{
				  // X Y
					{0,0}, //Top left
					{1,0}, //Top right
					{0,1}, //Bottom left
					{1,1}  //Bottom right
				};

				int index = static_cast<int>(circle.rotation) % 360;

				float sin = sinTable[index];
				float cos = cosTable[index];

				SDL_FColor color = { 1,1,1,1 };

				for (size_t j = 0; j < 4; j++)
				{
					float x = vertices[j].x;
					float y = vertices[j].y;

					vertices[j].x = ((x * cos) - (y * sin)) + circle.positionX;
					vertices[j].y = ((x * sin) + (y * cos)) + circle.positionY;

					circleVerticies[i * 4 + j].position = { vertices[j].x, vertices[j].y};
					circleVerticies[i * 4 + j].color = color;
					circleVerticies[i * 4 + j].tex_coord = textureCoords[j];
				}

				previousCirclePositions[i].x = circle.positionX;
				previousCirclePositions[i].y = circle.positionY;

				// Each circle is rendered as a quad (4 vertices).
				// A quad is made of 2 triangles, and each triangle needs 3 indices ->6 indices total.
				//
				// Indices are just numbers that reference vertices.
				// Vertices are points in space and do not form triangles on their own.
				//
				// i * 4 = starting vertex index for this circle
				// i * 6 = starting index position in the index buffer

				// Triangle 1: v0, v1, v3
				circleIndices[i * 6 + 0] = i * 4 + 0;
				circleIndices[i * 6 + 1] = i * 4 + 1;
				circleIndices[i * 6 + 2] = i * 4 + 3;

				// Triangle 2: v0, v2, v3
				circleIndices[i * 6 + 3] = i * 4 + 0;
				circleIndices[i * 6 + 4] = i * 4 + 2;
				circleIndices[i * 6 + 5] = i * 4 + 3;
			}
		}

		if (rectangles && rectangles->size() > 0)
		{
			for (size_t i = 0; i < rectangles->size(); i++)
			{
				SDL_Rectangle& Rect = (*rectangles)[i];

				if (Initialzed_Geometry && Rect.positionX == previousRectanglePositions[i].x && Rect.positionY == previousRectanglePositions[i].y && Rect.rotation == rectangleRotations[i])
					continue;

				SDL_FPoint verticies[4]
				{
					{-Rect.width, -Rect.height}, //v0
					{Rect.width, -Rect.height}, //v1
					{-Rect.width, Rect.height}, //v2
					{Rect.width, Rect.height} //v3
				};

				int index = static_cast<int>(Rect.rotation) % 360;

				float sin = sinTable[index];
				float cos = cosTable[index];

				for (size_t j = 0; j < 4; j++)
				{
					float x = verticies[j].x;
					float y = verticies[j].y;

					verticies[j].x = (x * cos - y * sin);
					verticies[j].y = (x * sin + y * cos);

					rectangleVerticies[i * 4 + j].position = { verticies[j].x + Rect.positionX, verticies[j].y + Rect.positionY};
					rectangleVerticies[i * 4 + j].color = Rect.fColor;
				}

				previousRectanglePositions[i].x = Rect.positionX;
				previousRectanglePositions[i].y = Rect.positionY;

				// Triangle 1: v0, v2, v3
				rectangleIndices[i * 6 + 0] = i * 4 + 0;
				rectangleIndices[i * 6 + 1] = i * 4 + 2;
				rectangleIndices[i * 6 + 2] = i * 4 + 3;

				// Triangle 2: v0, v1, v3
				rectangleIndices[i * 6 + 3] = i * 4 + 0;
				rectangleIndices[i * 6 + 4] = i * 4 + 1;
				rectangleIndices[i * 6 + 5] = i * 4 + 3;

				// Each Rectangle is rendered as a quad (4 vertices).
				// A quad is made of 2 triangles, and each triangle needs 3 indices ->6 indices total.
				//
				// Indices are just numbers that reference vertices.
				// Vertices are points in space and do not form triangles on their own.
				//
				// i * 4 = starting vertex index for this circle
				// i * 6 = starting index position in the index buffer
			}
		}

		if (triangles && triangles->size() > 0)
		{
			for (size_t i = 0; i < triangles->size(); i++)
			{
				SDL_Triangle& triangle = (*triangles)[i];

				if (triangle.positionX == trianglePositions[i].x && triangle.positionY == trianglePositions[i].y && triangle.rotation == triangleRotations[i])
					continue;

				SDL_FPoint verticies[3]
				{
					{0, -triangle.height},
					{-triangle.width / 2, 0},
					{triangle.width / 2, 0}
				};

				int index = static_cast<int>(triangle.rotation) % 360;

				float s = sinTable[index];
				float c = cosTable[index];

				float offset = triangle.width / 3.0;
		
				for (int j = 0; j < 3; j++)
				{
					float x = verticies[j].x;
					float y = verticies[j].y + offset;

					verticies[j].x = (x * c - y * s);
					verticies[j].y = (x * s + y * c);

					triangleVertices[i * 3 + j].position = { verticies[j].x + triangle.positionX, verticies[j].y + triangle.positionY};
					triangleVertices[i * 3 + j].color = triangle.fColor;
				}

				triangleRotations[i] = triangle.rotation;
				trianglePositions[i] = { triangle.positionX, triangle.positionY };

				triangleIndices[i * 3 + 0] = i * 3 + 0;
				triangleIndices[i * 3 + 1] = i * 3 + 1;
				triangleIndices[i * 3 + 2] = i * 3 + 2;
			}
		}

		// Improvement?: Make triangles verticies and indicies be with the rectangles so that becomes one draw call.
		if (circles)
			SDL_RenderGeometry(renderer, circleTexture, circleVerticies.data(), circleVerticies.size(), circleIndices.data(), circleIndices.size());
		if (rectangles)
			SDL_RenderGeometry(renderer, nullptr, rectangleVerticies.data(), rectangleVerticies.size(), rectangleIndices.data(), rectangleIndices.size());
		if (triangles)
			SDL_RenderGeometry(renderer, nullptr, triangleVertices.data(), triangleVertices.size(), triangleIndices.data(), triangleIndices.size());
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
		SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

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
		SDL_FPoint center;
		center.x = rect.w / 2;
		center.y = height - (height / 4);

		if (rect.w != width || rect.h != height)
		{
			rect.w = width;
			rect.h = height;
		}
		//Render texture
		if (texture)
		{
			SDL_SetTextureColorMod(texture, color.r, color.g, color.b);
			SDL_RenderTextureRotated(renderer, texture, nullptr, &rect, rotation, &center, SDL_FLIP_NONE);
			return;
		}

		SDL_Color before{ 0,0,0,0 };
		SDL_GetRenderDrawColor(renderer, &before.r, &before.g, &before.b, &before.a);

		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR32, SDL_TEXTUREACCESS_TARGET, Render_Resolution_Size, Render_Resolution_Size);
		SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

		SDL_SetRenderTarget(renderer, texture);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

		const double Area = sqrt(Render_Resolution_Size * Render_Resolution_Size + Render_Resolution_Size * Render_Resolution_Size);
		for (int x = 0; x < Render_Resolution_Size; x++)
		{
			for (int y = 0; y < Render_Resolution_Size; y++)
			{
				if (x + y <= Area + (Render_Resolution_Size / 8)) //(Render_Resolution / 8) is some padding ignore it, and do not change it
				{
					continue;
				}
				if (x > 0 && y > 0 && x > Render_Resolution_Size / 16 && y > Render_Resolution_Size / 16)
				{
					SDL_RenderPoint(renderer, x - (Render_Resolution_Size / 2), y);
					SDL_RenderPoint(renderer, Render_Resolution_Size - 1 - (x - Render_Resolution_Size / 2), y);
				}
			}
		}

		rectangleTexture = texture;

		SDL_SetRenderTarget(renderer, nullptr);
		SDL_RenderTextureRotated(renderer, texture, nullptr, &rect, rotation, &center, SDL_FLIP_NONE);
		SDL_SetRenderDrawColor(renderer, before.r, before.g, before.b, before.a);
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
			SDL_DestroyTexture(circleTexture);
			circleTexture = nullptr;
		}

		if (rectangleTexture != nullptr)
		{
			SDL_DestroyTexture(rectangleTexture);
			rectangleTexture = nullptr;
		}
	}
}