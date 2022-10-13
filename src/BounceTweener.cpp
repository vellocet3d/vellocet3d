#include <iostream>

#include "vel/BounceTweener.h"




namespace vel
{
	/*
		Smoothly translate from one glm::vec3 to another glm::vec3 at a given speed in units per second
	*/
	BounceTweener::BounceTweener(glm::vec3 from, glm::vec3 to, float speed) :
		tweener(Tweener(from, to, speed))
	{};

	glm::vec3 BounceTweener::update(float dt)
	{
		if (!this->tweener.isForwardComplete())
		{
			return this->tweener.updateForward(dt);
		}
		else
		{
			return this->tweener.updateBackward(dt);
		}
			
	}

	void BounceTweener::updateSpeed(float newSpeed)
	{
		this->tweener.updateSpeed(newSpeed);
	}

	glm::vec3 BounceTweener::getCurrentVec()
	{
		return this->tweener.getCurrentVec();
	}

}