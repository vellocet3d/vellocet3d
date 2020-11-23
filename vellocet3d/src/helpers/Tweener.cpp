//#include <iostream>

#include "vel/helpers/Tweener.h"

// need to convert this to use vector math and not just break down each component

namespace vel::helpers
{
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
		backwardZDirection(0)
	{
		
		this->setDirections();
		//std::cout << "F:(" << this->forwardXDirection << "," << this->forwardYDirection << "," << this->forwardZDirection << ")\n";
		//std::cout << "B:(" << this->backwardXDirection << "," << this->backwardYDirection << "," << this->backwardZDirection << ")\n";

	};

	glm::vec3 Tweener::updateForward(float dt)
	{
		// update this->currentVec
		this->currentVec.x += this->forwardXDirection * (this->speed * dt);
		this->currentVec.y += this->forwardYDirection * (this->speed * dt);
		this->currentVec.z += this->forwardZDirection * (this->speed * dt);

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
			//this->currentVec.x = 0.0f;
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
			//this->currentVec.y = 0.0f;
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
			//this->currentVec.z = 0.0f;
			this->currentVec.z = this->fromVec.z;
		}

		return this->currentVec;
	}

	glm::vec3 Tweener::updateBackward(float dt)
	{
		// update this->currentVec
		this->currentVec.x += this->backwardXDirection * (this->speed * dt);
		this->currentVec.y += this->backwardYDirection * (this->speed * dt);
		this->currentVec.z += this->backwardZDirection * (this->speed * dt);

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

}