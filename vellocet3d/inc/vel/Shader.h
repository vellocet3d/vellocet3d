#pragma once

#include <string>
#include <vector>

#include "glm/glm.hpp"


namespace vel
{
	struct Shader
	{
		unsigned int id;
		std::string name;
		std::string vertFile;
		std::string fragFile;
	};
}