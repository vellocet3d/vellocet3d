
#include "BulletCollision/CollisionDispatch/btInternalEdgeUtility.h"
#include "glm/glm.hpp"

#include "vel/App.h"
#include "vel/CollisionWorld.h"
#include "vel/functions.h"
#include "vel/RaycastCallback.h"
#include "vel/ConvexCastCallback.h"
#include "vel/CustomTriangleMesh.h"



namespace vel
{
	CollisionWorld::CollisionWorld(float gravity) :
		isActive(true),
		collisionConfiguration(new btDefaultCollisionConfiguration()),
		dispatcher(new btCollisionDispatcher(collisionConfiguration)),
		overlappingPairCache(new btDbvtBroadphase()),
		solver(new btSequentialImpulseConstraintSolver),
		dynamicsWorld(new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration)),
		camera(nullptr),
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

	void CollisionWorld::setCamera(Camera* c)
	{
		this->camera = c;
	}

	Camera* CollisionWorld::getCamera()
	{
		return this->camera;
	}

	bool CollisionWorld::getIsActive()
	{
		return this->isActive;
	}

	void CollisionWorld::setIsActive(bool b)
	{
		this->isActive = b;
	}

	void CollisionWorld::addCollisionObjectTemplate(std::string name, CollisionObjectTemplate cot)
	{
		this->collisionObjectTemplates[name] = cot;
	}
	
	CollisionObjectTemplate& CollisionWorld::getCollisionObjectTemplate(std::string name)
	{
		return this->collisionObjectTemplates[name];
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

	bool CollisionWorld::getDebugEnabled()
	{
		return this->collisionDebugDrawer != nullptr;
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


	//btCollisionShape* CollisionWorld::collisionShapeFromActor(Actor* actor)
	//{
	//	if (actor->getMesh() == nullptr)
	//		return nullptr;

	//	std::vector<glm::vec3> tmpVerts;
	//	std::vector<size_t> tmpInds;

	//	auto transformMatrix = actor->getWorldMatrix();
	//	auto mesh = actor->getMesh();

	//	size_t vertexOffset = tmpVerts.size();

	//	for (auto& vert : mesh->getVertices())
	//		tmpVerts.push_back(glm::vec3(transformMatrix * glm::vec4(vert.position, 1.0f)));

	//	for (auto& ind : mesh->getIndices())
	//		tmpInds.push_back(ind + vertexOffset);

	//	btTriangleMesh* mergedTriangleMesh = new btTriangleMesh();
	//	btVector3 p0, p1, p2;
	//	for (int triCounter = 0; triCounter < tmpInds.size() / 3; triCounter++)
	//	{
	//		p0 = glmToBulletVec3(tmpVerts[tmpInds[3 * triCounter]]);
	//		p1 = glmToBulletVec3(tmpVerts[tmpInds[3 * triCounter + 1]]);
	//		p2 = glmToBulletVec3(tmpVerts[tmpInds[3 * triCounter + 2]]);

	//		mergedTriangleMesh->addTriangle(p0, p1, p2);
	//	}

	//	btBvhTriangleMeshShape* bvhShape = new btBvhTriangleMeshShape(mergedTriangleMesh, true);
	//	bvhShape->setMargin(0);
	//	btCollisionShape* staticCollisionShape = bvhShape;
	//	this->collisionShapes[actor->getName() + "_shape"] = staticCollisionShape;
	//	
	//	return staticCollisionShape;
	//}

	btCollisionShape* CollisionWorld::collisionShapeFromActor(Actor* actor)
	{
		if (actor->getMesh() == nullptr)
			return nullptr;

		std::vector<glm::vec3> tmpVerts;
		std::vector<size_t> tmpInds;
		std::vector<glm::vec2> tmpTextureCoords;
		std::vector<glm::vec2> tmpLightmapCoords;

		auto transformMatrix = actor->getWorldMatrix();
		auto mesh = actor->getMesh();

		size_t vertexOffset = tmpVerts.size();

		for (auto& vert : mesh->getVertices())
		{
			tmpVerts.push_back(glm::vec3(transformMatrix * glm::vec4(vert.position, 1.0f)));
			tmpTextureCoords.push_back(vert.textureCoordinates);
			tmpLightmapCoords.push_back(vert.lightmapCoordinates);
		}
		
		for (auto& ind : mesh->getIndices())
			tmpInds.push_back(ind + vertexOffset);

		CustomTriangleMesh* mergedTriangleMesh = new CustomTriangleMesh();
		btVector3 p0, p1, p2;
		for (int triCounter = 0; triCounter < tmpInds.size() / 3; triCounter++)
		{
			p0 = glmToBulletVec3(tmpVerts[tmpInds[3 * triCounter]]);
			CustomTriangleMeshData p0CD;
			p0CD.textureUVs = tmpTextureCoords[tmpInds[3 * triCounter]];
			p0CD.lightmapUVs = tmpLightmapCoords[tmpInds[3 * triCounter]];
			p0CD.texture = nullptr;
			p0CD.lightmapTexture = actor->getLightMapTexture();

			p1 = glmToBulletVec3(tmpVerts[tmpInds[3 * triCounter + 1]]);
			CustomTriangleMeshData p1CD;
			p1CD.textureUVs = tmpTextureCoords[tmpInds[3 * triCounter + 1]];
			p1CD.lightmapUVs = tmpLightmapCoords[tmpInds[3 * triCounter + 1]];
			p1CD.texture = nullptr;
			p1CD.lightmapTexture = actor->getLightMapTexture();

			p2 = glmToBulletVec3(tmpVerts[tmpInds[3 * triCounter + 2]]);
			CustomTriangleMeshData p2CD;
			p2CD.textureUVs = tmpTextureCoords[tmpInds[3 * triCounter + 2]];
			p2CD.lightmapUVs = tmpLightmapCoords[tmpInds[3 * triCounter + 2]];
			p2CD.texture = nullptr;
			p2CD.lightmapTexture = actor->getLightMapTexture();

			if (actor->getStageRenderable() && actor->getStageRenderable().value()->getMaterial() && 
				actor->getStageRenderable().value()->getMaterial().value().getTextures().size() > 0)
			{
				p0CD.texture = actor->getStageRenderable().value()->getMaterial().value().getTextures().at(0);
				p1CD.texture = actor->getStageRenderable().value()->getMaterial().value().getTextures().at(0);
				p2CD.texture = actor->getStageRenderable().value()->getMaterial().value().getTextures().at(0);
			}

			mergedTriangleMesh->addTriangle(p0, p1, p2, p0CD, p1CD, p2CD);
		}

		btBvhTriangleMeshShape* bvhShape = new btBvhTriangleMeshShape(mergedTriangleMesh, true);
		bvhShape->setMargin(0);
		btCollisionShape* staticCollisionShape = bvhShape;
		this->collisionShapes[actor->getName() + "_shape"] = staticCollisionShape;

		return staticCollisionShape;
	}

	btRigidBody* CollisionWorld::addStaticCollisionBody(Actor* actor, int collisionFilterGroup, int collisionFilterMask)
	{
		auto staticCollisionShape = this->collisionShapeFromActor(actor);
		
		btScalar mass(0);
		btVector3 localInertia(0, 0, 0);
		btDefaultMotionState* defaultMotionState = new btDefaultMotionState();
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, defaultMotionState, staticCollisionShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		///////// added below to handle jitter when objects sliding across faces
		// https://stackoverflow.com/questions/25605659/avoid-ground-collision-with-bullet/25725502#25725502
		gContactAddedCallback = &CollisionWorld::contactAddedCallback;
		body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
		btTriangleInfoMap* triangleInfoMap = new btTriangleInfoMap();
		btGenerateInternalEdgeInfo((btBvhTriangleMeshShape*)staticCollisionShape, triangleInfoMap);

		this->dynamicsWorld->addRigidBody(body, collisionFilterGroup, collisionFilterMask);

		return body;
	}

	std::optional<RaycastResult> CollisionWorld::rayTest(btVector3 from, btVector3 to, int collisionFilterMask, std::vector<btCollisionObject*> blackList)
	{
		RaycastCallback raycast = RaycastCallback(from, to, blackList);
		raycast.m_collisionFilterGroup = 1;
		raycast.m_collisionFilterMask = collisionFilterMask;
		this->dynamicsWorld->rayTest(from, to, raycast);

		if (!raycast.hasHit() || !raycast.m_collisionObject)
			return {};

		RaycastResult r;
		r.collisionObject = raycast.m_collisionObject;
		r.hitpoint = raycast.m_hitPointWorld;
		r.normal = raycast.m_hitNormalWorld.normalized();
		r.distance = btVector3(from - r.hitpoint).length();
		r.normalUpDot = r.normal.dot(btVector3(0, 1, 0));
		r.triangleIndex = raycast.m_triangleIndex;

		return r;
	}

	std::optional<ConvexCastResult> CollisionWorld::convexSweepTest(btConvexShape* castShape, btVector3 from, btVector3 to, int collisionFilterMask, std::vector<btCollisionObject*> blackList)
	{
		btTransform convexFromWorld;
		convexFromWorld.setIdentity();
		convexFromWorld.setOrigin(from);

		btTransform convexToWorld;
		convexToWorld.setIdentity();
		convexToWorld.setOrigin(to);

		ConvexCastCallback convexCast(from, to, blackList);
		convexCast.m_collisionFilterGroup = 1;
		convexCast.m_collisionFilterMask = collisionFilterMask;

		this->dynamicsWorld->convexSweepTest(castShape, convexFromWorld, convexToWorld, convexCast);

		if (!convexCast.hasHit() || !convexCast.m_hitCollisionObject)
			return {};

		ConvexCastResult ccr;
		ccr.collisionObject = convexCast.m_hitCollisionObject;
		ccr.hitpoint = convexCast.m_hitPointWorld;
		ccr.normal = convexCast.m_hitNormalWorld.normalized();
		ccr.normalUpDot = ccr.normal.dot(btVector3(0, 1, 0));
		
		return ccr;
	}

	void CollisionWorld::getTriangleVertices(const btStridingMeshInterface* meshInterface, int triangleIndex, btVector3& v0, btVector3& v1, btVector3& v2, int& index0, int& index1, int& index2) {
		int numTrianglesTotal = 0;

		for (int i = 0; i < meshInterface->getNumSubParts(); ++i) {
			const unsigned char* vertexBase;
			int numVerts;
			PHY_ScalarType vertexType;
			int vertexStride;
			const unsigned char* indexBase;
			int indexStride;
			int numFaces;
			PHY_ScalarType indexType;

			meshInterface->getLockedReadOnlyVertexIndexBase(
				&vertexBase, numVerts, vertexType, vertexStride,
				&indexBase, indexStride, numFaces, indexType, i
			);

			numTrianglesTotal += numFaces;
		}

		if (triangleIndex < 0 || triangleIndex >= numTrianglesTotal) {
			std::cerr << "Invalid triangle index." << std::endl;
			return;
		}

		int currentTriangle = 0;

		for (int part = 0; part < meshInterface->getNumSubParts(); ++part) {
			const unsigned char* vertexBase;
			int numVerts;
			PHY_ScalarType vertexType;
			int vertexStride;
			const unsigned char* indexBase;
			int indexStride;
			int numFaces;
			PHY_ScalarType indexType;

			meshInterface->getLockedReadOnlyVertexIndexBase(
				&vertexBase, numVerts, vertexType, vertexStride,
				&indexBase, indexStride, numFaces, indexType, part
			);

			for (int face = 0; face < numFaces; ++face) {
				if (currentTriangle == triangleIndex) {
					int* triangleIndices = reinterpret_cast<int*>(const_cast<unsigned char*>(indexBase) + face * indexStride);

					index0 = triangleIndices[0];
					index1 = triangleIndices[1];
					index2 = triangleIndices[2];

					btScalar* vertex0 = reinterpret_cast<btScalar*>(const_cast<unsigned char*>(vertexBase) + index0 * vertexStride);
					btScalar* vertex1 = reinterpret_cast<btScalar*>(const_cast<unsigned char*>(vertexBase) + index1 * vertexStride);
					btScalar* vertex2 = reinterpret_cast<btScalar*>(const_cast<unsigned char*>(vertexBase) + index2 * vertexStride);

					v0.setValue(vertex0[0], vertex0[1], vertex0[2]);
					v1.setValue(vertex1[0], vertex1[1], vertex1[2]);
					v2.setValue(vertex2[0], vertex2[1], vertex2[2]);

					break;
				}

				currentTriangle++;
			}

			meshInterface->unLockReadOnlyVertexBase(part);

			if (currentTriangle > triangleIndex) {
				break;
			}
		}
	}

}