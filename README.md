# RayTracing

Hello! This GitHub repo serves as a front to present my BSc project on "Scene Rendering using Ray Tracing".
In this `README.md` file, I will explain fully the concepts related to ray tracing, and the steps to replicate the project.

# Section 0: Resources

Luckily for us, there is an abundance of resources for ray tracing since its first implementation in 1982:

<img src = "https://user-images.githubusercontent.com/108275763/223368461-63fd1ca4-da48-4b0d-8e5e-1fd7241e02ca.png" height = 128>

For the purposes of this project, we will be using the following:
- Ray Tracing in One Weekend Series <br> https://raytracing.github.io/
- Physically Based Rendering, From Theory to Implementation <br> https://www.pbr-book.org/

We are applying ray tracing techniques from the former book series in real time scenarios, as opposed to a program which simply writes pixels to a file.
The source content will be similar, but we will also implement and use triangles which can later be used to render 3D meshes.

Other resources:
- C++
- Windows 10
- Visual Studio 2022 <br> https://visualstudio.microsoft.com/
- Vulkan SDK <br> https://vulkan.lunarg.com/
- Desmos <br> https://www.desmos.com/calculator
- imgui <br> https://github.com/ocornut/imguib

# Section 1: Welcome to Ray Tracing!

## Step 1: Using Walnut

