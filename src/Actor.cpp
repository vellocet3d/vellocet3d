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
		parentActorBone(nullptr),
		rigidBody(nullptr),
		armature(nullptr),
		mesh(nullptr),
		autoTransform(true) //TODO: tf is the point of this? Well, ok, so this is needed so that we don't update a static actor that has a rigidbody association
	{}

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
		//TODO revise this to use a sac?
		for (size_t i = 0; i < this->childActors.size(); i++)
		{
			if (this->childActors.at(i) == aIn)
			{
				if (!calledFromRemoveParentActor)
					aIn->removeParentActor(true);

				this->childActors.erase(this->childActors.begin() + i); //TODO this will shift memory, possibly revise in future
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
		// TODO: revise this to account for all changes that have happened in MassiveRefactor

		auto newActor = *this;
		newActor.setName(newName);

		// Clear parents and children
		newActor.parentActor = nullptr;
		newActor.parentActorBone = nullptr;
		newActor.childActors.clear();

		// Clear rigidbody pointer, ghost pointer, and transform flag
		newActor.setRigidBody(nullptr);
		newActor.setGhostObject(nullptr);
		newActor.setAutoTransform(true);

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
		if (this->parentActor == nullptr)
			return this->transform.getMatrix();

		// if this actor is parented to another actor (and not a bone of that actor)
		if (this->parentActorBone == nullptr)
			return this->parentActor->getWorldMatrix() * this->transform.getMatrix();

		// TODO: this might be broken now?
		return this->parentActor->getWorldMatrix() * this->parentActorBone->matrix * this->transform.getMatrix();
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
		if (this->parentActor == nullptr)
			return actorMatrix;

		// if this actor is parented to another actor (and not a bone of that actor)
		if (this->parentActorBone == nullptr)
			return this->parentActor->getWorldRenderMatrix(alpha) * actorMatrix;

		// TODO: this might be broken now?
		auto boneMatrix = this->parentActorBone->getRenderMatrix(alpha);
		return this->parentActor->getWorldRenderMatrix(alpha) * boneMatrix * actorMatrix;
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
		this->parentActor = a;
		a->addChildActor(this);
	}

	void Actor::setParentActorBone(ArmatureBone* b)
	{
		this->parentActorBone = b;
	}

	void Actor::addChildActor(Actor* a)
	{
		this->childActors.push_back(a);
	}

	void Actor::setActiveBones(std::vector<std::pair<size_t, std::string>> activeBones)
	{
		this->activeBones = activeBones;
	}

	const std::vector<std::pair<size_t, std::string>>& Actor::getActiveBones() const
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