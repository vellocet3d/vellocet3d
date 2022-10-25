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

}