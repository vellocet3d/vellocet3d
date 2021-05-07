#pragma once

#include "btBulletCollisionCommon.h"


namespace vel::scene::stage
{

	struct ConvexResult
	{
		btVector3 point;
		btVector3 separatingVector;
	};

}