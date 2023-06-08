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

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	Ray ray;
	ray.Origin = camera.GetPosition();

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			ray.Direction = camera.GetRayDirections()[x + y * m_FinalImage->GetWidth()];
			

			// set the pixel colour to each pixel
			glm::vec4 color = TraceRay(scene, ray);
			// clamp the range to 0 and 1
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));

			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
		}
	}

	m_FinalImage->SetData(m_ImageData);
}

glm::vec4 Renderer::TraceRay(const Scene& scene, const Ray& ray)
{
	// rayDirection = glm::normalize(rayDirection);

	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// where
	// a = ray origin
	// b = ray direction
	// r = radius
	// t = hit distance

	// if there are no spheres in the scene, return black
	if (scene.Spheres.size() == 0)
	{
		return glm::vec4(0, 0, 0, 1);
	}

	// pointer to the closest sphere
	const Sphere* closestSphere = nullptr;

	// assign hit distance to maximum float
	float hitDistance = FLT_MAX;

	// run ray-sphere intersection calculations for every sphere in the scene
	for (const Sphere& sphere : scene.Spheres )
	{
		glm::vec3 origin = ray.Origin - sphere.Position;

		// float a = rayDirection.x * rayDirection.x + rayDirection.y * rayDirection.y + rayDirection.z * rayDirection.z;
		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2.0f * glm::dot(origin, ray.Direction);
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

		// quadratic formula discriminant
		// b^2 - 4ac

		float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0.0f)
		{
			continue; // move on to the next sphere
		}

		// (-b +- sqrt(discriminant)) / 2a
		// 
		// > 0, 2 solutions
		// = 0, 1 solution
		// < 0, 0 solutions

		// plus variant - not used for now
		// float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a);

		// minus variant
		float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);

		// checks if the closestT is less than the distance
		// sets the closestSphere as the last sphere that we hit
		if (closestT < hitDistance)
		{
			hitDistance = closestT;
			closestSphere = &sphere;
		}
	}
	
	// ie no spheres in the scene
	if (closestSphere == nullptr)
	{
		return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	glm::vec3 origin = ray.Origin - closestSphere->Position;

	glm::vec3 hitPoint = origin + ray.Direction * hitDistance;
	glm::vec3 normal = glm::normalize(hitPoint);

	// you can change the light direction here
	glm::vec3 lightDir = glm::normalize(glm::vec3(1,-1,-1));
	
	// dot(normal, -lightDir) == cos(angle)
	float lightIntensity = glm::max(glm::dot(normal, -lightDir), 0.0f);

	glm::vec3 sphereColor = closestSphere->Albedo;
	sphereColor *= lightIntensity;
	//sphereColor = normal * 0.5f + 0.5f;
	return glm::vec4(sphereColor, 1.0f);
}
