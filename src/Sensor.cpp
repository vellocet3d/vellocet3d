


#include "vel/Sensor.h"


namespace vel
{

	Sensor::Sensor(std::string name, std::function<void(btPersistentManifold* contactManifold, std::pair<btCollisionObject*, btCollisionObject*>	contactPair)> onContactDiscovered, 
		btCollisionObject* ob1, btCollisionObject* ob2, std::vector<btCollisionObject*> blackList) :
		name(name),
		onContactDiscovered(onContactDiscovered),
		contactPair(std::pair<btCollisionObject*, btCollisionObject*>(ob1, ob2)),
		blackList(blackList)
	{

	}

	bool Sensor::matchingManifold(const btCollisionObject* ob1, const btCollisionObject* ob2)
	{
		for (auto& blco : this->blackList)
			if (blco == ob1 || blco == ob2)
				return false;

		// if the second value in contactPair is nullptr then we match the first value with ALL other btCollisionObjects
		return (ob1 == this->contactPair.first && this->contactPair.second == nullptr) ||
			(ob2 == this->contactPair.first && this->contactPair.second == nullptr) ||
			(ob1 == this->contactPair.first && ob2 == this->contactPair.second) ||
			(ob1 == this->contactPair.second && ob2 == this->contactPair.first);
	}

}