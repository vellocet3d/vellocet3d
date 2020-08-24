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
		static bool								contactAddedCallback(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1);

	};


}