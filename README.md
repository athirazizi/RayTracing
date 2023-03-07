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

Tools:
- C++
- Windows 10
- Visual Studio 2022 <br> https://visualstudio.microsoft.com/
- Vulkan SDK <br> https://vulkan.lunarg.com/
- Desmos <br> https://www.desmos.com/calculator

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

## Step 3: Run the solution

As with any new project, hit F5 and run the solution. This is what we are greeted with:

<img src="https://user-images.githubusercontent.com/108275763/223400005-52ee0109-325f-4031-9c30-80f7d2b6e7b0.png" height = 512>

Side note: 32 bits is the size of the RGBA format; each channel is 1 byte. 
