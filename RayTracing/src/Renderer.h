#pragma once

#include "Walnut/Image.h" // for Render()

#include <memory> // for shared pointers

class Renderer
{
public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);
	void Render(); // renders every pixel 

	std::shared_ptr<Walnut::Image>GetFinalImage() const { return m_FinalImage; }
private:
	std::shared_ptr<Walnut::Image>m_FinalImage;
	uint32_t* m_ImageData = nullptr;
};