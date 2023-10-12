/*
	MIT License
	Copyright (c) 2023 Athir Azizi

	Title: WalnutApp.cpp
	Author: https://github.com/athirazizi
	Date: 2023

	Availability: https://github.com/athirazizi/RayTracing/blob/master/RayTracing/src/WalnutApp.cpp
	Adapted from: https://github.com/TheCherno/RayTracing/ (MIT License - Copyright (c) 2022 Studio Cherno)
*/

#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Renderer.h"
#include "Camera.h"

#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;

class FrontEnd : public Walnut::Layer
{
public:
	FrontEnd()
		: camera_(45.0f, 0.1f, 100.0f)
	{
		// materials in the scene
		Material& floor = scene_.Materials.emplace_back();
		floor.Albedo = { 0.1f, 0.1f, 0.1f };
		//floor.Albedo = { 1.0f, 1.0f, 1.0f };
		floor.Roughness = 0.0f;
		floor.EmissionColor = floor.Albedo;
		floor.EmissionPower = 0;

		Material& red = scene_.Materials.emplace_back();
		red.Albedo = { 1.0f, 0.0f, 0.0f };
		red.Roughness = 0.1f;
		red.EmissionColor = red.Albedo;
		red.EmissionPower = 0.5;

		Material& green = scene_.Materials.emplace_back();
		green.Albedo = { 0.0f, 1.0f, 0.0f };
		green.Roughness = 0.1f;
		green.EmissionColor = green.Albedo;
		green.EmissionPower = 0.5;

		Material& blue = scene_.Materials.emplace_back();
		blue.Albedo = { 0.0f, 0.0f, 1.0f };
		blue.Roughness = 0.1f;
		blue.EmissionColor = blue.Albedo;
		blue.EmissionPower = 0.5;

		Material& cyan = scene_.Materials.emplace_back();
		cyan.Albedo = { 0.0f, 1.0f, 1.0f };
		cyan.Roughness = 0.1f;
		cyan.EmissionColor = cyan.Albedo;
		cyan.EmissionPower = 0.5;

		Material& yellow = scene_.Materials.emplace_back();
		yellow.Albedo = { 1.0f, 1.0f, 0.0f };
		yellow.Roughness = 0.1f;
		yellow.EmissionColor = yellow.Albedo;
		yellow.EmissionPower = 0.5;

		Material& magenta = scene_.Materials.emplace_back();
		magenta.Albedo = { 1.0f, 0.0f, 1.0f };
		magenta.Roughness = 0.1f;
		magenta.EmissionColor = magenta.Albedo;
		magenta.EmissionPower = 0.5;

		// spheres in the scene
		{
			Sphere sphere;
			sphere.Position = { 0.0f, -1000.5f, 0.0f };
			sphere.Radius = 1000.0f;
			sphere.MaterialIndex = 0;
			scene_.Spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.Position = { 0.0f, 0.0f, 0.0f };
			sphere.Radius = 0.5f;
			sphere.MaterialIndex = 1;
			scene_.Spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.Position = { 1.1f, 0.0f, 0.0f };
			sphere.Radius = 0.5f;
			sphere.MaterialIndex = 2;
			scene_.Spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.Position = { 0.0f, 0.0f, 1.1f };
			sphere.Radius = 0.5f;
			sphere.MaterialIndex = 3;
			scene_.Spheres.push_back(sphere);
		}

	}

	virtual void OnUpdate(float ts) override
	{
		// reset accumulation when moving camera
		if (camera_.OnUpdate(ts))
		{
			renderer_.ResetFrameIndex();
		}
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("FPS: %.0f", fps_);
		ImGui::Text("Render time: %.2fms", render_time_);

		/*
		if (ImGui::Button("Render"))
		{
			Render();
		}
		*/

		// accumulate path tracing
		ImGui::Checkbox("Accumulate", &renderer_.GetSettings().Accumulate);

		if (ImGui::Button("Reset accumulation"))
		{
			renderer_.ResetFrameIndex();
		}

		ImGui::End();

		ImGui::Begin("Scene spheres");

		// iterate through spheres
		for (size_t i = 0; i < scene_.Spheres.size(); i++)
		{
			ImGui::PushID(i);

			Sphere& sphere = scene_.Spheres[i];
			ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f);
			ImGui::DragFloat("Radius", &sphere.Radius, 0.1f);
			ImGui::DragInt("Material", &sphere.MaterialIndex, 1.0f, 0, (int)scene_.Materials.size() - 1);

			ImGui::Separator();
			ImGui::PopID();
		}

		ImGui::End();


		ImGui::Begin("Scene materials");

		// iterate through materials
		for (size_t i = 0; i < scene_.Materials.size(); i++)
		{
			ImGui::PushID(i);

			Material& material = scene_.Materials[i];

			ImGui::ColorEdit3("Albedo", glm::value_ptr(material.Albedo));
			ImGui::DragFloat("Rougness", &material.Roughness, 0.05f, 0.0f, 1.0f);
			//ImGui::DragFloat("Metallic", &material.Metallic, 0.05f, 0.0f, 1.0f);
			ImGui::ColorEdit3("Emission colour", glm::value_ptr(material.EmissionColor));
			ImGui::DragFloat("Emission power", &material.EmissionPower, 0.05f, 0.0f, FLT_MAX);

			ImGui::Separator();
			ImGui::PopID();
		}


		ImGui::End();

		// gets rid of padding around the viewport
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		// this is the camera
		ImGui::Begin("Viewport");

		// these are float values
		viewport_width_ = ImGui::GetContentRegionAvail().x;
		viewport_height_ = ImGui::GetContentRegionAvail().y;

		auto image = renderer_.GetFinalImage();
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

		renderer_.OnResize(viewport_width_, viewport_height_);
		camera_.OnResize(viewport_width_, viewport_height_);
		renderer_.Render(scene_, camera_);

		fps_ = 1000.f / timer.ElapsedMillis();
		render_time_ = timer.ElapsedMillis();
	}
private:
	// data members

	Renderer renderer_;
	Camera camera_;
	Scene scene_;
	uint32_t viewport_width_ = 0, viewport_height_ = 0;

	float fps_ = 0.0f;
	float render_time_ = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Ray Tracing";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<FrontEnd>();
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