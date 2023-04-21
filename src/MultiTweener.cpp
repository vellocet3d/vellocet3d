
#include <iostream>
#include "vel/MultiTweener.h"
#include "glm/gtx/string_cast.hpp"



namespace vel
{
	/*
	Smoothly translate between a vector of glm::vec3 values
	*/
	MultiTweener::MultiTweener(std::vector<glm::vec3> vecs, float speed, bool repeat) :
		vecs(vecs),
		speed(speed),
		speedPerVec(speed * (float)vecs.size()),
		repeat(repeat),
		shouldPause(false),
		cycleComplete(false),
		foundPause(true),
		firstCycleStarted(false),
		shouldPlayForward(true),
		pauseAtPausePoint(true),
		useClosestPausePoint(false),
		currentTweenIndex(0),
		closestPausePointForward(true),
		closestPausePointFound(false),
		directionSwapNeedsCleared(false)
	{
		size_t i = 0;
		for (auto& v : this->vecs)
		{
			i++;
			if (i < this->vecs.size())
				this->tweens.push_back(Tweener(v, this->vecs[i], this->speedPerVec, TweenerDirection::Forward));
		}

		this->currentVec = this->vecs[0];
	};

	void MultiTweener::swapDirection()
	{
		int i = 0;
		for (auto& t : this->tweens)
		{
			// if the current tweener we are processing has not completed, then we do not want to reset it
			// as that will corrupt it's lerpVal, therefore if it's not completed then we only change it's direction,
			// otherwise it's treated like the rest of the tweens

			if ((i != this->currentTweenIndex) || (i == this->currentTweenIndex && t.isComplete()))
				t.reset();

			if (t.getDirection() == TweenerDirection::Forward)
				t.setDirection(TweenerDirection::Backward);
			else
				t.setDirection(TweenerDirection::Forward);

			i++;
		}
	}

	bool MultiTweener::getShouldPause()
	{
		return this->shouldPause;
	}

	bool MultiTweener::atPausePoint()
	{
		return this->foundPause;
	}

	void MultiTweener::updateSpeed(float newSpeed)
	{
		this->speed = newSpeed;
		this->speedPerVec = this->speed * (float)vecs.size();

		for (auto& t : this->tweens)
			t.updateSpeed(this->speedPerVec);
	}

	void MultiTweener::setPausePoints(std::vector<size_t> in)
	{
		this->pausePoints = in;
	}

	void MultiTweener::pause(bool pauseAtPausePoint, bool useClosestPausePoint)
	{
		this->shouldPause = true;
		this->pauseAtPausePoint = pauseAtPausePoint;
		this->useClosestPausePoint = useClosestPausePoint;
	}

	void MultiTweener::unpause()
	{
		if (this->shouldPause && this->closestPausePointFound && !this->closestPausePointForward && this->directionSwapNeedsCleared)
		{
			this->swapDirection();
			this->directionSwapNeedsCleared = false;
		}

		this->shouldPause = false;
		this->foundPause = false;
		this->closestPausePointFound = false;
	}

