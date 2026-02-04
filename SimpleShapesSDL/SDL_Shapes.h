#pragma once
#include <SDL3/SDL.h>
#include <map>
#include <vector>



namespace SDL_Shapes
{
	constexpr int Render_Resolution_Size = 512; // How big the texture will be in memory and quality of the texture

	//Class forward declarations
	class SDL_Shape;
	class SDL_Circle;
	class SDL_Rectangle;

	//Ptr and ref overload, for specific shapes
	void SDL_DrawShape(SDL_Renderer* renderer, SDL_Shape& shape);
	void SDL_DrawShape(SDL_Renderer* renderer, SDL_Shape* shape);


	//Base class
	class SDL_Shape
	{
	protected:
		bool Shape_CompareColors(SDL_Color& other1, SDL_Color& other2);
	public:
		virtual void RenderShape(SDL_Renderer*);
	};

	//Shape classes
	class SDL_Circle : public SDL_Shape
	{
	private:
		SDL_Color color;
		SDL_Texture* texture = nullptr;
		SDL_FRect rect;

	public:
		double radius;
		double positionX, positionY;

		SDL_Circle(double posX, double posY, double circleRadius)
		{
			positionX = posX;
			positionY = posY;
			radius = circleRadius;

			rect.x = positionX - radius;
			rect.y = positionY - radius;
			rect.h = radius * 2;
			rect.w = radius * 2;

			color = SDL_Color{ 255,255,255,255 };
		}

		SDL_Circle(double posX, double posY, double circleRadius, SDL_Color circleColor)
		{
			positionX = posX;
			positionY = posY;
			radius = circleRadius;

			rect.x = positionX - radius;
			rect.y = positionY - radius;
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

	//Textures
	extern SDL_Texture* rectangleTexture;
	extern SDL_Texture* circleTexture;

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
		double width;
		double height;
		double positionX;
		double positionY;

		SDL_Rectangle(double posX, double posY, double w, double h, SDL_Color squareColor)
		{
			positionX = posX;
			positionY = posY;

			width = w;
			height = h;

			rect.x = posX;
			rect.y = posY;
			rect.w = width;
			rect.h = height;

			color = squareColor;
		}

		SDL_Rectangle(double posX, double posY, double w, double h)
		{
			positionX = posX;
			positionY = posY;

			width = w;
			height = h;

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

