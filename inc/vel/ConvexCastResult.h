#pragma once


#include "glm/glm.hpp"
#include "btBulletCollisionCommon.h"



namespace vel
{
	struct ConvexCastResult
	{
		const btCollisionObject*	collisionObject;
		btVector3					hitpoint;
		btVector3					normal;
		float						normalUpDot;
	};
}