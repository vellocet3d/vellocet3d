#pragma once

#include <string>
#include <vector>

#include "vel/assets/material/ImageData.h"

namespace vel
{
	//struct Texture
	//{
	//	std::string					name;
	//	unsigned int				id;
	//	std::string					type;
	//	std::string					path;
	//	bool						alphaChannel;
	//	std::vector<std::string>	mips;
	//};

	struct Texture
	{
		std::string					name;
		unsigned int				id;
		std::string					type;
		bool						alphaChannel;
		ImageData					primaryImageData;
		std::vector<ImageData>		mips;
	};
}