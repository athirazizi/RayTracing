/*
	MIT License
	Copyright (c) 2023 Athir Azizi

	Title: Renderer.cpp
	Author: https://github.com/athirazizi
	Date: 2023

	Availability: https://github.com/athirazizi/RayTracing/blob/master/RayTracing/src/Renderer.cpp
	Adapted from: https://github.com/TheCherno/RayTracing/blob/master/RayTracing/src/Renderer.cpp (MIT License - Copyright (c) 2022 Studio Cherno)
*/

#include "Walnut/Random.h"
#include "Renderer.h"

namespace utility
{
	static uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);

		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
		return result;
	}
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (final_image_)
	{
		// no resize necessary
		if (final_image_->GetWidth() == width && final_image_->GetHeight() == height)
			return;

		final_image_->Resize(width, height);
	}
	else
	{
		final_image_ = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	// allocate image size
	delete[] image_data_;
	image_data_ = new uint32_t[width * height];

	// allocate accumulation data size
	delete[] accumulation_data_;
	accumulation_data_ = new glm::vec4[width * height];
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	active_scene_ = &scene;
	active_camera_ = &camera;

	// reset accumulation data on first frame
	if (frame_index_ == 1)
	{
		memset(accumulation_data_, 0, final_image_->GetWidth() * final_image_->GetHeight() * sizeof(glm::vec4));
	}

	for (uint32_t y = 0; y < final_image_->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < final_image_->GetWidth(); x++)
		{
			// set color to each pixel
			glm::vec4 color = RayGen(x, y);

			// accumulate colour to be returned
			accumulation_data_[x + y * final_image_->GetWidth()] += color;
			glm::vec4 accumulated_color = accumulation_data_[x + y * final_image_->GetWidth()];
			accumulated_color /= (float)frame_index_;

			// clamp range to between 0 and 1
			accumulated_color = glm::clamp(accumulated_color, glm::vec4(0.0f), glm::vec4(1.0f));

			// send color to image data
			image_data_[x + y * final_image_->GetWidth()] = utility::ConvertToRGBA(accumulated_color);
		}
	}

	final_image_->SetData(image_data_);

	// increments frame index if accumulation is turned on
	if (settings_.Accumulate == true)
	{
		frame_index_++;
	}
	else
	{
		//ResetFrameIndex();
		frame_index_ = 1;
	}
}

glm::vec4 Renderer::RayGen(uint32_t x, uint32_t y)
{
	// generate ray & set origin and direction
	Ray ray;
	ray.Origin = active_camera_->GetPosition();
	ray.Direction = active_camera_->GetRayDirections()[x + y * final_image_->GetWidth()];

	// final colour to be returned
	glm::vec3 final_color(0.0f);
	float multiplier = 1.0f;

	int bounces = 5;
	for (int i = 0; i < bounces; i++)
	{
		// get payload from trace ray
		Renderer::HitInfo payload = TraceRay(ray);
		if (payload.HitDistance < 0.0f)
		{
			// return background colour if missed
			glm::vec3 background_color = glm::vec3(0.7f, 0.9f, 1.0f);

			// take into account background colour when rendering
			final_color += background_color * multiplier;
			break;
		}

		// you can change the light direction here
		glm::vec3 light_direction = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));

		// dot(normal, -lightDir) == cos(angle)
		float light_intensity = glm::max(glm::dot(payload.WorldNormal, -light_direction), 0.0f); // == cos(angle)

		// intersected sphere
		const Sphere& sphere = active_scene_->Spheres[payload.ObjectIndex];
		// associated material
		const Material& material = active_scene_->Materials[sphere.MaterialIndex];

		glm::vec3 sphere_color = material.Albedo;
		sphere_color *= light_intensity;

		// darken the image by multiplier
		final_color += sphere_color * multiplier;

		multiplier *= 0.5f;

		// change origin and direction for the next bounce
		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;

		// reflect according to the microfacet model
		// i.e., roughness, and a range
		ray.Direction = glm::reflect(ray.Direction, 
			payload.WorldNormal + material.Roughness * Walnut::Random::Vec3(-0.5f, 0.5f));
	}

	//color = normal * 0.5f + 0.5f; // sets x,y,z as r,g,b
	return glm::vec4(final_color, 1.0f);
}

Renderer::HitInfo Renderer::TraceRay(const Ray& ray)
{
	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// a = ray origin, b = ray direction, r = radius, t = scalar hit distance

	// sphere object index
	int closestSphere = -1;

	// set hit distance to highest float value
	float hitDistance = FLT_MAX;

	// run ray-sphere intersection calculations for every sphere in the scene
	for (size_t i = 0; i < active_scene_->Spheres.size(); i++)
	{
		const Sphere& sphere = active_scene_->Spheres[i];
		glm::vec3 origin = ray.Origin - sphere.Position;

		//float a = rayDirection.x * rayDirection.x + rayDirection.y * rayDirection.y + rayDirection.z * rayDirection.z;
		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2.0f * glm::dot(origin, ray.Direction);
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

		// quadratic forumula discriminant : b^2 - 4ac

		float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0.0f)
		{
			// move on to the next sphere
			continue;
		}

		// quadratic formula: (-b +- sqrt(discriminant)) / 2a

		// > 0, 2 solutions
		// = 0, 1 solution
		// < 0, 0 solutions

		// plus variant - not used for now
		//float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a);

		// minus variant
		float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);

		// checks if the closestT is less than the hitdistance
		if (closestT > 0.0f && closestT < hitDistance)
		{
			hitDistance = closestT;

			// set closestSphere as the last sphere that we hit
			closestSphere = (int)i;
		}
	}

	if (closestSphere < 0)
	{
		// return miss payload if no spheres exist
		return Miss(ray);
	}

	// return closest hit payload
	return ClosestHit(ray, hitDistance, closestSphere);
}

Renderer::HitInfo Renderer::ClosestHit(const Ray& ray, float hit_distance, int object_index)
{
	// payload to return
	Renderer::HitInfo payload;
	payload.HitDistance = hit_distance;
	payload.ObjectIndex = object_index;

	const Sphere& closestSphere = active_scene_->Spheres[object_index];

	glm::vec3 origin = ray.Origin - closestSphere.Position;
	payload.WorldPosition = origin + ray.Direction * hit_distance;
	payload.WorldNormal = glm::normalize(payload.WorldPosition);

	// translate world pos by sphere pos
	payload.WorldPosition += closestSphere.Position;

	return payload;
}

Renderer::HitInfo Renderer::Miss(const Ray& ray)
{
	// payload to return
	Renderer::HitInfo payload;
	payload.HitDistance = -1.0f;
	return payload;
}