Walnut is an application development framework developed by GitHub user [@TheCherno](https://github.com/TheCherno/) and this will act as the base of the project.

- Walnut template <br> https://github.com/TheCherno/WalnutAppTemplate

<img src = "https://user-images.githubusercontent.com/108275763/223398284-b21ad558-b5c9-4c74-ba6e-f2620440e471.png" height = 256>

By clicking 'Use this template' on the GitHub Page, we have created a private repository (this repo!).

- https://github.com/athirazizi/RayTracing/

## Step 2: Clone the Repo

Next, we will clone the newly created repo using the following command:

```
git clone --recursive https://github.com/athirazizi/RayTracing/
```

## Step 3: Setting up the project

Once the repo has been cloned, run the `Setup.bat` script:

<img src = "https://user-images.githubusercontent.com/108275763/223405033-9927e8f8-fb72-4891-99ac-7ac27e061f81.png" height = 512>


Opening the `Walnut.cpp` file:

```cpp
#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"

class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Hello");
		ImGui::Button("Button");
		ImGui::End();

		ImGui::ShowDemoWindow();
	}
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Walnut Example";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}
```

As with any new project, hit F5 and run the solution. This is what we are greeted with:

<img src="https://user-images.githubusercontent.com/108275763/223400005-52ee0109-325f-4031-9c30-80f7d2b6e7b0.png" height = 512>

The UI elements are rendered using Vulkan. The menus can be resized and docked to the window.

- How do I load up an image?

To create an image, we use the following code:

```cpp
using namespace Walnut;

private:
	// this is the image
	std::shared_ptr<Image>m_Image;
```

Next, we want to modify the UI and create a button called Render, to render the image.

```cpp
if (ImGui::Button("Render"))
{
	// this renders every frame.
	Render();
}
```

The render function:

```cpp
void Render() 
{
	// create and image if there is no image
	if (!m_Image) 
	{
		m_Image = std::make_shared<Image>();
	}
}
```
- What do we enter in the parameters?

We need a width and height for the  image. For now we will make another window called Viewport, and this will act as the camera into the scene.

```cpp
// this is the camera
ImGui::Begin("Viewport");

// these are float values
m_ViewportWidth = ImGui::GetContentRegionAvail().x;	
m_ViewportHeight = ImGui::GetContentRegionAvail().y;

ImGui::End();
```

We want to store these float values as integers:

```cpp
private:
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
```

Now we can use the values to set the width and height of the image:

```cpp
void Render() 
{
	// create an image if there is no image
	if (!m_Image) 
	{
		m_Image = std::make_shared<Image>(m_ViewportWidth, m_ViewportHeight);
	}
}
```

- What happens if we resize the window?

We would have to recreate the image:

```cpp
void Render() 
{
	// create an image if there is no image
	if (!m_Image || m_ViewportWidth != m_Image->GetWidth() || m_ViewportHeight != m_Image->GetHeight()) 
	{
		m_Image = std::make_shared<Image>(m_ViewportWidth, m_ViewportHeight, ImageFormat::RGBA);
	}
}
```

For now we will use the RGBA image format.

> Side note: 32 bits is the size of the RGBA format; each channel is 1 byte.

We also have to create an image buffer for the pixel data:

```cpp
private:
	// buffer for image data
	uint32_t* m_ImageData = nullptr;
```

When we resize the viewport, we want to delete the old image data, then reallocate it.

```cpp
// create an image if there is no image
// or if the viewport lengths are not the same as the image lengths
if (!m_Image || m_ViewportWidth != m_Image->GetWidth() || m_ViewportHeight != m_Image->GetHeight()) 
{
	m_Image = std::make_shared<Image>(m_ViewportWidth, m_ViewportHeight, ImageFormat::RGBA);

	// delete the old image data
	delete[] m_ImageData;

	// reallocate the image data
	m_ImageData = new uint32_t[m_ViewportWidth * m_ViewportHeight];
}
```

Let's fill the image data buffer with some data, then send it to the GPU to render:

```
for (uint32_t i = 0; i < m_ViewportWidth * m_ViewportHeight; i++)
{
	m_ImageData[i] = 0xffff00ff;
}

// set data, which uploads to the GPU
m_Image->SetData(m_ImageData);
```

The image data is set in an AGBR format:

A|G|B|R
-|-|-|-
ff|ff|00|ff

Essentially the reverse of the RGBA format.

Next we want to display the image:

```cpp
if (m_Image)
{
	// if there is an image, then display the image
	ImGui::Image(m_Image->GetDescriptorSet(), { (float)m_Image->GetWidth(), (float)m_Image->GetHeight() });
}
```

If you have done everything correctly, hit F5 and click render. This is what you should get:

<img src = "https://user-images.githubusercontent.com/108275763/223688212-4abdd8ae-7538-4866-94fe-4290d2d1a6af.png">

If you want to fill out the Viewport with the image, we have to get rid of the padding:

```cpp
ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
ImGui::PopStyleVar();
```

So we've got a basic image to output. Great!

Lets add some randomness to the image: 

```cpp
#include "Walnut/Random.h"

for (uint32_t i = 0; i < m_ViewportWidth * m_ViewportHeight; i++)
{
	m_ImageData[i] = Random::UInt();
	m_ImageData[i] |= 0xff000000;
}
```

We don't want the alpha channel to be random so we want to retain its opacity.
It sets the most significant bytes, i.e., `ff` in `ff000000` to the alpha channel in the image data.

Hit F5 and render again and this is what you see:

![image](https://user-images.githubusercontent.com/108275763/223690636-5c304494-6497-45f7-bb15-4fae23ec7f7c.png)

Since the button calls the function Render, it will output a different random image every time.

We also want to add a timer to see how long it takes to render the image:

```cpp
void Render() 
{
	Timer timer;

	// create an image if there is no image
	// or if the viewport lengths are not the same as the image lengths
	if (!m_Image || m_ViewportWidth != m_Image->GetWidth() || m_ViewportHeight != m_Image->GetHeight()) 
	{
		m_Image = std::make_shared<Image>(m_ViewportWidth, m_ViewportHeight, ImageFormat::RGBA);

		// delete the old image data
		delete[] m_ImageData;

		// reallocate the image data
		m_ImageData = new uint32_t[m_ViewportWidth * m_ViewportHeight];
	}

	for (uint32_t i = 0; i < m_ViewportWidth * m_ViewportHeight; i++)
	{
		m_ImageData[i] = Random::UInt();
		m_ImageData[i] |= 0xff000000;
	}

	// set data, which uploads to the GPU
	m_Image->SetData(m_ImageData);

	m_LastRenderTime = timer.ElapsedMillis();
}
```

We'll display the time in the GUI:

```cpp
ImGui::Text("Last render: %.3fms", m_LastRenderTime);
```

![image](https://user-images.githubusercontent.com/108275763/223691524-385e279e-baaa-4b3a-aef2-7efdf8c21e18.png)

- Why is my render taking 57 miliseconds to complete?

It's because we are in the debug build; we should change  it to release:

![image](https://user-images.githubusercontent.com/108275763/223691851-475ea2fc-6e67-40a4-a960-7f8130b308da.png)

![image](https://user-images.githubusercontent.com/108275763/223691951-822beacc-de04-460b-811a-b584315cb6c5.png)

We can render this image every frame by adding `Render()` into the end of `OnUIRender()`.

![RayTracing_a3vckzBqnp](https://user-images.githubusercontent.com/108275763/223695777-e1fd89c9-bffa-4efe-a462-7c502fb904ee.gif)

At this point, the code should look like this:

```cpp
#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"


using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");

		ImGui::Text("Last render: %.3fms", m_LastRenderTime);

		if (ImGui::Button("Render"))
		{
			// this renders every frame.
			Render();
		}

		ImGui::End();

		// gets rid of border around the viewport
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		// this is the camera
		ImGui::Begin("Viewport");
		
		// these are float values
		m_ViewportWidth = ImGui::GetContentRegionAvail().x;	
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;
		
		if (m_Image)
		{
			// if there is an image, then display the image
			ImGui::Image(m_Image->GetDescriptorSet(), { (float)m_Image->GetWidth(), (float)m_Image->GetHeight() });
		}

		ImGui::End();
		ImGui::PopStyleVar();

		Render();
	}

	void Render() 
	{
		Timer timer;

		// create an image if there is no image
		// or if the viewport lengths are not the same as the image lengths
		if (!m_Image || m_ViewportWidth != m_Image->GetWidth() || m_ViewportHeight != m_Image->GetHeight()) 
		{
			m_Image = std::make_shared<Image>(m_ViewportWidth, m_ViewportHeight, ImageFormat::RGBA);
			
			// delete the old image data
			delete[] m_ImageData;

			// reallocate the image data
			m_ImageData = new uint32_t[m_ViewportWidth * m_ViewportHeight];
		}

		for (uint32_t i = 0; i < m_ViewportWidth * m_ViewportHeight; i++)
		{
			m_ImageData[i] = Random::UInt();
			m_ImageData[i] |= 0xff000000;
		}

		// set data, which uploads to the GPU
		m_Image->SetData(m_ImageData);

		m_LastRenderTime = timer.ElapsedMillis();
	}

private:
	// this is the image
	std::shared_ptr<Image>m_Image;
	// buffer for image data
	uint32_t* m_ImageData = nullptr;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
	float m_LastRenderTime = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "RayTracing";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;	
}
```

# Section 2: Rays, Sphere, and Mathematics

The first goal in this section is to render a sphere. 

We want to specify a camera and a sphere in 3D space. Then, we want to generate an image which describes this scene. 

- Why spheres?

Because they are simple to define mathematically:
- Spheres consist of a 3D coordinate which defines its origin in a 3D space,
- and a radius which defines how big the sphere is. 

In this section I will cover some of the mathematics related to ray tracing. The mathematics will be done in 2D (for the sake of simplicity), since the only difference is that a sphere will have a $z$ component, where a circle does not. In the actual code, of course we will have to acknowledge the 3rd dimension.

## Section 2.1: Lines and rays

To get started, we must understand what a line is, since it is conceptually similar to a ray.

On a cartesian plane, with an $x$ axis and a $y$ axis, the most basic line we can draw is $y=x$:

<img src="https://user-images.githubusercontent.com/108275763/223700356-6b3489e8-ac70-4538-b245-ee83ae72841a.png" height=512> 

All lines follow the equation $y=mx+c$, where $m$ is the gradient and $c$ is the $y$ intercept.

Instead of straight lines though, for ray tracing we will use vectors:

- We want to define a point in 3D space (the origin of our line)
- We want to specify a direction for the line to go towards.

In other words, this is a ray, expressed in terms of vectors.

In 2D, the origin will have an $x$ and $y$ component and the direction will also have an $x$ and $y$ component:

![image](https://user-images.githubusercontent.com/108275763/223703417-e6a1dde3-714a-40e5-9884-b5c352a5041b.png)

In addition, we want to define a point on the line somewhere. In this case, we are using a $t$ parameter, which is the distance of the point along the line. It is also called a scalar since it scales along the vector.

We can think of this ray as a function:

$$P_{x,y} (t) = a_{x,y} + b_{x,y} \cdot t$$

Suppose that the origin had coordinates $(2,2)$ and the direction of $(1,1)$:
<br> If we wanted to find a point a long the line of 2 units down that line, we would substitute $t=2$.

Using the equation:

$$P_{x,y} (2) = (2,2) + (1,1) \cdot (2)$$

This would return:

$$P_{x,y} (2) = (4,4)$$

![image](https://user-images.githubusercontent.com/108275763/223706941-771ff071-c817-4244-8f8b-53ae7c338a66.png)

If we enter a negative variable to $t$, we will go backwards from the origin:

![image](https://user-images.githubusercontent.com/108275763/223708157-bb4fff9c-7c6d-493a-831c-ac223e30217e.png)

If the values are not identical in $x$ and $y$, we can split up the function by dimension or by component:

$$P_{x} = a_{x} + b_{x} \cdot t$$

$$P_{y} = a_{y} + b_{y} \cdot t$$

These are called parametric equations. If there was a $z$ component, we would simply add another parametric equation.

## Section 2.2: Spheres and circles

Spheres are similar to circles, except that they have a $z$ dimension. 

The equation of a circle is:

$$(x-a)^2 + (y-b)^2 = r^2$$

Where $(a,b)$ are the coordinates of the origin and $r$ is the radius.

For example:

![image](https://user-images.githubusercontent.com/108275763/223714444-9c569d19-5bd6-4b5f-9625-32b5a160693a.png)

This circle has the origin $(0,0)$ and a radius of $2$.

We can substitute these values into the equation:

$$(x-0)^2 + (y-0)^2 = 2^2$$

$$(x)^2 + (y)^2 = 4$$

Let us rearrange the equation to resemble $y=mx+c$:

$$y=\sqrt{4-x^2}$$

Entering this into desmos and we get:

![image](https://user-images.githubusercontent.com/108275763/223716951-3b43e862-30d9-48f8-81de-1e848f5d4d9c.png)


- Why is the other half missing?

When you square something, there are two solutions to a square root:

$$\sqrt{4}=2$$

$$\sqrt{4}=-2$$

We know that:

$$2^2 = 4$$

$$(-2)^2 = 4$$

We do not know whether the answer to $\sqrt{4}$ is $2$ or $-2$. It could be either one. 

Because of this, to see the entire graph, we need to input plus or minus on the result of the square root:

$$y=\pm\sqrt{4-x^2}$$

![image](https://user-images.githubusercontent.com/108275763/223717043-35ec022c-daf3-4a13-af47-4ce940cd6f1f.png)

Now, we can see both halves because we take into account both solutions.

Suppose that a ray has an origin $(-3,-3)$ that goes in direction $(1,1)$, how would know if it collides with the circle above?

On Desmos, we can visually see the solutions:

![image](https://user-images.githubusercontent.com/108275763/223718246-f25fae88-402c-4188-b675-22329abcd6ac.png)

We need to formulate an equation to find these collisions in program code. 

The equation of sphere is similar to a circle, but with a $z$ component:

$$(x-a)^2 + (y-b)^2 + (z-c)^2 = r^2$$

## Section 2.3: How does this relate to ray tracing?

We want to render an image similar to this:

![image](https://user-images.githubusercontent.com/108275763/223720753-736b8c3b-b8be-43a8-9113-a32753af0263.png)

And later, we want to add some lighting to show that the object in the image is a 3D sphere:

![image](https://user-images.githubusercontent.com/108275763/223721019-a44fd361-ceb9-4d83-8eba-6454a63b25b6.png)

If we look at the image from a 2D perspective:

![image](https://user-images.githubusercontent.com/108275763/223722525-1b92b11c-37b2-42ca-8daa-33b42389c9f7.png)

We can see that we are sending rays from the camera and seeing if the rays collide with the sphere.

Depending on the points of collisions, which have coordinates of their own, we will set the colour of the pixel that was 'hit' to a particular value. 

## Section 2.4: How do we find the collision points?

Suppose that we have a ray $a_{-3,-3} + b_{1,1} \cdot t$ and a circle $x^2 + y^2 + 2^2 = 0$, we can set up an equation to see where it collides.

To start with, let us split up the ray by dimension:

$$a_x + b_x \cdot t$$

$$a_y + b_y \cdot t$$

We are able to substitute these equations into $x$ and $y$ components of the circle:

$$(a_x + b_x t)^2 + (a_y + b_y t)^2 - 4 = 0$$

Let us expand the equation:

$$(a_x + b_x t)(a_x + b_x t) + (a_y + b_y t)(a_y + b_y t) - 4 = 0$$

$$= a_{x}^2 + 2 a_{x} b_{x} t + b_{x}^2 t^2 + a_{y}^2 + 2 a_{y} b_{y} t + b_{y}^2 t^2 - 4 = 0$$

## Section 2.5: Quadratic equations

We want to solve for $t$, and we can do this by using the quadratic equation $ax^2 + bx + c = 0$.

We can rearrange the equation to express in terms of $t$:

$$(b_x^2 +b_y^2)t^2 + (2a_xb_x + 2a_yb_y)t + (a_x^2 + a_y^2 - 4) = 0$$

We know all values except for $t$:

$b_x$ and $b_y$ is the directional vector of the ray. <br>
$a_x$ and $a_y$ is the origin of the ray.

As such we can simply substitute the values:

$a=(-3,-3)$ <br>
$b=(1,1)$

$$(1^2 + 1^2)t^2 + (2(-3)(1) + 2(-3)(1))t + ((-3)^2 + (-3)^2 -4) = 0$$

$$2t^2 - 12t + 14$$

## Section 2.6 Solving the equation

We can solve the equation using the quadratic formula:

$$x=\frac{-b\pm\sqrt{b^2-4ac}}{2a}$$

Our values are:

$a=2$ <br>
$b=-12$ <br>
$c=14$

## Section 2.7 Using the discriminant

We can use the discriminant $\sqrt{b^2 - 4ac}$ to tell us if there are any solutions and how many solutions there are. Substituting our values into the discriminant and we get:

$$\sqrt{(-12)^2 - 4(2)(14)}$$

$$144 - 112 = 32$$

If we get a value $>0$, then we have 2 solutions. <br> 
If we get a value $=0$, we have 1 solution. <br>
If we get a value $<0$, we have 0 solutions. <br>

## Section 2.8 Finding exact intersection points

$$\frac{-b \pm \sqrt{32}}{2a}$$

$$= \frac{-(-12) \pm \sqrt{32}}{2(2)}$$

$$= \frac{12 \pm 5.66}{4}$$

$$t = 4.415, 1.585$$

We know know how far along the ray the intersections took place:

![image](https://user-images.githubusercontent.com/108275763/223733395-66e29afb-7b79-4eb1-af60-0071954c2e74.png)

We can plug in these values into our parametric equations:

$a_x +b_x t$ <br> 
$a_y +b_y t$ <br>

Since the $x$ and $y$ have the same values, we only have to substitute once.

$-3 + 1(1.585) = (-1.415,-1.415)$ <br>
$-3 + 1(4.415) = (1.415,1.415)$

The answer is correct compared to Desmos:

![image](https://user-images.githubusercontent.com/108275763/223718246-f25fae88-402c-4188-b675-22329abcd6ac.png)

There is a rounding error because we approximated $\sqrt{32}$ as $5.66$.

# Section 3: Rendering a Sphere Using Ray Tracing

We will be taking the mathematical concepts in section 2 and implementing them in code in this section.

## Section 3.1: Creating a Renderer class

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

The purpose of this class is to input a scene description, for the 3D world that we are trying to render. The output is an image which holds the pixels that the renderer has produced which describes that scene.

For now we will have a default constructor:

```cpp 
Renderer() = default;
```

And a function Render, which will render every pixel in the scene:

```cpp
void Render();
```

```cpp
void Renderer::Render()
{
	// render every pixel 
}
```

The target of this render function is to be an actual image, so we will have to add `#include "Walnut/Image.h"`.

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
	std::shared_ptr<Walnut::Image>m_Image;
};
```

Recall the `Render()` function in `WalnutApp.cpp`.

Before we render, we checked to see if the image dimensions are the same as the viewport dimensions. If they are not, then it will create a new image with the same dimensions as the viewport. 

Let us split this into another function `OnResize()` which takes a width and height:

```cpp
void OnResize(uint32_t width, uint32_t height);
```

Walnut has a function `Resize()`:

```cpp
void Image::Resize(uint32_t width, uint32_t height)
{
	if (m_Image && m_Width == width && m_Height == height)
		return;
	// TODO: max size?
	m_Width = width;
	m_Height = height;
	Release();
	AllocateMemory(m_Width * m_Height * Utils::BytesPerPixel(m_Format));
}
```

This checks if the image needs resizing, and if so, then it releases and reallocates the memory of the image.

In the `Render()` function, we want to call `Resize()` and then we can render.

We also had a CPU side buffer for the image data which we need to put in the class:

```cpp
uint32_t* m_ImageData = nullptr;
```

We will make another function `GetFinalImage()` which returns the final image.

At this point the code should look like this:

`Renderer.h`

```cpp
#pragma once

#include "Walnut/Image.h" // for Render()

#include <memory> // for shared pointers

class Renderer
{
public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);
	void Render(); // renders every pixel 

	std::shared_ptr<Walnut::Image>GetFinalImage() const { return m_FinalImage; }
private:
	std::shared_ptr<Walnut::Image>m_FinalImage;
	uint32_t* m_ImageData = nullptr;
};
```

`Renderer.cpp`

```cpp
#include "Renderer.h"

#include "Walnut/Random.h"

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		// No resize necessary
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;

		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];
}

void Renderer::Render()
{
	for (uint32_t i = 0; i < m_FinalImage->GetWidth() * m_FinalImage->GetHeight(); i++)
	{
		m_ImageData[i] = Walnut::Random::UInt();
		m_ImageData[i] |= 0xff000000;
	}

	m_FinalImage->SetData(m_ImageData);
}
```

`WalnutApp.cpp`

```cpp
#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Renderer.h"

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");

		ImGui::Text("Last render: %.3fms", m_LastRenderTime);

		if (ImGui::Button("Render"))
		{
			// this renders every frame.
			Render();
		}

		ImGui::End();

		// gets rid of border around the viewport
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		// this is the camera
		ImGui::Begin("Viewport");
		
		// these are float values
		m_ViewportWidth = ImGui::GetContentRegionAvail().x;	
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;
		
		auto image = m_Renderer.GetFinalImage();

		if (image)
		{
			// if there is an image, then display the image
			ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() });
		
		}

		ImGui::End();
		ImGui::PopStyleVar();

		Render();
	}

	void Render() 
	{
		Timer timer;

		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Renderer.Render();

		m_LastRenderTime = timer.ElapsedMillis();
	}

private:
	Renderer m_Renderer;
	// buffer for image data
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
	float m_LastRenderTime = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "RayTracing";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;	
}
```
![RayTracing_53vDX6V4SI](https://user-images.githubusercontent.com/108275763/224484548-68710aa0-b839-4913-8181-b63542869e4c.gif)

We have now successfully refactored the program.

## Section 3.2: Structuring our code to work like a pixel fragment/shader

- What are pixel shaders?

These are GPU programs that will run for every pixel of an image that we are rendering. 

For example when triangles get rasterised, every pixel that gets generated as a result of that rasterisation process will invoke an instance of this pixel shader. We will be writing a pixel shader in C++.

A great example of this is [Shadertoy](https://www.shadertoy.com/new):

![firefox_558jn7S5rH](https://user-images.githubusercontent.com/108275763/224481367-ef20e588-9f3f-41c0-9a40-3addcf1dafbb.gif)

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

![image](https://user-images.githubusercontent.com/108275763/224481534-179de138-258e-4dbd-af2b-a3329125cac6.png)

The value between $0$ and $1$ across the $x$ axis is being displayed as the red channel, and from $0$ to $1$ across the $y$ axis is being displayed as the green channel.

Let us try to implement this in our code. We will make a function `PerPixel()`. It returns a uint_32t value and has a vec2 parameter which takes in coordinates.

```cpp
uint32_t PerPixel(glm::vec2 coord);
```

Coming back to the code in Shadertoy:

```cpp
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = fragCoord/iResolution.xy;
    fragColor = vec4(uv,0.0,1.0);
}
```

`fragCoord` is a pixel coordinate in pixel space. So in a resolution of 1280x720, a coordinate could be 600x500. The division `fragCoord/iResolution.xy` maps the value to a range from $0$ to $1$. 

Let us modify the `Render()` to contain two for loops, for $x$ and $y$ coordinates:

```cpp
for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
{
	for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
	{
		PerPixel();

		m_ImageData[i] = Walnut::Random::UInt();
		m_ImageData[i] |= 0xff000000;
	}
}
```

The reason we iterate the $y$ axis in the outer loop is because we want to be more friendly with the CPU cache (in terms of the layout of the buffer in memory):
- as we go 1 uint32_t forwards we are going horizontally across the image
- if we iterated the $x$ axis in the outer loop we would be skipping a portion of memory by going a full row forwards and this will slow down our program because the CPU cannot fetch the memory as easily.

We can easily calculate what the coordinate should be in the nested for loop:

```cpp
glm::vec2 coord = { (float)x / (float)m_FinalImage->GetWidth(), (float)y / (float)m_FinalImage->GetHeight() };
```

The coordinate is `x` divided by the total width of the image, and `y` divided by the total height. <br> 
We have to cast the variables as float to perform float division, if not, it will perform integer division.

Then, we can set each pixel colour using the `PerPixel()` function at the appropriate location:

```cpp
m_ImageData[x + y * m_FinalImage->GetWidth()] = PerPixel(coord);
```

We are multiplying the $y$ coordinate by how large each row supposedly is.

Now, it is the responsiblity of the `PerPixel()` function to provide some sort of colour. <br> For now, we will set each pixel to green:

```cpp
uint32_t Renderer::PerPixel(glm::vec2 coord)
{
	return 0xff00ff00;
}
```

The result is:
![image](https://user-images.githubusercontent.com/108275763/226174453-5257bcf3-7e46-4e28-8c9a-57b75145260b.png)

Let's try to imitate the output of code we used in Shadertoy:

```cpp
uint32_t Renderer::PerPixel(glm::vec2 coord)
{
	uint8_t r = (uint8_t)coord.x * 255.0f;
	uint8_t g = (uint8_t)coord.y * 255.0f;

	return 0xff000000 | (g << 8) | r;
}
```

Here we are setting the red channel to the `x` coordinate and green to the `y` coordinate.

The result is:
![image](https://user-images.githubusercontent.com/108275763/226175125-1ed5b48c-8e7e-48f4-b08e-cccbbdbc4b7f.png)

The image is flipped upside down because of how ImGui displays the image. We have to add another parameter to `ImGui::Image` (the `uv0` and `uv1` parameters) as seen below:

![image](https://user-images.githubusercontent.com/108275763/226175271-b43e3223-d393-4378-a7a3-c05ccb4bbb71.png)

```cpp
if (image)
{
	// if there is an image, then display the image
	ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() },
		ImVec2(0, 1), ImVec2(1, 0));
}
```

We are reversing the `uv` coordinate of the $y$ axis. The `v` of the `uv` has now been inverted, and our image is now flipped right side up. <br> Run the program again and this is what we get:

![image](https://user-images.githubusercontent.com/108275763/226175456-26bb06b1-2ac1-4f42-9e5b-8a3fb8caf0f1.png)

We can also check the `OnResize()` function here:

![RayTracing_dWHEgBvw9y](https://user-images.githubusercontent.com/108275763/226175514-0e13f6c4-0762-48a9-a195-b19c827e7855.gif)

This coordinate system and the `PerPixel()` function is what we will be using to decide where to shoot our rays from the camera to see if they intersect with our sphere.

The code at this point looks like this:

`Renderer.h`
```cpp
#pragma once

#include "Walnut/Image.h"

#include <memory>
#include <glm/glm.hpp>

class Renderer
{
public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);
	void Render();

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }
private:
	uint32_t PerPixel(glm::vec2 coord);
private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;
};
```

`Renderer.cpp`
```cpp
#include "Renderer.h"

#include "Walnut/Random.h"

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		// No resize necessary
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;

		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];
}

void Renderer::Render()
{
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			// assign a coordinate
			glm::vec2 coord = { (float)x / (float)m_FinalImage->GetWidth(), (float)y / (float)m_FinalImage->GetHeight() };

			// set the pixel colour to each pixel
			m_ImageData[x + y * m_FinalImage->GetWidth()] = PerPixel(coord);
		}
	}

	m_FinalImage->SetData(m_ImageData);
}

uint32_t Renderer::PerPixel(glm::vec2 coord)
{
	uint8_t r = (uint8_t)(coord.x * 255.0f);
	uint8_t g = (uint8_t)(coord.y * 255.0f);
	return 0xff000000 | (g << 8) | r;
}

```

`WalnutApp.cpp`
```cpp
#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Renderer.h"

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");

		ImGui::Text("Last render: %.3fms", m_LastRenderTime);

		if (ImGui::Button("Render"))
		{
			// this renders every frame.
			Render();
		}

		ImGui::End();

		// gets rid of border around the viewport
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		// this is the camera
		ImGui::Begin("Viewport");
		
		// these are float values
		m_ViewportWidth = ImGui::GetContentRegionAvail().x;	
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;
		
		auto image = m_Renderer.GetFinalImage();

		if (image)
		{
			// if there is an image, then display the image
			ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() },
				ImVec2(0, 1), ImVec2(1, 0));
		}

		ImGui::End();
		ImGui::PopStyleVar();

		Render();
	}

	void Render() 
	{
		Timer timer;

		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Renderer.Render();

		m_LastRenderTime = timer.ElapsedMillis();
	}

