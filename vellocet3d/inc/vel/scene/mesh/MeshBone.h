#pragma once

#include <vector>

#include "glm/glm.hpp"


namespace vel
{
	struct MeshBone
	{
		std::string name;
		glm::mat4 offsetMatrix;
	};
}