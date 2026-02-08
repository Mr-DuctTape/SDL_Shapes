#include "SDL_Shapes.h"

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
			circleVerticies.reserve(200000 * 4);
			rectangleVerticies.reserve(200000 * 4);
			circleIndices.reserve(400000 * 6);
			circleVerticies.reserve(400000 * 6);
		}

		// Rezise Vertices and indicies
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
				SDL_Circle& circle = circles->at(i);

				if (Initialzed_Geometry && circle.positionX == previousCirclePositions[i].x && circle.positionY == previousCirclePositions[i].y)
					continue;

				float cX = circle.positionX;
				float cY = circle.positionY;
				float radius = circle.radius;

				previousCirclePositions[i].x = circle.positionX;
				previousCirclePositions[i].y = circle.positionY;

				SDL_FColor color = { circle.color.r / 255.0f, circle.color.g / 255.0f,  circle.color.b / 255.0f,  circle.color.a / 255.0f };

				SDL_Vertex& v0 = circleVerticies[i * 4 + 0];
				SDL_Vertex& v1 = circleVerticies[i * 4 + 1];
				SDL_Vertex& v2 = circleVerticies[i * 4 + 2];
				SDL_Vertex& v3 = circleVerticies[i * 4 + 3];

				// Each circle is rendered as a quad (4 vertices).
				// A quad is made of 2 triangles, and each triangle needs 3 indices ->6 indices total.
				//
				// Indices are just numbers that reference vertices.
				// Vertices are points in space and do not form triangles on their own.
				//
				// i * 4 = starting vertex index for this circle
				// i * 6 = starting index position in the index buffer

				// Triangle 1: v0, v1, v2
				circleIndices[i * 6 + 0] = i * 4 + 0;
				circleIndices[i * 6 + 1] = i * 4 + 1;
				circleIndices[i * 6 + 2] = i * 4 + 2;

				// Triangle 2: v2, v3, v0
				circleIndices[i * 6 + 3] = i * 4 + 2;
				circleIndices[i * 6 + 4] = i * 4 + 3;
				circleIndices[i * 6 + 5] = i * 4 + 0;

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

		if (rectangles && rectangles->size() > 0)
		{
			for (size_t i = 0; i < rectangles->size(); i++)
			{
				SDL_Rectangle& Rect = rectangles->at(i);

				if (Initialzed_Geometry && Rect.positionX == previousRectanglePositions[i].x && Rect.positionY == previousRectanglePositions[i].y && Rect.rotation == rectangleRotations[i])
					continue;

				previousRectanglePositions[i].x = Rect.positionX;
				previousRectanglePositions[i].y = Rect.positionY;

				SDL_FColor color{ Rect.color.r / 255.0f, Rect.color.g / 255.0f, Rect.color.b / 255.0f, Rect.color.a / 255.0f };

				SDL_Vertex& v0 = rectangleVerticies[i * 4 + 0];
				SDL_Vertex& v1 = rectangleVerticies[i * 4 + 1];
				SDL_Vertex& v2 = rectangleVerticies[i * 4 + 2];
				SDL_Vertex& v3 = rectangleVerticies[i * 4 + 3];

				double rad = Rect.rotation * PI / 180.0;
				double s = sin(rad);
				double c = cos(rad);

				float cX = Rect.positionX;
				float cY = Rect.positionY;
				float rW = Rect.width;
				float rH = Rect.height;

				float p0X = cX - rW;
				float p0Y = cY - rH;

				float p1X = cX + rW;
				float p1Y = cY - rH;

				float p2X = cX + rW;
				float p2Y = cY + rH;

				float p3X = cX - rW;
				float p3Y = cY + rH;

				//Offsetting
				p0X -= cX;
				p0Y -= cY;

				p1X -= cX;
				p1Y -= cY;

				p2X -= cX;
				p2Y -= cY;

				p3X -= cX;
				p3Y -= cY;

				//Rotating
				float t0X = p0X;
				float t0Y = p0Y;

				p0X = t0X * c - t0Y * s;
				p0Y = t0X * s + t0Y * c;

				float t1X = p1X;
				float t1Y = p1Y;

				p1X = t1X * c - t1Y * s;
				p1Y = t1X * s + t1Y * c;

				float t2X = p2X;
				float t2Y = p2Y;

				p2X = t2X * c - t2Y * s;
				p2Y = t2X * s + t2Y * c;

				float t3X = p3X;
				float t3Y = p3Y;

				p3X = t3X * c - t3Y * s;
				p3Y = t3X * s + t3Y * c;

				//Resetting
				p0X += cX;
				p0Y += cY;

				p1X += cX;
				p1Y += cY;

				p2X += cX;
				p2Y += cY;

				p3X += cX;
				p3Y += cY;


				v0.position = { p0X , p0Y}; // Top left 
				v0.color = color;
				v0.tex_coord = { 0,0 }; // 0,0 is top left of texture

				v1.position = { p1X , p1Y }; // Top right
				v1.color = color;
				v1.tex_coord = { 1,0 };

				v2.position = { p2X, p2Y }; // Bottom right
				v2.color = color;
				v2.tex_coord = { 1,1 };

				v3.position = { p3X, p3Y }; // Bottom left
				v3.tex_coord = { 0,1 };
				v3.color = color;

				// Triangle 1: v0, v1, v2
				rectangleIndices[i * 6 + 0] = i * 4 + 0;
				rectangleIndices[i * 6 + 1] = i * 4 + 1;
				rectangleIndices[i * 6 + 2] = i * 4 + 2;

				// Triangle 2: v2, v3, v0
				rectangleIndices[i * 6 + 3] = i * 4 + 2;
				rectangleIndices[i * 6 + 4] = i * 4 + 3;
				rectangleIndices[i * 6 + 5] = i * 4 + 0;

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
				SDL_Triangle& triangle = triangles->at(i);

				if (Initialzed_Geometry && triangle.positionX == trianglePositions[i].x && triangle.positionY == trianglePositions[i].y && triangle.rotation == triangleRotations[i])
					continue;

				SDL_Vertex& v0 = triangleVertices[i * 3 + 0];
				SDL_Vertex& v1 = triangleVertices[i * 3 + 1];
				SDL_Vertex& v2 = triangleVertices[i * 3 + 2];

				SDL_FColor color = { triangle.color.r / 255.0f, triangle.color.g / 255.0f,  triangle.color.b / 255.0f,  triangle.color.a / 255.0f };

				float x = triangle.positionX;
				float y = triangle.positionY;
				float h = triangle.height;
				float w = triangle.width;

				double rad = triangle.rotation * PI / 180.0;

				double s = sin(rad);
				double c = cos(rad);

				double cX = x; //Center
				double cY = y - h / 3.0;

				//Vertice points x & y
				float p0X = x;
				float p1X = x - w;
				float p2X = x + w;

				float p0Y = y - h;
				float p1Y = y;
				float p2Y = y;

				//Offsetting
				p0X -= cX;
				p1X -= cX;
				p2X -= cX;

				p0Y -= cY;
				p1Y -= cY;
				p2Y -= cY;

				//Rotating
				float t0X = p0X;
				float t0Y = p0Y;

				p0X = t0X * c - t0Y * s;
				p0Y = t0X * s + t0Y * c;

				float t1X = p1X;
				float t1Y = p1Y;

				p1X = t1X * c - t1Y * s;
				p1Y = t1X * s + t1Y * c;

				float t2X = p2X;
				float t2Y = p2Y;

				p2X = t2X * c - t2Y * s;
				p2Y = t2X * s + t2Y * c;

				//Resetting
				p0X += cX;
				p1X += cX;
				p2X += cX;

				p0Y += cY;
				p1Y += cY;
				p2Y += cY;

				v0.position = { p0X, p0Y };
				v1.position = { p1X, p1Y };
				v2.position = { p2X, p2Y };

				v0.color = color;
				v1.color = color;
				v2.color = color;

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