private:
	Renderer m_Renderer;
	// buffer for image data
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
	float m_LastRenderTime = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "RayTracing";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;	
}
```

## Section 3.3: Rendering the sphere

Let's attempt to write code using the mathematics in Section 2. Recall that we've done the maths in 2D, but here we will be implementing it in 3D. As such, we have to add an extra $z$ axis to our equations to account for the $z$ dimension.

The quadratic equation for $t$ was:
$$(b_x^2 +b_y^2)t^2 + (2a_xb_x + 2a_yb_y)t + (a_x^2 + a_y^2 - 4) = 0$$

We can simplify the equation further:
$$(b_x^2 +b_y^2)t^2 + (2(a_xb_x + a_yb_y))t + (a_x^2 + a_y^2 - r^2) = 0$$

$t$ is what we are solving for, which is the distance along that ray. The unknown variables that we have are $a$ and $b$ and $r$.

```cpp
// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
// where
// a = ray origin
// b = ray direction
// r = radius
// t = hit distance
```

We will name the variables `a`, `b`, and `c` because that is how they appear in the quadratic formula.

$$x=\frac{-b\pm\sqrt{b^2-4ac}}{2a}$$

Recall that $a$, $b$, and $c$ are the coefficients of the quadratic equation.

Suppose that we have a camera at $(0,0,0)$. All we have to do to calculate a ray per pixel is by using the coordinate that we have. The current setup we have for the coordinate is that it goes from $(0,0)$ to $(1,1)$. If we are using this as the direction, we would only shoot out our rays like so:

![image](https://user-images.githubusercontent.com/108275763/226182672-99733608-fd60-4202-a526-64aa4656ff56.png)

We can remap the coordiantes to go from $-1$ to $1$ rather than $0$ to $1$:

```cpp
// remap to -1 -> 1
coord = coord * 2.0f - 1.0f; 
```

We've done this before in the Algorithms module, where the output is refactored into a different scale.

```cpp
float a = coord.x * coord.x + coord.y * coord.y + coord.z * coord.z;
```

Currently, the coordinate is of data type vec2.

- How do we deal with this?

Let us reimagine the ray tracing scene again, except this time, the $y$ axis is flipped $90\degree$:

![image](https://user-images.githubusercontent.com/108275763/226183557-d6ad7dee-b986-4680-8aa5-c60687c82c8a.png)

We want the scene to have some sort of depth to it, so we will add a `z` coordinate with a range of $-1$ to $1$.

We will use $-1$ for now (OpenGL treats this as the forward direction), though it depends on if you are dealing with a left handed coordinate system or a right handed system.

```cpp
glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
```

If we want this as a unit vector, we can normalise it:

```cpp
rayDirection = glm::normalize(rayDirection);
```

```cpp
float a = rayDirection.x * rayDirection.x + rayDirection.y * rayDirection.y + rayDirection.z * rayDirection.z;
```

This is actually the dot product of the ray direction with itself. We are multiplying each term by itself and adding up the value to a float. We can rewrite this like so:

```cpp
float a = glm::dot(rayDirection, rayDirection);
```

Now we have our $a$ coefficient.

$b$ is actually also a dot product, because we are multiplying each component of the ray origin and each component of the ray direction and adding them up:

```cpp
float b = 2.0f * glm::dot(rayOrigin, rayDirection);
```

Similarly, $c$ is a dot product of the ray origin and itself, minus the radius squared:

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

Result | # Solutions
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
If we hit the sphere, we should see a green colour. Else, it is black.

Hit F5 and we get:

![image](https://user-images.githubusercontent.com/108275763/226185270-8f7efaad-3e8a-470d-a39f-4c9cd54fe5b9.png)

It seems that we hit the sphere. Think of it this way:

The ray origin is at $(0,0,0)$ and the sphere origin is also at $(0,0,0)$. To visualise, that looks like this:

![image](https://user-images.githubusercontent.com/108275763/226185573-2e5bcbe3-abda-4210-b53b-f4ec853eb410.png)

Essentially, the ray is inside the sphere, and the sphere is filling up the viewport. We can move the sphere back from the viewport by translating it in the $z$ axis such that it does not fill the camera viewport. 

Alternatively, we could move the camera back:

```cpp
glm::vec3 rayOrigin(0.0f, 0.0f, -2.0f);
```

The result is:

![image](https://user-images.githubusercontent.com/108275763/226185723-5938e105-856e-41b0-a92a-151e61d003b7.png)

This visually looks like a circle, but it is actually a sphere in 3D space. It is hard to tell because of the flat lighting.

Another problem is that when we resize the viewport, the circle gets stretched:

![RayTracing_G0ssGR5DS5](https://user-images.githubusercontent.com/108275763/226185950-d3c6ddb0-527d-4279-a41b-300f3bc3a08f.gif)

Currently there is no concept of aspect ratio in the code. We are still in the $-1$ to $1$ space, horizontally and vertically. We will fix this in the section.

For now, the code looks like this:

`Renderer.h`
```cpp
#pragma once

