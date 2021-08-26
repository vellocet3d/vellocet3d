
#define GLM_FORCE_ALIGNED_GENTYPES
#include "glm/gtx/matrix_decompose.hpp"

#include "vel/App.h"
#include "vel/Camera.h"



namespace vel
{
	Camera::Camera(CameraType type, float nearPlane, float farPlane, float fovScale) :
		type(type),
		screenSize(&App::get().getScreenSize()),
		nearPlane(nearPlane),
		farPlane(farPlane),
		fovScale(fovScale),
		position(glm::vec3(0.0f, 0.0f, 0.0f)),
		lookAt(glm::vec3(0.0f, 0.0f, 0.0f)),
		up(glm::vec3(0.0f, 1.0f, 0.0f)),
		viewMatrix(glm::mat4(1.0f)),
		projectionMatrix(glm::mat4(1.0f))
	{

	}

	glm::ivec2 Camera::getScreenSize()
	{
		return glm::ivec2(this->screenSize->x, this->screenSize->y);
	}

	void Camera::updateProjectionMatrix()
	{
		if (type == CameraType::ORTHOGRAPHIC)
		{
			float aspect = (float)this->screenSize->x / (float)this->screenSize->y;
			float sizeX = fovScale * aspect;
			float sizeY = fovScale;
			this->projectionMatrix = glm::ortho(-sizeX, sizeX, -sizeY, sizeY, this->nearPlane, this->farPlane);
		}
		else
		{
			if (this->screenSize->x > 0 && this->screenSize->y > 0) // crashes when windowing out of fullscreen if this condition is not checked
			{
				this->projectionMatrix = glm::perspective(glm::radians(this->fovScale),
					(float)this->screenSize->x / (float)this->screenSize->y, this->nearPlane, this->farPlane);
			}
		}

	}

	void Camera::updateViewMatrix()
	{
		this->viewMatrix = glm::lookAt(this->position, this->lookAt, this->up);
	}

	void Camera::update()
	{
		this->updateViewMatrix();
		this->updateProjectionMatrix();
	}

	glm::mat4 Camera::getViewMatrix()
	{
		return this->viewMatrix;
	}

	glm::mat4 Camera::getProjectionMatrix()
	{
		return this->projectionMatrix;
	}

	void Camera::setPosition(float x, float y, float z)
	{
		this->position = glm::vec3(x, y, z);
	}

	void Camera::setPosition(glm::vec3 position)
	{
		this->position = position;
	}

	void Camera::setLookAt(float x, float y, float z)
	{
		this->lookAt = glm::vec3(x, y, z);
	}

	void Camera::setLookAt(glm::vec3 direction)
	{
		this->lookAt = direction;
	}

	glm::vec3 Camera::getPosition()
	{
		return this->position;
	}



}