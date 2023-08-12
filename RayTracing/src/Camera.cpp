/*
	MIT License
	Copyright (c) 2022 Studio Cherno

	Title: Walnut Camera - Camera.cpp
	Author: https://github.com/TheCherno
	Date: 2023

	Availability: https://github.com/TheCherno/RayTracing/blob/master/RayTracing/src/Camera.cpp

	Notes: This file has been modified to suit the project's needs.
*/

#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Walnut/Input/Input.h"

using namespace Walnut;

Camera::Camera(float verticalFOV, float nearClip, float farClip)
	: m_VerticalFOV(verticalFOV), m_NearClip(nearClip), m_FarClip(farClip) {
	m_ForwardDirection = glm::vec3(0, 0, -1);
	m_Position = glm::vec3(0, 0, 6);
}

bool Camera::OnUpdate(float ts) {
	// Capture mouse input
	glm::vec2 mousePos = Input::GetMousePosition();

	// Calculate position moved with the mouse
	glm::vec2 delta = (mousePos - m_LastMousePosition) * 0.002f;
	m_LastMousePosition = mousePos;

	// If right click is now held down, return
	// Camera movement is only active when right click is held
	if (!Input::IsMouseButtonDown(MouseButton::Right)) {
		Input::SetCursorMode(CursorMode::Normal);
		return false;
	}

	// Locks cursor to window and hides the cursor
	Input::SetCursorMode(CursorMode::Locked);

	// Used to check if ray directions and matrices need to be recalculated 
	bool moved = false;

	// Calculate the y direction of the camera
	constexpr glm::vec3 upDirection(0.0f, 1.0f, 0.0f);

	// Calculate the right direction, which is the cross product
	// between the forward direction and the up direction
	glm::vec3 rightDirection = glm::cross(m_ForwardDirection, upDirection);

	float speed = 5.0f;

	// Movement
	if (Input::IsKeyDown(KeyCode::W)) {
		m_Position += m_ForwardDirection * speed * ts;
		moved = true;
	} else if (Input::IsKeyDown(KeyCode::S)) {
		m_Position -= m_ForwardDirection * speed * ts;
		moved = true;
	}
	if (Input::IsKeyDown(KeyCode::A)) {
		m_Position -= rightDirection * speed * ts;
		moved = true;
	} else if (Input::IsKeyDown(KeyCode::D)) {
		m_Position += rightDirection * speed * ts;
		moved = true;
	}
	if (Input::IsKeyDown(KeyCode::LeftControl)) {
		m_Position -= upDirection * speed * ts;
		moved = true;
	} else if (Input::IsKeyDown(KeyCode::Space)) {
		m_Position += upDirection * speed * ts;
		moved = true;
	}

	// Rotation
	if (delta.x != 0.0f || delta.y != 0.0f) {

		float pitchDelta = delta.y * GetRotationSpeed();

		float yawDelta = delta.x * GetRotationSpeed();

		// Captures the delta in all axes
		glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, rightDirection),
			glm::angleAxis(-yawDelta, glm::vec3(0.f, 1.0f, 0.0f))));
		m_ForwardDirection = glm::rotate(q, m_ForwardDirection);

		moved = true;
	}

	if (moved) {
		RecalculateView();
		RecalculateRayDirections();
	}

	return moved;
}

void Camera::OnResize(uint32_t width, uint32_t height) {
	if (width == m_ViewportWidth && height == m_ViewportHeight)
		return;

	m_ViewportWidth = width;
	m_ViewportHeight = height;

	RecalculateProjection();
	RecalculateRayDirections();
}

float Camera::GetRotationSpeed() {
	return 0.3f;
}

void Camera::RecalculateProjection() {
	m_Projection = glm::perspectiveFov(glm::radians(m_VerticalFOV), (float)m_ViewportWidth, (float)m_ViewportHeight, m_NearClip, m_FarClip);
	m_InverseProjection = glm::inverse(m_Projection);
}

void Camera::RecalculateView() {
	// Calculate the view matrix
	m_View = glm::lookAt(m_Position, m_Position + m_ForwardDirection, glm::vec3(0, 1, 0));
	m_InverseView = glm::inverse(m_View);
}

void Camera::RecalculateRayDirections() {
	m_RayDirections.resize(m_ViewportWidth * m_ViewportHeight);

	for (uint32_t y = 0; y < m_ViewportHeight; y++) {
		for (uint32_t x = 0; x < m_ViewportWidth; x++) {
			glm::vec2 coord = { (float)x / (float)m_ViewportWidth, (float)y / (float)m_ViewportHeight };
			coord = coord * 2.0f - 1.0f; // -1 -> 1

			// Intermediate vector
			// Inverse projection * coordinates
			glm::vec4 target = m_InverseProjection * glm::vec4(coord.x, coord.y, 1, 1);

			// Inverse view * target/perspective division
			glm::vec3 rayDirection = glm::vec3(m_InverseView * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0)); // World space

			// Cache the ray direction
			m_RayDirections[x + y * m_ViewportWidth] = rayDirection;
		}
	}
}
