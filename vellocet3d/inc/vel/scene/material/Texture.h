#pragma once

#include <string>
#include <vector>

namespace vel
{
	struct Texture
	{
		std::string					name;
		unsigned int				id;
		std::string					type;
		std::string					path;
		bool						alphaChannel;
		std::vector<std::string>	mips;
	};
}