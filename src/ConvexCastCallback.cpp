
#include "vel/ConvexCastCallback.h"

namespace vel
{
	ConvexCastCallback::ConvexCastCallback(btVector3 from, btVector3 to, std::vector<btCollisionObject*> blackList) :
		btCollisionWorld::ClosestConvexResultCallback(from, to),
		blackList(blackList)
	{

	}

	btScalar ConvexCastCallback::addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace)
	{
		for (auto& co : this->blackList)
		{
			if (convexResult.m_hitCollisionObject == co)
				return 1.0f;
		}

		return ClosestConvexResultCallback::addSingleResult(convexResult, normalInWorldSpace);
	}

}