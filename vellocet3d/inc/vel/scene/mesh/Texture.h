#pragma once

#include <string>


namespace vel::scene::mesh
{
	struct Texture
	{
		unsigned int id;
		std::string type;
		std::string path;
	};
}