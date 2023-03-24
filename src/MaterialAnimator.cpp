#pragma once

#include <iostream>

#include "vel/MaterialAnimator.h"


namespace vel
{
	MaterialAnimator::MaterialAnimator(){}

	unsigned int MaterialAnimator::getTextureCurrentFrame(unsigned int index)
	{
		return this->textureAnimators.at(index).getCurrentFrame();
	}

	void MaterialAnimator::addTextureAnimator(float frameCount, float fps)
	{
		this->textureAnimators.push_back(TextureAnimator(frameCount, fps));
	}

	void MaterialAnimator::update(float frameTime)
	{
		for (auto& ta : this->textureAnimators)
		{
			//std::cout << "here002\n";
			ta.update(frameTime);
		}
	}

	void MaterialAnimator::setTextureAnimatorPauseAfterCycles(unsigned int textureId, unsigned int cycles)
	{
		this->textureAnimators.at(textureId).setPauseAfterCycles(cycles);
	}

	void MaterialAnimator::setTextureAnimatorPause(unsigned int textureId, bool isPaused)
	{
		this->textureAnimators.at(textureId).setPaused(isPaused);
	}

	bool MaterialAnimator::getTexturePaused(unsigned int textureId)
	{
		return this->textureAnimators.at(textureId).getPaused();
	}

	void MaterialAnimator::setAnimatedTextureReverse(unsigned int textureId, bool reverse)
	{
		this->textureAnimators.at(textureId).setReverse(reverse);
	}

	bool MaterialAnimator::getAnimatedTextureReversed(unsigned int textureId)
	{
		return this->textureAnimators.at(textureId).getIsPlayingReversed();
	}

}