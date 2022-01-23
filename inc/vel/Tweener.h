#pragma once

#include "glm/glm.hpp"


namespace vel
{

	class Tweener
	{
	private:
		glm::vec3	fromVec;
		glm::vec3	toVec;
		glm::vec3	currentVec;
		float		speed;
		glm::vec3	speeds;

		int			forwardXDirection; // -1, 1, 0
		int			backwardXDirection; // -1, 1, 0
		int			forwardYDirection; // -1, 1, 0
		int			backwardYDirection; // -1, 1, 0
		int			forwardZDirection; // -1, 1, 0
		int			backwardZDirection; // -1, 1, 0
		
		void		setDirections();
		void		setSpeeds();

		bool		forwardUpdateComplete;
		bool		backwardUpdateComplete;
		void		updateCompletionStatus();

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