#pragma once
#include <SDL3/SDL.h>
#include <vector>



namespace SDL_Shapes
{
	//Class forward declarations
	class SDL_Shape;
	class SDL_Circle;
	class SDL_Rectangle;
	class SDL_Triangle;

	// How big the texture will be in memory, quality of the texture. Could effect performance in certain scenarios
	// Texture is N*N, example: 312*312
	// Recommended values: 1028, 512, 256
	constexpr int Render_Resolution_Size = 256; 

	//Textures
	extern SDL_Texture* rectangleTexture;
	extern SDL_Texture* circleTexture;
	extern SDL_Texture* triangleTexture;

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

	//SDL_DrawAllShapes
	//Converts the shapes into verticies, indicies and sent to the GPU.
	//Instead of a seperate draw call per shape (600k draw calls for 600k shapes)
	//It'll instead become (1 drawcall per 600k shapes)
	//If the shapes are different it's at worst 1 drawcall per different shape 
	//(1 for triangles, 1 for squares, 1 for circles etc..)
	void SDL_DrawAllShapes(SDL_Renderer* renderer, std::vector<SDL_Circle>* circles, std::vector<SDL_Rectangle>* rectangles, std::vector<SDL_Triangle>* triangles);

	//Base class
	class SDL_Shape
	{
	protected:
		bool Shape_CompareColors(SDL_Color& other1, SDL_Color& other2);

		virtual void RenderShape(SDL_Renderer*);

		friend void SDL_DrawShape(SDL_Renderer* renderer, SDL_Shape& shape);
		friend void SDL_DrawShape(SDL_Renderer* renderer, SDL_Shape* shape);

	public:
		SDL_FRect rect{};
		SDL_Texture* texture = nullptr;
	};

	//Shape classes
	class SDL_Circle : public SDL_Shape
	{
	private:
		SDL_Texture* texture = nullptr;
	public:
		double radius;
		double positionX, positionY;
		SDL_Color color;

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

		inline void set_color(const SDL_Color& circleColor)
		{
			color = circleColor;
		}
	};

	class SDL_Triangle : public SDL_Shape
	{
	public:
		SDL_Color color;
		double height;
		double width;
		double positionX, positionY;
		double rotation = 0; // In degrees not radians

		SDL_Triangle(double h, double w, double x, double y, SDL_Color color) 
			: height(h), width(w), positionX(x), positionY(y) ,color(color)
		{

			rect.h = height;
			rect.w = width;
			rect.x = positionX;
			rect.y = positionY;
		}
		SDL_Triangle(double h, double w, double x, double y)
			: height(h), width(w), positionX(x), positionY(y)
		{
			color = SDL_Color{ 255,255,255,255 };
			rect.h = height;
			rect.w = width;
			rect.x = positionX;
			rect.y = positionY;
		}
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
	public:
		SDL_Color color;
		double width;
		double height;
		double positionX;
		double positionY;
		double rotation = 0;

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

