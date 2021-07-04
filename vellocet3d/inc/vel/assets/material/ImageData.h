#pragma once

#include <string>
#include <vector>

namespace vel
{
	struct ImageData
	{
		unsigned char*		data;
		int					width;
		int					height;
		int					nrComponents;
		unsigned int		format;
	};
}