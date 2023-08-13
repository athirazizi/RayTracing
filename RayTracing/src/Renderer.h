/*
	MIT License
	Copyright (c) 2023 Athir Azizi

	Title: Renderer.cpp
	Author: https://github.com/athirazizi
	Date: 2023

	Availability: https://github.com/athirazizi/RayTracing/blob/master/RayTracing/src/Renderer.h
	Adapted from: https://github.com/TheCherno/RayTracing/blob/master/RayTracing/src/Renderer.h (MIT License - Copyright (c) 2022 Studio Cherno)
*/

#pragma once

#include "Walnut/Image.h"

#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

#include <memory>
#include <glm/glm.hpp>

class Renderer
{
public:
	struct Settings
	{
		// 
		bool Accumulate = true;
	};

public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);
	void Render(const Scene& scene, const Camera& camera);

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return final_image_; }

	// to reset the frame index when the camera moves
	void ResetFrameIndex() { frame_index_ = 1; }

	// return settings struct
	Settings& GetSettings() { return settings_; }
private:
	struct HitInfo
	{
		float HitDistance;
		glm::vec3 WorldPosition;
		glm::vec3 WorldNormal;

		int ObjectIndex;
	};

	// ray generation shader
	glm::vec4 RayGen(uint32_t x, uint32_t y);
	HitInfo TraceRay(const Ray& ray);

	// closest hit shader
	HitInfo ClosestHit(const Ray& ray, float hit_distance, int object_index);

	// miss shader
	HitInfo Miss(const Ray& ray);
private:
	std::shared_ptr<Walnut::Image> final_image_;

	const Scene* active_scene_ = nullptr;
	const Camera* active_camera_ = nullptr;

	uint32_t* image_data_ = nullptr;
	glm::vec4* accumulation_data_ = nullptr;

	// to count the number of frames since the first render
	uint32_t frame_index_ = 1;

	Settings settings_;

	// iterate x and y
	std::vector<uint32_t> image_x_iterator_, image_y_iterator_;
};