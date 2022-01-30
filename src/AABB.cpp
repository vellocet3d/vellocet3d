#include "vel/AABB.h"

namespace vel
{
	AABB::AABB(std::vector<glm::vec3> inputVectors) :
		firstPass(true)
	{
		// find min/max edge vectors
		for (auto& v : inputVectors)
		{
			if (this->firstPass)
			{
				this->firstPass = false;
				this->minEdge = v;
				this->maxEdge = v;
				continue;
			}

			if (v.x > maxEdge.x)
				maxEdge.x = v.x;
			if (v.y > maxEdge.y)
				maxEdge.y = v.y;
			if (v.z > maxEdge.z)
				maxEdge.z = v.z;

			if (v.x < minEdge.x)
				minEdge.x = v.x;
			if (v.y < minEdge.y)
				minEdge.y = v.y;
			if (v.z < minEdge.z)
				minEdge.z = v.z;
		}

		// calculate all eight corner vectors
		this->corners.push_back(maxEdge);
		this->corners.push_back(minEdge);
		this->corners.push_back(glm::vec3(minEdge.x, maxEdge.y, maxEdge.z));
		this->corners.push_back(glm::vec3(minEdge.x, minEdge.y, maxEdge.z));
		this->corners.push_back(glm::vec3(maxEdge.x, minEdge.y, maxEdge.z));
		this->corners.push_back(glm::vec3(maxEdge.x, maxEdge.y, minEdge.z));
		this->corners.push_back(glm::vec3(minEdge.x, maxEdge.y, minEdge.z));
		this->corners.push_back(glm::vec3(maxEdge.x, minEdge.y, minEdge.z));

	};

	const std::vector<glm::vec3>& AABB::getCorners()
	{
		return this->corners;
	}

	glm::vec3 AABB::getFarthestCorner()
	{
		float checkVal = 0.0f;
		glm::vec3 returnVector;

		for (auto& c : this->corners)
		{
			float cornerLength = glm::length(c);
			if (cornerLength > checkVal)
			{
				checkVal = cornerLength;
				returnVector = c;
			}
		}

		return returnVector;
	}
}