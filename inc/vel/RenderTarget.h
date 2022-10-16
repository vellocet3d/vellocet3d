#pragma once

#include "glm/glm.hpp"

namespace vel
{
	struct RenderTarget
	{
		glm::ivec2 resolution;
		unsigned int FBO;
		unsigned int FBOTexture;
		uint64_t FBOTextureDSAHandle;
		unsigned int RBO;
	};
}