#include "Walnut/Image.h"

#include <memory>
#include <glm/glm.hpp>

class Renderer
{
public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);
	void Render();

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }
private:
	uint32_t PerPixel(glm::vec2 coord);
private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;
};
```


`Renderer.cpp`
```cpp
#include "Renderer.h"

#include "Walnut/Random.h"

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		// No resize necessary
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;

		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];
}

void Renderer::Render()
{
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			// assign a coordinate
			glm::vec2 coord = { (float)x / (float)m_FinalImage->GetWidth(), (float)y / (float)m_FinalImage->GetHeight() };
			// remap to -1 -> 1
			coord = coord * 2.0f - 1.0f; 

			// set the pixel colour to each pixel
			m_ImageData[x + y * m_FinalImage->GetWidth()] = PerPixel(coord);
		}
	}

	m_FinalImage->SetData(m_ImageData);
}

uint32_t Renderer::PerPixel(glm::vec2 coord)
{
	glm::vec3 rayOrigin(0.0f, 0.0f, 2.0f);
	glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
	float radius = 0.5f;
	// rayDirection = glm::normalize(rayDirection);

	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// where
	// a = ray origin
	// b = ray direction
	// r = radius
	// t = hit distance

	// float a = rayDirection.x * rayDirection.x + rayDirection.y * rayDirection.y + rayDirection.z * rayDirection.z;
	float a = glm::dot(rayDirection, rayDirection);
	float b = 2.0f * glm::dot(rayOrigin, rayDirection);
	float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;

	// quadratic formula discriminant
	// b^2 - 4ac

	float discriminant = b * b - 4.0f * a * c;

	// > 0, 2 solutions
	// = 0, 1 solution
	// < 0, 0 solutions

	if (discriminant >= 0.0f)
	{
		return 0xff00ff00;
	}

	return 0xff000000;
}

