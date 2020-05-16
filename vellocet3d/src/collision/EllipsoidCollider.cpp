
#include "vel/collision/EllipsoidCollider.h"
#include "vel/scene/stage/Actor.h"
#include "vel/App.h"
#include "vel/helpers.h"

namespace vel::collision
{
	EllipsoidCollider::EllipsoidCollider(glm::vec3 ellipsoidSpace, glm::vec3 gravity) :
		rayCaster(RayCaster()),
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

		for (auto cd : this->collisionData)
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
				auto origin = newPosition * this->ellipsoidSpace;
				origin.y = origin.y - this->ellipsoidSpace.y;

				this->rayCaster.setCastOrigin(origin);
				this->rayCaster.setCastDirection(glm::vec3(0.0f, -1.0f, 0.0f));
				this->rayCaster.setCollisionData(this->collisionData);
				this->rayCaster.execute();

				float slideOffOffset = 0.1f;
				bool shouldSlide = origin.y - this->rayCaster.getIntersectionPoint().y > slideOffOffset ? true : false;

				if (!shouldSlide)
				{
					return newPosition;
				}
			}
		}

		float planeConstant = this->calculatePlaneConstant(slidePlaneOrigin, slidePlaneNormal);

		float distanceBetweenDestinationPointAndSlidingPlane = (glm::dot(destinationPoint, slidePlaneNormal) + planeConstant);

		glm::vec3 newDestinationPoint = destinationPoint - distanceBetweenDestinationPointAndSlidingPlane * slidePlaneNormal;

		glm::vec3 newVelocityVector = newDestinationPoint - this->intersectionPoint.value();

		if (glm::length(newVelocityVector) < veryCloseDistance)
		{
			return newPosition;
		}

		this->recursionDepth++;
		this->ePosition = newPosition;
		this->eVelocity = newVelocityVector;

		return this->collideWithWorld();
	}

	bool EllipsoidCollider::sphereCollidingWithTriangle(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2, glm::vec3& triNormal)
	{
		float facing = glm::dot(triNormal, this->eVelocityNormalized);

		if (facing <= 0)
		{
			glm::vec3 velocity = this->eVelocity;
			glm::vec3 position = this->ePosition;

			float t0, t1;
			bool sphereInPlane = false;
			
			float planeConstant = this->calculatePlaneConstant(p0, triNormal);
			
			float signedDistanceFromPositionToTriPlane = glm::dot(position, triNormal) + planeConstant;

			float planeNormalDotVelocity = glm::dot(triNormal, velocity);

			if (planeNormalDotVelocity == 0.0f) // parallel to plane?
			{
				if (fabs(signedDistanceFromPositionToTriPlane) >= 1.0f)
				{
					return false;
				}

				sphereInPlane = true;
			}
			else
			{
				t0 = (1.0f - signedDistanceFromPositionToTriPlane) / planeNormalDotVelocity;
				t1 = (-1.0f - signedDistanceFromPositionToTriPlane) / planeNormalDotVelocity;

				if (t0 > t1) 
				{
					float temp = t0;
					t0 = t1;
					t1 = temp;
				}

				if (t0 > 1.0f || t1 < 0.0f) 
				{
					return false;
				}

				if (t0 < 0.0)
				{
					t0 = 0.0;
				}

				if (t1 > 1.0)
				{
					t1 = 1.0;
				}
			}

			glm::vec3 collisionPoint;
			bool collidingWithTri = false;
			float t = 1.0f;

			if (!sphereInPlane)
			{
				glm::vec3 planeIntersectionPoint = (position + t0 * velocity - triNormal);

				if (checkPointInTriangle(planeIntersectionPoint, p0, p1, p2)) 
				{
					collidingWithTri = true;
					t = t0;
					collisionPoint = planeIntersectionPoint;
				}

			}

			if (!collidingWithTri)
			{
				float a, b, c;
				
				float velocityLengthSquared = glm::length(velocity);
				velocityLengthSquared *= velocityLengthSquared;

				a = velocityLengthSquared;

				float newT;

				// Collision test with sphere and p0
				b = 2.0f * glm::dot(velocity, position - p0);
				c = glm::length((p0 - position));
				c = (c*c) - 1.0f;

				if (getLowestRoot(a, b, c, t, &newT)) 
				{
					t = newT;
					collidingWithTri = true;
					collisionPoint = p0;
				}

				// Collision test with sphere and p1
				b = 2.0f * glm::dot(velocity, position - p1);
				c = glm::length((p1 - position));
				c = (c*c) - 1.0f;

				if (getLowestRoot(a, b, c, t, &newT)) 
				{
					t = newT;
					collidingWithTri = true;
					collisionPoint = p1;
				}

				// Collision test with sphere and p2
				b = 2.0f * glm::dot(velocity, position - p2);
				c = glm::length((p2 - position));
				c = (c*c) - 1.0f;
				if (getLowestRoot(a, b, c, t, &newT)) 
				{
					t = newT;
					collidingWithTri = true;
					collisionPoint = p2;
				}

				glm::vec3 edge, spherePositionToVertex;
				float edgeLengthSquared, edgeDotVelocity, edgeDotSpherePositionToVertex, spherePositionToVertexLengthSquared;

				// Edge (p0, p1)
				edge = p1 - p0;
				spherePositionToVertex = p0 - position;
				edgeLengthSquared = glm::length(edge);
				edgeLengthSquared *= edgeLengthSquared;
				edgeDotVelocity = glm::dot(edge, velocity);
				edgeDotSpherePositionToVertex = glm::dot(edge, spherePositionToVertex);
				spherePositionToVertexLengthSquared = glm::length(spherePositionToVertex);
				spherePositionToVertexLengthSquared *= spherePositionToVertexLengthSquared;

				a = edgeLengthSquared * -velocityLengthSquared + (edgeDotVelocity * edgeDotVelocity);
				b = edgeLengthSquared * (2.0f * glm::dot(velocity, spherePositionToVertex)) - (2.0f * edgeDotVelocity * edgeDotSpherePositionToVertex);
				c = edgeLengthSquared * (1.0f - spherePositionToVertexLengthSquared) + (edgeDotSpherePositionToVertex * edgeDotSpherePositionToVertex);

				if (getLowestRoot(a, b, c, t, &newT)) 
				{
					float f = (edgeDotVelocity * newT - edgeDotSpherePositionToVertex) / edgeLengthSquared;

					if (f >= 0.0f && f <= 1.0f) 
					{
						t = newT;
						collidingWithTri = true;
						collisionPoint = p0 + f * edge;
					}
				}

				// Edge (p1, p2)
				edge = p2 - p1;
				spherePositionToVertex = p1 - position;
				edgeLengthSquared = glm::length(edge);
				edgeLengthSquared *= edgeLengthSquared;
				edgeDotVelocity = glm::dot(edge, velocity);
				edgeDotSpherePositionToVertex = glm::dot(edge, spherePositionToVertex);
				spherePositionToVertexLengthSquared = glm::length(spherePositionToVertex);
				spherePositionToVertexLengthSquared *= spherePositionToVertexLengthSquared;

				a = edgeLengthSquared * -velocityLengthSquared + (edgeDotVelocity * edgeDotVelocity);
				b = edgeLengthSquared * (2.0f * glm::dot(velocity, spherePositionToVertex)) - (2.0f * edgeDotVelocity * edgeDotSpherePositionToVertex);
				c = edgeLengthSquared * (1.0f - spherePositionToVertexLengthSquared) + (edgeDotSpherePositionToVertex * edgeDotSpherePositionToVertex);

				if (getLowestRoot(a, b, c, t, &newT))
				{
					float f = (edgeDotVelocity * newT - edgeDotSpherePositionToVertex) / edgeLengthSquared;

					if (f >= 0.0f && f <= 1.0f)
					{
						t = newT;
						collidingWithTri = true;
						collisionPoint = p1 + f * edge;
					}
				}

				// Edge (p2, p0)
				edge = p0 - p2;
				spherePositionToVertex = p2 - position;
				edgeLengthSquared = glm::length(edge);
				edgeLengthSquared *= edgeLengthSquared;
				edgeDotVelocity = glm::dot(edge, velocity);
				edgeDotSpherePositionToVertex = glm::dot(edge, spherePositionToVertex);
				spherePositionToVertexLengthSquared = glm::length(spherePositionToVertex);
				spherePositionToVertexLengthSquared *= spherePositionToVertexLengthSquared;

				a = edgeLengthSquared * -velocityLengthSquared + (edgeDotVelocity * edgeDotVelocity);
				b = edgeLengthSquared * (2.0f * glm::dot(velocity, spherePositionToVertex)) - (2.0f * edgeDotVelocity * edgeDotSpherePositionToVertex);
				c = edgeLengthSquared * (1.0f - spherePositionToVertexLengthSquared) + (edgeDotSpherePositionToVertex * edgeDotSpherePositionToVertex);

				if (getLowestRoot(a, b, c, t, &newT))
				{
					float f = (edgeDotVelocity * newT - edgeDotSpherePositionToVertex) / edgeLengthSquared;

					if (f >= 0.0f && f <= 1.0f)
					{
						t = newT;
						collidingWithTri = true;
						collisionPoint = p2 + f * edge;
					}
				}
			}

			if (collidingWithTri)
			{
				float distToCollision = t * glm::length(velocity);

				if (!this->foundCollision || distToCollision < this->nearestDistance) 
				{
					this->nearestDistance = distToCollision;
					this->intersectionPoint = collisionPoint;

					this->foundCollision = true;
					return true;
				}
			}

			return false;
		}

		return false;
	}

	bool EllipsoidCollider::getLowestRoot(float a, float b, float c, float maxR, float* root) 
	{
		float determinant = b * b - 4.0f*a*c;

		if (determinant < 0.0f)
		{
			return false;
		}

		float sqrtD = sqrt(determinant);
		float r1 = (-b - sqrtD) / (2 * a);
		float r2 = (-b + sqrtD) / (2 * a);

		if (r1 > r2) 
		{
			float temp = r2;
			r2 = r1;
			r1 = temp;
		}

		if (r1 > 0 && r1 < maxR) 
		{
			*root = r1;
			return true;
		}

		if (r2 > 0 && r2 < maxR) 
		{
			*root = r2;
			return true;
		}

		return false;
	}

	bool EllipsoidCollider::checkPointInTriangle(const glm::vec3& point, const glm::vec3& triV1, const glm::vec3& triV2, const glm::vec3& triV3) 
	{
		glm::vec3 cp1 = glm::cross((triV3 - triV2), (point - triV2));
		glm::vec3 cp2 = glm::cross((triV3 - triV2), (triV1 - triV2));

		if (glm::dot(cp1, cp2) >= 0) 
		{
			cp1 = glm::cross((triV3 - triV1), (point - triV1));
			cp2 = glm::cross((triV3 - triV1), (triV2 - triV1));

			if (glm::dot(cp1, cp2) >= 0) 
			{
				cp1 = glm::cross((triV2 - triV1), (point - triV1));
				cp2 = glm::cross((triV2 - triV1), (triV3 - triV1));

				if (glm::dot(cp1, cp2) >= 0) 
				{

					return true;

				}
			}
		}

		return false;
	}

	float EllipsoidCollider::calculatePlaneConstant(glm::vec3 point, glm::vec3 normal)
	{
		return -((normal.x * point.x) + (normal.y * point.y) + (normal.z * point.z));
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