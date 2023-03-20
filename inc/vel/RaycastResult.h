#pragma once


#include "glm/glm.hpp"
#include "btBulletCollisionCommon.h"



namespace vel
{
	struct RaycastResult 
	{
		const btCollisionObject*	collisionObject;
		btVector3					hitpoint;
		btVector3					normal;
		float						distance;
		float						normalUpDot;
		int							triangleIndex;
	};
}