```

`WalnutApp.cpp`
```cpp
#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Renderer.h"

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");

		ImGui::Text("Last render: %.3fms", m_LastRenderTime);

		if (ImGui::Button("Render"))
		{
			// this renders every frame.
			Render();
		}

		ImGui::End();

		// gets rid of border around the viewport
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		// this is the camera
		ImGui::Begin("Viewport");
		
		// these are float values
		m_ViewportWidth = ImGui::GetContentRegionAvail().x;	
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;
		
		auto image = m_Renderer.GetFinalImage();

		if (image)
		{
			// if there is an image, then display the image
			ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() },
				ImVec2(0, 1), ImVec2(1, 0));
		}

		ImGui::End();
		ImGui::PopStyleVar();

		Render();
	}

	void Render() 
	{
		Timer timer;

		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Renderer.Render();

		m_LastRenderTime = timer.ElapsedMillis();
	}

private:
	Renderer m_Renderer;
	// buffer for image data
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
	float m_LastRenderTime = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "RayTracing";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;	
}
```

# Section 4: Ray casting and sphere intersection

## Section 4.1: Using floats for colours

Let us repurpose the `PerPixel` function to output a `glm::vec4` data type:
```cpp
glm::vec4 Renderer::PerPixel(glm::vec2 coord)

if (discriminant >= 0.0f)
{
	return glm::vec4(0, 1, 0, 1); // this makes green (R, G, B, A)
}

