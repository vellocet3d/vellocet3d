#pragma once

#include <string>
#include <vector>

#include "vel/ImageData.h"

namespace vel
{
	struct Cubemap
	{
		std::string		name;
		ImageData		primaryImageData;
        unsigned int    hdrTexture;
        unsigned int    envCubemap;
        unsigned int    irradianceMap;
        unsigned int    prefilterMap;
        unsigned int    brdfLUTTexture;
	};
}