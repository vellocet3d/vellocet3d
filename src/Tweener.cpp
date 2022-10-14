#define GLM_FORCE_ALIGNED_GENTYPES
#include <glm/gtx/string_cast.hpp>
#include "glm/gtx/compatibility.hpp"

#include "vel/Tweener.h"

namespace vel
{
	/*
		Smoothly translate from one glm::vec3 to another glm::vec3 at a given speed in units per second
	*/
	Tweener::Tweener(glm::vec3 from, glm::vec3 to, float speed) :
		fromVec(from),
		toVec(to),
		distance(glm::distance(from, to)),
		currentVec(from),
		speed(speed),
		lerpVal(0.0f),
		forwardUpdateComplete(false),
		backwardUpdateComplete(true)
	{};

	void Tweener::reset()
	{
		this->currentVec = this->fromVec;
		this->forwardUpdateComplete = false;
		this->backwardUpdateComplete = true;
	}

	bool Tweener::isForwardComplete()
	{
		return this->forwardUpdateComplete;
	}

	bool Tweener::isBackwardComplete()
	{
		return this->backwardUpdateComplete;
	}

	glm::vec3 Tweener::updateForward(float dt)
	{
		if (!this->forwardUpdateComplete)
		{
			this->lerpVal += (1.0f / (this->distance / this->speed)) * dt;

			if (this->lerpVal >= 1.0f)
			{
				this->currentVec = this->toVec;
				this->lerpVal = 0.0f;
				this->forwardUpdateComplete = true;
				this->backwardUpdateComplete = false;
			}
			else
			{
				this->currentVec = glm::lerp(this->fromVec, this->toVec, this->lerpVal);
			}
		}

		return this->currentVec;
	}

	glm::vec3 Tweener::updateBackward(float dt)
	{
		if (!this->backwardUpdateComplete)
		{
			this->lerpVal += (1.0f / (this->distance / this->speed)) * dt;

			if (this->lerpVal >= 1.0f)
			{
				this->currentVec = this->fromVec;
				this->lerpVal = 0.0f;
				this->forwardUpdateComplete = false;
				this->backwardUpdateComplete = true;
			}
			else
			{
				this->currentVec = glm::lerp(this->toVec, this->fromVec, this->lerpVal);
			}
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