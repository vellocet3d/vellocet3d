#pragma once

#include "glm/glm.hpp"


namespace vel
{
	class Tweener
	{
	private:
		glm::vec3	fromVec;
		glm::vec3	toVec;
		float		distance;
		glm::vec3	currentVec;
		float		speed;
		float		lerpVal;

		bool		forwardUpdateComplete;
		bool		backwardUpdateComplete;

	public:
		Tweener(glm::vec3 from, glm::vec3 to, float speed);

		glm::vec3	updateForward(float dt);
		glm::vec3	updateBackward(float dt);
		glm::vec3	getCurrentVec();

		bool		isForwardComplete();
		bool		isBackwardComplete();
		void		reset();

		void		updateSpeed(float newSpeed);
	};
}