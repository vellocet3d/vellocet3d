
#include "vel/collision/EllipsoidCollider.h"
#include "vel/scene/stage/Actor.h"
#include "vel/App.h"

namespace vel::collision
{
	EllipsoidCollider::EllipsoidCollider(glm::vec3 ellipsoidSpace) :
		ellipsoidSpace(ellipsoidSpace),
		collisionVertices(std::vector<glm::vec3>()),
		collisionIndices(std::vector<size_t>())
	{};

	
	void EllipsoidCollider::addActorToCollisionSoup(vel::scene::stage::Actor& actor)
	{
		if (!actor.getMeshIndex())
		{
			return;
		}

		auto transformMatrix = actor.getWorldMatrix();
		auto mesh = &App::get().getScene()->getMesh(actor.getMeshIndex().value());

		size_t vertexOffset = this->collisionVertices.size();

		for (auto& vert : mesh->getVertices())
		{
			this->collisionVertices.push_back(glm::vec3(transformMatrix * glm::vec4(vert.position, 1.0f)));
		}

		for (auto& ind : mesh->getIndices())
		{
			this->collisionIndices.push_back(ind + vertexOffset);
		}

	}
}