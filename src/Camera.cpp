
#define GLM_FORCE_ALIGNED_GENTYPES
#include "glm/gtx/matrix_decompose.hpp"



#include "vel/App.h"
#include "vel/Camera.h"


using json = nlohmann::json;

namespace vel
{
	Camera::Camera(std::string name, CameraType type, float nearPlane, float farPlane, float fovScale) :
		name(name),
		type(type),
		viewportSize(&App::get().getScreenSize()),
		nearPlane(nearPlane),
		farPlane(farPlane),
		fovScale(fovScale),
		position(glm::vec3(0.0f, 0.0f, 0.0f)),
		lookAt(glm::vec3(0.0f, 0.0f, 0.0f)),
		up(glm::vec3(0.0f, 1.0f, 0.0f)),
		viewMatrix(glm::mat4(1.0f)),
		projectionMatrix(glm::mat4(1.0f)),
		useCustomViewportSize(false),
		customViewportSize(glm::ivec2(0, 0)),
		finalRenderCam(true),
		previousTickViewportSize(glm::ivec2(0, 0))
	{

	}

	glm::vec3 Camera::getLookAt()
	{
		return this->lookAt;
	}

	RenderTarget* Camera::getRenderTarget()
	{
		return &this->renderTarget.value();
	}

	void Camera::setRenderTarget(RenderTarget rt)
	{
		this->renderTarget = rt;
	}

	bool Camera::isFinalRenderCam()
	{
		return this->finalRenderCam;
	}

	void Camera::setFinalRenderCam(bool b)
	{
		this->finalRenderCam = b;
	}

	void Camera::setCustomViewportSize(int width, int height)
	{
		this->customViewportSize = glm::ivec2(width, height);
		this->useCustomViewportSize = true;
	}

	void Camera::setCustomViewportSize(bool b)
	{
		this->customViewportSize = glm::ivec2(0,0);
		this->useCustomViewportSize = false;
	}

	void Camera::setFovOrScale(float fos)
	{
		this->fovScale = fos;
	}

	void Camera::setNearPlane(float np)
	{
		this->nearPlane = np;
	}

	void Camera::setFarPlane(float fp)
	{
		this->farPlane = fp;
	}

	void Camera::setType(CameraType type)
	{
		this->type = type;
	}

	std::string Camera::getName()
	{
		return this->name;
	}

	glm::ivec2 Camera::getViewportSize()
	{
		if (this->useCustomViewportSize)
			return this->customViewportSize;

		return glm::ivec2(this->viewportSize->x, this->viewportSize->y);
	}

	void Camera::updateProjectionMatrix()
	{
		glm::ivec2 vps = this->getViewportSize();

		if (type == CameraType::ORTHOGRAPHIC)
		{
			float aspect = (float)vps.x / (float)vps.y;
			float sizeX = fovScale * aspect;
			float sizeY = fovScale;
			this->projectionMatrix = glm::ortho(-sizeX, sizeX, -sizeY, sizeY, this->nearPlane, this->farPlane);
		}
		else
		{
			if (vps.x > 0 && vps.y > 0) // crashes when windowing out of fullscreen if this condition is not checked
			{
				this->projectionMatrix = glm::perspective(glm::radians(this->fovScale),
					(float)vps.x / (float)vps.y, this->nearPlane, this->farPlane);
			}
		}

	}

	void Camera::updateViewMatrix()
	{
		this->viewMatrix = glm::lookAt(this->position, this->lookAt, this->up);
	}

	void Camera::update()
	{
		// get current viewport size
		auto currentViewportSize = this->getViewportSize();

		// if current viewport size does not equal the previous tick's viewport size, we have to rebuild
		// the render target
		if (currentViewportSize != this->previousTickViewportSize)
		{
			std::cout << "viewport size altered" << std::endl;
			//this->renderTarget->resolution = currentViewportSize;
			//App::get().getGPU()->updateRenderTarget(&this->renderTarget.value());

			// create a new render target
			RenderTarget rt = App::get().getGPU()->createRenderTarget(currentViewportSize.x, currentViewportSize.y);

			// clear existing render target
			App::get().getGPU()->clearRenderTarget(&this->renderTarget.value());

			// attach new render target to camera
			this->renderTarget = rt;

		}

		this->previousTickViewportSize = currentViewportSize;

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

	json Camera::toJson()
	{
		json j;
		j["name"] = this->name;
		j["type"] = this->type == CameraType::ORTHOGRAPHIC ? "orthographic" : "perspective";
		j["near"] = this->nearPlane;
		j["far"] = this->farPlane;
		j["fovOrScale"] = this->fovScale;
		j["position"] = { this->position.x, this->position.y, this->position.z };
		j["lookat"] = { this->lookAt.x, this->lookAt.y, this->lookAt.z };

		return j;
	}

}