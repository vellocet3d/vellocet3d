#pragma once

#include "glm/glm.hpp"

#include "vel/Texture.h"

namespace vel
{
	struct RenderTarget
	{
		glm::ivec2 resolution;
		unsigned int FBO;
		unsigned int RBO;
		Texture texture;

		//unsigned int FBOTexture;
		//uint64_t FBOTextureDSAHandle;
		
	};
}