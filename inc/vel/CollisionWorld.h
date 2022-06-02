#pragma once

#include <functional>
#include <optional>
#include <vector>
#include <unordered_map>
#include <string>

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include "BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.h"
#include "BulletCollision/NarrowPhaseCollision/btGjkPairDetector.h"
#include "BulletCollision/NarrowPhaseCollision/btPointCollector.h"

#include "sac.h"
#include "vel/Actor.h"
#include "vel/Sensor.h"
#include "vel/RaycastResult.h"
#include "vel/CollisionDebugDrawer.h"
#include "vel/CollisionObjectTemplate.h"
#include "vel/ConvexCastResult.h"


namespace vel
{
	class Stage;
	class Camera;

	class CollisionWorld
	{
	private:
		bool									isActive;
		btDefaultCollisionConfiguration*		collisionConfiguration;
		btCollisionDispatcher*					dispatcher;
		btBroadphaseInterface*					overlappingPairCache;
		btSequentialImpulseConstraintSolver*	solver;
		btDiscreteDynamicsWorld*				dynamicsWorld;
		std::unordered_map<std::string, btCollisionShape*> collisionShapes;
		sac<Sensor>								sensors;
		Camera*									camera;
		CollisionDebugDrawer* 					collisionDebugDrawer;
		std::unordered_map<std::string, CollisionObjectTemplate> collisionObjectTemplates;


		//void									removeSensorsUsingCollisionObject(btCollisionObject* co);

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
		std::optional<ConvexCastResult>			convexSweepTest(btConvexShape* castShape, btVector3 from, btVector3 to, std::vector<btCollisionObject*> blackList = {});

		void									useDebugDrawer(Shader* s, int debugMode = 1);
		CollisionDebugDrawer* 					getDebugDrawer();
		bool									getDebugEnabled();
        
        btCollisionShape*                       getCollisionShape(std::string name);
		void									addCollisionObjectTemplate(std::string name, CollisionObjectTemplate cot);
		CollisionObjectTemplate&				getCollisionObjectTemplate(std::string name);

		bool									getIsActive();
		void									setIsActive(bool b);

		void									setCamera(Camera* c);
		Camera*									getCamera();

	};


}