return glm::vec4(0, 0, 0, 1); // this returns black
```

This makes the code more readable as we are consistently using the RGBA format.

Now, we have to convert our `PerPixel` function to the RGBA format:
![image](https://user-images.githubusercontent.com/108275763/233859470-6f7a9e26-360f-4788-b68f-9c498749c625.png)

![image](https://user-images.githubusercontent.com/108275763/233859557-81520c49-4740-49e9-86f6-d6849e6a9819.png)

We will have to make a function to convert the colour to the RGBA format.

In a separate namespace the function is defined: 

```cpp
namespace Utils {
	static uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		color.r * 255.0f;
	}
}
```

Note that the color in each channel should be within the range of 0 to 1 to prevent 'spilling' out into other channels. Vulkan, DirectX, and OpenGL already does this for us, but since we have no graphics pipeline or GPU drivers, we have to code it ourselves.

```cpp
// clamp the range to 0 and 1
color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
```

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

![image](https://user-images.githubusercontent.com/108275763/233860146-a91f35e3-cfa5-4ceb-a3da-39bbfc751681.png)

## Section 4.2: Calculating sphere hit coordinates

Next, we want to implement the second half of the quadratic formula:

$$\frac{-b\pm\sqrt{discriminant}}{2a}$$

Let us rework the `PerPixel` function to return black if the discriminant is less than zero, else, continue to capture the sphere:

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

## Section 4.3: Closest intersection point

Recall that $t$ is the distance from the origin along the ray direction to our actual hit point. The smallest $t$ value would naturally be the closest intersection point. 

In this case, `t1` would always be the lower value since we are subtracting $-b$ with the discriminant. Let's write this in code:

```cpp
float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);

glm::vec3 hitPoint = rayOrigin + rayDirection * closestT;

glm::vec3 sphereColor(0, 1, 0);
sphereColor = hitPoint;
return glm::vec4(sphereColor, 1.0f);
```

The result is:
![image](https://user-images.githubusercontent.com/108275763/233863574-1c20bce8-75a5-4f9e-8b94-4f880fc95d4e.png)

The sphere now has some colour which shows depth. 

![image](https://user-images.githubusercontent.com/108275763/234369012-a3ed304f-5ab5-4f09-8118-5327ebc9e4c6.png)

## Section 4.4: Using colour to visualise numbers

In this case, $x$ and $y$ are negative set to $0$. $z$ is positive, and it is the blue colour that we are initialising. Recall that each hitpoint has an $x$, $y$, $z$ coordinate for every pixel. We are using those values and outputting them as a colour. Essentially:

$$x=r$$

$$y=b$$

$$z=g$$

Where each plane represents a colour channel.

## Section 4.4: How lighting and shading works

The easiest way to explain lighting and shading is by example:

If the object is facing the light, its appearance is brighter.
![image](https://user-images.githubusercontent.com/108275763/233867103-0783a83a-b8af-43f4-83e9-131c1aa9465d.png)

If the object is not facing the light, its appearance is dimmer.
![image](https://user-images.githubusercontent.com/108275763/233867108-bf6b3c18-5649-422a-890d-d90ef1511eb2.png)

Light has different components such as intensity, colour, direction and a source. The final result of a scene with an object is dictated by the object's surface facing this light. The direction that each pixel faces towards the light has to be captured such that lighting can be rendered accordingly (surfaces facing the light appear brighter, else it is dimmer). 

## Section 4.5: Calculating lighting using normal vectors

In this section, we want to determine, for each pixel, which direction it is facing. In other words, this is we want to calculate the normal (a vector perpendicular to the surface):

![image](https://user-images.githubusercontent.com/108275763/234367075-26d50a3e-a99d-4aef-af50-bd34b19b1cc2.png)

![image](https://user-images.githubusercontent.com/108275763/234367339-ec807699-1af5-4e14-a6ef-895639b16a55.png)

We already know the position of the sphere's origin and the position of the hitpoints, we can subtract these two vectors. 

Since our current sphere origin is $(0,0,0)$, we know that the hitpoint is the normal. We have to normalise this vector because the radius of the sphere affects the magnitude of the vector. Recall the size of the radius:

```cpp
float radius = 0.5f;
```

Consequently, the colours of the previously rendered sphere appears dark because the range is actually from 0 to 0.5. To find the normal we can use `glm::normalize`:

```cpp
glm::vec3 normal = glm::normalize(hitPoint);
```

Usually, we would subtract the sphere origin from the hitpoint. We will do this in a future section.

Let us visualise the normal:

```cpp
glm::vec3 sphereColor(1, 0, 1);
sphereColor = normal;
return glm::vec4(sphereColor, 1.0f);	
```

The result is:

![image](https://user-images.githubusercontent.com/108275763/234369929-605b22aa-1a42-4855-8a43-19831884cb68.png)

After moving the camera one unit forward (towards the sphere):

![image](https://user-images.githubusercontent.com/108275763/234370401-be18cd79-57ae-4aba-b858-7b4084343a3e.png)

We are sending the rays towards the $-z$ direction. Therefore, the normal is facing towards the $+z$ direction, which makes the colour blue much more potent.

## Section 4.6: Fixing normal visualisations

However, the normal vector can have valid values from $-1$ to $1$ for each component. We cannot see values which are less than $0$ in this case, resulting in less red and less green. 

We can shift the values like so: 

$$-1\cdot0.5+0.5=0.0$$
$$1\cdot0.5+0.5=0.0$$
$$0\cdot0.5+0.5=0.0$$

```cpp
sphereColor = normal * 0.5f + 0.5f;
```

The result is:

![image](https://user-images.githubusercontent.com/108275763/234373539-071a830e-c28d-4d89-97f9-43ba2558b092.png)

We now get the full spectrum of colours.

## Section 4.7: Calculating light and shade on a sphere

We need to set the light source to face the $-z$ direction and $-y$ direction. We also need to calculate the ingoing and outgoing vectors. This is the light direction:

```cpp
glm::vec3 lightDir = glm::normalize(glm::vec3(-1,-1,-1));
```

Recall that we are trying to capture the direction of the normal vs the direction of the light. The angle of between these two vectors will show how much light is shared between them.

Note that the vectors are in opposite directions. We can negate the light direction to calculate the angle between the two vectors by using a vector dot product:

$$a\cdot b = \cos(\theta)$$

In code, it looks like this:

```cpp
float d = glm::dot(normal, -lightDir);
```

Because it is within the cosine function, it returns a value between $-1$ and $1$ and it indicates how much the normal is facing towards the light source. 

For example, if the dot product returns $0$, then the cosine of the angle was 0. The cosine of 90 is also 0. This means that the angle between the normal and the light direction in this case is 90 degrees. 

At this point we also know that any angles above 90 will return a negative value, hence, a brighter colour.

Now, we can multiply the sphere colour by the dot product, and this is what we get:

```cpp
sphereColor *= d;
```

![image](https://user-images.githubusercontent.com/108275763/234424595-38608de4-a833-4bc3-a2a3-aadf2dfed868.png)

We have successfully applied shading to the object.

At this point, the `Renderer.cpp` file should look like this:

<details>
<summary>Click here to view code</summary>

`Renderer.cpp`
```cpp
#include "Renderer.h"

