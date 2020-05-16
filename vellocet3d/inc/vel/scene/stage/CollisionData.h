#pragma once

#include <vector>
#include <optional>

#include "glm/glm.hpp"


namespace vel::scene::stage
{
	enum CollisionDataType
	{
		STATIC,					// floors, walls, ceilings and other non-removable world objects
		STATIC_REMOVABLE,		// any object in the collision world that has a static position, but can be removed
		DYNAMIC					// transformable objects with variable removability...use as few of these as possible 
								// (every vertex of the mesh has to be mulitiplied by a matrix EVERY FRAME)
	};

	struct CollisionData
	{
		std::vector<glm::vec3>	vertices;
		std::vector<size_t>		indices;
	};

}