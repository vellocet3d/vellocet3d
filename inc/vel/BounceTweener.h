#pragma once

#include "glm/glm.hpp"

#include "vel/Tweener.h"

namespace vel
{
	class BounceTweener
	{
	private:
		Tweener		tweener;

	public:
					BounceTweener(glm::vec3 from, glm::vec3 to, float speed);

		glm::vec3	update(float dt);
		void		updateSpeed(float newSpeed);
		glm::vec3	getCurrentVec();
	};
}