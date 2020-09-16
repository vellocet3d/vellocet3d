#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace vel::scene::stage
{
	struct TRS
	{
		glm::vec3		translation;
		glm::quat		rotation;
		glm::vec3		scale;
	};

}