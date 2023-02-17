#pragma once

#include <string>
#include <vector>

#include "TextureData.h"

namespace vel
{
	struct Texture
	{
		std::string					name;
		std::vector<TextureData>	frames;
		bool						alphaChannel;
		unsigned int				uvWrapping;
	};
}