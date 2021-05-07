#pragma once

#include <vector>

#include "btBulletCollisionCommon.h"

namespace vel::scene::stage
{
	class BroadphaseCallback : public btBroadphaseAabbCallback
	{
	public:
		std::vector<btRigidBody*>& result;

		BroadphaseCallback(std::vector<btRigidBody*>& result) : result(result) {}

		virtual bool process(const btBroadphaseProxy* proxy);
	};
}
