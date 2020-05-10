#pragma once

#include <vector>

#include "glm/glm.hpp"


namespace vel::scene::mesh
{
	struct Bone
	{
		std::string name;
		glm::mat4 offsetMatrix;
	};
}