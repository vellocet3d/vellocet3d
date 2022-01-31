#pragma once

#include <vector>

#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"



namespace vel
{

	class ConvexCastCallback : public btCollisionWorld::ClosestConvexResultCallback
	{
	private:
		std::vector<btCollisionObject*> blackList;

	public:
					ConvexCastCallback(btVector3 from, btVector3 to, std::vector<btCollisionObject*> blackList = {});
		btScalar	addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace);
	};

}