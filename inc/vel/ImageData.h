#pragma once

#include <string>
#include <vector>

#include "glm/glm.hpp"

namespace vel
{
	struct ImageData
	{
		unsigned char*		data = nullptr;
		int					width;
		int					height;
		int					nrComponents;
		unsigned int		format;
		unsigned int		sizedFormat;

		glm::vec3			getPixelRGB(int x, int y);
		glm::vec3			getPixelRGBFromUV(float u, float v);
	};
}