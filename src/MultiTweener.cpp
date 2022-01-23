
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
		foundPause(false),
		firstCycleStarted(false),
		pauseAtPausePoint(true),
		useClosestPausePoint(false),
		closestPausePointForward(true),
		closestPausePointFound(false),
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
		this->pauseAtPausePoint = true;
		this->useClosestPausePoint = useClosestPausePoint;
	}

	void MultiTweener::unpause()
	{
		this->shouldPause = false;
		this->foundPause = false;
		this->closestPausePointFound = false;
	}


	glm::vec3 MultiTweener::playForward(float dt)
	{
		if ((this->shouldPause && this->foundPause) || (this->shouldPause && !this->firstCycleStarted))
			return this->currentVec;

		for (; this->currentTweenIndex < this->tweens.size(); this->currentTweenIndex++)
		{
			this->firstCycleStarted = true;
			this->cycleComplete = false;

			if (this->shouldPause && this->useClosestPausePoint)
			{
				//std::cout << "YEET001" << std::endl;
				if (!this->closestPausePointFound)
				{
					//std::cout << "YEET002" << std::endl;
					this->findClosestPausePointDirection();
				}
					
				//std::cout << "YEET003" << std::endl;

				if (!this->closestPausePointForward)
				{
					//std::cout << "YEET004" << std::endl;
					this->currentVec = this->tweens[this->currentTweenIndex].updateBackward(dt);

					if (this->tweens[this->currentTweenIndex].isBackwardComplete())
					{
						//std::cout << "YEET005" << std::endl;
						if (this->shouldPause && this->pausePointExists(this->currentTweenIndex))
						{
							//std::cout << "YEET006" << std::endl;
							this->foundPause = true;
							return this->currentVec;
						}
						else
						{
							//std::cout << "YEET006b" << std::endl;
							this->currentTweenIndex--;
							return this->currentVec;
						}
					}
					else
					{
						//std::cout << "YEET007" << std::endl;
						return this->currentVec;
					}
				}
			}

			//std::cout << "YEET008" << std::endl;

			this->currentVec = this->tweens[this->currentTweenIndex].updateForward(dt);

			if (this->tweens[this->currentTweenIndex].isForwardComplete())
			{
				//std::cout << "YEET009" << std::endl;
				if (this->shouldPause && this->pausePointExists(this->currentTweenIndex + 1))
				{
					//std::cout << "YEET010" << std::endl;
					this->foundPause = true;
					return this->currentVec;
				}
			}
			else
			{
				//std::cout << "YEET011" << std::endl;
				return this->currentVec;
			}
		}

		//std::cout << "YEET012" << std::endl;

		this->cycleComplete = true;

		if (!this->repeat)
		{
			//std::cout << "YEET013" << std::endl;
			return this->currentVec;
		}
			

		for (auto& t : this->tweens)
			t.reset();

		this->currentTweenIndex = 0;

		//std::cout << "YEET014" << std::endl;

		return this->update(dt);
	}

	//glm::vec3 MultiTweener::playBackward(float dt)
	//{
	//	int endIndex = this->tweens.size() - 1;

	//	if ((this->shouldPause && this->foundPause) || (this->shouldPause && !this->firstCycleStarted))
	//		return this->currentVec;

	//	for (; this->currentTweenIndex >= 0; this->currentTweenIndex--)
	//	{
	//		this->firstCycleStarted = true;
	//		this->cycleComplete = false;

	//		if (this->shouldPause && this->useClosestPausePoint && !this->closestPausePointFound)
	//		{
	//			std::cout << "YEET003" << std::endl;
	//			if (this->isClosestPausePointForward())
	//			{
	//				std::cout << "YEET004" << std::endl;
	//				return this->playForward(dt);
	//			}
	//				
	//		}
	//			
	//		std::cout << "YEET005" << std::endl;

	//		this->currentVec = this->tweens[this->currentTweenIndex].updateBackward(dt);

	//		if (this->tweens[this->currentTweenIndex].isBackwardComplete())
	//		{
	//			std::cout << "YEET006" << std::endl;
	//			if (this->shouldPause && this->pausePointExists(this->currentTweenIndex))
	//			{
	//				std::cout << "YEET007" << std::endl;
	//				this->foundPause = true;
	//				return this->currentVec;
	//			}
	//		}
	//		else
	//		{
	//			std::cout << "YEET008" << std::endl;
	//			return this->currentVec;
	//		}
	//	}

	//	std::cout << "YEET009" << std::endl;

	//	this->cycleComplete = true;

	//	if (!this->repeat)
	//		return this->currentVec;

	//	for (auto& t : this->tweens)
	//		t.reset();

	//	this->currentTweenIndex = endIndex;

	//	return this->update(dt);
	//}

	glm::vec3 MultiTweener::update(float dt)
	{
		//std::cout << this->currentTweenIndex << std::endl;

		if (this->shouldPlayForward)
			return this->playForward(dt);
		
		//return this->playBackward(dt);
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
		//std::cout << "isClosestPausePointForward()----------------------------------" << std::endl;
		//std::cout << "All vecs---------------------------" << std::endl;
		//for (auto& v : this->vecs)
		//	std::cout << glm::to_string(v) << std::endl;
		//std::cout << "-----------------------------------" << std::endl;
		//std::cout << "currentVec: " << glm::to_string(this->currentVec) << std::endl;

		// Forward
		bool foundForwardPausePoint = false;
		float distToClosestForwardPausePoint = 0.0f;
		auto lastVec = this->currentVec;
		int i = this->currentTweenIndex;
		//std::cout << "forward loop----------------------------" << std::endl;
		while (i < (this->vecs.size() - 1) && !foundForwardPausePoint)
		{
			auto nextVec = this->vecs[i + 1];

			//std::cout << "------" << std::endl;
			//std::cout << "nextVec: " << glm::to_string(nextVec) << std::endl;
			//std::cout << "lastVec: " << glm::to_string(lastVec) << std::endl;
			//std::cout << "dist: " << glm::distance(lastVec, nextVec) << std::endl;


			distToClosestForwardPausePoint += glm::distance(lastVec, nextVec);

			//std::cout << "accdist: " << distToClosestForwardPausePoint << std::endl;
			//std::cout << "------" << std::endl;


			lastVec = nextVec;

			if (this->pausePointExists(i + 1))
				foundForwardPausePoint = true;

			i++;
		}
		//std::cout << "-----------------------------------" << std::endl;

		// Backward
		bool foundBackwardPausePoint = false;
		float distToClosestBackwardPausePoint = 0.0f;
		lastVec = this->currentVec;
		i = this->currentTweenIndex;
		//std::cout << "backward loop----------------------------" << std::endl;
		while (i >= 0 && !foundBackwardPausePoint)
		{
			auto nextVec = this->vecs[i];

			//std::cout << "------" << std::endl;
			//std::cout << "nextVec: " << glm::to_string(nextVec) << std::endl;
			//std::cout << "lastVec: " << glm::to_string(lastVec) << std::endl;
			//std::cout << "dist: " << glm::distance(lastVec, nextVec) << std::endl;

			distToClosestBackwardPausePoint += glm::distance(lastVec, nextVec);

			//std::cout << "accdist: " << distToClosestBackwardPausePoint << std::endl;
			//std::cout << "------" << std::endl;

			lastVec = nextVec;

			//if (this->pausePointExists(i - 1))
			if (this->pausePointExists(i))
				foundBackwardPausePoint = true;

			i--;
		}
		//std::cout << "-----------------------------------" << std::endl;

		this->closestPausePointFound = true;

		//std::cout << "fd:" << distToClosestForwardPausePoint << " bd:" << distToClosestBackwardPausePoint;
		if (distToClosestBackwardPausePoint < distToClosestForwardPausePoint) 
		{
			//std::cout << " - backward closest" << std::endl;
			//return false;
			this->closestPausePointForward = false;
		}
		else // if forward distance is closest or equal to backward distance
		{
			//std::cout << " - forward closest" << std::endl;
			//return true;
			this->closestPausePointForward = true;
		}
			
		//std::cout << "----------------------------------" << std::endl;
		


		//if (distToClosestForwardPausePoint < distToClosestBackwardPausePoint)
		//	return true;

		//return false;
	}

}