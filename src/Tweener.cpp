

#include "vel/Tweener.h"




namespace vel
{
	/*
		Smoothly translate from one glm::vec3 to another glm::vec3 at a given speed in units per second
	*/
	Tweener::Tweener(glm::vec3 from, glm::vec3 to, float speed) :
		fromVec(from),
		toVec(to),
		currentVec(from),
		speed(speed),
		forwardXDirection(0),
		backwardXDirection(0),
		forwardYDirection(0),
		backwardYDirection(0),
		forwardZDirection(0),
		backwardZDirection(0),
		forwardUpdateComplete(false),
		backwardUpdateComplete(true)
	{
		
		this->setDirections();
		this->setSpeeds();

	};

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

	void Tweener::updateCompletionStatus()
	{
		if (this->currentVec == this->toVec)
		{
			this->forwardUpdateComplete = true;
		}
		else
		{
			this->forwardUpdateComplete = false;
		}

		if (this->currentVec == this->fromVec)
		{
			this->backwardUpdateComplete = true;
		}
		else
		{
			this->backwardUpdateComplete = false;
		}
	}

	glm::vec3 Tweener::updateForward(float dt)
	{
		this->updateCompletionStatus();

		if (!this->forwardUpdateComplete)
		{
			// update this->currentVec
			this->currentVec.x += this->forwardXDirection * (this->speeds.x * dt);
			this->currentVec.y += this->forwardYDirection * (this->speeds.y * dt);
			this->currentVec.z += this->forwardZDirection * (this->speeds.z * dt);

			// correct x positions
			if (this->forwardXDirection == 1)
			{
				if (this->currentVec.x > this->toVec.x)
				{
					this->currentVec.x = this->toVec.x;
				}
			}
			else if (this->forwardXDirection == -1)
			{
				if (this->currentVec.x < this->toVec.x)
				{
					this->currentVec.x = this->toVec.x;
				}
			}
			else // 0
			{
				this->currentVec.x = this->fromVec.x;
			}

			// correct y positions
			if (this->forwardYDirection == 1)
			{
				if (this->currentVec.y > this->toVec.y)
				{
					this->currentVec.y = this->toVec.y;
				}
			}
			else if (this->forwardYDirection == -1)
			{
				if (this->currentVec.y < this->toVec.y)
				{
					this->currentVec.y = this->toVec.y;
				}
			}
			else // 0
			{
				this->currentVec.y = this->fromVec.y;
			}

			// correct z positions
			if (this->forwardZDirection == 1)
			{
				if (this->currentVec.z > this->toVec.z)
				{
					this->currentVec.z = this->toVec.z;
				}
			}
			else if (this->forwardZDirection == -1)
			{
				if (this->currentVec.z < this->toVec.z)
				{
					this->currentVec.z = this->toVec.z;
				}
			}
			else // 0
			{
				this->currentVec.z = this->fromVec.z;
			}
		}
		
		return this->currentVec;
	}

	glm::vec3 Tweener::updateBackward(float dt)
	{
		this->updateCompletionStatus();

		if (!this->backwardUpdateComplete)
		{
			// update this->currentVec
			this->currentVec.x += this->backwardXDirection * (this->speeds.x * dt);
			this->currentVec.y += this->backwardYDirection * (this->speeds.y * dt);
			this->currentVec.z += this->backwardZDirection * (this->speeds.z * dt);

			// correct x positions
			if (this->backwardXDirection == 1)
			{
				if (this->currentVec.x > this->fromVec.x)
				{
					this->currentVec.x = this->fromVec.x;
				}
			}
			else if (this->backwardXDirection == -1)
			{
				if (this->currentVec.x < this->fromVec.x)
				{
					this->currentVec.x = this->fromVec.x;
				}
			}
			else // 0
			{
				this->currentVec.x = this->fromVec.x;
			}

			// correct y positions
			if (this->backwardYDirection == 1)
			{
				if (this->currentVec.y > this->fromVec.y)
				{
					this->currentVec.y = this->fromVec.y;
				}
			}
			else if (this->backwardYDirection == -1)
			{
				if (this->currentVec.y < this->fromVec.y)
				{
					this->currentVec.y = this->fromVec.y;
				}
			}
			else // 0
			{
				this->currentVec.y = this->fromVec.y;
			}

			// correct z positions
			if (this->backwardZDirection == 1)
			{
				if (this->currentVec.z > this->fromVec.z)
				{
					this->currentVec.z = this->fromVec.z;
				}
			}
			else if (this->backwardZDirection == -1)
			{
				if (this->currentVec.z < this->fromVec.z)
				{
					this->currentVec.z = this->fromVec.z;
				}
			}
			else // 0
			{
				this->currentVec.z = this->fromVec.z;
			}
		}

		return this->currentVec;
	}

