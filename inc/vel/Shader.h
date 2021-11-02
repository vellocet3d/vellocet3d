#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "glm/glm.hpp"


typedef int GLint;

namespace vel
{
	struct Shader
	{
		unsigned int id;
		std::string name;
		std::string vertFile;
		std::string fragFile;
		std::unordered_map<std::string, GLint> uniformLocations;
	};
}