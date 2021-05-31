#include <iostream>

#include "vel/helpers/functions.h"
#include "vel/scene/stage/Actor.h"


namespace vel
{
    Actor::Actor(std::string name) :
        name(name),
        deleted(false),
		visible(true),
		dynamic(false),
        transform(Transform()) ,
		rigidBody(nullptr),
		armature(nullptr),
		manualTransform(true) //TODO: tf is the point of this?
		//parentStage(parentStage),

    {}

	void Actor::processTransform()
	{
		this->updatePreviousTransform();
		
		if (!this->manualTransform && this->rigidBody != nullptr)
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

				this->childActors.erase(this->childActors.begin() + i); //TODO this will shift memory, possibly revise in future
			}
		}	
	}

	void Actor::removeParentActor(bool calledFromRemoveChildActor)
	{
		if (this->parentActor.has_value())
		{
			if (!calledFromRemoveChildActor)
			{
				// remove this actor's pointer from it's parent's childActors container
				this->parentActor.value()->removeChildActor(this, true);
			}

			this->parentActor.reset();
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
		auto newActor = *this;
		newActor.setName(newName);

		// Clear parents and children
		newActor.parentActor = std::nullopt;
		newActor.parentActorBone = std::nullopt;
		newActor.childActors.clear();

		// Clear rigidbody pointer, ghost pointer, and transform flag
		newActor.setRigidBody(nullptr);
		newActor.setGhostObject(nullptr);
		newActor.setManualTransform(true);

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
	}

	std::optional<Renderable>& Actor::getRenderable()
	{
		return this->tempRenderable;
	}

	void Actor::clearTempRenderable()
	{
		this->tempRenderable.reset();
	}

	std::optional<glm::mat4> Actor::getParentMatrix()
	{
		if (!this->parentActor)
		{
			return std::nullopt;
		}

		if (!this->parentActorBone)
		{
			return this->parentActor.value()->getTransform().getMatrix();
		}

		return this->parentActor.value()->getTransform().getMatrix() * this->parentActorBone.value()->matrix;
	}

	glm::mat4 Actor::getWorldMatrix()
	{
		// if this actor has no parent, simply return the matrix of it's transform
		if (!this->parentActor)
		{
			return this->transform.getMatrix();
		}

		// if this actor is parented to another actor (and not a bone of that actor)
		if (!this->parentActorBone)
		{
			return this->parentActor.value()->getTransform().getMatrix() * this->transform.getMatrix();
		}

		return this->parentActor.value()->getTransform().getMatrix() * this->parentActorBone.value()->matrix * this->transform.getMatrix();

	}

	glm::mat4 Actor::getWorldRenderMatrix(float alpha)
	{
		// actor is not dynamic (does not move) so interpolation is not required, simply return it's world matrix
		if (!this->isDynamic() || !this->previousTransform)
		//if (!this->isDynamic() || !this->previousTransform || (App::get().getFrameTime() >= App::get().getLogicTime()))
		{
			return this->getWorldMatrix();
		}


		auto actorMatrix = Transform::interpolateTransforms(this->previousTransform.value(), this->transform, alpha);

		// if this actor has no parent, simply return the matrix of it's transform
		if (!this->parentActor)
		{
			return actorMatrix;
		}

		auto parentActorMatrix = Transform::interpolateTransforms(this->parentActor.value()->getPreviousTransform().value(), this->parentActor.value()->getTransform(), alpha);

		// if this actor is parented to another actor (and not a bone of that actor)
		if (!this->parentActorBone)
		{
			return parentActorMatrix * actorMatrix;
		}

		auto boneMatrix = this->parentActorBone.value()->getRenderMatrix(alpha);

		return parentActorMatrix * boneMatrix * actorMatrix;

	}

	glm::vec3 Actor::getInterpolatedTranslation(float alpha)
	{
		if (this->previousTransform) // insure we have a value for previousTransform from which to interpolate
		{
			return Transform::interpolateTranslations(this->previousTransform.value(), this->transform, alpha);
		}
		return this->transform.getTranslation();
	}

	glm::quat Actor::getInterpolatedRotation(float alpha)
	{
		if (this->previousTransform) // insure we have a value for previousTransform from which to interpolate
		{
			return Transform::interpolateRotations(this->previousTransform.value(), this->transform, alpha);
		}
		//return this->transform.getTranslation();
		return this->transform.getRotation();
	}

	glm::vec3 Actor::getInterpolatedScale(float alpha)
	{
		if (this->previousTransform) // insure we have a value for previousTransform from which to interpolate
		{
			return Transform::interpolateScales(this->previousTransform.value(), this->transform, alpha);
		}
		//return this->transform.getTranslation();
		return this->transform.getScale();
	}

	const bool Actor::isDynamic() const
	{
		return this->dynamic;
	}

	void Actor::updatePreviousTransform()
	{
		if (!this->isDeleted() && this->isDynamic())
		{
			this->previousTransform = this->getTransform();
		}
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

	const std::optional<std::vector<std::pair<size_t, std::string>>>& Actor::getActiveBones() const
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

	void Actor::setManualTransform(bool mt)
	{
		this->manualTransform = mt;
	}

	btRigidBody* Actor::getRigidBody()
	{
		return this->rigidBody;
	}

	bool Actor::getManualTransform()
	{
		return this->manualTransform;
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

	const size_t& Actor::getContainerIndex() const
	{
		if (!this->containerIndex)
		{
			std::cout << "Attempting to get actor container index before it has been set\n";
			std::cin.get();
			exit(EXIT_FAILURE);
		}

		return this->containerIndex.value();
	}

	void Actor::setContainerIndex(size_t ci)
	{
		this->containerIndex = ci;
	}

    const size_t& Actor::getParentRenderableIndex() const
    {
		if (!this->parentRenderableIndex)
		{
			std::cout << "Attempting to get actor parentRenderableIndex before it has been set\n";
			std::cin.get();
			exit(EXIT_FAILURE);
		}
        return this->parentRenderableIndex.value();
    }

    void Actor::setParentRenderableIndex(size_t ri)
    {
		this->parentRenderableIndex = ri;
    }

	const bool Actor::isAnimated() const
	{
		if (this->armature) 
		{
			return true;
		}
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