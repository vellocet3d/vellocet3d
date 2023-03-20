#pragma once

#include "glm/glm.hpp"

#include "vel/Texture.h"

namespace vel
{
	struct CustomTriangleMeshData
	{
		glm::vec2 textureUVs;
		glm::vec2 lightmapUVs;
		Texture* texture; // remember that image data values only stay in ram if denoted when loading textures
		Texture* lightmapTexture;
	};
}