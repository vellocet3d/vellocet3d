#pragma once

#include "glm/glm.hpp"

namespace vel
{
	struct FontGlyphInfo 
	{
		glm::vec3 positions[4];
		glm::vec2 uvs[4];
		float offsetX = 0;
		float offsetY = 0;
	};
}