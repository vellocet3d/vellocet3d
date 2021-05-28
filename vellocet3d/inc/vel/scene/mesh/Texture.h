#pragma once

#include <string>


namespace vel
{
	struct Texture
	{
		unsigned int id;
		std::string type;
		std::string fullPath;
		std::string path;
		std::string filename;
		std::string fileExt;
		bool		alphaChannel;
	};
}