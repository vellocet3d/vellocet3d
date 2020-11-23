#pragma once

#include <vector>

#include "glm/glm.hpp"

#include "vel/helpers/Tweener.h"

namespace vel::helpers
{

	class MultiTweener
	{
	private:
		std::vector<glm::vec3>	vecs;
		float					speed;
		float					speedPerVec;
		bool					repeat;
		std::vector<Tweener>	tweens;

	public:
		MultiTweener(std::vector<glm::vec3> vecs, float speed, bool repeat = false);

		glm::vec3 update(float dt);

	};
}