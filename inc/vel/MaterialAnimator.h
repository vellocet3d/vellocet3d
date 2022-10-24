#pragma once

#include <string>
#include <vector>

#include "vel/TextureAnimator.h"

namespace vel
{
	class MaterialAnimator
	{
	private:
		
		std::vector<TextureAnimator>	textureAnimators;

	public:
		std::string name;

		MaterialAnimator(std::string name);

		void addTextureAnimator(float frameCount, float fps);
		void update(float frameTime);
		
	};
}