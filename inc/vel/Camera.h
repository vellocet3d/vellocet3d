#pragma once

#include <optional>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "nlohmann/json.hpp"

#include "vel/Transform.h"
#include "vel/RenderTarget.h"

namespace vel
{
	enum CameraType {
		ORTHOGRAPHIC,
		PERSPECTIVE
	};

	class Camera
	{
	private:
		CameraType              type;
		std::string				name;
		float                   fovScale;
		const glm::ivec2*       viewportSize;
		float                   nearPlane;
		float                   farPlane;
		glm::vec3			    position;
		glm::vec3			    lookAt;
		glm::vec3			    up;
		glm::mat4			    viewMatrix;
		glm::mat4			    projectionMatrix;

		void                    updateViewMatrix();
		void                    updateProjectionMatrix();

		bool					useCustomViewportSize;
		glm::ivec2				customViewportSize;

		bool					finalRenderCam;

		// defined as optional so that we can set at a later stage in the pipeline as opposed to during initialization
		std::optional<RenderTarget> renderTarget;

	public:
		Camera(std::string name, CameraType type, float nearPlane, float farPlane, float fovScale);
		void                    update();
		std::string				getName();
		glm::mat4               getViewMatrix();
		glm::mat4               getProjectionMatrix();
		glm::vec3               getPosition();
		glm::ivec2				getViewportSize();
		glm::vec3				getLookAt();
		void                    setPosition(float x, float y, float z);
		void                    setPosition(glm::vec3 position);
		void                    setLookAt(float x, float y, float z);
		void                    setLookAt(glm::vec3 direction);
		void					setType(CameraType type);
		void					setNearPlane(float np);
		void					setFarPlane(float fp);
		void					setFovOrScale(float fos);

		void					setCustomViewportSize(int width, int height);
		void					setCustomViewportSize(bool b);

		void					setFinalRenderCam(bool b); // whether or not this camera is used to draw to screen buffer
		bool					isFinalRenderCam();

		void					setRenderTarget(RenderTarget rt);
		RenderTarget*			getRenderTarget();


		nlohmann::json			toJson();
	};
}