	glm::vec3 Tweener::getCurrentVec()
	{
		return this->currentVec;
	}

	void Tweener::setDirections()
	{
		// set forward directions
		if ((this->toVec.x - this->fromVec.x) > 0.0f)
		{
			this->forwardXDirection = 1;
		}
		else if ((this->toVec.x - this->fromVec.x) < 0.0f)
		{
			this->forwardXDirection = -1;
		}

		if ((this->toVec.y - this->fromVec.y) > 0.0f)
		{
			this->forwardYDirection = 1;
		}
		else if ((this->toVec.y - this->fromVec.y) < 0.0f)
		{
			this->forwardYDirection = -1;
		}

		if ((this->toVec.z - this->fromVec.z) > 0.0f)
		{
			this->forwardZDirection = 1;
		}
		else if ((this->toVec.z - this->fromVec.z) < 0.0f)
		{
			this->forwardZDirection = -1;
		}

		// set backward directions
		if ((this->fromVec.x - this->toVec.x) > 0.0f)
		{
			this->backwardXDirection = 1;
		}
		else if ((this->fromVec.x - this->toVec.x) < 0.0f)
		{
			this->backwardXDirection = -1;
		}

		if ((this->fromVec.y - this->toVec.y) > 0.0f)
		{
			this->backwardYDirection = 1;
		}
		else if ((this->fromVec.y - this->toVec.y) < 0.0f)
		{
			this->backwardYDirection = -1;
		}

		if ((this->fromVec.z - this->toVec.z) > 0.0f)
		{
			this->backwardZDirection = 1;
		}
		else if ((this->fromVec.z - this->toVec.z) < 0.0f)
		{
			this->backwardZDirection = -1;
		}
	}

	void Tweener::setSpeeds()
	{
		// find the component which has the greatest distance, this component will use the actual speed, while
		// the others will use the product of the percentage of their distance and the greatest distance and the actual speed
		float xDis = fabs(this->toVec.x - this->fromVec.x);
		float yDis = fabs(this->toVec.y - this->fromVec.y);
		float zDis = fabs(this->toVec.z - this->fromVec.z);

		if (xDis >= yDis && xDis >= zDis)
		{
			this->speeds.x = this->speed;
			this->speeds.y = yDis > 0.0f ? ((yDis / xDis) * this->speed) : 0.0f;
			this->speeds.z = zDis > 0.0f ? ((zDis / xDis) * this->speed) : 0.0f;
		}
		else if (yDis >= xDis && yDis >= zDis)
		{
			this->speeds.x = xDis > 0.0f ? ((xDis / yDis) * this->speed) : 0.0f;
			this->speeds.y = this->speed;
			this->speeds.z = zDis > 0.0f ? ((zDis / yDis) * this->speed) : 0.0f;
		}
		else if (zDis >= xDis && zDis >= yDis)
		{
			this->speeds.x = xDis > 0.0f ? ((xDis / zDis) * this->speed) : 0.0f;
			this->speeds.y = yDis > 0.0f ? ((yDis / zDis) * this->speed) : 0.0f;
			this->speeds.z = this->speed;
		}
	}

}