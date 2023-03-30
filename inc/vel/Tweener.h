#pragma once

#include "glm/glm.hpp"


namespace vel
{
	enum TweenerDirection 
	{
		Forward,
		Backward
	};

	class Tweener
	{
	private:
		glm::vec3			fromVec;
		glm::vec3			toVec;
		float				distance;
		glm::vec3			currentVec;
		float				speed;
		float				lerpVal;

		TweenerDirection	defaultDirection;
		TweenerDirection	direction;
		bool				completed;

	public:
		Tweener(glm::vec3 from, glm::vec3 to, float speed, TweenerDirection defaultDirection = TweenerDirection::Forward);

		void				setDirection(TweenerDirection d);
		TweenerDirection	getDirection();

		glm::vec3	update(float dt);
		glm::vec3	getCurrentVec();

		bool		isComplete();
		void		reset();

		void		updateSpeed(float newSpeed);

		glm::vec3	getFrom();
		glm::vec3	getTo();

	};
}