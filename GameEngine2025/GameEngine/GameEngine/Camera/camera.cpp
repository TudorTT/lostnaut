#include "camera.h"

Camera::Camera(glm::vec3 cameraPosition)
{
	this->cameraPosition = cameraPosition;
	this->cameraViewDirection = glm::vec3(0.0f, 0.0f, -1.0f);
	this->cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	this->cameraRight = glm::cross(cameraViewDirection, cameraUp);
	this->rotationOx = 0.0f;   // Pitch
	this->rotationOy = -90.0f; // Yaw (facing -Z)
}

Camera::Camera()
{
	this->cameraPosition = glm::vec3(0.0f, 5.0f, 0.0f);
	this->cameraViewDirection = glm::vec3(0.0f, 0.0f, -1.0f);
	this->cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	this->cameraRight = glm::cross(cameraViewDirection, cameraUp);
	this->rotationOx = 0.0f;   // Pitch
	this->rotationOy = -90.0f; // Yaw (facing -Z)
}

Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraViewDirection, glm::vec3 cameraUp)
{
	this->cameraPosition = cameraPosition;
	this->cameraViewDirection = cameraViewDirection;
	this->cameraUp = cameraUp;
	this->cameraRight = glm::cross(cameraViewDirection, cameraUp);
	this->rotationOx = 0.0f;
	this->rotationOy = -90.0f;
}

Camera::~Camera()
{
}

void Camera::setCameraPosition(const glm::vec3& pos)
{
	cameraPosition = pos;
}

void Camera::setCameraViewDirection(const glm::vec3& direction)
{
	// Normalize the input direction
	cameraViewDirection = glm::normalize(direction);
	
	// Calculate pitch (rotationOx) from the y component
	rotationOx = glm::degrees(asin(cameraViewDirection.y));
	
	// Calculate yaw (rotationOy) from x and z components
	rotationOy = glm::degrees(atan2(cameraViewDirection.z, cameraViewDirection.x));
	
	// Recalculate right and up vectors
	cameraRight = glm::normalize(glm::cross(cameraViewDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
	cameraUp = glm::normalize(glm::cross(cameraRight, cameraViewDirection));
}

void Camera::keyboardMoveFront(float cameraSpeed)
{
	cameraPosition += cameraViewDirection * cameraSpeed;
}

void Camera::keyboardMoveBack(float cameraSpeed)
{
	cameraPosition -= cameraViewDirection * cameraSpeed;
}

void Camera::keyboardMoveLeft(float cameraSpeed)
{
	cameraPosition -= cameraRight * cameraSpeed;
}

void Camera::keyboardMoveRight(float cameraSpeed)
{
	cameraPosition += cameraRight * cameraSpeed;
}

void Camera::keyboardMoveUp(float cameraSpeed)
{
	cameraPosition += cameraUp * cameraSpeed;
}

void Camera::keyboardMoveDown(float cameraSpeed)
{
	cameraPosition -= cameraUp * cameraSpeed;
}

void Camera::rotateOx(float angle)
{
	cameraViewDirection = glm::normalize(glm::vec3((glm::rotate(glm::mat4(1.0f), angle, cameraRight) * glm::vec4(cameraViewDirection, 1))));
	cameraUp = glm::normalize(glm::cross(cameraRight, cameraViewDirection));
	cameraRight = glm::cross(cameraViewDirection, cameraUp);
}

void Camera::rotateOy(float angle)
{
	cameraViewDirection = glm::normalize(glm::vec3((glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(cameraViewDirection, 1))));
	cameraRight = glm::normalize(glm::cross(cameraViewDirection, cameraUp));
	cameraUp = glm::normalize(glm::cross(cameraRight, cameraViewDirection));
}

void Camera::mouseLook(float deltaX, float deltaY, float sensitivity)
{
	// Apply sensitivity
	deltaX *= sensitivity;
	deltaY *= sensitivity;
	
	// Update yaw and pitch
	rotationOy += deltaX;
	rotationOx += deltaY;
	
	// Clamp pitch to prevent flipping
	if (rotationOx > 89.0f)
		rotationOx = 89.0f;
	if (rotationOx < -89.0f)
		rotationOx = -89.0f;
	
	// Calculate new view direction from yaw and pitch
	glm::vec3 direction;
	direction.x = cos(glm::radians(rotationOy)) * cos(glm::radians(rotationOx));
	direction.y = sin(glm::radians(rotationOx));
	direction.z = sin(glm::radians(rotationOy)) * cos(glm::radians(rotationOx));
	
	cameraViewDirection = glm::normalize(direction);
	
	// Recalculate right and up vectors
	cameraRight = glm::normalize(glm::cross(cameraViewDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
	cameraUp = glm::normalize(glm::cross(cameraRight, cameraViewDirection));
}

glm::mat4 Camera::getViewMatrix()
{
	return glm::lookAt(cameraPosition, cameraPosition + cameraViewDirection, cameraUp);
}

glm::vec3 Camera::getCameraPosition()
{
	return cameraPosition;
}

glm::vec3 Camera::getCameraViewDirection()
{
	return cameraViewDirection;
}

glm::vec3 Camera::getCameraUp()
{
	return cameraUp;
}