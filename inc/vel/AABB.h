#pragma once

#include <vector>

#include "glm/glm.hpp"


namespace vel
{
	class AABB
	{
	private:
		bool							firstPass;
		glm::vec3						minEdge;
		glm::vec3						maxEdge;
		std::vector<glm::vec3>			corners;

	public:
										AABB(std::vector<glm::vec3> inputVectors);
		
		const std::vector<glm::vec3>&	getCorners();
		glm::vec3						getFarthestCorner();

	};
}
