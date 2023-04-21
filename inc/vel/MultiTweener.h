#pragma once

#include <vector>

#include "glm/glm.hpp"

#include "vel/Tweener.h"

namespace vel
{

	class MultiTweener
	{
	private:
		std::vector<glm::vec3>	vecs;
		float					speed;
		float					speedPerVec;
		bool					repeat;
		std::vector<Tweener>	tweens;
		glm::vec3				currentVec;
		

		bool					shouldPause;
		std::vector<size_t>		pausePoints;
		bool					pausePointExists(size_t in);
		bool					cycleComplete;
		bool					foundPause;
		bool					firstCycleStarted;
		
		bool					shouldPlayForward;
		glm::vec3				playForward(float dt);
		//glm::vec3				playBackward(float dt);

		bool					useClosestPausePoint;
		bool					pauseAtPausePoint;
		void					findClosestPausePointDirection();

		int						currentTweenIndex;
		bool					closestPausePointForward;
		bool					closestPausePointFound;
		bool					directionSwapNeedsCleared;

	public:
		MultiTweener(std::vector<glm::vec3> vecs, float speed, bool repeat = false);

		glm::vec3	update(float dt);
		void		pause(bool pauseAtPausePoint = true, bool useClosestPausePoint = false);
		void		unpause();
		void		setPausePoints(std::vector<size_t> in);

		void		updateSpeed(float newSpeed);

		bool		atPausePoint();
		bool		getShouldPause();

		void		swapDirection();


	};
}