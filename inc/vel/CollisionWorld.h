#pragma once

#include <functional>
#include <optional>
#include <vector>
#include <map>
#include <string>

#include "plf_colony/plf_colony.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include "BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.h"
#include "BulletCollision/NarrowPhaseCollision/btGjkPairDetector.h"
#include "BulletCollision/NarrowPhaseCollision/btPointCollector.h"


#include "vel/Actor.h"
#include "vel/Sensor.h"
#include "vel/RaycastResult.h"
#include "vel/CollisionDebugDrawer.h"
#include "vel/CollisionObjectTemplate.h"



namespace vel
{
	class Stage;

	class CollisionWorld
	{
	private:
		btDefaultCollisionConfiguration*		collisionConfiguration;
		btCollisionDispatcher*					dispatcher;
		btBroadphaseInterface*					overlappingPairCache;
		btSequentialImpulseConstraintSolver*	solver;
		btDiscreteDynamicsWorld*				dynamicsWorld;
		std::map<std::string, btCollisionShape*> collisionShapes; // TODO: this can be a faster container like robin_hood map
		plf::colony<Sensor>						sensors;
		CollisionDebugDrawer* 					collisionDebugDrawer;
		std::map<std::string, CollisionObjectTemplate> collisionObjectTemplates;


		void									removeSensorsUsingCollisionObject(btCollisionObject* co);

	public:
		static bool								contactAddedCallback(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1);

		CollisionWorld(float gravity = -10);
		~CollisionWorld();
		btDiscreteDynamicsWorld* const			getDynamicsWorld();
		void									addCollisionShape(std::string name, btCollisionShape* shape);
		
		btRigidBody*							addStaticCollisionBody(Actor* actor);
		btCollisionShape* 						collisionShapeFromActor(Actor* actor);

		void									removeRigidBody(btRigidBody* rb);
		void									removeGhostObject(btPairCachingGhostObject* go);
		Sensor*									addSensor(Sensor s);
		void									removeSensor(Sensor* s);
		void									processSensors();


		std::optional<RaycastResult>			rayTest(btVector3 from, btVector3 to, std::vector<btCollisionObject*> blackList = {});

		void									useDebugDrawer(Shader* s, int debugMode = 1);
		CollisionDebugDrawer* 					getDebugDrawer();
        
        btCollisionShape*                       getCollisionShape(std::string name);
		void									addCollisionObjectTemplate(std::string name, CollisionObjectTemplate cot);
		CollisionObjectTemplate&				getCollisionObjectTemplate(std::string name);

	};


}