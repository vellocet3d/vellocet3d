#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "nlohmann/json.hpp"

#include "vel/Transform.h"

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
		const glm::ivec2*       screenSize;
		float                   nearPlane;
		float                   farPlane;
		glm::vec3			    position;
		glm::vec3			    lookAt;
		glm::vec3			    up;
		glm::mat4			    viewMatrix;
		glm::mat4			    projectionMatrix;

		void                    updateViewMatrix();
		void                    updateProjectionMatrix();

	public:
		Camera(std::string name, CameraType type, float nearPlane, float farPlane, float fovScale);
		void                    update();
		std::string				getName();
		glm::mat4               getViewMatrix();
		glm::mat4               getProjectionMatrix();
		glm::vec3               getPosition();
		glm::ivec2				getScreenSize();
		void                    setPosition(float x, float y, float z);
		void                    setPosition(glm::vec3 position);
		void                    setLookAt(float x, float y, float z);
		void                    setLookAt(glm::vec3 direction);
		void					setType(CameraType type);
		void					setNearPlane(float np);
		void					setFarPlane(float fp);
		void					setFovOrScale(float fos);

		nlohmann::json			toJson();
	};
}