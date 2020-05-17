#pragma once

#include <vector>
#include <optional>

#include "glm/glm.hpp"

#include "vel/collision/RayCaster.h"
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
		RayCaster							rayCaster;
		glm::vec3							gravity;
		glm::vec3							ellipsoidSpace;
		std::vector<vel::scene::stage::CollisionData*>	collisionData;
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
		float								calculatePlaneConstant(glm::vec3 point, glm::vec3 normal);
		bool								checkPointInTriangle(const glm::vec3& point, const glm::vec3& triV1, const glm::vec3& triV2, const glm::vec3& triV3);
		bool								getLowestRoot(float a, float b, float c, float maxR, float* root);
		bool								sphereCollidingWithTriangle(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2, glm::vec3& tri_normal);
		


	public:
											EllipsoidCollider(glm::vec3 ellipsoidSpace);
		void								setGravity(glm::vec3 gravity);
		void								setCollisionData(std::vector<vel::scene::stage::CollisionData*> collisionData);
		void								setSlidingPlaneThreshold(float threshold);
		void								setJumping(bool jumping);
		void								setFalling(bool falling);
		bool								getFalling();

		glm::vec3							getCorrectedPosition(glm::vec3 position, glm::vec3 velocity);
		

	};

}