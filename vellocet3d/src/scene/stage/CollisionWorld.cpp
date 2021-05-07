
#include "BulletCollision/CollisionDispatch/btInternalEdgeUtility.h"
#include "glm/glm.hpp"

#include "vel/App.h"
#include "vel/scene/stage/CollisionWorld.h"
#include "vel/helpers/functions.h"
#include "vel/scene/stage/BroadphaseCallback.h"
#include "vel/scene/stage/RaycastCallback.h"



namespace vel::scene::stage
{
	CollisionWorld::CollisionWorld(Stage* stage, float gravity) :
		stage(stage),
		collisionConfiguration(new btDefaultCollisionConfiguration()),
		dispatcher(new btCollisionDispatcher(collisionConfiguration)),
		overlappingPairCache(new btDbvtBroadphase()),
		solver(new btSequentialImpulseConstraintSolver),
		dynamicsWorld(new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration)),
		sensors(std::vector<std::unique_ptr<Sensor>>())
	{
		btVector3 gravityVec(0.0f, gravity, 0.0f);
		this->dynamicsWorld->setGravity(gravityVec);
	}

	CollisionWorld::~CollisionWorld()
	{
		//remove the rigidbodies from the dynamics world and delete them
		for (int i = this->dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
		{
			btCollisionObject* obj = this->dynamicsWorld->getCollisionObjectArray()[i];
			btRigidBody* body = btRigidBody::upcast(obj);
			if (body && body->getMotionState())
			{
				delete body->getMotionState();
			}
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
		//for (int j = 0; j < this->collisionShapes.size(); j++)
		//{
		//	btCollisionShape* shape = this->collisionShapes[j];
		//	this->collisionShapes[j] = 0;
		//	delete shape;
		//}

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

	void CollisionWorld::processSensors()
	{
		for (int i = 0; i < this->dispatcher->getNumManifolds(); i++)
		{
			btPersistentManifold* contactManifold = this->dispatcher->getManifoldByIndexInternal(i);
			if (contactManifold->getNumContacts() > 0)
			{
				for (auto& sen : this->sensors)
				{
					if (!sen->matchingManifold(contactManifold->getBody0(), contactManifold->getBody1()))
						continue;

					sen->onContactDiscovered(contactManifold, sen->contactPair);
				}
			}
		}
		
	}

	void CollisionWorld::addSensor(Sensor* ct)
	{
		this->sensors.push_back(std::move(std::unique_ptr<Sensor>(ct)));
	}

	void CollisionWorld::removeSensorsUsingCollisionObject(btCollisionObject* co)
	{
		size_t index = 0;
		for (auto& s : this->sensors)
		{
			if (s->contactPair.first == co || s->contactPair.second == co)
			{
				this->sensors.erase(this->sensors.begin() + index);
			}

			index++;
		}
	}

	void CollisionWorld::removeGhostObject(btPairCachingGhostObject* go)
	{
		this->removeSensorsUsingCollisionObject(go);
		this->dynamicsWorld->removeCollisionObject(go);
		delete go;
	}

	void CollisionWorld::removeRigidBody(btRigidBody* rb)
	{
		this->removeSensorsUsingCollisionObject(rb);

		if (rb->getMotionState())
		{
			delete rb->getMotionState();
		}

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

	/*
		Create a single btBvhTriangleMeshShape and corresponding btRigidBody object from
		the vector of passed Actor pointers. Call callback at the end of the method passing
		the newly generated btRigidBody object allowing users to modify it as necessary (applying
		friction and whatnot)
	*/
	btRigidBody* CollisionWorld::addStaticCollisionBody(std::string collisionObjectName, std::vector<Actor*> actors, std::optional<std::function<void(btRigidBody* body)>> callback)
	{
		std::vector<glm::vec3> tmpVerts;
		std::vector<size_t> tmpInds;

		for (auto actor : actors)
		{
			if (!actor->getMeshIndex())
			{
				//return; //not sure why i was returning here...probably should have been continue
				continue;
			}

			auto transformMatrix = actor->getWorldMatrix();
			auto mesh = &this->stage->getParentScene()->getMesh(actor->getMeshIndex().value());

			size_t vertexOffset = tmpVerts.size();

			for (auto& vert : mesh->getVertices())
			{
				tmpVerts.push_back(glm::vec3(transformMatrix * glm::vec4(vert.position, 1.0f)));
			}

			for (auto& ind : mesh->getIndices())
			{
				tmpInds.push_back(ind + vertexOffset);
			}
		}

		btTriangleMesh* mergedTriangleMesh = new btTriangleMesh();
		btVector3 p0, p1, p2;
		for (int triCounter = 0; triCounter < tmpInds.size() / 3; triCounter++)
		{
			p0 = vel::helpers::functions::glmToBulletVec3(tmpVerts[tmpInds[3 * triCounter]]);
			p1 = vel::helpers::functions::glmToBulletVec3(tmpVerts[tmpInds[3 * triCounter + 1]]);
			p2 = vel::helpers::functions::glmToBulletVec3(tmpVerts[tmpInds[3 * triCounter + 2]]);

			mergedTriangleMesh->addTriangle(p0, p1, p2);
		}

		//btCollisionShape* staticCollisionShape = new btBvhTriangleMeshShape(mergedTriangleMesh, true);
		btBvhTriangleMeshShape* bvhShape = new btBvhTriangleMeshShape(mergedTriangleMesh, true);
		btCollisionShape* staticCollisionShape = bvhShape;
		this->collisionShapes[collisionObjectName] = staticCollisionShape;

		

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
		btGenerateInternalEdgeInfo(bvhShape, triangleInfoMap);

		/////////


		this->dynamicsWorld->addRigidBody(body);

		if (callback)
		{
			callback.value()(body);
		}

		return body;
	}

	std::optional<ConvexResult>	CollisionWorld::gjkIntersection(const btConvexShape *shape1, const btTransform &transform1,
		const btConvexShape *shape2, const btTransform &transform2)
	{
		btGjkPairDetector gjk(shape1, shape2, &this->voronoiSolver, &this->epaSolver);

		btDiscreteCollisionDetectorInterface::ClosestPointInput input;
		input.m_transformA = transform1;
		input.m_transformB = transform2;

		btPointCollector res;

		gjk.getClosestPointsNonVirtual(input, res, 0);

		if (res.m_hasResult && res.m_distance < 0)
		{
			ConvexResult r;

			r.point = res.m_pointInWorld;
			r.separatingVector = res.m_normalOnBInWorld.normalized() * -res.m_distance;

			return r;
		}

		return {};
	}

	std::vector<btRigidBody*> CollisionWorld::broadphase(const Aabb& aabb)
	{
		std::vector<btRigidBody*> r;

		BroadphaseCallback callback(r);
		this->dynamicsWorld->getBroadphase()->aabbTest(aabb.min, aabb.max, callback);

		return r;
	}

	std::optional<RaycastResult> CollisionWorld::rayTest(btVector3 from, btVector3 to, std::vector<btCollisionObject*> blackList)
	{
		RaycastCallback raycast = RaycastCallback(from, to, blackList);
		this->dynamicsWorld->rayTest(from, to, raycast);

		if (!raycast.hasHit() || !raycast.m_collisionObject)
			return {};

		RaycastResult r;
		r.collisionObject = raycast.m_collisionObject;
		r.hitpoint = raycast.m_hitPointWorld;
		r.normal = raycast.m_hitNormalWorld.normalized();
		r.distance = btVector3(from - r.hitpoint).length();

		return r;
	}

}