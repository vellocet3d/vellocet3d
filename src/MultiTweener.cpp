

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
		useClosestPausePoint(false)
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

	glm::vec3 MultiTweener::update(float dt)
	{
		// if we should pause, and pause point has been found, OR if we should pause, but have not yet begun
		// tweening, then return the current vector as there is nothing else to process
		if ((this->shouldPause && this->foundPause) || (this->shouldPause && !this->firstCycleStarted))
			return this->currentVec;

		// Loop through each tween that was created during initialization
		for (int i = 0; i < this->tweens.size(); i++)
		{
			this->firstCycleStarted = true; // we have begun processing tweens

			// If this tween has completed it's forward cycle, continue onto the next tween
			if (this->tweens[i].isForwardComplete())
				continue;

			// If we should pause and we are processing the first tween, and the first tween's from vector is a pause point,
			// and the multitween's cycle has completed then flip foundPause flag and set currentVec to the
			// initial vector passed during initialization (the "from" vector of the first tween) and return this value
			if (this->shouldPause && i == 0 && this->pausePointExists(0) && this->cycleComplete)
			{
				this->foundPause = true;
				this->currentVec = this->vecs[0];
				return this->currentVec;
			}

			// If we have made it this far, then we know we are not currently paused (although we could be
			// flagged to pause at the next pause point) and we should continue processing the current tween
			this->cycleComplete = false;

			// Currently only updating forward, which means that we can only find pause points in the future,
			// so if pause is called, the tween will continue progressing forward until it reaches the next
			// designated pause point
			this->currentVec = this->tweens[i].updateForward(dt);

			// If this tween has completed it's forward cycle, then check to see if pause flag has been set,
			// AND if so then check to see if the "to" vector of the current tween is a pause point (this is
			// done by passing the current tween index plus 1 to pausePointExists as the to vector within a tween
			// will always be the tween index plus 1), if so then set foundPause to true and return currentVec
			if (this->tweens[i].isForwardComplete())
			{
				if (this->shouldPause && this->pausePointExists(i + 1))
				{
					this->foundPause = true;
					return this->currentVec;
				}
			}

			return this->currentVec;
		}

		// We have cycled through all tweens within this multitween, therefore this cycle has been completed
		this->cycleComplete = true;

		// If repeat is not set, then return currentVec as the multitween has completed and we should not execute it again
		if (!this->repeat)
			return this->currentVec;

		// Reset all tweens to their defaults so that they are ready to be processed again if repeat is set
		for (auto& t : this->tweens)
			t.reset();

		// If we have made it this far, then do recursive call to restart the process as we need to repeat the multitween
		return this->update(dt);
	}

	bool MultiTweener::pausePointExists(size_t in)
	{
		for (auto& pp : this->pausePoints)
			if (pp == in)
				return true;

		return false;
	}

}