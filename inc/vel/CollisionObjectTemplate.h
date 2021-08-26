#pragma once

#include <string>
#include <optional>

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


namespace vel
{
	struct CollisionObjectTemplate
	{

		// ACTIVATION STATES
		// ACTIVE_TAG 1
		// ISLAND_SLEEPING 2
		// WANTS_DEACTIVATION 3
		// DISABLE_DEACTIVATION 4
		// DISABLE_SIMULATION 5
		// FIXED_BASE_MULTI_BODY 6
		
		std::string					type = "";
		std::string					name = "";
		btCollisionShape*			collisionShape = nullptr;
		std::optional<btScalar>		mass;
		std::optional<btScalar>		friction;
		std::optional<btScalar>		restitution;
		std::optional<btScalar>		linearDamping;
		std::optional<btVector3>	gravity;
		std::optional<btVector3>	angularFactor;
		std::optional<int>			activationState;
	};
}