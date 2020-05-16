#pragma once

#include <vector>

#include "glm/glm.hpp"

#include "vel/scene/stage/CollisionData.h"


namespace vel::collision
{
	class RayCaster
	{

	private:
		std::vector<vel::scene::stage::CollisionData*>	collisionData;
		glm::vec3					castOrigin;
		glm::vec3					castDirection;

		std::vector<glm::vec3>		points;
		glm::vec3					intersectionPoint;
		glm::vec3					normal;
		bool						foundIntersection;

		bool						rayIntersectsTriangle(glm::vec3 vertex0, glm::vec3 vertex1, glm::vec3 vertex2, glm::vec3& intersection_point);



	public:
									RayCaster();
		void						setCollisionData(std::vector<vel::scene::stage::CollisionData*> data);
		void						setCastOrigin(glm::vec3 origin);
		void						setCastDirection(glm::vec3 direction);
		void						execute();

		bool						getFoundIntersection();
		glm::vec3					getIntersectionPoint();
		glm::vec3					getNormal();
		std::vector<glm::vec3>		getPoints();



	};

}