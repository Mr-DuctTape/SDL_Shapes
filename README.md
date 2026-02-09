# SDL_Shapes

SDL_Shapes is a small C++ helper library for drawing simple 2D shapes using **SDL3’s geometry rendering** and texture-based drawing functions.

It provides basic shape primitives and optional batching on top of `SDL_RenderGeometry`, with the goal of reducing draw calls while staying within SDL’s normal renderer-based workflow.

This library is intended to be copied directly into a project and used alongside an existing `SDL_Renderer`.

---

`main.cpp` serves as a simple showcase and usage example.

---

## What it’s for

- Drawing common 2D shapes (rectangles, circles, triangles)
- Rendering large numbers of shapes using GPU geometry
- Projects that want to stay within SDL’s renderer API
- Prototypes, tools, simulations, and simple games

---

## What it’s not

- Not a full rendering engine
- Not a replacement for OpenGL, Vulkan, or DirectX
- Not designed for advanced effects or complex materials

---

## Why this exists

SDL does not include built-in shape primitives beyond basic rectangles and lines.  
SDL_Shapes fills that gap by offering higher-level shape drawing while still relying on SDL’s geometry API.

If you’re already using SDL’s renderer, no additional graphics setup is required.

---

## Performance notes

Performance will vary depending on hardware and usage patterns.  
The library is still in early development, but the following results were recorded on a development system:

**Test system**
- Resolution: 1920×1080  
- GPU: NVIDIA MX150 (roughly GT 1030–class)
- CPU: Intel i7-8650U

**Results**
- 200k non-moving rectangles (batched): **60+ FPS** (vsync limited)
- 200k moving rectangles (batched): **~47–48 FPS**

The worst-case scenario is when all shapes are moving or rotating, as their vertices must be recalculated each frame. Static shapes are significantly cheaper to render due to reduced vertex updates.

---