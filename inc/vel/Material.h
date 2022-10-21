#pragma once

#include <string>
#include <optional>
#include <vector>


namespace vel
{
	struct Texture;

	struct Material
	{
		std::string				name;
		glm::vec4				color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		std::vector<Texture*>	textures;
		bool					hasAlphaChannel = false;

		void addTexture(Texture* t) 
		{
			this->textures.push_back(t);
		}
	};
}