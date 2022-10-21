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
		std::vector<float>		heightScales;
		bool					hasAlphaChannel = false;

		void addTexture(Texture* t) 
		{
			this->textures.push_back(t);
		}

		void addHeightScale(float hs)
		{
			this->heightScales.push_back(hs);
		}
	};
}