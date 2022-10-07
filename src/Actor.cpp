#include <iostream>

#include "vel/functions.h"
#include "vel/Actor.h"


namespace vel
{
	Actor::Actor(std::string name) :
		name(name),
		deleted(false),
		visible(true),
		dynamic(false),
		transform(Transform()),
		parentActor(nullptr),
		parentArmatureBone(nullptr),
		armature(nullptr),
		mesh(nullptr),
		collisionWorld(nullptr),
		rigidBody(nullptr),
		ghostObject(nullptr),
		autoTransform(true), // this is needed so that we don't update a static actor that has a rigidbody association
		color(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f))
	{}

	void Actor::setColor(glm::vec4 c)
	{
		this->color = c;
	}
	const glm::vec4& Actor::getColor()
	{
		return this->color;
	}

	void Actor::setCollisionWorld(CollisionWorld* cw)
	{
		this->collisionWorld = cw;
	}

	CollisionWorld* Actor::getCollisionWorld()
	{
		return this->collisionWorld;
	}

	void Actor::addContactSensor(Sensor* s)
	{
		this->contactSensors.push_back(s);
	}
	
	void Actor::clearContactSensors()
	{
		this->contactSensors.clear();
	}

	std::vector<Sensor*>& Actor::getContactSensors()
	{
		return this->contactSensors;
	}

	void Actor::processTransform()
	{
		this->updatePreviousTransform();

		// TODO: this can be avoided by deriving our own motionstate class from btMotionState, 
		// but will need to put thought into how that will affect our current interpolation process
		// for framerate independent logic
		// https://pybullet.org/Bullet/phpBB3/viewtopic.php?t=1205
		if (this->autoTransform && this->rigidBody != nullptr)
		{
			this->transform.setTranslation(bulletToGlmVec3(this->rigidBody->getWorldTransform().getOrigin()));
			this->transform.setRotation(bulletToGlmQuat(this->rigidBody->getWorldTransform().getRotation()));
		}
	}

	void Actor::clearPreviousTransform()
	{
		this->previousTransform.reset();
	}

	void Actor::removeChildActor(Actor* aIn, bool calledFromRemoveParentActor)
	{
		for (size_t i = 0; i < this->childActors.size(); i++)
		{
			if (this->childActors.at(i) == aIn)
			{
				if (!calledFromRemoveParentActor)
					aIn->removeParentActor(true);

				this->childActors.erase(this->childActors.begin() + i);
			}
		}
	}

	void Actor::removeParentActor(bool calledFromRemoveChildActor)
	{
		if (this->parentActor != nullptr)
		{
			if (!calledFromRemoveChildActor)
				this->parentActor->removeChildActor(this, true);

			this->parentActor = nullptr;
		}
	}

	void Actor::setDynamic(bool dynamic)
	{
		this->dynamic = dynamic;
	}

	void Actor::setGhostObject(btPairCachingGhostObject* go)
	{
		this->ghostObject = go;
	}

	btPairCachingGhostObject* Actor::getGhostObject()
	{
		return this->ghostObject;
	}

	Actor Actor::cleanCopy(std::string newName)
	{
		// TODO: this might need some more work

		auto newActor = *this;
		newActor.setName(newName);

		// Clear parents and children
		newActor.setParentActor(nullptr);
		newActor.setParentArmatureBone(nullptr);

		// Clear rigidbody pointer, ghost pointer, and transform flag
		newActor.setRigidBody(nullptr);
		newActor.setGhostObject(nullptr);
		newActor.setAutoTransform(true);
		newActor.setArmature(nullptr);
		newActor.clearContactSensors();

		// TODO: In the future we may need to implement methods for:
		// > automatically duplicating an entire actor hierarchy including all of it's children
		// > automatically copying the rigidbody component and adding to collision world

		return newActor;
	}

	void Actor::setName(std::string newName)
	{
		this->name = newName;
	}

	void Actor::addRenderable(Renderable r)
	{
		this->tempRenderable = r;
		this->mesh = r.getMesh();
	}

	Mesh* Actor::getMesh()
	{
		return this->mesh;
	}

	std::optional<Renderable>& Actor::getTempRenderable()
	{
		return this->tempRenderable;
	}

	void Actor::clearTempRenderable()
	{
		this->tempRenderable.reset();
	}

	glm::mat4 Actor::getWorldMatrix()
	{
		// if this actor has no parent, simply return the matrix of it's transform
		if (this->parentActor == nullptr && this->parentArmatureBone == nullptr)
			return this->transform.getMatrix();

		// if this actor is parented to another actor (parenting to actor will override bone parenting)
		if (this->parentArmatureBone == nullptr)
			return this->parentActor->getWorldMatrix() * this->transform.getMatrix();

		//return this->parentActor->getWorldMatrix() * this->parentArmatureBone->matrix * this->transform.getMatrix();
		return this->parentArmatureBone->matrix * this->transform.getMatrix();
	}

	glm::mat4 Actor::getWorldRenderMatrix(float alpha)
	{
		// actor is not dynamic (does not move) so interpolation is not required, simply return it's world matrix
		// TODO: but what if at some point it is parented to a dynamic actor? Well, the easiest solution would just be
		// to assume that only dynamic actors can ever be parented to other dynamic actors...which in a way makes sense
		// as really any actor that is not dynamic would be objects such as non-interactive map geometry. Moving forward
		// with this approach (at least for the time being)
		if (!this->isDynamic() || !this->previousTransform)
			return this->getWorldMatrix();

		auto actorMatrix = Transform::interpolateTransforms(this->previousTransform.value(), this->transform, alpha);

		// if this actor has no parent, simply return the matrix of it's transform
		if (this->parentActor == nullptr && this->parentArmatureBone == nullptr)
			return actorMatrix;

		// if this actor is parented to another actor (parenting to actor will override bone parenting)
		if (this->parentArmatureBone == nullptr)
			return this->parentActor->getWorldRenderMatrix(alpha) * actorMatrix;

		//auto boneMatrix = this->parentArmatureBone->getRenderMatrix(alpha);
		//return this->parentActor->getWorldRenderMatrix(alpha) * boneMatrix * actorMatrix;
		
		if (this->parentArmatureBone->parentArmature->getShouldInterpolate())
			return this->parentArmatureBone->getRenderMatrixInterpolated(alpha) * actorMatrix;
		
		return this->parentArmatureBone->getRenderMatrix() * actorMatrix;
	}

	glm::vec3 Actor::getInterpolatedTranslation(float alpha)
	{
		if (this->previousTransform) // insure we have a value for previousTransform from which to interpolate
			return Transform::interpolateTranslations(this->previousTransform.value(), this->transform, alpha);

		return this->transform.getTranslation();
	}

	glm::quat Actor::getInterpolatedRotation(float alpha)
	{
		if (this->previousTransform) // insure we have a value for previousTransform from which to interpolate
			return Transform::interpolateRotations(this->previousTransform.value(), this->transform, alpha);

		return this->transform.getRotation();
	}

	glm::vec3 Actor::getInterpolatedScale(float alpha)
	{
		if (this->previousTransform) // insure we have a value for previousTransform from which to interpolate
			return Transform::interpolateScales(this->previousTransform.value(), this->transform, alpha);

		return this->transform.getScale();
	}

	const bool Actor::isDynamic() const
	{
		return this->dynamic;
	}

	void Actor::updatePreviousTransform()
	{
		if (!this->isDeleted() && this->isDynamic())
			this->previousTransform = this->getTransform();
	}

	void Actor::setParentActor(Actor* a)
	{
		// set the parent relationship
		if(a != nullptr)
		{
			this->parentActor = a;
			a->addChildActor(this);
		}
		// remove the parent relationship
		else
		{
			this->removeParentActor();
		}		
	}

	void Actor::setParentArmatureBone(ArmatureBone* b)
	{
		// set the parent relationship
		if(b != nullptr)
		{
			this->parentArmatureBone = b;
			b->childActors.push_back(this);
		}
		// remove the parent relationship
		else if(this->parentArmatureBone != nullptr)
		{
			size_t i = 0;
			for(auto& a : this->parentArmatureBone->childActors)
			{
				if(a == this)
					this->parentArmatureBone->childActors.erase(this->parentArmatureBone->childActors.begin() + i);
				
				i++;
			}
			this->parentArmatureBone = nullptr;
		}
	}

	void Actor::addChildActor(Actor* a)
	{
		this->childActors.push_back(a);
	}

	void Actor::setActiveBones(std::vector<std::pair<size_t, unsigned int>> activeBones)
	{
		this->activeBones = activeBones;
	}

	const std::vector<std::pair<size_t, unsigned int>>& Actor::getActiveBones() const
	{
		return this->activeBones;
	}

	Transform& Actor::getTransform()
	{
		return this->transform;
	}

	std::optional<Transform>& Actor::getPreviousTransform()
	{
		return this->previousTransform;
	}

	const bool Actor::isDeleted() const
	{
		return this->deleted;
	}

	void Actor::setDeleted(bool d)
	{
		this->deleted = d;
	}

	void Actor::setRigidBody(btRigidBody* rb)
	{
		this->rigidBody = rb;
	}

	void Actor::setAutoTransform(bool mt)
	{
		this->autoTransform = mt;
	}

	btRigidBody* Actor::getRigidBody()
	{
		return this->rigidBody;
	}

	bool Actor::getAutoTransform()
	{
		return this->autoTransform;
	}

	const bool Actor::isVisible() const
	{
		return this->visible;
	}

	void Actor::setVisible(bool v)
	{
		this->visible = v;

		for (auto& ca : this->childActors)
			ca->setVisible(v);
	}

	const std::string Actor::getName() const
	{
		return this->name;
	}

	std::optional<Renderable*> Actor::getStageRenderable()
	{
		return this->stageRenderable;
	}

	void Actor::setStageRenderable(Renderable* r)
	{
		this->stageRenderable = r;
	}

	const bool Actor::isAnimated() const
	{
		if (this->armature)
			return true;

		return false;
	}

	void Actor::setArmature(Armature* arm)
	{
		this->armature = arm;
	}

	Armature* Actor::getArmature()
	{
		return this->armature;
	}

}