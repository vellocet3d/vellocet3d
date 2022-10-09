#pragma once

#include <string>
#include <vector>

#include "vel/ImageData.h"

namespace vel
{
	struct Texture
	{
		std::string					name;
		unsigned int				id; // buffer object id
		unsigned int				dsaIdIndex; // index of handle in ubo
		std::string					type;
		bool						alphaChannel;
		ImageData					primaryImageData;
		std::vector<ImageData>		mips;
	};
}