#pragma once

#include <vector>

#include "glm/glm.hpp"

#include "Vertex.h"

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
										AABB(std::vector<glm::vec3>& inputVectors);
										AABB(const std::vector<Vertex>& inputVertices);
		
		const std::vector<glm::vec3>&	getCorners();
		glm::vec3						getFarthestCorner();
		glm::vec3						getSize();
		glm::vec3						getHalfExtents();
		glm::vec3						getMinEdge();
		glm::vec3						getMaxEdge();

	};
}
