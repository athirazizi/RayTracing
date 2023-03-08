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
- imgui <br> https://github.com/ocornut/imgui

# Section 1: Welcome to Ray Tracing!

## Step 1: Using Walnut

Walnut is an application development framework developed by GitHub user @TheCherno and this will act as the base of the project.

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
