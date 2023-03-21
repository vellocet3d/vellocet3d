#include "vel/ImageData.h"

namespace vel 
{
	glm::vec3 ImageData::getPixelRGB(int x, int y)
	{
		if (x >= 0 && x < width && y >= 0 && y < height)
		{
			int index = (y * width + x) * nrComponents;
			//return glm::vec3(data[index], data[index + 1], data[index + 2]);
			//return glm::vec3(data[index] / 255, data[index + 1] / 255, data[index + 2] / 255);
			return glm::vec3((data[index] / 255.0f), (data[index + 1] / 255.0f), (data[index + 2] / 255.0f));
		}

		return glm::vec3(1.0f);
	}

	glm::vec3 ImageData::getPixelRGBFromUV(float u, float v)
	{
		int x = (int)round(u * width);
		//int y = (int)round(height - round(v * height));
		int y = (int)round(v * height);

		return this->getPixelRGB(x, y);
	}
}