/*
	MIT License
	Copyright (c) 2023 Athir Azizi

	Title: Ray header file
	Author: https://github.com/athirazizi
	Date: 2023

	Availability: https://github.com/athirazizi/RayTracing/blob/master/RayTracing/src/Scene.h
	Adapted from: https://github.com/TheCherno/RayTracing/blob/master/RayTracing/src//Scene.h (MIT License - Copyright (c) 2022 Studio Cherno)
*/

#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Material
{
	glm::vec3 Albedo{ 1.0f };
	float Roughness = 1.0f;
	float Metallic = 0.0f;
	glm::vec3 EmissionColor{ 0.0f };
	float EmissionPower = 0.0f;

	glm::vec3 GetEmission()const { return EmissionColor * EmissionPower; }
};

struct Sphere
{
	float Radius = 0.5f;
	glm::vec3 Position{ 0.0f };
	int MaterialIndex = 0;
};

struct Scene
{
	std::vector<Sphere> Spheres;
	std::vector<Material> Materials;
};