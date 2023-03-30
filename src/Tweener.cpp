#include <iostream>

#define GLM_FORCE_ALIGNED_GENTYPES
#include <glm/gtx/string_cast.hpp>
#include "glm/gtx/compatibility.hpp"

#include "vel/Tweener.h"

namespace vel
{
	/*
		Smoothly translate from one glm::vec3 to another glm::vec3 at a given speed in units per second.
		Allow for a direction to be given, which will alter the direction of the tween in realtime, as long
		as the tween has not been completed. If the tween has been completed, it must be reset, then the direction
		must be changed
	*/
	Tweener::Tweener(glm::vec3 from, glm::vec3 to, float speed, TweenerDirection defaultDirection) :
		fromVec(from),
		toVec(to),
		distance(glm::distance(from, to)),
		currentVec(from),
		speed(speed),
		lerpVal(0.0f),
		defaultDirection(defaultDirection),
		direction(defaultDirection),
		completed(false)
	{};

	glm::vec3 Tweener::getFrom()
	{
		return this->fromVec;
	}

	glm::vec3 Tweener::getTo()
	{
		return this->toVec;
	}

	TweenerDirection Tweener::getDirection()
	{
		return this->direction;
	}

	void Tweener::setDirection(TweenerDirection d)
	{
		if(this->direction == d)
			return;
		
		this->direction = d;
		
		// If this->lerpVal does not equal 0.0f, then we assume we are in the middle of
		// tweening between our from and to vectors, therefore we need to set this->lerpVal
		// equal to the difference between 1.0f and this->lerpVal.
		if(this->lerpVal != 0.0f)
			this->lerpVal = 1.0f - this->lerpVal;
	}

	void Tweener::reset()
	{
		if (this->defaultDirection == TweenerDirection::Forward)
			this->currentVec = this->fromVec;
		else
			this->currentVec = this->toVec;

		this->completed = false;
		this->lerpVal = 0.0f;
	}

	bool Tweener::isComplete()
	{
		return this->completed;
	}

	glm::vec3 Tweener::update(float dt)
	{
		if (this->completed)
			return this->currentVec;

		this->lerpVal += (1.0f / (this->distance / this->speed)) * dt;

		if (this->lerpVal >= 1.0f)
		{
			this->lerpVal = 1.0f; // insure we equal exactly 1.0f
			this->completed = true;
			this->currentVec = this->direction == TweenerDirection::Forward ? this->toVec : this->fromVec;
		}
		else
		{
			this->currentVec = this->direction == TweenerDirection::Forward ? 
				glm::lerp(this->fromVec, this->toVec, this->lerpVal) :
				this->currentVec = glm::lerp(this->toVec, this->fromVec, this->lerpVal);
		}

		return this->currentVec;
	}

	glm::vec3 Tweener::getCurrentVec()
	{
		return this->currentVec;
	}

	void Tweener::updateSpeed(float newSpeed)
	{
		this->speed = newSpeed;
	}

}