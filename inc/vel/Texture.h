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
		uint64_t					dsaHandle;
		std::string					type;
		bool						alphaChannel;
		ImageData					primaryImageData;
		std::vector<ImageData>		mips;
	};
}