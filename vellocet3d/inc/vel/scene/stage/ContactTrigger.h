#pragma once

#include <utility>

#include "btBulletCollisionCommon.h"

namespace vel::scene::stage
{
	class ContactTrigger
	{
	public:
		ContactTrigger(btCollisionObject* ob1, btCollisionObject* ob2 = nullptr);
		
		std::pair<btCollisionObject*, btCollisionObject*> contactPair;

		bool			matchingManifold(const btCollisionObject* ob1, const btCollisionObject* ob2);

		virtual void	onContactDiscovered(btPersistentManifold* contactManifold) = 0;
		virtual void	forEachContactPoint(btManifoldPoint& contactPoint, int index) = 0;

	};
}