#pragma once

#include <functional>
#include <optional>
#include <vector>
#include <map>
#include <string>

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include "BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.h"
#include "BulletCollision/NarrowPhaseCollision/btGjkPairDetector.h"
#include "BulletCollision/NarrowPhaseCollision/btPointCollector.h"


#include "vel/scene/stage/Actor.h"
#include "vel/scene/stage/Sensor.h"
#include "vel/scene/stage/RaycastResult.h"



namespace vel
{
	class Stage;

	class CollisionWorld
	{
	private:
		Stage*									stage;
		btDefaultCollisionConfiguration*		collisionConfiguration;
		btCollisionDispatcher*					dispatcher;
		btBroadphaseInterface*					overlappingPairCache;
		btSequentialImpulseConstraintSolver*	solver;
		btDiscreteDynamicsWorld*				dynamicsWorld;
		std::map<std::string, btCollisionShape*> collisionShapes;
		std::vector<std::unique_ptr<Sensor>>	sensors;
		

		void									removeSensorsUsingCollisionObject(btCollisionObject* co);
	
	public:
		static bool								contactAddedCallback(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1);
												
												CollisionWorld(Stage* stage, float gravity = -10);
												~CollisionWorld();
		btDiscreteDynamicsWorld* const			getDynamicsWorld();
		void									addCollisionShape(std::string name, btCollisionShape* shape);
		btRigidBody*							addStaticCollisionBody(std::string collisionObjectName, std::vector<Actor*> actors, std::optional<std::function<void(btRigidBody* body)>> callback = std::nullopt);
		void									addStaticCollisionBodies(std::vector<Actor*> actors, std::optional<std::function<void(btRigidBody* body)>> callback = std::nullopt);
		void									removeRigidBody(btRigidBody* rb);
		void									removeGhostObject(btPairCachingGhostObject* go);
		void									addSensor(Sensor* ct);
		void									processSensors();


		std::optional<RaycastResult>			rayTest(btVector3 from, btVector3 to, std::vector<btCollisionObject*> blackList = {});
		

	};


}