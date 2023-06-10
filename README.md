# Scene Rendering with Ray Tracing

This GitHub repo serves as a front to present the researcher's BSc project on 'Scene Rendering using Ray Tracing'.

In this `README.md` file, we will explain the concepts related to ray tracing, the implementation of said concepts, and the steps to replicate the project.

# 00 Resources

The project emphasises the use of modern practical based literature concerning ray tracing:

- [Ray Tracing in One Weekend](https://raytracing.github.io/)
- [Physically Based Rendering: From Theory to Implementation](https://www.pbr-book.org/)

We are applying ray tracing techniques from the former book series in real time scenarios, as opposed to a program which simply writes pixels to a file. The principles of the latter book, "physically based rendering" will be used in the implementation of various ray tracing concepts.

Other resources:
- C++
- Windows 10
- [Visual Studio 2022](https://visualstudio.microsoft.com/)
- [Vulkan SDK](https://vulkan.lunarg.com/) (required)
- [Desmos](https://www.desmos.com/calculator)
- [Shadertoy](https://www.shadertoy.com/)
- [Scratchapixel](https://www.scratchapixel.com/)
- [Dear ImGui](https://github.com/ocornut/imgui)
- [TheCherno/WalnutAppTemplate](https://github.com/TheCherno/WalnutAppTemplate)
- [TheCherno/RayTracing](https://github.com/TheCherno/RayTracing)

# 01 Setting up the project

## 1.1 Using the Walnup App Template

[Walnut](https://github.com/TheCherno/WalnutAppTemplate) is an application development framework which will act as the base of the project.

<div align="center">
	<img src="https://i.imgur.com/vvIsZ7j.png" width="90%">
	<br><sub>Figure 1. The Walnut app template repo.</sub>
</div><br>

By clicking `Use this template` on the GitHub page, we can create a [private repository](https://github.com/athirazizi/RayTracing/). 

## 1.2 Cloning the Repo

Next, we will clone the newly created repo using the following `git` command:

```
git clone --recursive https://github.com/athirazizi/RayTracing/
```

## 1.3 Setting up the Project

Once the repo has been cloned, run the `Setup.bat` script:

<div align="center">
	<img src="https://i.imgur.com/93u0PCD.png" width="90%">
	<br><sub>Figure 2. The Setup.bat script.</sub>
</div><br>

This will create a `.sln` file for the Walnup App. The code for this can be found [here](https://github.com/TheCherno/WalnutAppTemplate/blob/master/WalnutApp/src/WalnutApp.cpp).

As with any new project, hit F5 and run the solution. This is what we are greeted with:

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/223400005-52ee0109-325f-4031-9c30-80f7d2b6e7b0.png" width="90%">
	<br><sub>Figure 3. The default Walnut App inferface.</sub>
</div><br>

The UI elements are rendered using Vulkan. The menus can be resized and docked to the window.

## 1.4 Rendering an Image

We can declare an image of the data type `Image` from the `Walnut` namespace:

```cpp
using namespace Walnut;

private:
	// this is the image
	std::shared_ptr<Image>image_;
```

Next, we want to modify the UI and create a button called Render which will call the `Render()` function to render the image.

```cpp
if (ImGui::Button("Render"))
{
	Render();
}
```

The `Render()` function:

```cpp
void Render() 
{
	// create an image if there is no image
	if (!image_) 
	{
		image_ = std::make_shared<Image>();
	}
}
```

We need a width and height for the image. For now we will make another window called Viewport, and this will act as the camera into the scene.

```cpp
// this is the camera
ImGui::Begin("Viewport");

// these are float values
viewport_width_ = ImGui::GetContentRegionAvail().x;	
viewport_height_ = ImGui::GetContentRegionAvail().y;

ImGui::End();
```

The values returned by the `ImGui` functions are floats; we can store these values as integers:

```cpp
private:
	uint32_t viewport_width_ = 0, viewport_height_ = 0;
```

Now we can use the values to set the width and height of the image:

```cpp
void Render() 
{
	// create an image if there is no image
	if (!image_) 
	{
		image_ = std::make_shared<Image>(viewport_width_, viewport_height_);
	}
}
```

We would also have to recreate the image if we resize the window/viewport: 

```cpp
void Render() 
{
	// create an image if there is no image or if the window has been resized
	if (!image_ || viewport_width_ != image_->GetWidth() || viewport_height_ != image_->GetHeight()) 
	{
		image_ = std::make_shared<Image>(viewport_width_, viewport_height_, ImageFormat::RGBA);
	}
}
```

For now we will use the RGBA image format. 32 bits is the size of the RGBA format; each channel is 1 byte.

We also have to create an image buffer for the pixel data:

```cpp
private:
	// buffer for image data
	uint32_t* image_data_ = nullptr;
```

When we resize the viewport, we want to delete the old image data, then reallocate it. If the image data is null, then nothing will be deleted.

```cpp
// create an image if there is no image
// or if the viewport lengths are not the same as the image lengths
if (!image_ || viewport_width_ != image_->GetWidth() || viewport_height_ != image_->GetHeight()) 
{
	image_ = std::make_shared<Image>(viewport_width_, viewport_height_, ImageFormat::RGBA);

	// delete the old image data
	delete[] image_data_;

	// reallocate the image data
	image_data_ = new uint32_t[viewport_width_ * viewport_height_];
}
```

To render our first image, we can assign the image data buffer with some data, then send it to the GPU to render.

```cpp
for (uint32_t i = 0; i < viewport_width_ * viewport_height_; i++)
{
	image_data_[i] = 0xffff00ff;
}

// set data, which uploads to the GPU to render
image_->SetData(image_data_);
```

The image data is set in an ABGR format, which is the reverse of the RGBA format.:

A|B|G|R
-|-|-|-
ff|ff|00|ff

We can then display the image:

```cpp
if (image_)
{
	// if there is an image, then display the image
	ImGui::Image(image_->GetDescriptorSet(), { (float)image_->GetWidth(), (float)image_->GetHeight() });
}
```

Hit F5 and click render and this is what we get:

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/223688212-4abdd8ae-7538-4866-94fe-4290d2d1a6af.png" width="90%">
	<br><sub>Figure 4. The first image render.</sub>
</div><br>

To fill out the entire Viewport with the image, we can get rid of the window padding:

```cpp
ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
ImGui::PopStyleVar();
```

## 1.5 Returning Random Colours for each Pixel

We can experiment further when returning a colour to render a pixel. For example, we can set the colour to be random:

```cpp
#include "Walnut/Random.h"

for (uint32_t i = 0; i < viewport_width_ * viewport_height_; i++)
{
	image_data_[i] = Random::UInt();
	image_data_[i] |= 0xff000000;
}
```

First, we assign a random value to each channel in the ABGR format. We can retain the opacity of each pixel by statically setting the alpha channel. This sets the most significant bytes, i.e., `ff` in `ff000000` to the alpha channel in the image data.

Hit F5 and click render again and this is what the program returns:

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/223690636-5c304494-6497-45f7-bb15-4fae23ec7f7c.png" width="90%">
	<br><sub>Figure 5. Returning random colours for each pixel in an image.</sub>
</div><br>

Since the button calls the `Render()` function, it will output different pixel colours every time it is clicked.

## 1.6 Timing the Render Function

We can add a `Timer` from the Walnut library to see how long it takes to render the image:

```cpp
void Render() 
{
	Timer timer;

	// create an image if there is no image
	// or if the viewport lengths are not the same as the image lengths
	if (!image_ || viewport_width_ != image_->GetWidth() || viewport_height_ != image_->GetHeight()) 
	{
		image_ = std::make_shared<Image>(viewport_width_, viewport_height_, ImageFormat::RGBA);

		// delete the old image data
		delete[] image_data_;

		// reallocate the image data
		image_data_ = new uint32_t[viewport_width_ * viewport_height_];
	}

	for (uint32_t i = 0; i < viewport_width_ * viewport_height_; i++)
	{
		image_data_[i] = Random::UInt();
		image_data_[i] |= 0xff000000;
	}

	// set data, which uploads to the GPU
	image_->SetData(image_data_);

	render_time_ = timer.ElapsedMillis();
}
```

Next we can display the time in the UI:

```cpp
ImGui::Text("Last render: %.3fms", render_time_);
```

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/223691524-385e279e-baaa-4b3a-aef2-7efdf8c21e18.png">
	<br><sub>Figure 6. Displaying the render time.</sub>
</div><br>

57 miliseconds to render a 720p image is not great. This is due to the project being configured as a debug build which has additional runtime checks which can slow render time. We configure the project using the Release build instead:

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/223691851-475ea2fc-6e67-40a4-a960-7f8130b308da.png">
	<br><sub>Figure 7. Configuring the project to Release build.</sub>
</div><br>

Run the project again and this is what the program returns:

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/223691951-822beacc-de04-460b-811a-b584315cb6c5.png">
	<br><sub>Figure 8. Shorter render time using the release build.</sub>
</div><br>

## 1.7 Realtime Rendering

We can render a random image every frame by adding `Render()` into the end of `OnUIRender()`.

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/223695777-e1fd89c9-bffa-4efe-a462-7c502fb904ee.gif" width="90%">
	<br><sub>Figure 9. Returning random colours for each pixel on UI render.</sub>
</div><br>

The code at this point can be seen [here](https://github.com/athirazizi/RayTracing/blob/35085b6828845eb0e34acbf00eea32b7d3e68ae5/RayTracing/src/WalnutApp.cpp).

# 02 Mathematics for Ray-Sphere Intersection

The main goal in section 2 and 3 is to render a sphere.

We want to specify a camera (to shoot our rays from) and a sphere in 3D space. Then, we want to generate an image which describes this scene.

Spheres are simple to define mathematically because they consist of:
- a 3D coordinate which defines its origin in 3D space,
- and a radius which defines how large the sphere is. 

In this section we will cover some of the mathematics related to ray tracing. The mathematics will be done in 2D (for the sake of simplicity) using a circle, since the only difference is that a sphere will have a $z$ component, whereas a circle does not. In the actual code, we will have to acknowledge the 3rd dimension.

## 2.1 Lines and rays

Relevant sources:

- [PBRT - 1.2.2 Ray-Object Intersections](https://www.pbr-book.org/3ed-2018/Introduction/Photorealistic_Rendering_and_the_Ray-Tracing_Algorithm#RayndashObjectIntersections)
- [RT in One Weekend - 5.1 Ray-Sphere Intersection](https://raytracing.github.io/books/RayTracingInOneWeekend.html#addingasphere/ray-sphereintersection)

We must first understand what a line is, since it is conceptually similar to a ray.

On a cartesian plane with an $x$ and $y$ axis, the most basic line we can draw is $y=x$:

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/223700356-6b3489e8-ac70-4538-b245-ee83ae72841a.png" width="90%">
	<br><sub>Figure 10. A straight line on a cartesian plane.</sub>
</div><br>

All lines follow the equation $y=mx+c$, where $m$ is the gradient and $c$ is the $y$ intercept.

Instead of straight lines, we will use vectors for ray tracing:

- We want to define a point in 3D space (the origin of our line)
- We want to specify a direction for the line to go towards.

In other words, this is a ray, expressed in terms of vectors.

In 2D, the origin will have an $x$ and $y$ component and the direction will also have an $x$ and $y$ component:

<div align="center">
	<img src="https://i.imgur.com/6lDkEjU.png">
	<br><sub>Figure 11. A ray with vec2 components.</sub>
</div><br>

In addition, we want to define a point along the line. In this case, we are using a $t$ parameter, which is the distance of the point along the line. It is also called a scalar since it scales along the vector.

We can think of this ray as a function:

$$P_{x,y} (t) = a_{x,y} + b_{x,y} \cdot t$$

Suppose that the origin had coordinates $(2,2)$ and a direction of $(1,1)$. If we wanted to find a point which is 2 units down that line, we would substitute $t=2$.

Substituting $t=2$ using the equation:

$$P_{x,y} (2) = (2,2) + (1,1) \cdot (2)$$

$$P_{x,y} (2) = (4,4)$$

The result is visualised below:

<div align="center">
	<img src="https://i.imgur.com/kFrQsjE.png">
	<br><sub>Figure 12. Substituting t = 2.</sub>
</div><br>

If we enter a negative variable to $t$, we will go backwards from the origin:

<div align="center">
	<img src="https://i.imgur.com/y9cIPgb.png">
	<br><sub>Figure 13. Negative and positive t parameters.</sub>
</div><br>

If the $x$ and $y$ values are not identical, we can split up the function by dimension/component:

$$P_{x} = a_{x} + b_{x} \cdot t$$

$$P_{y} = a_{y} + b_{y} \cdot t$$

These are called parametric equations. If there was a $z$ component, we would simply add another parametric equation:

$$P_{z} = a_{z} + b_{z} \cdot t$$

## 2.2 Spheres and circles

Spheres are similar to circles, except that they have a $z$ dimension. 

The equation of a sphere is similar to a circle, but with a $z$ component:

$$(x-a)^2 + (y-b)^2 + (z-c)^2 = r^2$$

The equation of a circle is:

$$(x-a)^2 + (y-b)^2 = r^2$$

Where $(a,	b)$ are the coordinates of the origin and $r$ is the radius.

For example:

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/223714444-9c569d19-5bd6-4b5f-9625-32b5a160693a.png">
	<br><sub>Figure 14. A circle on a cartesian plane.</sub>
</div><br>

This circle has the origin $(0,0)$ and a radius of $2$.

We can substitute these values into the equation:

$$(x-0)^2 + (y-0)^2 = 2^2$$

$$(x)^2 + (y)^2 = 4$$

Let us rearrange the equation to resemble $y=mx+c$:

$$y=\sqrt{4-x^2}$$

If we enter this equation into Desmos we would get:

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/223716951-3b43e862-30d9-48f8-81de-1e848f5d4d9c.png" width="90%">
	<br><sub>Figure 15. Entering the positive square root into Desmos.</sub>
</div><br>

We notice that only the first half of the circle is graphed.

When a value is squared, there are two solutions to a square root:

$$\sqrt{4}=2$$

$$\sqrt{4}=-2$$

We know that:

$$2^2 = 4$$

$$(-2)^2 = 4$$

We do not know whether the answer to $\sqrt{4}$ is $2$ or $-2$. Both solutions are acceptable. 

Because of this, we need to input plus or minus to the result of the square root to see the entire graph:

$$y=\pm\sqrt{4-x^2}$$

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/223717043-35ec022c-daf3-4a13-af47-4ce940cd6f1f.png" width="90%">
	<br><sub>Figure 16. Entering both positive and negative square roots.</sub>
</div><br>

Now, we can see both halves because we took into account both solutions.

Suppose that a ray has an origin $(-3,-3)$ that goes in the direction $(1,1)$. How would we know if it collides with the circle above?

On Desmos, we can visually see the solutions:

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/223718246-f25fae88-402c-4188-b675-22329abcd6ac.png" width="90%">
	<br><sub>Figure 17. Points of ray-object intersection.</sub>
</div><br>

We need to formulate an equation to find these points of intersect in program code. 

## 2.3 Ray-Sphere Intersection in Ray Tracing

We want to render an image similar to this:

<div align="center">
	<img src="https://i.imgur.com/JP84mO9.png">
	<br><sub>Figure 18. A sphere in 3D space.</sub>
</div><br>

We want to add a directional light to show that the object in the image is a 3D sphere:

<div align="center">
	<img src="https://i.imgur.com/D9Iaxmv.png">
	<br><sub>Figure 19. A sphere with a directional light in 3D space.</sub>
</div><br>

The image can be looked at from a 2D perspective:

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/223722525-1b92b11c-37b2-42ca-8daa-33b42389c9f7.png">
	<br><sub>Figure 20. Sending rays from the camera onto a scene with a sphere.</sub>
</div><br>

We can see that we are sending rays from the camera and determining if the rays collide with the sphere.

Depending on the points of collisions, which have coordinates of their own, we can set the colour of the pixel that was 'hit' to a particular value. 

## 2.4 Mathematics for Points of Collision

Suppose that we have a ray $a_{-3,-3} + b_{1,1} \cdot t$ and a circle $x^2 + y^2 + 2^2 = 0$, we can set up an equation to see where it collides.

To start with, let us split up the ray by the $x$ and $y$ dimension:

$$a_x + b_x \cdot t$$

$$a_y + b_y \cdot t$$

We can substitute these equations into $x$ and $y$ components of the circle:

$$(a_x + b_x t)^2 + (a_y + b_y t)^2 - 4 = 0$$

Expanding the equation:

$$(a_x + b_x t)(a_x + b_x t) + (a_y + b_y t)(a_y + b_y t) - 4 = 0$$

$$= a_{x}^2 + 2 a_{x} b_{x} t + b_{x}^2 t^2 + a_{y}^2 + 2 a_{y} b_{y} t + b_{y}^2 t^2 - 4 = 0$$

## 2.5 Quadratic Equations

We want to solve for $t$, and we can do this by using the quadratic equation $ax^2 + bx + c = 0$.

We can refactor the equation to be expressed in terms of $t$:

$$(b_x^2 +b_y^2)t^2 + (2a_xb_x + 2a_yb_y)t + (a_x^2 + a_y^2 - 4) = 0$$

We know all values except for $t$:

$b_x$ and $b_y$ is the directional vector of the ray. <br>
$a_x$ and $a_y$ is the origin of the ray.

As such we can simply substitute the values:

$$a=(-3,-3)$$

$$b=(1,1)$$

$$(1^2 + 1^2)t^2 + (2(-3)(1) + 2(-3)(1))t + ((-3)^2 + (-3)^2 -4) = 0$$

$$2t^2 - 12t + 14$$

## 2.6 Solving the equation

We can solve the equation using the quadratic formula:

$$x=\frac{-b\pm\sqrt{b^2-4ac}}{2a}$$

Our values are:

$$a=2$$

$$b=-12$$

$$c=14$$

## 2.7 Using the discriminant

We can use the discriminant $\sqrt{b^2 - 4ac}$ to tell us if there are any solutions and exactly how many solutions there are. 

Substituting our values into the discriminant and we get:

$$\sqrt{(-12)^2 - 4(2)(14)}$$

$$144 - 112 = 32$$

- If we get a value $>0$, then we have 2 solutions.
- If we get a value $=0$, we have 1 solution.
- If we get a value $<0$, we have 0 solutions.

## 2.8 Finding exact intersection points

Continue to solve the discriminant:

$$\frac{-b \pm \sqrt{32}}{2a}$$

$$= \frac{-(-12) \pm \sqrt{32}}{2(2)}$$

$$= \frac{12 \pm 5.66}{4}$$

$$t = 4.415, 1.585$$

We now know how far along the ray the intersections took place.

We can substitute these values into our parametric equations:

$$a_x +b_x t$$

$$a_y +b_y t$$

Since the $x$ and $y$ have the same values, we only have to substitute once.

$$-3 + 1(1.585) = (-1.415,-1.415)$$
$$-3 + 1(4.415) = (1.415,1.415)$$

The answer matches the coordinates in Desmos:

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/223718246-f25fae88-402c-4188-b675-22329abcd6ac.png" width="90%">
	<br><sub>Figure 21. Coordinates from Desmos.</sub>
</div><br>

Note that there is a rounding error because we approximated $\sqrt{32}$ as $5.66$.

# 03 Rendering a Sphere Using Ray Tracing

We will be taking the mathematical concepts in section 2 and implementing them in code in this section.

## 3.1 The Renderer Class

To start, we will be making two new files:

- `Renderer.h`
- `Renderer.cpp`

In `Renderer.h`, we will be declaring a class Renderer:

```cpp
#pragma once
class Renderer
{
public:
private:
};
```

The purpose of this class is to input a scene description for the 3D world that we are trying to render. The output is an image which holds the pixels that the renderer has produced which describes that scene.

For now we will have a default constructor:

```cpp 
Renderer() = default;
```

The `Render()` function will render every pixel in the scene:

```cpp
void Render();
```

```cpp
void Renderer::Render()
{
	// render every pixel 
}
```

The target of the `Render()` function is an Image from the Walnut library.

We will be moving the image we created in `WalnutApp.cpp` into `Renderer.h` like so:

```cpp
#pragma once
#include "Walnut/Image.h" // for Render()
#include <memory> // for shared pointers
class Renderer
{
public:
	Renderer() = default;
	void Render(); // renders every pixel 
private:
	std::shared_ptr<Walnut::Image>image_;
};
```

Recall the `Render()` function in `WalnutApp.cpp`. Before we render, we checked to see if the image dimensions are the same as the viewport dimensions. If they are not, then it will create a new image with the same dimensions as the viewport. 

We can split this into another function `OnResize()` which takes a width and height:

```cpp
void OnResize(uint32_t width, uint32_t height);
```

The Walnut library has a function `Resize()`:

```cpp
void Image::Resize(uint32_t width, uint32_t height)
{
	if (image_ && m_Width == width && m_Height == height)
		return;
	// TODO: max size?
	m_Width = width;
	m_Height = height;
	Release();
	AllocateMemory(m_Width * m_Height * Utils::BytesPerPixel(m_Format));
}
```

This checks if the image needs resizing, and if so, it releases and reallocates the memory of the image.

In the `Render()` function, we want to call `Resize()` before we render the image.

We also had a CPU side buffer for the image data which we need to put in the class:

```cpp
uint32_t* image_data_ = nullptr;
```

We will make another function `GetFinalImage()` which returns the final image.

The refactored code at this point can be seen [here](https://github.com/athirazizi/RayTracing/tree/974bdbdebc4bd9cda54c752c89c0ea9738824520/RayTracing/src).

## 3.2 Pixel Shaders

Pixel shaders are GPU programs that are executed for every pixel of an image that is rendered.

When triangles are rasterised, every pixel that gets generated as a result of that rasterisation process will invoke an instance of this pixel shader. We will be writing a pixel shader in C++.

An example of a pixel shader can be seen in [Shadertoy](https://www.shadertoy.com/new):

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/224481367-ef20e588-9f3f-41c0-9a40-3addcf1dafbb.gif" width="90%">
	<br><sub>Figure 22. Shadertoy.</sub>
</div><br>

Here, you can write a pixel shader and it will be rendered in real time. 

The code below gets invoked for every pixel of the viewport:

```cpp
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.xy;

    // Time varying pixel color
    vec3 col = 0.5 + 0.5*cos(iTime+uv.xyx+vec3(0,2,4));

    // Output to screen
    fragColor = vec4(col,1.0);
}
```

To explain it in a simpler, way:

```cpp
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = fragCoord/iResolution.xy;
    fragColor = vec4(uv,0.0,1.0);
}
```

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/224481534-179de138-258e-4dbd-af2b-a3329125cac6.png" width="90%">
	<br><sub>Figure 23. Simplified pixel shader in Shadertoy.</sub>
</div><br>

The $x$ axis is confined to a value between $0$ and $1$ and it is displayed as the red channel, and the $y$ axis acts similarly but it instead displays the green channel.

We will implment a pixel shader through a function `PerPixel()`. It returns a `uint_32t` value and has a `vec2` parameter which takes in coordinates.

```cpp
uint32_t PerPixel(glm::vec2 coord);
```

Let us examine the pixel shader in Shadertoy:

```cpp
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = fragCoord/iResolution.xy;
    fragColor = vec4(uv,0.0,1.0);
}
```

`fragCoord` is a pixel coordinate in pixel space. So in a resolution of 1280x720, a coordinate could be `(600,500)`. The division `fragCoord/iResolution.xy` maps the value to a range from $0$ to $1$. 

We can modify the `Render()` to contain two for loops, for $x$ and $y$ coordinates:

```cpp
for (uint32_t y = 0; y < final_image_->GetHeight(); y++)
{
	for (uint32_t x = 0; x < final_image_->GetWidth(); x++)
	{
		PerPixel();

		image_data_[i] = Walnut::Random::UInt();
		image_data_[i] |= 0xff000000;
	}
}
```

The reason we iterate the $y$ axis in the outer loop is because we want to be more friendly with the CPU cache (in terms of the layout of the buffer in memory):
- as we go $1$ `uint32_t` forwards we are going horizontally across the image.
- If we iterated the $x$ axis in the outer loop we would be skipping a portion of memory by going a full row forwards. This will slow down our program because the CPU cannot fetch the memory as easily.

We can determine the coordinate in the nested for loop:

```cpp
glm::vec2 coord = { (float)x / (float)final_image_->GetWidth(), (float)y / (float)final_image_->GetHeight() };
```

The coordinate is `x` divided by the total width of the image, and `y` divided by the total height.

We have to typecast the variables as float to perform float division, if not it will perform integer division.

We can then set each pixel colour using the `PerPixel()` function at the appropriate coordinate:

```cpp
image_data_[x + y * final_image_->GetWidth()] = PerPixel(coord);
```

We are multiplying the $y$ coordinate by how large each row supposedly is.

The `PerPixel()` function is responsible for providing a colour for each pixel. For now, we will set each pixel to green:

```cpp
uint32_t Renderer::PerPixel(glm::vec2 coord)
{
	return 0xff00ff00;
}
```

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/226174453-5257bcf3-7e46-4e28-8c9a-57b75145260b.png" width="90%">
	<br><sub>Figure 24. Using the PerPixel function to return a single colour.</sub>
</div><br>

We can recreate the Shadertoy pixel shader here:

```cpp
uint32_t Renderer::PerPixel(glm::vec2 coord)
{
	uint8_t r = (uint8_t)coord.x * 255.0f;
	uint8_t g = (uint8_t)coord.y * 255.0f;

	return 0xff000000 | (g << 8) | r;
}
```

Here we are setting the red channel to the `x` coordinate and green to the `y` coordinate.

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/226175125-1ed5b48c-8e7e-48f4-b08e-cccbbdbc4b7f.png" width="90%">
	<br><sub>Figure 25. Recreating the Shadertoy pixel shader.</sub>
</div><br>

The image is flipped upside down because of how ImGui displays the image. We have to add another parameter to `ImGui::Image` (the `uv0` and `uv1` parameters) as seen below:

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/226175271-b43e3223-d393-4378-a7a3-c05ccb4bbb71.png">
	<br><sub>Figure 26. ImGui Image parameters.</sub>
</div><br>

```cpp
if (image)
{
	// if there is an image, then display the image
	ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() },
		ImVec2(0, 1), ImVec2(1, 0));
}
```

We are reversing the `uv` coordinate of the $y$ axis. The `v` of the `uv` has now been inverted, and our image is now flipped right side up. Run the program again and this is what we get:

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/226175456-26bb06b1-2ac1-4f42-9e5b-8a3fb8caf0f1.png" width="90%">
	<br><sub>Figure 27. Corrected recreation of the pixel shader.</sub>
</div><br>

We can also test the `OnResize()` function here:

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/226175514-0e13f6c4-0762-48a9-a195-b19c827e7855.gif" width="90%">
	<br><sub>Figure 28. Testing the OnResize() function.</sub>
</div><br>

This coordinate system and the `PerPixel()` function is what we will be using to decide where to shoot our rays from and see if they intersect with the sphere.

The code at this point can be seen [here](https://github.com/athirazizi/RayTracing/tree/430039cafc33cf48d6ec4511cdf419ac265e1b5a/RayTracing/src).

## 3.3: Rendering the sphere

In this section, we will implement the mathematics in Section 2 into code. Recall that we have conducted the mathematics in 2D, but here we will be implementing it in 3D. As such, we have to add an extra $z$ axis to our equations to account for the $z$ dimension.

The quadratic equation for $t$ was:
$$(b_x^2 +b_y^2)t^2 + (2a_xb_x + 2a_yb_y)t + (a_x^2 + a_y^2 - 4) = 0$$

We can simplify the equation further:
$$(b_x^2 +b_y^2)t^2 + (2(a_xb_x + a_yb_y))t + (a_x^2 + a_y^2 - r^2) = 0$$

We are solving for $t$ , which is the distance along that ray. The unknown variables that we have are $a$, $b$, and $r$.

```cpp
// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
// 
// a = ray origin
// b = ray direction
// r = radius
// t = hit distance
```

We will name the variables `a`, `b`, and `c` because that is how they appear in the quadratic formula.

$$x=\frac{-b\pm\sqrt{b^2-4ac}}{2a}$$

Recall that $a$, $b$, and $c$ are coefficients of the quadratic equation.

Suppose that we have a camera at $(0,0,0)$. To calculate a ray per pixel, we can use the input coordinate of the `PerPixel()` function. The current setup for the coordinate system is that it goes from $(0,0)$ to $(1,1)$. If we are using this as the direction, we would only shoot out our rays like so:

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/226182672-99733608-fd60-4202-a526-64aa4656ff56.png">
	<br><sub>Figure 29. Current coordinate system.</sub>
</div><br>

We can remap the coordinates to go from $-1$ to $1$ rather than $0$ to $1$:

```cpp
// remap to -1 -> 1
coord = coord * 2.0f - 1.0f; 
```

The first variable `a` can then be calculated like so:

```cpp
float a = coord.x * coord.x + coord.y * coord.y + coord.z * coord.z;
```

Currently, the coordinate is of data type `vec2`. However, our coordinate system will use `vec3`.

Let us reimagine the scene again, except this time, the $y$ axis is flipped by $90\degree$:

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/226183557-d6ad7dee-b986-4680-8aa5-c60687c82c8a.png">
	<br><sub>Figure 30. Flipping the scene by 90 degrees.</sub>
</div><br>

A 3D coordinate system will have depth to it, so we will add a `z` axis with a range of $-1$ to $1$.

[OpenGL](https://learnopengl.com/Getting-started/Coordinate-Systems) uses a right-handed coordinate system where the positive $z$-direction [points out the page](https://pbr-book.org/3ed-2018/Geometry_and_Transformations/Coordinate_Systems). As such, $-1$ is the forward direction:

```cpp
glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
```

We can transform this to a unit vector using `glm::normalize()`:

```cpp
rayDirection = glm::normalize(rayDirection);
```

```cpp
float a = rayDirection.x * rayDirection.x + rayDirection.y * rayDirection.y + rayDirection.z * rayDirection.z;
```

This is actually the [dot product of the ray direction with itself](https://www.youtube.com/watch?v=piMDQD8Igg8). We are multiplying each term by itself and adding up the value to a float. We can rewrite this like so:

```cpp
float a = glm::dot(rayDirection, rayDirection);
```

Now we have our $a$ coefficient.

Variable `b` is actually also a dot product, because we are multiplying each component of the ray origin and each component of the ray direction and adding them up:

```cpp
float b = 2.0f * glm::dot(rayOrigin, rayDirection);
```

Similarly, variable `c` is a dot product of the ray origin and itself, minus the radius squared:

```cpp
float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;
```

The implementation of the discriminant is seen below:

```cpp
// quadratic formula discriminant
// b^2 - 4ac

float discriminant = b * b - 4.0f * a * c;
```

We can now check if the ray hits the object depending on the value of the discriminant:

Result | # of Solutions
--- | ---
$>0$ | 2 Solutions
$=0$ | 1 Solution
$<0$ | 0 Solutions

Here is an example:

```cpp
// > 0, 2 solutions
// = 0, 1 solution
// < 0, 0 solutions

if (discriminant >= 0.0f)
{
	return 0xff00ff00;
}

return 0xff000000;
```
If we hit the sphere, we should see a green colour. Else, it returns black.

Hit F5 and the program returns:

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/226185270-8f7efaad-3e8a-470d-a39f-4c9cd54fe5b9.png" width="90%">
	<br><sub>Figure 31. Running the ray-sphere intersection code for the first time.</sub>
</div><br>

It seems that every pixel returns green, so this means that every pixel is a ray-sphere intersect.

The ray origin is at $(0,0,0)$ and the sphere origin is also at $(0,0,0)$. This is visualised below:

<div align="center">
	<img src="https://i.imgur.com/ixncpPM.png">
	<br><sub>Figure 32. What happens when the ray origin and sphere origin share the same coordinates.</sub>
</div><br>

Essentially, the ray is inside the sphere and the sphere is filling up the viewport. 

We can move the sphere in the $-z$ direction such that it does not fill the camera viewport. Alternatively, we could move the ray origin in the $+z$ direction:

```cpp
glm::vec3 rayOrigin(0.0f, 0.0f, 2.0f);
```

Now the program returns:

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/226185723-5938e105-856e-41b0-a92a-151e61d003b7.png" width="90%">
	<br><sub>Figure 33. A render of a sphere.</sub>
</div><br>

Visually this looks like a circle, but it is actually a sphere in 3D space. It is hard to tell because of the flat lighting.

Another problem is that when we resize the viewport, the circle gets stretched:

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/226185950-d3c6ddb0-527d-4279-a41b-300f3bc3a08f.gif" width="90%">
	<br><sub>Figure 34. Resizing the render of the sphere.</sub>
</div><br>

Currently there is no concept of aspect ratio in the code. We are still in the $-1$ to $1$ space, horizontally and vertically. We will fix this in the next section.

The code at this point can be seen [here](https://github.com/athirazizi/RayTracing/tree/36b8991d39a194ca2fcd1f221e37f8f6a283bfa5/RayTracing/src).

# 04 Ray Casting and Sphere Intersection

## 4.1 Using Floats for Representing Colours

In this section, we can experiment with the 3D coordinate system by assigning each coordinate a colour. We can repurpose the `PerPixel()` function to output a `glm::vec4` data type:

```cpp
glm::vec4 Renderer::PerPixel(glm::vec2 coord)

if (discriminant >= 0.0f)
{
	return glm::vec4(0, 1, 0, 1); // this returns green (R, G, B, A)
}

return glm::vec4(0, 0, 0, 1); // this returns black
```

This way, the colours are represented within the $0$ to $1$ range. Note that the current coordinate system is within the $-1$ to $1$ range and we will have to refactor the range later.

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/233859470-6f7a9e26-360f-4788-b68f-9c498749c625.png">
	<br><sub>Figure 35. Recall that the PerPixel function returns a vec4 value.</sub>
</div><br>

Now, we have to convert our `PerPixel()` function to the RGBA format:

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/233859557-81520c49-4740-49e9-86f6-d6849e6a9819.png">
	<br><sub>Figure 36. A function to convert to RGBA format.</sub>
</div><br>

We will have to make a function to convert the colour to RGBA format. We can define the function in a separate namespace:

```cpp
namespace Utils {
	static uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		uint8_t r = color.r * 255.0f;
		uint8_t g = color.g * 255.0f;
		uint8_t b = color.b * 255.0f;
		uint8_t a = color.a * 255.0f;

		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
		return result;
	}
}
```

The ABGR format can be represented in a `uint32_t` value as seen above. The relevant channels are shifted to their appropriate bit ranges.

Note that the color in each channel should be within the range of $0$ to $1$ to prevent 'spilling' out into other channels. 


Graphics APIs like Vulkan, DirectX, and OpenGL [already clamps colour values](https://gamedev.stackexchange.com/a/132536), but since we currently have no graphics pipeline or GPU drivers, we have to code it ourselves.

```cpp
// clamp the range to 0 and 1
color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
```

## 4.2 Calculating Sphere Hit Coordinates

Next, we want to implement the second half of the quadratic formula:

$$\frac{-b\pm\sqrt{discriminant}}{2a}$$

Let us rework the `PerPixel()` function to return black if the discriminant is less than zero, else, return a defined sphere colour:

```cpp
if (discriminant < 0.0f)
{
	return glm::vec4(0, 0, 0, 1); // return black
}

glm::vec3 sphereColor(0, 1, 0);
return glm::vec4(sphereColor, 1.0f);
```

Implementating the rest of the quadratic formula:

```cpp
float discriminant = b * b - 4.0f * a * c;

// (-b +- sqrt(discriminant)) / 2a
// 
// > 0, 2 solutions
// = 0, 1 solution
// < 0, 0 solutions

// plus variant
float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a);
// minus variant
float t1 = (-b - glm::sqrt(discriminant)) / (2.0f * a);

glm::vec3 h0 = rayOrigin + rayDirection * t0;
glm::vec3 h1 = rayOrigin + rayDirection * t1;
```

The `h0` and `h1` values represent the potential hit points that can happen when a ray intersects with a sphere.

<div align="center">
	<img src="https://i.imgur.com/Upk9ilq.png">
	<br><sub>Figure 37. H0 is the hit upon entry; H1 is the hit upon leaving the sphere.</sub>
</div><br>

## 4.3 Closest Intersection Point

Recall that $t$ is the distance from the origin along the ray direction to our actual hit point. The smallest $t$ value would naturally be the closest intersection point. 

In this case, `t1` would always be the lower value since we are subtracting $-b$ with the discriminant.

```cpp
float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);

glm::vec3 hitPoint = rayOrigin + rayDirection * closestT;

glm::vec3 sphereColor(0, 1, 0);
sphereColor = hitPoint;
return glm::vec4(sphereColor, 1.0f);
```

## 4.4 Using colour to visualise numbers

Recall that each hitpoint has an $x$, $y$, $z$ coordinate for every pixel. We are using those values and outputting them as a colour.

$$x=r$$

$$y=r$$

$$z=b$$

Each plane represents a colour channel.

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/233863574-1c20bce8-75a5-4f9e-8b94-4f880fc95d4e.png" width="90%">
	<br><sub>Figure 38. Visualising coordinates as colours.</sub>
</div><br>

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/234369012-a3ed304f-5ab5-4f09-8118-5327ebc9e4c6.png">
	<br><sub>Figure 39. The x-axis representing red, y-axis representing green, and z-axis representing blue.</sub>
</div><br>

## 4.4 Lighting and Shading

Relevant sources:

- [PBRT - 12 Light Sources](https://www.pbr-book.org/3ed-2018/Light_Sources)

PBRT covers various light sources such as point lights, distant lights, area lights, and infinite area lights. For now we will be implementing the simplest light source which is a point light.

The easiest way to explain lighting and shading is by example:

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/233867103-0783a83a-b8af-43f4-83e9-131c1aa9465d.png" width="90%">
	<br><sub>Figure 40. If the object is facing the light, its appearance is brighter.</sub>
</div><br>

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/233867108-bf6b3c18-5649-422a-890d-d90ef1511eb2.png" width="90%">
	<br><sub>Figure 41. If the object is not facing the light, its appearance is dimmer.</sub>
</div><br>

This real-life observation can be implemented in our program. The idea is that we want to determine the direction that each pixel (of a surface) is facing. As mentioned earlier, if this pixel is facing towards a directional light source, also known as a [point light](https://www.pbr-book.org/3ed-2018/Light_Sources/Point_Lights), it will be rendered brighter compared to those pixels which are not facing towards the light.

## 4.5 Calculating Lighting using Normal Vectors

In this section, we want to determine, for each pixel, which direction it is facing. In other words, we want to calculate the normal (a vector perpendicular to the surface) of each pixel:

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/234367075-26d50a3e-a99d-4aef-af50-bd34b19b1cc2.png">
	<br><sub>Figure 42. Example of a surface normal.</sub>
</div><br>

Since we already know the position of the sphere's origin and the position of the hitpoints, we can subtract these two vectors. 

We know that the normal is the same as the hitpoint because the current sphere origin is $(0,0,0)$. However, we have to normalise this vector because the radius of the sphere affects the magnitude of the vector. Recall the size of the radius:

```cpp
float radius = 0.5f;
```

Consequently, the colours of the previously rendered sphere appears darker than expected because the colour range is actually from $0$ to $0.5$ and not $0$ to $1$. To find the normal we can use `glm::normalize`:

```cpp
glm::vec3 normal = glm::normalize(hitPoint);
```

Usually, we would subtract the sphere origin from the hitpoint. We will do this in a future section.

We can visualise the normal by assigning it to the sphere colour:

```cpp
glm::vec3 sphereColor(1, 0, 1);
sphereColor = normal;
return glm::vec4(sphereColor, 1.0f);	
```

The result is:

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/234369929-605b22aa-1a42-4855-8a43-19831884cb68.png" width="90%">
	<br><sub>Figure 43. Visualising normals on the sphere colour.</sub>
</div><br>

We are sending the rays towards the $-z$ direction. Therefore, the normal is facing towards the $+z$ direction, which makes the blue colour much more saturated.

## 4.6 Fixing Normal Visualisations

The normal vector can have valid values from $-1$ to $1$ for each component. We cannot see values which are less than $0$ in this case, resulting in less saturation for red and green colours. 

We can shift the values like so: 

$$-1\cdot0.5+0.5=0.0$$

$$1\cdot0.5+0.5=0.0$$

$$0\cdot0.5+0.5=0.0$$

```cpp
sphereColor = normal * 0.5f + 0.5f;
```

The result is:

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/234373539-071a830e-c28d-4d89-97f9-43ba2558b092.png" width="90%">
	<br><sub>Figure 44. Visualising a full range of colours on a sphere.</sub>
</div><br>

## 4.7 Calculating Lighting and Shading on a Sphere

For now we can set a simple point light to face the $-z$ direction and $-y$ direction. We also need to calculate the ingoing and outgoing vectors. 

This is the light direction:

```cpp
glm::vec3 lightDir = glm::normalize(glm::vec3(-1,-1,-1));
```

Recall that we are trying to determine the direction of the normal vs the direction of the light. The angle between these two vectors will show exactly how much light is shared between them.

Note that the vectors are facing in opposite directions. We can negate the light direction to calculate the angle between the two vectors by using a vector dot product:

$$a\cdot b = \cos(\theta)$$

In code, it looks like this:

```cpp
float d = glm::dot(normal, -lightDir);
```

The normal and the light direction have already been normalised, so they are unit vectors. As such, the above code is equivalent to the cosine of the angle. This returns a value between $-1$ and $1$ since it has the range of cosine, and it indicates how much the normal is facing towards the light source. 

For example, if the dot product returns $0$, then the cosine of the angle was $0$. The cosine of $90$ is also $0$. This means that the angle between the normal and the light direction in this case was $90\degree$. 

At this point we also know that any angles above 90 will return a negative value, hence, a brighter colour.

Now, we can assign the sphere colour by multiplying itself with the dot product, and this is what we get:

```cpp
sphereColor *= d;
```

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/234424595-38608de4-a833-4bc3-a2a3-aadf2dfed868.png" width="90%">
	<br><sub>Figure 45. A render of a sphere with shading based on a simple light source.</sub>
</div><br>

The code at this point can be seen [here](https://github.com/athirazizi/RayTracing/tree/747be4c357e9c5ba474d4a160df675c9a5ff9a05/RayTracing/src).

# 05 Implementing a User Interactive 3D Camera System

## 5.1 How 3D Cameras Work

Functionality wise, the user will have access to moving the position of the camera using the `W,A,S,D` keys on the keyboard, and moving the camera direction using the mouse.

This happens to be the traditional method used in modern first-person 3D video games, or any real time rasterisation/ray tracing applications.

The camera has two properties: the position (ray origin) and the ray direction. Currently the camera is implemented like so:

```cpp
glm::vec3 rayOrigin(0.0f, 0.0f, 1.0f);
glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
```

Essentially the rays are being sent out from the camera. Currently, there is no way to rotate the camera or adjust the field of view due to the static implementation of the origin and direction. 

## 5.2 The Camera Class Overview

The latest version of Walnut contains a `Camera` class. This can be seen here:

- [Camera.h](https://github.com/athirazizi/RayTracing/blob/master/RayTracing/src/Camera.h)
- [Camera.cpp](https://github.com/athirazizi/RayTracing/blob/master/RayTracing/src/Camera.cpp)

This camera system is similar to [Unity](https://docs.unity3d.com/Manual/class-Camera.html)/[Unreal Engine](https://docs.unrealengine.com/4.26/en-US/InteractiveExperiences/UsingCameras/) which allows the user to hold right click to rotate the camera and use `W,A,S,D` to move within the environment. 

The Camera constructor takes in the verticalFOV, a nearClip and a farClip:

```cpp
Camera(float verticalFOV, float nearClip, float farClip);
```

These parameters are visualised in the figure below:

<div align="center">
	<img src="https://user-images.githubusercontent.com/108275763/234670873-b9c32829-6350-48df-99c9-839e90317328.png" width="90%">
	<br><sub>Figure 46. A viewing frustum. Adapted from <a href="https://en.wikipedia.org/wiki/Viewing_frustum#/media/File:ViewFrustum.svg">wikipedia</a>.</sub>
</div><br>

A viewing frustum represents the camera field of view in a 3D region. The near plane indicated in yellow is the `nearClip` and the far plane indicated in blue is the `farClip`. Anything outside of this frustum is not rendered in the final scene because they are out of sight.

There are also various utility functions which we can use :
```cpp
const glm::mat4& GetProjection() const { return m_Projection; }
const glm::mat4& GetInverseProjection() const { return m_InverseProjection; }
const glm::mat4& GetView() const { return m_View; }
const glm::mat4& GetInverseView() const { return m_InverseView; }
const glm::vec3& GetPosition() const { return m_Position; }
const glm::vec3& GetDirection() const { return m_ForwardDirection; }
const std::vector<glm::vec3>& GetRayDirections() const { return m_RayDirections; }
float GetRotationSpeed();
```

The function `OnUpdate()` is called every frame with the timestep (which allows the camera to move at constant speed, independent of the frame rate):

`OnUpdate()` captures the delta of the mouse position from the previous frame to the current frame. This value is updated constantly (using each timestep) to make the system feel more responsive. 

If right click is not held down, the cursor mode is set to normal and it returns nothing. Else, we lock the cursor to the window and hide the cursor.

The `moved` flag is used to check if we need to recalculate the ray directions and matrices. 

The `upDirection` vector captures the $y$-directional vector.

The `rightDirection` vector captures the $x$-directional vector. This is the cross product of the `upDirection` and `m_ForwardDirection`.

The `W,A,S,D` keys are used as input keys:

If the user presses `W`, the camera moves forward. The distance it moves forward is multiplied by a fixed `speed` and `ts` (timestep, so the movement is scaled appropriately based on how fast the program is running). Similarly, the `S` key moves backwards, `D` moves to the right, `A` moves to the left. `E` moves the position of the camera upwards, and `Q` downwards. Additionally, the `moved` flag is set to true so the ray directions are recalculated. We can change the input keys using the `KeyCode` if the user prefers another movement setup.

Likewise, we will have to recalculate ray directions if the camera rotates. If the mouse moves up and down, the pitch will change depending on the `delta.y` component. If the mouse moves left and right, the yaw will change depending on the `delta.x` component.

The `pitchDelta` and `yawDelta` are used in a `glm::quat` quaternion, which captures the delta in all axes. This is used to produce a new forward direction by taking into account the quaternion. If the delta has changed in the $x$ or $y$ axis, then the view and the ray direction will have to be recalculated. This can be summed up in a [view matrix](https://forum.unity.com/threads/view-matrix-explanation.1198456/).

```cpp
void Camera::RecalculateView() {
	m_View = glm::lookAt(m_Position, m_Position + m_ForwardDirection, glm::vec3(0, 1, 0));
	m_InverseView = glm::inverse(m_View);
}
```

This function assigns a position, a vector to look at, and an a $y$-directional vector to the `m_View` attribute. We also calculate the inverse view by inverting the view matrix.

The function `OnResize()` is used to recalculate the projection matrix:
```cpp
void Camera::OnResize(uint32_t width, uint32_t height) {
	if (width == viewport_width_ && height == viewport_height_)
		return;

	viewport_width_ = width;
	viewport_height_ = height;

	RecalculateProjection();
	RecalculateRayDirections();
}
```

`RecalculateProjection()` creates a perspective matrix based on the FOV, the viewport width & height to calculate the aspect ratio, as well as the near and far clip plane:

```cpp
void Camera::RecalculateProjection() {
	m_Projection = glm::perspectiveFov(glm::radians(m_VerticalFOV), (float)viewport_width_, (float)viewport_height_, m_NearClip, m_FarClip);
	m_InverseProjection = glm::inverse(m_Projection);
}
```

<div align="center">
	<img src="http://www.songho.ca/opengl/files/gl_projectionmatrix01.png" width="90%">
	<br><sub>Figure 47. Perspective frustum. Adapted from <a href="http://www.songho.ca/opengl/gl_projectionmatrix.html#perspective">OpenGL</a>.</sub>
</div><br>

## 5.3 Calculating & Caching Per-Pixel Ray Directions

The Walnut camera has an attribute `m_RayDirections`: 

```cpp 
std::vector<glm::vec3> m_RayDirections;
```

It is associated with the `RecalculateRayDirections()` function which some cached calculations based on the view and projection matrices:

```cpp
void Camera::RecalculateRayDirections() {
	m_RayDirections.resize(viewport_width_ * viewport_height_);

	for (uint32_t y = 0; y < viewport_height_; y++) {
		for (uint32_t x = 0; x < viewport_width_; x++) {
			glm::vec2 coord = { (float)x / (float)viewport_width_, (float)y / (float)viewport_height_ };
			coord = coord * 2.0f - 1.0f; // -1 -> 1

			glm::vec4 target = m_InverseProjection * glm::vec4(coord.x, coord.y, 1, 1);
			glm::vec3 rayDirection = glm::vec3(m_InverseView * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0)); // World space
			m_RayDirections[x + y * viewport_width_] = rayDirection;
		}
	}
}
```

Note that this code is similar to what we currently have in `Renderer::Render()`. 

OpenGL and DirectX uses column-major maths which has matrices for [projection, view, model, and a vertex position](http://www.opengl-tutorial.org/beginners-tutorials/tutorial-3-matrices/#the-model-view-and-projection-matrices).

In a vertex shader, the vertex position is multiplied by the transform to specify its position in world space. Then, this is further multiplied by the view matrix and the projection matrix. 

For HLSL or DirectX (row-major), we would perform the inverse of these operations:

$$vertex \cdot transform \cdot view \cdot projection$$

The GLSL, OpenGL, or Vulkan (column-major) maths, the order would look like this:

$$projection \cdot view \cdot transform \cdot vertex$$

This produces normalised device coordinates (NDC) which has a range of $-1$ to $1$. We already know the NDC as these are our pixels. We are ray casting in world space, but we are restricted in the $-1$ to $1$ range. We can resolve this by multiplying the coordinate by the inverse of each matrix and perform a perspective division to get a coordinate in world space.

The `target` variable is an intermetiate vector which is the inverse projection multiplied by the coordinates.

The `rayDirection` multiplies the inverse view matrix by the normalised target with a perspective division.

We then cache the ray directions, as the normalisation operation and the matrix multiplication operations might slow the CPU down. [Single instruction multiple data](https://en.wikipedia.org/wiki/Single_instruction,_multiple_data) (SIMD) performs these operations in fewer instructions, leading to faster execution times. Later on, this code will be refactored to be run on the GPU inside a shader (computer/raygen shader) where execution times will be fast.

## 5.4 Using the Camera Class

In `WalnutApp.cpp`, our functions are called in the class `ExampleLayer`.

We can declare a camera as a member inside the Layer: 
```cpp
Camera camera_
```

and then define it in the constructor with the FOV, near clip, and farclip:

```cpp
ExampleLayer()
	: camera_(45.0f, 0.1f, 100.0f){}
```

We can override the function `OnUpdate()` in the `Layer` class. Here, we will update the camera:

```cpp
virtual void OnUpdate(float ts) override
{
	camera_.OnUpdate(ts);
}
```

We also want to pass in this camera into the renderer member:
```cpp
renderer_.Render(camera_);
```

and so we have to update the `Render()` function to take a camera as an input:

```cpp
void Render(const Camera& camera);
```

As we loop through every pixel in the `Render()` function, we need to obtain the appropriate cached ray direction. For now, the ray origin is a fixed value. Instead, we can change the ray origin to the current camera position.

```cpp
const glm::vec3& rayOrigin = camera.GetPosition();
```

The ray direction is used for every pixel coordinate so it is called inside two loops for the rendered scene:

```cpp
for (uint32_t y = 0; y < final_image_->GetHeight(); y++)
{
	for (uint32_t x = 0; x < final_image_->GetWidth(); x++)
	{
		const glm::vec3& rayDirection = camera.GetRayDirections()[x + y * final_image_->GetWidth()];
```

We have to refactor `PerPixel()` since we have removed the `coord` variable. Let use rename it to `TraceRay()` and take in a ray as a parameter. Recall that a ray has an origin and a direction. We will make a Ray header file which contains a data structure containing a ray:

`Ray.h`

```cpp
#pragma once

#include <glm/glm.hpp>

struct Ray
{
	glm::vec3 Origin;
	glm::vec3 Direction;
};
```

We can now create an object for the ray in `Render()`:

```cpp
void Renderer::Render(const Camera& camera)
{
	Ray ray;
	ray.Origin = camera.GetPosition();

	for (uint32_t y = 0; y < final_image_->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < final_image_->GetWidth(); x++)
		{
			ray.Direction = camera.GetRayDirections()[x + y * final_image_->GetWidth()];
			

			// set the pixel colour to each pixel
			glm::vec4 color = TraceRay(ray);
```

`TraceRay()` is refactored to use the new ray data structure and it can be seen [here](https://github.com/athirazizi/RayTracing/blob/c2bde46b7204a39fb344536672e427fd92949631/RayTracing/src/Renderer.cpp#L61).

Hit F5 and we can test the new inputs:

<div align="center">
	<img src="https://i.imgur.com/sAWGYQj.gif" width="90%">
	<br><sub>Figure 48. Camera movement in the x-axis.</sub>
</div><br>

<div align="center">
	<img src="https://i.imgur.com/sBn3JRt.gif" width="90%">
	<br><sub>Figure 49. Camera movement in the y-axis.</sub>
</div><br>

<div align="center">
	<img src="https://i.imgur.com/V36riev.gif" width="90%">
	<br><sub>Figure 50. Camera movement in the z-axis.</sub>
</div><br>

<div align="center">
	<img src="https://i.imgur.com/vcQdghg.gif" width="90%">
	<br><sub>Figure 51. Camera rotation; pitch and yaw.</sub>
</div><br>

The code at this point can be seen [here](https://github.com/athirazizi/RayTracing/tree/266ff1055a740664caad671735f7c737976878ab/RayTracing/src).

# 06 Scene Description

Relevant sources:

- [PBRT - Scene Description Interface](https://www.pbr-book.org/3ed-2018/Scene_Description_Interface)
- [RT in One Weekend - 9.5 A Scene with Metal Spheres](https://raytracing.github.io/books/RayTracingInOneWeekend.html#metal/ascenewithmetalspheres)

## 6.1 The Scene Struct

A scene description is a set of statements in program code which determines the positioning, geometry, materials, and lights in the scene which the user sees, and the effects that are used on objects in the environment.

PBRT designs their scene description interface based on this descriptive approach, which is what we will be doing as well, mainly due to these reasons:

- Users can specify properties of the scene at a high level
- Users can modify the algorithms of the system to better suit their needs

Our current scene description specifies a sphere at $(0,0,0)$, a camera at $(0,0,6)$, and a point light facing $(-1,-1,-1)$. We send our rays from the camera position and determine if they intersect with the sphere.

The code at this point only specifies one sphere, but what if we wanted to render multiple spheres? We would have to re-run our ray-sphere intersection code - the `TraceRay()` function - for additional spheres.

The `closestT` value is the distance from the surface of the sphere to the camera. We would have to compute this value for every sphere that is intersected in the scene.

Assuming we want to have a scene with only spheres, we can define them in a header file like so:

```cpp
#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Sphere
{
	glm::vec3 Position{0.0f};
	float Radius = 0.5f;

	glm::vec3 Albedo{1.0f};
};

struct Scene 
{
	std::vector<Sphere> Spheres;
};
```

The `Scene` is a struct containing `Spheres`. The sphere struct has a default position of $(0,0,0)$ and a default radius of $0.5$. Additionally, it has an [albedo](https://en.wikipedia.org/wiki/Albedo) component which is the diffuse reflected colour.

## 6.2 Sphere Positioning in Scenes

Recall the sphere equation:

$$(x-a)^2 + (y-b)^2 + (z-c)^2 = r^2$$

Where $x,y,z$ are coordinates of the sphere. Remember that our current sphere position is at the origin and so it is not taken into account when calculating $a,b,c$. As such we will have to refactor these calculations in `TraceRay()`.

Another change we can make is separating the ray origin and the camera position. To visualise, that looks like this:

<div align="center">
	<img src="https://i.imgur.com/MXYg3om.png">
	<br><sub>Figure 52. Separating the camera from the ray.</sub>
</div><br>

We can define an `origin` which is the ray origin minus the sphere position:

```cpp
glm::vec3 origin = ray.Origin - glm::vec3(1.0f, 0.0f, 0.0f);
```

This is then used in the calculations for the hitpoint. The result is:

<div align="center">
	<img src="https://i.imgur.com/GiN5Fe5.png" width="90%">
	<br><sub>Figure 53. The sphere is translated 1 unit to the right.</sub>
</div><br>

## 6.3 Renderer with a Scene Parameter

We can refactor the `Render()` function to take a scene as an input:

```cpp
void Render(const Scene& scene, const Camera& camera);
```

The same thing can be done for the `TraceRay()` function:

```cpp
glm::vec4 TraceRay(const Scene& scene, const Ray& ray);
```

An example of a sphere in a scene is seen below:

```cpp
// define a sphere
const Sphere& sphere = scene.Spheres[0];
```

We can then change the relevant statements using the sphere's components:

```cpp
glm::vec3 origin = ray.Origin - sphere.Position;
float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;
glm::vec3 sphereColor = sphere.Albedo;
```

In `WalnutApp.cpp`, we can include a scene as a private member and pass it into the renderer:

```cpp

void Render(){
	renderer_.Render(scene_, camera_);
}
private:
	Scene scene_;
```

In the `ExampleLayer()` constructor, we can define the spheres in the scene:

```cpp
ExampleLayer()
	: camera_(45.0f, 0.1f, 100.0f)
{
	{
		Sphere sphere;
		sphere.Position = { 0.0f, 0.0f, 0.0f };
		sphere.Radius = 1.0f;
		sphere.Albedo = { 0.0f,1.0f,0.0f };
		scene_.Spheres.push_back(sphere);
	}
}
```

We can make the ray tracing application more user interactable by adding ImGui controls to change the properties of the sphere in real time, like so:

```cpp
ImGui::DragFloat3("Position", glm::value_ptr(scene_.Spheres[0].Position), 0.1f);
ImGui::DragFloat("Radius", &scene_.Spheres[0].Radius, 0.1f);
ImGui::ColorEdit3("Albedo", glm::value_ptr(scene_.Spheres[0].Albedo));
```

Here we are using [DragFloats](https://wiki.giderosmobile.com/index.php/ImGui.Core:dragFloat3) which are sliders that allow us to edit the position and radius. [ColorEdit3](https://wiki.giderosmobile.com/index.php/ImGui.Core:colorEdit3) allows us to edit the RGB values of the sphere albedo.

<div align="center">
	<img src="https://i.imgur.com/PVugax9.png" width="90%">
	<br><sub>Figure 54. Customisable sphere position, radius, and albedo in real time.</sub>
</div><br>

## 6.4 Rendering multiple spheres

As mentioned before, there is only sphere in the scene. We can add more spheres like so:

```cpp
ExampleLayer()
	: camera_(45.0f, 0.1f, 100.0f)
{
	{
		Sphere sphere;
		sphere.Position = { 0.0f, 0.0f, 0.0f };
		sphere.Radius = 1.0f;
		sphere.Albedo = { 0.0f,1.0f,0.0f };
		scene_.Spheres.push_back(sphere);
	}
	{
		Sphere sphere;
		sphere.Position = { -1.0f, 0.0f, -5.0f };
		sphere.Radius = 1.0f;
		sphere.Albedo = { 0.0f,1.0f,1.0f };
		scene_.Spheres.push_back(sphere);
	}
}
```

However, only the sphere with lowest `closestT` value will be rendered. We can run the ray-sphere intersection calculations in a [for loop for every sphere in the scene](https://github.com/athirazizi/RayTracing/blob/1d88255b3fc612ce7601ff42767314be4089edc2/RayTracing/src/Renderer.cpp#L85).

We can also add controls for each sphere in the scene:
```cpp
for (size_t i = 0; i < scene_.Spheres.size(); i++)
{
	ImGui::PushID(i);
	Sphere& sphere = scene_.Spheres[i];

	ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f);
	ImGui::DragFloat("Radius", &sphere.Radius, 0.1f);
	ImGui::ColorEdit3("Albedo", glm::value_ptr(sphere.Albedo));

	ImGui::Separator();

	ImGui::PopID();
}
```

The result is:

<div align="center">
	<img src="https://i.imgur.com/LTlVjfp.png" width="90%">
	<br><sub>Figure 55. A scene with two customisable spheres.</sub>
</div><br>

The code at this point can be seen [here](https://github.com/athirazizi/RayTracing/tree/1d88255b3fc612ce7601ff42767314be4089edc2/RayTracing/src).

# 07 Modern Ray Tracing Pipelines

Relevant sources:

- [NVIDIA - The Ray Tracing Pipeline](https://www.youtube.com/watch?v=LoKUmbvbcRY)
- [NVIDIA - Ray Tracing Effects](https://www.youtube.com/watch?v=Rk5nD8tt_W4)
- [NVIDIA - DirectX Ray Tracing](https://developer.nvidia.com/blog/introduction-nvidia-rtx-directx-ray-tracing/)
- [NVIDIA - DX12 Ray Tracing Tutorial - Part 2](https://developer.nvidia.com/rtx/raytracing/dxr/dx12-raytracing-tutorial-part-2)

In this section, we will be refactoring the program to use modern ray tracing pipeline components. NVIDIA's DirectX uses 5 distinct shaders for ray tracing:

1. **Ray generation shader** - responsible for generating rays; sets up initial ray parameters
2. **Intersection shader** - determines if a ray-object intersection occurs and provides information about that intersection (surface normals, textures, material properties)
3. **Miss shader** - invoked when no ray-object intersection occurs, typically returning the background colour
4. **Closest hit shader** - invoked when a ray-object intersection occurs; returns the final colour and appearance of the intersected object, while considering its properties
5. **Any-hit shader** - optional shader used for transparent/translucent textures 

<div align="center">
	<img src="https://i.imgur.com/Dv0V4BE.png" width="90%">
	<br><sub>Figure 56. The ray tracing pipeline. Adapted from <a href="https://arxiv.org/pdf/2006.11348.pdf">NVIDIA</a>.</sub>
</div><br>

The current `Render()` function sends a ray for each pixel in the viewport and within the function, `TraceRay()` returns a colour for each ray that is traced. However, in modern ray tracing pipelines, typically more than one ray is casted for each pixel to decide its final colour.

<div align="center">
	<img src="https://i.imgur.com/oUHS2ri.png" width="90%">
	<br><sub>Figure 57. The ray tracing process. Adapted from <a href="https://www.youtube.com/watch?v=LoKUmbvbcRY">NVIDIA</a>.</sub>
</div><br>

After sending out rays, they will traverse throughout the scene. Depending on the invoked shaders, such as the closest hit shader, the rays could recurse and additional rays are sent out and these would affect the final rendered colour of each pixel.

We will be using NVIDIA's naming conventions for the mentioned shaders:

- `RayGen()` - ray generation shader
- `ClosestHit()` - closest hit shader
- `Miss()` - miss shader

The intersection shader is built into the `TraceRay()` function and the `AnyHit()` shader is skipped for now.

## 7.1 Ray Generation Shader

`TraceRay()` is invoked in the `RayGen()` function and it should only be responsible for shooting out rays and return something called a payload ("the result of the execution of various shaders potentially invoked during raytracing").

We will store this payload into a struct `HitInfo`:

```cpp
struct HitInfo {
	float HitDistance;
	glm::vec3 WorldPosition;
	glm::vec3 WorldNormal;
	uint32_t ObjectIndex;
};
```

This is the new return data type for `TraceRay()`.

Additionally, the `HitInfo` is used for `ClosestHit()` and `Miss()`:

```cpp
// ray generation shader
glm::vec4 RayGen(uint32_t x, uint32_t y);
HitInfo TraceRay(const Ray& ray);

// closest-hit shader
HitInfo ClosestHit(const Ray& ray, uint32_t object_index, float hit_distance);

// miss shader
HitInfo Miss(const Ray& ray);
```

The `RayGen()` implementation can be seen [here](https://github.com/athirazizi/RayTracing/blob/37887afd320b09105da20d9d737fbd7c8cb97a5c/RayTracing/src/Renderer.cpp#L72). It generates rays for every pixel in the scene and invokes `TraceRay()` which sets the colour for each pixel. It also handles lighting/shading calculations.

The `TraceRay()` implementation can be seen [here](https://github.com/athirazizi/RayTracing/blob/37887afd320b09105da20d9d737fbd7c8cb97a5c/RayTracing/src/Renderer.cpp#L123). It is responsible for tracing each ray in the scene, and determines if ray-object intersection occurs. If it does, `ClosestHit()` is invoked, else `Miss()` is invoked.

## 7.2 Closest-hit shader

The `ClosestHit()` implementation can be seen [here](https://github.com/athirazizi/RayTracing/blob/37887afd320b09105da20d9d737fbd7c8cb97a5c/RayTracing/src/Renderer.cpp#L186). This calculates the payload position and normal.

## 7.3 Miss shader

The `Miss()` implementation can be seen [here](https://github.com/athirazizi/RayTracing/blob/37887afd320b09105da20d9d737fbd7c8cb97a5c/RayTracing/src/Renderer.cpp#L205). This simply returns the payload if no objects have been hit.

## 7.4 Reflection

As mentioned before, it is possible to recurse and send out additional rays for various rendering effects like reflection, ambient occlusion, and hard shadows.

If the statements in `RayGen()` were iterated for each bounce, we could calculate the origin and direction for reflection rays like so:

```cpp
// change origin and direction for the next bounce
ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
ray.Direction = glm::reflect(ray.Direction, payload.WorldNormal);
```

The `glm::reflect()` [function](https://registry.khronos.org/OpenGL-Refpages/gl4/html/reflect.xhtml) calculates the reflection direction for an incident vector. Note that we have to offset `payload.WorldNormal` by a small amount to ensure the reflection ray bounces off the surface of the sphere.

Hit F5 and this is what we get:

<div align="center">
	<img src="https://i.imgur.com/60VSm5y.png" width="90%">
	<br><sub>Figure 58. Reflection between two spheres.</sub>
</div><br>

<div align="center">
	<img src="https://i.imgur.com/r6uWZMY.png" width="90%">
	<br><sub>Figure 59. Reflection between four spheres.</sub>
</div><br>

The code at this point can be seen [here](https://github.com/athirazizi/RayTracing/tree/37887afd320b09105da20d9d737fbd7c8cb97a5c/RayTracing/src).

# 08 Materials & Physically Based Rendering (PBR)

Relevant sources:

- source
- source

# 09 Path Tracing

Relevant sources:

- source
- source

# 10 Multithreaded Rendering, Parallel Processing, & Optimisation

Relevant sources:

- source
- source

# 11 Further system improvements

Relevant sources:

- source
- source

# 12 Emission & Emissive Materials

Relevant sources:

- source
- source