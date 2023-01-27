#include "vel/AABB.h"

namespace vel
{
	AABB::AABB(std::vector<glm::vec3>& inputVectors) :
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

	AABB::AABB(const std::vector<Vertex>& inputVertices) :
		firstPass(true)
	{
		// find min/max edge vectors
		for (auto& v : inputVertices)
		{
			if (this->firstPass)
			{
				this->firstPass = false;
				this->minEdge = v.position;
				this->maxEdge = v.position;
				continue;
			}

			if (v.position.x > maxEdge.x)
				maxEdge.x = v.position.x;
			if (v.position.y > maxEdge.y)
				maxEdge.y = v.position.y;
			if (v.position.z > maxEdge.z)
				maxEdge.z = v.position.z;

			if (v.position.x < minEdge.x)
				minEdge.x = v.position.x;
			if (v.position.y < minEdge.y)
				minEdge.y = v.position.y;
			if (v.position.z < minEdge.z)
				minEdge.z = v.position.z;
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

	glm::vec3 AABB::getSize()
	{
		return glm::vec3(
			fabsf(maxEdge.x - minEdge.x),
			fabsf(maxEdge.y - minEdge.y),
			fabsf(maxEdge.z - minEdge.z)
		);
	}

	glm::vec3 AABB::getHalfExtents()
	{
		return this->getSize() * 0.5f;
	}

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

	glm::vec3 AABB::getMinEdge()
	{
		return this->minEdge;
	}

	glm::vec3 AABB::getMaxEdge()
	{
		return this->maxEdge;
	}

}