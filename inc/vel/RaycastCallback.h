#pragma once

#include <vector>

#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"



namespace vel
{

	class RaycastCallback : public btCollisionWorld::ClosestRayResultCallback
	{
	private:
		std::vector<btCollisionObject*> blackList;

	public:
		int m_triangleIndex;
		RaycastCallback(btVector3 from, btVector3 to, std::vector<btCollisionObject*> blackList = {});
		btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace);
		
	};

}