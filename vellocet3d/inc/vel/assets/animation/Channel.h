#pragma once

#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"


namespace vel
{
	struct Channel
	{
		std::vector<float> positionKeyTimes;
		std::vector<float> rotationKeyTimes;
		std::vector<float> scalingKeyTimes;
		std::vector<glm::vec3> positionKeyValues;
		std::vector<glm::quat> rotationKeyValues;
		std::vector<glm::vec3> scalingKeyValues;
	};	
}