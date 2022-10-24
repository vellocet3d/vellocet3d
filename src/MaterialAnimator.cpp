#pragma once


#include "vel/MaterialAnimator.h"


namespace vel
{
	MaterialAnimator::MaterialAnimator(std::string name) :
		name(name)
	{

	}

	void MaterialAnimator::addTextureAnimator(float frameCount, float fps)
	{
		this->textureAnimators.push_back(TextureAnimator(frameCount, fps));
	}

	void MaterialAnimator::update(float frameTime)
	{
		for (auto& ta : this->textureAnimators)
			ta.update(frameTime);
	}

}