#include "Walnut/Random.h"

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

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		// No resize necessary
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;

		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];
}

void Renderer::Render()
{
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			// assign a coordinate
			glm::vec2 coord = { (float)x / (float)m_FinalImage->GetWidth(), (float)y / (float)m_FinalImage->GetHeight() };
			// remap to -1 -> 1
			coord = coord * 2.0f - 1.0f; 

			// set the pixel colour to each pixel
			glm::vec4 color = PerPixel(coord);
			// clamp the range to 0 and 1
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));

			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
		}
	}

	m_FinalImage->SetData(m_ImageData);
}

glm::vec4 Renderer::PerPixel(glm::vec2 coord)
{
	glm::vec3 rayOrigin(0.0f, 0.0f, 1.0f);
	glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
	float radius = 0.5f;
	// rayDirection = glm::normalize(rayDirection);

	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// where
	// a = ray origin
	// b = ray direction
	// r = radius
	// t = hit distance

	// float a = rayDirection.x * rayDirection.x + rayDirection.y * rayDirection.y + rayDirection.z * rayDirection.z;
	float a = glm::dot(rayDirection, rayDirection);
	float b = 2.0f * glm::dot(rayOrigin, rayDirection);
	float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;

	// quadratic formula discriminant
	// b^2 - 4ac

	float discriminant = b * b - 4.0f * a * c;
	if (discriminant < 0.0f)
	{
		return glm::vec4(0, 0, 0, 1); // return black
	}

	// (-b +- sqrt(discriminant)) / 2a
	// 
	// > 0, 2 solutions
	// = 0, 1 solution
	// < 0, 0 solutions

	// plus variant
	float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a);
	// minus variant
	float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);

	glm::vec3 hitPoint = rayOrigin + rayDirection * closestT;
	glm::vec3 normal = glm::normalize(hitPoint);

	glm::vec3 lightDir = glm::normalize(glm::vec3(-1,-1,-1));
	
	// dot(normal, -lightDir) == cos(angle)
	float d = glm::max(glm::dot(normal, -lightDir), 0.0f);

	glm::vec3 sphereColor(0, 1, 0);
	sphereColor *= d;
	return glm::vec4(sphereColor, 1.0f);	
}

```
</details>

# Section 5: Implementing a user interactive 3D camera system

## Section 5.1: How 3D cameras work

Functionality wise, the user will have access to moving the position of the camera using the `W,A,S,D` keys on the keyboard, and moving where the camera faces using the mouse.

This also happens to be the traditional method used in first-person 3D video games, or any real time rasterisation applications.

The camera has two properties: the position (ray origin) and the ray direction. The camera is implemented in code like so:

```cpp
glm::vec3 rayOrigin(0.0f, 0.0f, 1.0f);
glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
```

Currently, there is no way to rotate the camera or to adjust the field of view due to the static implementation of the origin and direction. 

## Section 5.2: The Camera class overview

The latest version of walnut contains a `Camera` class. This can be seen here:

<details>
<summary>Click here to view code</summary>

`Camera.h`
```cpp
#pragma once

#include <glm/glm.hpp>
#include <vector>

class Camera {
public:
	Camera(float verticalFOV, float nearClip, float farClip);

	bool OnUpdate(float ts);
	void OnResize(uint32_t width, uint32_t height);

	const glm::mat4& GetProjection() const { return m_Projection; }
	const glm::mat4& GetInverseProjection() const { return m_InverseProjection; }
	const glm::mat4& GetView() const { return m_View; }
	const glm::mat4& GetInverseView() const { return m_InverseView; }

	const glm::vec3& GetPosition() const { return m_Position; }
	const glm::vec3& GetDirection() const { return m_ForwardDirection; }

	const std::vector<glm::vec3>& GetRayDirections() const { return m_RayDirections; }

	float GetRotationSpeed();
private:
	void RecalculateProjection();
	void RecalculateView();
	void RecalculateRayDirections();
private:
	glm::mat4 m_Projection{ 1.0f };
	glm::mat4 m_View{ 1.0f };
	glm::mat4 m_InverseProjection{ 1.0f };
	glm::mat4 m_InverseView{ 1.0f };

	float m_VerticalFOV = 45.0f;
	float m_NearClip = 0.1f;
	float m_FarClip = 100.0f;

	glm::vec3 m_Position{ 0.0f, 0.0f, 0.0f };
	glm::vec3 m_ForwardDirection{ 0.0f, 0.0f, 0.0f };

	// Cached ray directions
	std::vector<glm::vec3> m_RayDirections;

	glm::vec2 m_LastMousePosition{ 0.0f, 0.0f };

	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
};

