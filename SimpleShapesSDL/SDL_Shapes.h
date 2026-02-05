#pragma once
#include <SDL3/SDL.h>
#include <map>
#include <vector>



namespace SDL_Shapes
{
	//Class forward declarations
	class SDL_Shape;
	class SDL_Circle;
	class SDL_Rectangle;

	constexpr int Render_Resolution_Size = 512; // How big the texture will be in memory and quality of the texture

	//Textures
	extern SDL_Texture* rectangleTexture;
	extern SDL_Texture* circleTexture;

	/**
	 *  SDL_DrawShape
	 *
	 *	Draws singular shape passed into the parameter.
	 *  Much slower than SDL_DrawAllShapes for larger numbers of shapes.
	 *
	 * @param renderer, SDL_Renderer for the window.
	 * @param circles, Vector of circles to draw (nullptr if not used).
	 * @param rectangles, Vector of rectangles to draw (nullptr if not used).
	 * @param triangles, Vector of triangles to draw (nullptr if not used).
	 */
	void SDL_DrawShape(SDL_Renderer* renderer, SDL_Shape& shape);
	void SDL_DrawShape(SDL_Renderer* renderer, SDL_Shape* shape);

	/**
	 * SDL_DrawAllShapes
	 *
	 * Draw all shapes using GPU batching (much faster than SDL_DrawShape for large numbers of shapes).
	 *
	 * @param renderer, SDL_Renderer for the window.
	 * @param circles, Vector of circles to draw (nullptr if not used).
	 * @param rectangles, Vector of rectangles to draw (nullptr if not used).
	 * @param triangles, Vector of triangles to draw (nullptr if not used).
	 */
	void SDL_DrawAllShapes(SDL_Renderer* renderer, std::vector<SDL_Circle>* circles, std::vector<SDL_Rectangle>* rectangles);

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
		SDL_Texture* texture = nullptr;
		SDL_FRect rect;

	public:
		SDL_Color color;
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
		SDL_FRect rect;
		SDL_Texture* texture = nullptr;

	public:
		SDL_Color color;
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

