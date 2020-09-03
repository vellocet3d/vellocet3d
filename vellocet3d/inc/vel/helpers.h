#pragma once

#include <vector>
#include <string>

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "LinearMath/btVector3.h"
#include "LinearMath/btQuaternion.h"

namespace vel
{
    std::vector<std::string> explode_string(std::string const & s, char delim);
    std::string char_to_string(char* a);
	bool sin_vector(std::string needle, std::vector<std::string> haystack);
	bool string_contains(std::string needle, std::string haystack);
	bool approximatelyEqual(float a, float b, float epsilon);
	bool essentiallyEqual(float a, float b, float epsilon);
	bool definitelyGreaterThan(float a, float b, float epsilon);
	bool definitelyLessThan(float a, float b, float epsilon);
	btVector3 glmToBulletVec3(glm::vec3 glmVec);
	glm::vec3 bulletToGlmVec3(btVector3 btVec);
	glm::quat bulletToGlmQuat(btQuaternion btQuat);
	btQuaternion glmToBulletQuat(glm::quat glmQuat);
}