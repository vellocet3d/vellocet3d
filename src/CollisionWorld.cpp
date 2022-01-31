
#include "BulletCollision/CollisionDispatch/btInternalEdgeUtility.h"
#include "glm/glm.hpp"

#include "vel/App.h"
#include "vel/CollisionWorld.h"
#include "vel/functions.h"
#include "vel/RaycastCallback.h"
#include "vel/ConvexCastCallback.h"




namespace vel
{
	CollisionWorld::CollisionWorld(float gravity) :
		collisionConfiguration(new btDefaultCollisionConfiguration()),
		dispatcher(new btCollisionDispatcher(collisionConfiguration)),
		overlappingPairCache(new btDbvtBroadphase()),
		solver(new btSequentialImpulseConstraintSolver),
		dynamicsWorld(new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration)),
		collisionDebugDrawer(nullptr)
	{
		this->dynamicsWorld->getPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
		
		btVector3 gravityVec(0.0f, gravity, 0.0f);
		this->dynamicsWorld->setGravity(gravityVec);
	}

	CollisionWorld::~CollisionWorld()
	{
		// remove debug drawer
		if (this->collisionDebugDrawer)
			delete this->collisionDebugDrawer;

		//remove the rigidbodies from the dynamics world and delete them (TODO: 90% sure this handles ghostObjects as well)
		for (int i = this->dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
		{
			btCollisionObject* obj = this->dynamicsWorld->getCollisionObjectArray()[i];
			btRigidBody* body = btRigidBody::upcast(obj);

			if (body && body->getMotionState())
				delete body->getMotionState();

			this->dynamicsWorld->removeCollisionObject(obj);
			delete obj;
		}

		//delete collision shapes
		for (auto& cs : this->collisionShapes)
		{
			btCollisionShape* shape = cs.second;
			cs.second = 0;
			delete shape;
		}

		//delete dynamics world
		delete this->dynamicsWorld;

		//delete solver
		delete this->solver;

		//delete broadphase
		delete this->overlappingPairCache;

		//delete dispatcher
		delete this->dispatcher;

		delete this->collisionConfiguration;

		//next line is optional: it will be cleared by the destructor when the array goes out of scope
		this->collisionShapes.clear();
	}

	void CollisionWorld::addCollisionObjectTemplate(std::string name, CollisionObjectTemplate cot)
	{
		this->collisionObjectTemplates[name] = cot;
	}
	
	CollisionObjectTemplate& CollisionWorld::getCollisionObjectTemplate(std::string name)
	{
		return this->collisionObjectTemplates[name];
	}

	//TODO feels terrible to be looping through every sensor for every contact manifold, there might
	// be some way to hook into the collisions when they happen within the bullet api???? which in turn
	// would be substantially quicker, but will require a complete reworking of our current "Sensors" logic
	void CollisionWorld::processSensors()
	{
		for (int i = 0; i < this->dispatcher->getNumManifolds(); i++)
		{
			btPersistentManifold* contactManifold = this->dispatcher->getManifoldByIndexInternal(i);
			if (contactManifold->getNumContacts() > 0)
			{
				for (auto sen : this->sensors.getAll())
				{
					if (!sen->matchingManifold(contactManifold->getBody0(), contactManifold->getBody1()))
						continue;

					sen->onContactDiscovered(contactManifold, sen->contactPair);
				}
			}
		}

	}

	void CollisionWorld::useDebugDrawer(Shader* s, int debugMode)
	{
		this->collisionDebugDrawer = new CollisionDebugDrawer();
		this->collisionDebugDrawer->setDebugMode(debugMode);
		this->collisionDebugDrawer->setShaderProgram(s);

		this->dynamicsWorld->setDebugDrawer(this->collisionDebugDrawer);
	}

	CollisionDebugDrawer* CollisionWorld::getDebugDrawer() //TODO should this really return nullptr OOORrrrrrrr??????
	{
		return this->collisionDebugDrawer;
	}

	Sensor* CollisionWorld::addSensor(Sensor s)
	{
		return this->sensors.insert(s.name, s);
	}

	void CollisionWorld::removeSensor(Sensor* s)
	{
		this->sensors.erase(s);
	}

	void CollisionWorld::removeGhostObject(btPairCachingGhostObject* go)
	{
		this->dynamicsWorld->removeCollisionObject(go);
		delete go;
	}

	void CollisionWorld::removeRigidBody(btRigidBody* rb)
	{
		if (rb->getMotionState())
			delete rb->getMotionState();

		this->dynamicsWorld->removeCollisionObject(rb);

		delete rb;
	}

	bool CollisionWorld::contactAddedCallback(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1)
	{
		btAdjustInternalEdgeContacts(cp, colObj1Wrap, colObj0Wrap, partId1, index1);
		return true;
	}

	void CollisionWorld::addCollisionShape(std::string name, btCollisionShape* shape)
	{
		this->collisionShapes[name] = shape;
	}

	btDiscreteDynamicsWorld* const	CollisionWorld::getDynamicsWorld()
	{
        return this->dynamicsWorld;
	}
    
	btCollisionShape* CollisionWorld::getCollisionShape(std::string name)
	{
		return this->collisionShapes[name];
	}


	btCollisionShape* CollisionWorld::collisionShapeFromActor(Actor* actor)
	{
		if (actor->getMesh() == nullptr)
			return nullptr;

		std::vector<glm::vec3> tmpVerts;
		std::vector<size_t> tmpInds;

		auto transformMatrix = actor->getWorldMatrix();
		auto mesh = actor->getMesh();

		size_t vertexOffset = tmpVerts.size();

		for (auto& vert : mesh->getVertices())
			tmpVerts.push_back(glm::vec3(transformMatrix * glm::vec4(vert.position, 1.0f)));

		for (auto& ind : mesh->getIndices())
			tmpInds.push_back(ind + vertexOffset);

		btTriangleMesh* mergedTriangleMesh = new btTriangleMesh();
		btVector3 p0, p1, p2;
		for (int triCounter = 0; triCounter < tmpInds.size() / 3; triCounter++)
		{
			p0 = glmToBulletVec3(tmpVerts[tmpInds[3 * triCounter]]);
			p1 = glmToBulletVec3(tmpVerts[tmpInds[3 * triCounter + 1]]);
			p2 = glmToBulletVec3(tmpVerts[tmpInds[3 * triCounter + 2]]);

			mergedTriangleMesh->addTriangle(p0, p1, p2);
		}

		btBvhTriangleMeshShape* bvhShape = new btBvhTriangleMeshShape(mergedTriangleMesh, true);
		btCollisionShape* staticCollisionShape = bvhShape;
		this->collisionShapes[actor->getName() + "_shape"] = staticCollisionShape;
		
		return staticCollisionShape;
	}

	btRigidBody* CollisionWorld::addStaticCollisionBody(Actor* actor)
	{
		auto staticCollisionShape = this->collisionShapeFromActor(actor);
		
		btScalar mass(0.0);
		btVector3 localInertia(0, 0, 0);
		btDefaultMotionState* defaultMotionState = new btDefaultMotionState();
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, defaultMotionState, staticCollisionShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		///////// added below to handle jitter when objects sliding across faces
		// https://stackoverflow.com/questions/25605659/avoid-ground-collision-with-bullet/25725502#25725502
		gContactAddedCallback = &CollisionWorld::contactAddedCallback;
		body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
		btTriangleInfoMap* triangleInfoMap = new btTriangleInfoMap();
		//btGenerateInternalEdgeInfo(bvhShape, triangleInfoMap);
		//TODO: make sure this works
		btGenerateInternalEdgeInfo((btBvhTriangleMeshShape*)staticCollisionShape, triangleInfoMap);
		/////////

		body->setUserPointer(actor);
		actor->setRigidBody(body);

		this->dynamicsWorld->addRigidBody(body);

		return body;
	}

	std::optional<RaycastResult> CollisionWorld::rayTest(btVector3 from, btVector3 to, std::vector<btCollisionObject*> blackList)
	{
		RaycastCallback raycast = RaycastCallback(from, to, blackList);
		this->dynamicsWorld->rayTest(from, to, raycast);

		if (!raycast.hasHit() || !raycast.m_collisionObject)
			return {};

		//TODO: why not just return the RaycastCallback instead?
		RaycastResult r;
		r.collisionObject = raycast.m_collisionObject;
		r.hitpoint = raycast.m_hitPointWorld;
		r.normal = raycast.m_hitNormalWorld.normalized();
		r.distance = btVector3(from - r.hitpoint).length();

		return r;
	}

	std::optional<ConvexCastResult> CollisionWorld::convexSweepTest(btConvexShape* castShape, btVector3 from, btVector3 to, std::vector<btCollisionObject*> blackList)
	{
		btTransform convexFromWorld;
		convexFromWorld.setIdentity();
		convexFromWorld.setOrigin(from);

		btTransform convexToWorld;
		convexToWorld.setIdentity();
		convexToWorld.setOrigin(to);

		ConvexCastCallback convexCast(from, to, blackList);

		this->dynamicsWorld->convexSweepTest(castShape, convexFromWorld, convexToWorld, convexCast);

		ConvexCastResult ccr;
		ccr.collisionObject = convexCast.m_hitCollisionObject;
		ccr.hitpoint = convexCast.m_hitPointWorld;
		ccr.normal = convexCast.m_hitNormalWorld.normalized();
		ccr.distance = btVector3(from - ccr.hitpoint).length();

		return ccr;
	}

}