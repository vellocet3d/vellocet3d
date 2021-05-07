#pragma once

#include "btBulletCollisionCommon.h"


namespace vel::scene::stage
{
	class Aabb
	{
	public:
		Aabb();
		Aabb(const btVector3 &min, const btVector3 &max);

		bool intersects(const Aabb &a) const;

		btVector3 min;
		btVector3 max;



	};
}