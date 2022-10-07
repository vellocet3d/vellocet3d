#pragma once

#include <string>
#include <optional>


namespace vel
{
	struct Texture;

	struct Material
	{
		std::string		name;
		glm::vec4		color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		Texture*		diffuse = nullptr;
		bool			hasAlphaChannel = false;
	};
}