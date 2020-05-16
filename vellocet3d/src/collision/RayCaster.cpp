
#include "vel/collision/RayCaster.h"


namespace vel::collision
{
	RayCaster::RayCaster() : 
		foundIntersection(false)
	{};

	void RayCaster::execute()
	{
		this->points.clear();
		this->intersectionPoint = glm::vec3();
		this->normal = glm::vec3();
		this->foundIntersection = false;

		bool firstIntersection = true;
		glm::vec3 nearestIntersectionPoint;
		glm::vec3 nearestNormal;
		glm::vec3 p0, p1, p2, tmpIntersectionPoint, tmpNormal;

		for (auto cd : this->collisionData)
		{
			for (int triCounter = 0; triCounter < cd->indices.size() / 3; triCounter++) 
			{
				p0 = cd->vertices[cd->indices[3 * triCounter]];
				p1 = cd->vertices[cd->indices[3 * triCounter + 1]];
				p2 = cd->vertices[cd->indices[3 * triCounter + 2]];

				if (rayIntersectsTriangle(p0, p1, p2, tmpIntersectionPoint)) 
				{
					this->foundIntersection = true;

					tmpNormal = glm::normalize(glm::cross((p1 - p0), (p2 - p0)));

					if (firstIntersection) 
					{
						nearestIntersectionPoint = tmpIntersectionPoint;
						nearestNormal = tmpNormal;
						firstIntersection = false;
					}
					else
					{
						if (glm::distance(this->castOrigin, tmpIntersectionPoint) < glm::distance(this->castOrigin, nearestIntersectionPoint)) 
						{
							nearestIntersectionPoint = tmpIntersectionPoint;
							nearestNormal = tmpNormal;
						}
					}
				}
			}
		}

		if (this->foundIntersection) 
		{
			this->points = {p0, p1, p2};
			this->intersectionPoint = nearestIntersectionPoint;
			this->normal = nearestNormal;
		}
	}

	bool RayCaster::rayIntersectsTriangle(glm::vec3 vertex0, glm::vec3 vertex1, glm::vec3 vertex2, glm::vec3& intersection_point)
	{
		const float EPSILON = 0.0000001f;

		glm::vec3 edge1, edge2, h, s, q;

		float a, f, u, v;

		edge1 = vertex1 - vertex0;
		edge2 = vertex2 - vertex0;

		h = glm::cross(this->castDirection, edge2);
		a = glm::dot(edge1, h);
		if (a > -EPSILON && a < EPSILON) 
		{
			return false; // This ray is parallel to this triangle
		}

		f = 1.0 / a;
		s = this->castOrigin - vertex0;
		u = f * glm::dot(s, h);
		if (u < 0.0 || u > 1.0) 
		{
			return false;
		}

		q = glm::cross(s, edge1);
		v = f * glm::dot(this->castDirection, q);
		if (v < 0.0 || u + v > 1.0) 
		{
			return false;
		}

		// At this stage we can compute t to find out where the intersection point is on the line
		float t = f * glm::dot(edge2, q);
		if (t > EPSILON) // ray intersection
		{ 
			intersection_point = this->castOrigin + this->castDirection * t;
			return true;
		}
		else // there is a line intersection but not a ray intersection
		{ 
			return false;
		}
	}

	void RayCaster::setCollisionData(std::vector<vel::scene::stage::CollisionData*> data)
	{
		this->collisionData = data;
	}

	void RayCaster::setCastOrigin(glm::vec3 origin)
	{
		this->castOrigin = origin;
	}

	void RayCaster::setCastDirection(glm::vec3 direction)
	{
		this->castDirection = direction;
	}

	bool RayCaster::getFoundIntersection()
	{
		return this->foundIntersection;
	}

	glm::vec3 RayCaster::getIntersectionPoint()
	{
		return this->intersectionPoint;
	}

	glm::vec3 RayCaster::getNormal()
	{
		return this->normal;
	}

	std::vector<glm::vec3> RayCaster::getPoints()
	{
		return this->points;
	}

}