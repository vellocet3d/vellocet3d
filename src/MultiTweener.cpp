
#include <iostream>
#include "vel/MultiTweener.h"




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
		foundPause(false),
		firstCycleStarted(false),
		useClosestPausePoint(false),
		shouldPlayForward(true),
		currentTweenIndex(0)
	{
		size_t i = 0;
		for (auto& v : this->vecs)
		{
			i++;
			if (i < this->vecs.size())
				this->tweens.push_back(Tweener(v, this->vecs[i], this->speedPerVec));
		}

		this->currentVec = this->vecs[0];
	};

	void MultiTweener::setPausePoints(std::vector<size_t> in)
	{
		this->pausePoints = in;
	}

	void MultiTweener::pause(bool useClosestPausePoint)
	{
		this->shouldPause = true;
		this->useClosestPausePoint = useClosestPausePoint;
	}

	void MultiTweener::unpause()
	{
		this->shouldPause = false;
		this->foundPause = false;
	}

	// TODO LEFT OFF AROUND HERE>>>NEED TO START ON playBackward METHOD
	glm::vec3 MultiTweener::playForward(float dt)
	{
		// if we should pause, and pause point has been found, OR if we should pause, but have not yet begun
		// tweening, then return the current vector as there is nothing else to process
		if ((this->shouldPause && this->foundPause) || (this->shouldPause && !this->firstCycleStarted))
			return this->currentVec;

		// Loop through each tween that was created during initialization
		for (; this->currentTweenIndex < this->tweens.size(); this->currentTweenIndex++)
		{
			this->firstCycleStarted = true; // we have begun processing tweens

			// If this tween has completed it's forward cycle, continue onto the next tween
			// THERE SHOULD BE NO NEED FOR THIS SINCE WE TRACK CURRENT TWEEN NOW
			//if (this->tweens[this->currentTweenIndex].isForwardComplete())
			//	continue;

			// If we should pause and we are processing the first tween, and the first tween's from vector is a pause point,
			// and the multitween's cycle has completed then flip foundPause flag and set currentVec to the
			// initial vector passed during initialization (the "from" vector of the first tween) and return this value
			if (this->shouldPause && this->currentTweenIndex == 0 && this->pausePointExists(0) && this->cycleComplete)
			{
				this->foundPause = true;
				this->currentVec = this->vecs[0];
				return this->currentVec;
			}

			// If we have made it this far, then we know we are not currently paused (although we could be
			// flagged to pause at the next pause point) and we should continue processing the current tween
			this->cycleComplete = false;

			// If we should pause AND use the closest pause point, AND that pause point is not forward,
			// then pass control to playBackward
			if (this->shouldPause && this->useClosestPausePoint)
				if(!this->isClosestPausePointForward())
					return this->playBackward(dt);

			// Update forward on currentTween
			this->currentVec = this->tweens[this->currentTweenIndex].updateForward(dt);

			// If this tween has completed it's forward cycle, then check to see if pause flag has been set,
			// AND if so then check to see if the "to" vector of the current tween is a pause point (this is
			// done by passing the current tween index plus 1 to pausePointExists as the to vector within a tween
			// will always be the tween index plus 1), if so then set foundPause to true and return currentVec
			if (this->tweens[this->currentTweenIndex].isForwardComplete())
			{
				if (this->shouldPause && this->pausePointExists(this->currentTweenIndex + 1))
				{
					this->foundPause = true;
					return this->currentVec;
				}
			}
			//TODO something needs to happen here
			return this->currentVec;
		}

		// If we have made it this far without returning a value, then the MultiTween has reached it's end AND is not paused
		this->cycleComplete = true;

		// If repeat is not set, then return currentVec as the multitween has completed and we should not execute it again
		if (!this->repeat)
			return this->currentVec;

		// Reset all tweens to their defaults so that they are ready to be processed again if repeat is set
		for (auto& t : this->tweens)
			t.reset();

		// Reset currentTweenIndex to zero since this is the playForward method and we would be starting at the 0th index
		this->currentTweenIndex = 0;

		// If we have made it this far, then do recursive call to restart the process as we need to repeat the multitween
		return this->update(dt);
	}

	glm::vec3 MultiTweener::playBackward(float dt)
	{
		int endIndex = this->tweens.size() - 1;

		if ((this->shouldPause && this->foundPause) || (this->shouldPause && !this->firstCycleStarted))
			return this->currentVec;

		for (; this->currentTweenIndex >= 0; this->currentTweenIndex--)
		{
			this->firstCycleStarted = true;

			if (this->shouldPause && this->currentTweenIndex == endIndex && this->pausePointExists(endIndex + 1) && this->cycleComplete)
			{
				this->foundPause = true;
				this->currentVec = this->vecs[endIndex + 1];
				return this->currentVec;
			}

			this->cycleComplete = false;

			if (this->shouldPause && this->useClosestPausePoint)
				if (this->isClosestPausePointForward())
					return this->playForward(dt);

			this->currentVec = this->tweens[this->currentTweenIndex].updateBackward(dt);

			if (this->tweens[this->currentTweenIndex].isBackwardComplete())
			{
				if (this->shouldPause && this->pausePointExists(this->currentTweenIndex))
				{
					this->foundPause = true;
					return this->currentVec;
				}
			}

			return this->currentVec;
		}


		this->cycleComplete = true;

		if (!this->repeat)
			return this->currentVec;

		for (auto& t : this->tweens)
			t.reset();

		this->currentTweenIndex = endIndex;

		return this->update(dt);
	}

	glm::vec3 MultiTweener::update(float dt)
	{
		if (this->shouldPlayForward)
			return this->playForward(dt);
		
		return this->playBackward(dt);
	}

	bool MultiTweener::pausePointExists(size_t in)
	{
		for (auto& pp : this->pausePoints)
			if (pp == in)
				return true;

		return false;
	}

	bool MultiTweener::isClosestPausePointForward()
	{
		return true;
	}

}