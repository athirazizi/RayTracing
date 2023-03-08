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
