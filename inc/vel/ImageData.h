#pragma once

#include <string>
#include <vector>

namespace vel
{
	struct ImageData
	{
		unsigned char*		data = nullptr;
		float*				dataf = nullptr;
		int					width;
		int					height;
		int					nrComponents;
		unsigned int		format;
		unsigned int		sizedFormat;
	};
}