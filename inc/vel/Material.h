#pragma once

#include <string>
#include <optional>
#include <vector>

#include "vel/MaterialAnimator.h"


namespace vel
{
	struct Texture;

	struct Material
	{
		std::string				name;
		glm::vec4				color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		std::vector<Texture*>	textures;
		bool					hasAlphaChannel = false;

		std::optional<MaterialAnimator> materialAnimator;

		void addTexture(Texture* t) 
		{
			this->textures.push_back(t);
		}

		void addAnimatedTexture(Texture* t, float fps)
		{
			if (!this->materialAnimator.has_value())
				this->materialAnimator = MaterialAnimator();
			
			this->addTexture(t);
			this->materialAnimator->addTextureAnimator(t->frames.size(), fps);
		}
	};
}