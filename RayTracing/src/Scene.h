/*
	MIT License
	Copyright (c) 2023 Athir Azizi

	Title: Ray header file
	Author: https://github.com/athirazizi
	Date: 2023

	Availability: https://github.com/athirazizi/RayTracing/blob/master/RayTracing/src/Scene.h
*/

#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Sphere
{
	float Radius = 0.5f;
	glm::vec3 Position{ 0.0f };
	glm::vec3 Albedo{ 1.0f };
};

struct Scene
{
	std::vector<Sphere> Spheres;
};