```

`Camera.cpp`
```cpp
#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Walnut/Input/Input.h"

using namespace Walnut;

Camera::Camera(float verticalFOV, float nearClip, float farClip)
	: m_VerticalFOV(verticalFOV), m_NearClip(nearClip), m_FarClip(farClip) {
	m_ForwardDirection = glm::vec3(0, 0, -1);
	m_Position = glm::vec3(0, 0, 6);
}

bool Camera::OnUpdate(float ts) {
	glm::vec2 mousePos = Input::GetMousePosition();
	glm::vec2 delta = (mousePos - m_LastMousePosition) * 0.002f;
	m_LastMousePosition = mousePos;

	if (!Input::IsMouseButtonDown(MouseButton::Right)) {
		Input::SetCursorMode(CursorMode::Normal);
		return false;
	}

	Input::SetCursorMode(CursorMode::Locked);

	bool moved = false;

	constexpr glm::vec3 upDirection(0.0f, 1.0f, 0.0f);
	glm::vec3 rightDirection = glm::cross(m_ForwardDirection, upDirection);

	float speed = 5.0f;

	// Movement
	if (Input::IsKeyDown(KeyCode::W)) {
		m_Position += m_ForwardDirection * speed * ts;
		moved = true;
	} else if (Input::IsKeyDown(KeyCode::S)) {
		m_Position -= m_ForwardDirection * speed * ts;
		moved = true;
	}
	if (Input::IsKeyDown(KeyCode::A)) {
		m_Position -= rightDirection * speed * ts;
		moved = true;
	} else if (Input::IsKeyDown(KeyCode::D)) {
		m_Position += rightDirection * speed * ts;
		moved = true;
	}
	if (Input::IsKeyDown(KeyCode::Q)) {
		m_Position -= upDirection * speed * ts;
		moved = true;
	} else if (Input::IsKeyDown(KeyCode::E)) {
		m_Position += upDirection * speed * ts;
		moved = true;
	}

	// Rotation
	if (delta.x != 0.0f || delta.y != 0.0f) {
		float pitchDelta = delta.y * GetRotationSpeed();
		float yawDelta = delta.x * GetRotationSpeed();

		glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, rightDirection),
			glm::angleAxis(-yawDelta, glm::vec3(0.f, 1.0f, 0.0f))));
		m_ForwardDirection = glm::rotate(q, m_ForwardDirection);

		moved = true;
	}

	if (moved) {
		RecalculateView();
		RecalculateRayDirections();
	}

	return moved;
}

void Camera::OnResize(uint32_t width, uint32_t height) {
	if (width == m_ViewportWidth && height == m_ViewportHeight)
		return;

	m_ViewportWidth = width;
	m_ViewportHeight = height;

	RecalculateProjection();
	RecalculateRayDirections();
}

float Camera::GetRotationSpeed() {
	return 0.3f;
}

void Camera::RecalculateProjection() {
	m_Projection = glm::perspectiveFov(glm::radians(m_VerticalFOV), (float)m_ViewportWidth, (float)m_ViewportHeight, m_NearClip, m_FarClip);
	m_InverseProjection = glm::inverse(m_Projection);
}

void Camera::RecalculateView() {
	m_View = glm::lookAt(m_Position, m_Position + m_ForwardDirection, glm::vec3(0, 1, 0));
	m_InverseView = glm::inverse(m_View);
}

void Camera::RecalculateRayDirections() {
	m_RayDirections.resize(m_ViewportWidth * m_ViewportHeight);

	for (uint32_t y = 0; y < m_ViewportHeight; y++) {
		for (uint32_t x = 0; x < m_ViewportWidth; x++) {
			glm::vec2 coord = { (float)x / (float)m_ViewportWidth, (float)y / (float)m_ViewportHeight };
			coord = coord * 2.0f - 1.0f; // -1 -> 1

			glm::vec4 target = m_InverseProjection * glm::vec4(coord.x, coord.y, 1, 1);
			glm::vec3 rayDirection = glm::vec3(m_InverseView * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0)); // World space
			m_RayDirections[x + y * m_ViewportWidth] = rayDirection;
		}
	}
}

```
</details>

This camera system is similar to Unity/Unreal Engine which allows the user to hold right click to rotate the camera and use `W,A,S,D` to move within the environment. 

The Camera constructor takes in the verticalFOV, a nearClip and a farClip:

```cpp
Camera(float verticalFOV, float nearClip, float farClip);
```

These parameters can be visualised in the image below:

![image](https://user-images.githubusercontent.com/108275763/234670873-b9c32829-6350-48df-99c9-839e90317328.png)

Figure: A viewing frustum.
Source: Adapted from wikipedia

A viewing frustum represents the field of view of the camera in a 3D region. The near plane indicated in yellow is the `nearClip` and the far plane indicated in blue is the `farClip`. Anything outside of this frustum is not rendered in the final scene.

The function `OnUpdate` is called every frame with the timestep:

<details>
<summary>Click here to view `OnUpdate`</summary>

```cpp
bool Camera::OnUpdate(float ts) {
	glm::vec2 mousePos = Input::GetMousePosition();
	glm::vec2 delta = (mousePos - m_LastMousePosition) * 0.002f;
	m_LastMousePosition = mousePos;

	if (!Input::IsMouseButtonDown(MouseButton::Right)) {
		Input::SetCursorMode(CursorMode::Normal);
		return false;
	}

	Input::SetCursorMode(CursorMode::Locked);

	bool moved = false;

	constexpr glm::vec3 upDirection(0.0f, 1.0f, 0.0f);
	glm::vec3 rightDirection = glm::cross(m_ForwardDirection, upDirection);

	float speed = 5.0f;

	// Movement
	if (Input::IsKeyDown(KeyCode::W)) {
		m_Position += m_ForwardDirection * speed * ts;
		moved = true;
	} else if (Input::IsKeyDown(KeyCode::S)) {
		m_Position -= m_ForwardDirection * speed * ts;
		moved = true;
	}
	if (Input::IsKeyDown(KeyCode::A)) {
		m_Position -= rightDirection * speed * ts;
		moved = true;
	} else if (Input::IsKeyDown(KeyCode::D)) {
		m_Position += rightDirection * speed * ts;
		moved = true;
	}
	if (Input::IsKeyDown(KeyCode::Q)) {
		m_Position -= upDirection * speed * ts;
		moved = true;
	} else if (Input::IsKeyDown(KeyCode::E)) {
		m_Position += upDirection * speed * ts;
		moved = true;
	}

	// Rotation
	if (delta.x != 0.0f || delta.y != 0.0f) {
		float pitchDelta = delta.y * GetRotationSpeed();
		float yawDelta = delta.x * GetRotationSpeed();

		glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, rightDirection),
			glm::angleAxis(-yawDelta, glm::vec3(0.f, 1.0f, 0.0f))));
		m_ForwardDirection = glm::rotate(q, m_ForwardDirection);

		moved = true;
	}

	if (moved) {
		RecalculateView();
		RecalculateRayDirections();
	}

	return moved;
}
```
</details>

This allows us to move at constant speed, independent of the frame rate.

The function `OnResize` is used to recalculate the projection matrix:
```cpp
void OnResize(uint32_t width, uint32_t height);
```

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

test change