	glm::vec3 MultiTweener::playForward(float dt)
	{
		/*
			If this->shouldPause is true, and we have not begun processing any tweening, simply return
			this->currentVec. This allows for us to initialize an instance, then call pause() and have no
			tweening occur until we call unpause()
		*/
		if(this->shouldPause && !this->firstCycleStarted)
			return this->currentVec;
		
		/*
			If this->shouldPause is true, and we should not pause at a particular pause point (ie, we
			should be pausing immediately), then return this->currentVec
		*/
		if(this->shouldPause && !this->pauseAtPausePoint)
			return this->currentVec;
		
		/*
			If this->shouldPause is true, and we have to wait until we reach a pause point before pausing AND
			we have successfully reached this pause point, then return this->currentVec
		*/
		if (this->shouldPause && this->pauseAtPausePoint && this->foundPause)
			return this->currentVec;
		
		/*
			If a full cycle has been completed successfully AND we should not repeat the cycle, then return this->currentVec
		*/
		if (this->cycleComplete && !this->repeat)
			return this->currentVec;

		/*
			Begin looping through each Tween
		*/
		for (; this->currentTweenIndex < this->tweens.size(); this->currentTweenIndex++)
		{
			// If we have made it this far then we know that we can set this->firstCycleStarted to true
			// and this->cycleComplete to false because we still have a distance to cover within at least one
			// of the tweens, otherwise this->currentTweenIndex would have been incremented to the size of tweens,
			// breaking us out of this loop
			this->firstCycleStarted = true;
			this->cycleComplete = false;

			// The should pause flag has been set, as well as the useClosestPausePoint flag, meaning that we need to find
			// the pause point that is closest to the point in which this->currentVec is currently set to, whether it be forward
			// or backward
			if (this->shouldPause && this->useClosestPausePoint)
			{
				// this->closestPausePointFound is false, so we need to find the closest pause point
				if (!this->closestPausePointFound)
					this->findClosestPausePointDirection();

				// If the closest pause point IS NOT FORWARD, then we need to update the direction of tween at this->currentTweenIndex
				// to play backward
				if (!this->closestPausePointForward)
				{
					this->currentVec = this->tweens[this->currentTweenIndex].update(dt);

					if (this->tweens[this->currentTweenIndex].isComplete())
					{
						if (this->pausePointExists(this->currentTweenIndex))
						{
							// we have reached our pause point after translating backwards, therefore we
							// set this->foundPause to true and call this->swapDirection() to reset our
							// direction to forward, and return this->currentVec
							this->foundPause = true;

							this->swapDirection();

							this->directionSwapNeedsCleared = false;

							return this->currentVec;
						}
						else
						{
							this->currentTweenIndex--;
							return this->currentVec;
						}
					}
					else
					{
						return this->currentVec;
					}

					return this->currentVec;
				}
				
				/*
					If we have made it here, then the closest pause point is forward from this position,
					therefore we continue forward within the loop
				*/
				
			}			

			// If we have made it here, then we should not be pausing, therefore we set this->currentVec equal to
			// the value returned by the tween stored at this->currentTweenIndex
			this->currentVec = this->tweens[this->currentTweenIndex].update(dt);

			// If the tween stored at this->currentTweenIndex has completed it's cycle
			if (this->tweens[this->currentTweenIndex].isComplete())
			{
				// If we should pause AND the next tween index is a pause point, set this->foundPause equal to true
				// (since the from vec of the next tween will be the pause point, and this->currentVec IS also the same
				// value of the next tween's from position because of how we initialize our tweens in the constructor),
				// and return this->currentVec
				if (this->shouldPause && this->pausePointExists(this->currentTweenIndex + 1))
				{
					this->foundPause = true;
					return this->currentVec;
				}
				
				/* !!!!!!!!!!!!!!!
					If we've made it here, then we fall through and the next iteration of the loop
					plays out, the loop increments this->currentTweenIndex
				*/
				
			}
			// If the tween stored at this->currentTweenIndex HAS NOT completed it's cycle, then we simply return
			// this->currentVec (which will have been previously updated) until it has
			else
			{
				return this->currentVec;
			}
		}

		/*
			If we have made it here, then every tween has completed it's cycle
		*/
		
		this->cycleComplete = true;
		
		for (auto& t : this->tweens)
			t.reset();
			
		this->currentTweenIndex = 0;

		return this->update(dt);
	}

	glm::vec3 MultiTweener::update(float dt)
	{
		//std::cout 
		//	<< "------------------------------------------------\n"
		//	<< "currentTweenIndex:" << this->currentTweenIndex << "\n"
		//	<< "currentTweenFrom:" << glm::to_string(this->tweens[this->currentTweenIndex].getFrom()) << "\n"
		//	<< "currentTweenTo:" << glm::to_string(this->tweens[this->currentTweenIndex].getTo()) << "\n"
		//	<< "isForwardComplete:" << this->tweens[this->currentTweenIndex].isForwardComplete() << "\n"
		//	<< "isBackwardComplete:" << this->tweens[this->currentTweenIndex].isBackwardComplete() << "\n"
		//	<< "currentVec:" << glm::to_string(this->tweens[this->currentTweenIndex].getCurrentVec()) << "\n";

		return this->playForward(dt);
	}

	bool MultiTweener::pausePointExists(size_t in)
	{
		for (auto& pp : this->pausePoints)
			if (pp == in)
				return true;

		return false;
	}

	void MultiTweener::findClosestPausePointDirection()
	{
		// Forward
		bool foundForwardPausePoint = false;
		float distToClosestForwardPausePoint = 0.0f;
		auto lastVec = this->currentVec;
		int i = this->currentTweenIndex;
		while (i < (this->vecs.size() - 1) && !foundForwardPausePoint)
		{
			auto nextVec = this->vecs[i + 1];

			distToClosestForwardPausePoint += glm::distance(lastVec, nextVec);

			lastVec = nextVec;

			if (this->pausePointExists(i + 1))
				foundForwardPausePoint = true;

			i++;
		}

		// Backward
		bool foundBackwardPausePoint = false;
		float distToClosestBackwardPausePoint = 0.0f;
		lastVec = this->currentVec;
		i = this->currentTweenIndex;
		while (i >= 0 && !foundBackwardPausePoint)
		{
			auto nextVec = this->vecs[i];

			distToClosestBackwardPausePoint += glm::distance(lastVec, nextVec);

			lastVec = nextVec;

			if (this->pausePointExists(i))
				foundBackwardPausePoint = true;

			i--;
		}

		this->closestPausePointFound = true;

		if (distToClosestBackwardPausePoint < distToClosestForwardPausePoint)
		{
			this->closestPausePointForward = false;
			this->swapDirection();
			this->directionSwapNeedsCleared = true;
		}
		else
		{
			this->closestPausePointForward = true;
		}
			
	}

}