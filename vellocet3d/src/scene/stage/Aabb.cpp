
#include "vel/scene/stage/Aabb.h"

namespace vel::scene::stage
{
	Aabb::Aabb() {};
	Aabb::Aabb(const btVector3 &min, const btVector3 &max) :
		min(min),
		max(max)
	{

	}

	bool Aabb::intersects(const Aabb &a) const
	{
		return !(max.getX() < a.min.getX() || min.getX() > a.max.getX() ||
			max.getY() < a.min.getY() || min.getY() > a.max.getY() || 
			max.getZ() < a.min.getZ() || min.getZ() > a.max.getZ());
	}
}
