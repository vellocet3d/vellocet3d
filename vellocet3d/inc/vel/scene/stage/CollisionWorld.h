#pragma once

#include <functional>
#include <optional>
#include <vector>

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include "vel/scene/stage/Actor.h"

namespace vel::scene::stage
{
	struct CollisionWorld
	{
		btDefaultCollisionConfiguration*		collisionConfiguration;
		btCollisionDispatcher*					dispatcher;
		btBroadphaseInterface*					overlappingPairCache;
		btSequentialImpulseConstraintSolver*	solver;
		btDiscreteDynamicsWorld*				dynamicsWorld;
		btAlignedObjectArray<btCollisionShape*> collisionShapes;
	

												CollisionWorld(float gravity = -10);
												~CollisionWorld();
		void									addStaticCollisionBody(std::vector<Actor*> actors, std::optional<std::function<void(btRigidBody* body)>> callback = std::nullopt);

	};


}