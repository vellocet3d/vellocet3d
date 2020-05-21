#pragma once

#include <vector>
#include <string>

#include "glm/glm.hpp"
#include "bullet/LinearMath/btVector3.h"

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
}