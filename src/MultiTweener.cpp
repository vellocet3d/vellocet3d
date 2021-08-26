

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
		firstCycleStarted(false)
	{

		size_t i = 0;
		for (auto& v : this->vecs)
		{
			i++;
			if (i < this->vecs.size())
			{
				this->tweens.push_back(Tweener(v, this->vecs[i], this->speedPerVec));
			}
		}

		this->currentVec = this->vecs[0];

	};

	void MultiTweener::setPausePoints(std::vector<size_t> in)
	{
		this->pausePoints = in;
	}

	void MultiTweener::pause()
	{
		this->shouldPause = true;
	}

	void MultiTweener::unpause()
	{
		this->shouldPause = false;
		this->foundPause = false;
	}

	glm::vec3 MultiTweener::update(float dt)
	{
		if ((this->shouldPause && this->foundPause) || (this->shouldPause && !this->firstCycleStarted))
		{
			return this->currentVec;
		}

		for (int i = 0; i < this->tweens.size(); i++)
		{
			this->firstCycleStarted = true;

			if (this->tweens[i].isForwardComplete())
			{
				continue;
			}

			if (this->shouldPause && i == 0 && this->pausePointExists(0) && this->cycleComplete)
			{
				this->foundPause = true;
				this->currentVec = this->vecs[0];
				return this->currentVec;
			}

			this->cycleComplete = false;

			this->currentVec = this->tweens[i].updateForward(dt);

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

		this->cycleComplete = true;

		if (!this->repeat)
			return this->currentVec;

		for (auto& t : this->tweens)
		{
			t.reset();
		}

		return this->update(dt);
	}

	bool MultiTweener::pausePointExists(size_t in)
	{
		for (auto& pp : this->pausePoints)
		{
			if (pp == in)
				return true;
		}

		return false;
	}

}