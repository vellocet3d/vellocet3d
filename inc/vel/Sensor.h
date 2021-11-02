#pragma once

#include <utility>
#include <functional>
#include <string>

#include "btBulletCollisionCommon.h"

namespace vel
{
	struct Sensor
	{
		std::string																name;

		std::pair<btCollisionObject*, btCollisionObject*>						contactPair;

		std::function<void(btPersistentManifold* contactManifold,
			std::pair<btCollisionObject*, btCollisionObject*> contactPair)>		onContactDiscovered;

		std::vector<btCollisionObject*> 										blackList;


		Sensor(std::string name, std::function<void(btPersistentManifold* contactManifold, std::pair<btCollisionObject*, btCollisionObject*> contactPair)> onContactDiscovered,
			btCollisionObject* ob1, btCollisionObject* ob2 = nullptr, std::vector<btCollisionObject*> blackList = {});

		bool																	matchingManifold(const btCollisionObject* ob1, const btCollisionObject* ob2);





	};
}