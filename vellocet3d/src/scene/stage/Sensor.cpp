


#include "vel/scene/stage/Sensor.h"


namespace vel::scene::stage
{

	Sensor::Sensor(std::function<void(btPersistentManifold* contactManifold, std::pair<btCollisionObject*, btCollisionObject*>	contactPair)> onContactDiscovered, 
		btCollisionObject* ob1, btCollisionObject* ob2) :
		onContactDiscovered(onContactDiscovered),
		contactPair(std::pair<btCollisionObject*, btCollisionObject*>(ob1, ob2))
	{

	}

	bool Sensor::matchingManifold(const btCollisionObject* ob1, const btCollisionObject* ob2)
	{
		// if the second value in contactPair is nullptr then we match the first value with ALL other btCollisionObjects
		return (ob1 == this->contactPair.first && this->contactPair.second == nullptr) ||
			(ob2 == this->contactPair.first && this->contactPair.second == nullptr) ||
			(ob1 == this->contactPair.first && ob2 == this->contactPair.second) ||
			(ob1 == this->contactPair.second && ob2 == this->contactPair.first);
	}

}