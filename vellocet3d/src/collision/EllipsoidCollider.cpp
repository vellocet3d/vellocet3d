
#include "vel/collision/EllipsoidCollider.h"
#include "vel/scene/stage/Actor.h"
#include "vel/App.h"
#include "vel/helpers.h"

namespace vel::collision
{
	EllipsoidCollider::EllipsoidCollider(glm::vec3 ellipsoidSpace, glm::vec3 gravity) :
		gravity(gravity),
		ellipsoidSpace(ellipsoidSpace),
		jumping(false),
		falling(false),
		activeResponseType(EllipsoidCollisionResponseType::SLIDE),
		slidingPlaneThreshold(10.0f),
		recursionDepth(0),
		foundCollision(false),
		nearestDistance(0.0f),
		veryCloseDistance(0.0001f)
	{};

	glm::vec3 EllipsoidCollider::getCorrectedPosition(glm::vec3 position, glm::vec3 velocity)
	{
		this->ePosition = position / this->ellipsoidSpace;
		this->eVelocity = velocity / this->ellipsoidSpace;

		this->activeResponseType = EllipsoidCollisionResponseType::SLIDE;
		this->recursionDepth = 0;
		glm::vec3 finalPosition = this->collideWithWorld();

		if (!this->jumping)
		{
			float yBeforeGravity = finalPosition.y;

			this->ePosition = finalPosition;
			this->eVelocity = this->gravity / this->ellipsoidSpace;
			this->recursionDepth = 0;
			this->activeResponseType = EllipsoidCollisionResponseType::SLIDE_WITH_THRESHOLD;

			finalPosition = this->collideWithWorld();

			if (!this->falling && !this->foundCollision)
			{
				this->falling = true;
			}

			if (this->falling)
			{
				if (essentiallyEqual(yBeforeGravity, finalPosition.y, 0.01))
				{
					this->falling = false;
				}
			}
		}

		this->foundCollision = false;
		this->intersectionPoint.reset();
		this->recursionDepth = 0;
		this->nearestDistance = 0.0f;

		return finalPosition * this->ellipsoidSpace;
	}

	glm::vec3 EllipsoidCollider::collideWithWorld()
	{
		if (this->recursionDepth > 5)
		{
			return this->ePosition;
		}

		this->eVelocityNormalized = glm::normalize(this->eVelocity);
		this->foundCollision = false;
		this->nearestDistance = 0.0f;

		for (auto& cd : this->collisionData)
		{
			glm::vec3 p0, p1, p2, triNormal;
			for (int triCounter = 0; triCounter < cd->indices.size() / 3; triCounter++)
			{
				p0 = cd->vertices[cd->indices[3 * triCounter]];
				p1 = cd->vertices[cd->indices[3 * triCounter + 1]];
				p2 = cd->vertices[cd->indices[3 * triCounter + 2]];

				p0 = p0 / this->ellipsoidSpace;
				p1 = p1 / this->ellipsoidSpace;
				p2 = p2 / this->ellipsoidSpace;

				triNormal = glm::normalize(glm::cross((p1 - p0), (p2 - p0)));

				sphereCollidingWithTriangle(p0, p1, p2, triNormal);
			}
		}
		
		if (!this->foundCollision)
		{
			return this->ePosition + this->eVelocity;
		}

		glm::vec3 destinationPoint = this->ePosition + this->eVelocity;
		glm::vec3 newPosition = this->ePosition;

		if (this->nearestDistance >= this->veryCloseDistance)
		{
			glm::vec3 V = glm::normalize(this->eVelocity);
			V = V * (this->nearestDistance - this->veryCloseDistance);
			newPosition = this->ePosition + V;

			V = glm::normalize(V);
			this->intersectionPoint.value() -= this->veryCloseDistance * V;
		}

		if (this->activeResponseType == EllipsoidCollisionResponseType::STOP)
		{
			return newPosition;
		}

		glm::vec3 slidePlaneOrigin = this->intersectionPoint.value();
		glm::vec3 slidePlaneNormal = glm::normalize((newPosition - this->intersectionPoint.value()));

		if (this->activeResponseType == EllipsoidCollisionResponseType::SLIDE_WITH_THRESHOLD)
		{
			float dAngle = glm::dot(this->eVelocityNormalized, slidePlaneNormal);
			float slidePlaneAngle = (dAngle + 1) * 90;

			if (slidePlaneAngle < this->slidingPlaneThreshold)
			{

			}

		}

	}

	bool EllipsoidCollider::sphereCollidingWithTriangle(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2, glm::vec3& tri_normal)
	{

	}

	void EllipsoidCollider::setSlidingPlaneThreshold(float threshold)
	{
		this->slidingPlaneThreshold = threshold;
	}

	void EllipsoidCollider::setJumping(bool jumping)
	{
		this->jumping = jumping;
	}

	void EllipsoidCollider::setFalling(bool falling)
	{
		this->falling = falling;
	}

	bool EllipsoidCollider::getFalling()
	{
		return this->falling;
	}

}