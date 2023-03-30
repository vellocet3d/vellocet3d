#include <iostream>

#include "vel/BounceTweener.h"




namespace vel
{
	BounceTweener::BounceTweener(glm::vec3 from, glm::vec3 to, float speed) :
		tweener(Tweener(from, to, speed, TweenerDirection::Forward))
	{};

	glm::vec3 BounceTweener::update(float dt)
	{
		if (this->tweener.getDirection() == TweenerDirection::Forward && this->tweener.isComplete())
		{
			this->tweener.reset();
			this->tweener.setDirection(TweenerDirection::Backward);
		}
		else if (this->tweener.getDirection() == TweenerDirection::Backward && this->tweener.isComplete())
		{
			this->tweener.reset();
			this->tweener.setDirection(TweenerDirection::Forward);
		}

		return this->tweener.update(dt);
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