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
		
		void setTextureAnimatorPauseAfterCycles(unsigned int textureId, unsigned int cycles);
		void setTextureAnimatorPause(unsigned int textureId, bool isPaused);
		bool getTexturePaused(unsigned int textureId);

		void setAnimatedTextureReverse(unsigned int textureId, bool reverse);
		bool getAnimatedTextureReversed(unsigned int textureId);

		unsigned int getTextureCurrentFrame(unsigned int index);
		
	};
}