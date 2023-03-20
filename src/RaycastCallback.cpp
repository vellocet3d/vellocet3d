
#include "vel/RaycastCallback.h"

namespace vel
{
	RaycastCallback::RaycastCallback(btVector3 from, btVector3 to, std::vector<btCollisionObject*> blackList) :
		btCollisionWorld::ClosestRayResultCallback(from, to),
		blackList(blackList)
	{
		
	}

	btScalar RaycastCallback::addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
	{
		for (auto& co : this->blackList)
		{
			if (rayResult.m_collisionObject == co)
				return 1.0f;
		}

		if (rayResult.m_localShapeInfo == nullptr)
			this->m_triangleIndex = -1;
		else
			this->m_triangleIndex = rayResult.m_localShapeInfo->m_triangleIndex;

		return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
	}

}
