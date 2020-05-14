#pragma once

#include <vector>

#include "glm/glm.hpp"

namespace vel::scene::stage
{
	class Actor;
}

namespace vel::collision
{
	class EllipsoidCollider
	{

	private:
		glm::vec3				ellipsoidSpace;
		std::vector<glm::vec3>	collisionVertices;
		std::vector<size_t>		collisionIndices;


	public:
								EllipsoidCollider(glm::vec3 ellipsoidSpace);
		void					addActorToCollisionSoup(vel::scene::stage::Actor& actor);
		

	};

}