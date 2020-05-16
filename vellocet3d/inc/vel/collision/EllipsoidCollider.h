#pragma once

#include <vector>
#include <optional>

#include "glm/glm.hpp"

#include "vel/scene/stage/CollisionData.h"


namespace vel::collision
{
	enum EllipsoidCollisionResponseType 
	{
		SLIDE,					// Velocity will be projected and the object will move along a sliding plane
		SLIDE_WITH_THRESHOLD,	// Velocity will be projected along sliding plane only if it is within a specified threshold
		STOP					// Object will be moved as close as possible to the collision point
	};

	class EllipsoidCollider
	{

	private:
		glm::vec3							gravity;
		glm::vec3							ellipsoidSpace;
		std::vector<CollisionData*>			collisionData;
		bool								jumping;
		bool								falling;
		EllipsoidCollisionResponseType		activeResponseType;
		float								slidingPlaneThreshold;
		glm::vec3							position;
		glm::vec3							velocity;
		glm::vec3							ePosition;
		glm::vec3							eVelocity;
		glm::vec3							eVelocityNormalized;
		size_t								recursionDepth;
		bool								foundCollision;
		float								nearestDistance;
		float								veryCloseDistance;
		std::optional<glm::vec3>			intersectionPoint;

		glm::vec3							collideWithWorld();
		bool								sphereCollidingWithTriangle(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2, glm::vec3& tri_normal);
		


	public:
											EllipsoidCollider(glm::vec3 ellipsoidSpace, glm::vec3 gravity);
		void								setSlidingPlaneThreshold(float threshold);
		void								setJumping(bool jumping);
		void								setFalling(bool falling);
		bool								getFalling();

		glm::vec3							getCorrectedPosition(glm::vec3 position, glm::vec3 velocity);
		

	};

}