/*
	MIT License
	Copyright (c) 2023 Athir Azizi

	Title: Ray.h
	Author: https://github.com/athirazizi
	Date: 2023

	Availability: https://github.com/athirazizi/RayTracing/blob/master/RayTracing/src/Ray.h
*/

#pragma once

#include <glm/glm.hpp>

struct Ray
{
	glm::vec3 Origin;
	glm::vec3 Direction;
};