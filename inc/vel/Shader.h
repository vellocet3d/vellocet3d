#pragma once

#include <string>
#include <vector>

#include "glm/glm.hpp"
#include "robin_hood/robin_hood.h"

typedef int GLint;

namespace vel
{
	struct Shader
	{
		unsigned int id;
		std::string name;
		std::string vertFile;
		std::string fragFile;
		robin_hood::unordered_node_map<std::string, GLint> uniformLocations;
	};
}