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
		MaterialAnimator();

		void addTextureAnimator(float frameCount, float fps);
		void update(float frameTime);
		
		unsigned int getTextureCurrentFrame(unsigned int index);
		
	};
}