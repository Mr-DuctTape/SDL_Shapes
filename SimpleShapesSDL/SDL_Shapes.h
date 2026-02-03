#pragma once
#include <SDL3/SDL.h>
#include <map>
#include <vector>

namespace SDL_Shapes
{
	class SDL_Shape;

	struct CircleKey
	{
		double radius;
		SDL_Color color;
		SDL_Texture* texture;
	};

	struct RectangleKey
	{
		double width, height;
		SDL_Color color;
		SDL_Texture* texture;
	};

	extern std::vector<RectangleKey> rectangleTextures;
	extern std::vector<CircleKey> circleTextures;

	//Ptr and ref overload
	void SDL_DrawShape(SDL_Renderer*, SDL_Shape& shape);
	void SDL_DrawShape(SDL_Renderer*, SDL_Shape* shape);

	//Base class
	class SDL_Shape
	{
	protected:
		bool Shape_CompareColors(SDL_Color other1, SDL_Color other2);
	public:
		virtual void RenderShape(SDL_Renderer*);
	};

	//Different Shape classes
	class SDL_Circle : public SDL_Shape
	{
	private:
		SDL_Color color;
		SDL_Texture* texture = nullptr;
		SDL_FRect rect;

	public:
		double radius;
		double centerX, centerY;

		SDL_Circle(double posX, double posY, double circleRadius)
		{
			centerX = posX;
			centerY = posY;
			radius = circleRadius;

			rect.x = centerX - radius;
			rect.y = centerY - radius;
			rect.h = radius * 2;
			rect.w = radius * 2;

			color = SDL_Color{ 255,255,255,255 };
		}

		SDL_Circle(double posX, double posY, double circleRadius, SDL_Color circleColor)
		{
			centerX = posX;
			centerY = posY;
			radius = circleRadius;

			rect.x = centerX - radius;
			rect.y = centerY - radius;
			rect.h = radius * 2;
			rect.w = radius * 2;

			color = circleColor;
		}

		void RenderShape(SDL_Renderer* renderer) override;

		inline void set_color(SDL_Color circleColor)
		{
			color = circleColor;
		}
	};

	class SDL_Triangle : public SDL_Shape
	{
	private:
		SDL_Color color;
		SDL_Texture* texture;

	public:
		void RenderShape(SDL_Renderer* renderer) override;
		inline void set_color(SDL_Color triangleColor)
		{
			color = triangleColor;
		}
	};

	class SDL_Rectangle : public SDL_Shape
	{
	private: 
		SDL_Color color;
		SDL_Texture* texture = nullptr;
		SDL_FRect rect;
	public:

		SDL_Rectangle(double posX, double posY, double width, double height, SDL_Color squareColor)
		{
			rect.x = posX;
			rect.y = posY;
			rect.w = width;
			rect.h = height;

			color = squareColor;
		}

		SDL_Rectangle(double posX, double posY, double width, double height)
		{
			rect.x = posX;
			rect.y = posY;
			rect.w = width;
			rect.h = height;

			color = SDL_Color{ 255,255,255,255 };
		}

		void RenderShape(SDL_Renderer* renderer) override;
		inline void set_color(SDL_Color rectangleColor)
		{
			color = rectangleColor;
		}
	};

	void SDL_CleanTextureCache();
}

