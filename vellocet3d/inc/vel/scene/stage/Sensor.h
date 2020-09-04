#pragma once

#include <utility>
#include <functional>

#include "btBulletCollisionCommon.h"

namespace vel::scene::stage
{
	struct Sensor
	{
		Sensor(std::function<void(btPersistentManifold* contactManifold, std::pair<btCollisionObject*, btCollisionObject*>	contactPair)> onContactDiscovered, 
			btCollisionObject* ob1, btCollisionObject* ob2 = nullptr);

		std::pair<btCollisionObject*, btCollisionObject*>			contactPair;
		bool														matchingManifold(const btCollisionObject* ob1, const btCollisionObject* ob2);
		std::function<void(btPersistentManifold* contactManifold, 
			std::pair<btCollisionObject*, btCollisionObject*>	contactPair)>	onContactDiscovered;

		

	};
}