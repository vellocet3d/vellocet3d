

#include "vel/scene/stage/BroadphaseCallback.h"
#include "btBulletDynamicsCommon.h"

namespace vel::scene::stage
{
	bool BroadphaseCallback::process(const btBroadphaseProxy* proxy)
	{
		btCollisionObject* o = static_cast<btCollisionObject*>(proxy->m_clientObject);

		if (btRigidBody* b = btRigidBody::upcast(o))
		{
			result.push_back(b);
			return true;
		}

		return false;
	}
}

