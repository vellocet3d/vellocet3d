


#include "vel/scene/stage/ContactTrigger.h"


namespace vel::scene::stage
{

	ContactTrigger::ContactTrigger(btCollisionObject* ob1, btCollisionObject* ob2) :
		contactPair(std::pair<btCollisionObject*, btCollisionObject*>(ob1, ob2))
	{

	}

	bool ContactTrigger::matchingManifold(const btCollisionObject* ob1, const btCollisionObject* ob2)
	{
		// if the second value in contactPair is nullptr then we match the first value with ALL other btCollisionObjects
		return (ob1 == this->contactPair.first && this->contactPair.second == nullptr) ||
			(ob2 == this->contactPair.first && this->contactPair.second == nullptr) ||
			(ob1 == this->contactPair.first && ob2 == this->contactPair.second) ||
			(ob1 == this->contactPair.second && ob2 == this->contactPair.first);
	}

}