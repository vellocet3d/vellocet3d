

#include "vel/helpers/functions.h"
#include "vel/App.h"
#include "vel/scene/stage/Actor.h"


namespace vel::scene::stage
{
    Actor::Actor(std::string name, Transform t) :
        name(name),
        deleted(false),
		visible(true),
		dynamic(false),
        transform(t) ,
		rigidBody(nullptr),
		armature(nullptr),
		manualTransform(true)
    {}

	void Actor::processTransform()
	{
		this->updatePreviousTransform();
		
		if (!this->manualTransform && this->rigidBody != nullptr)
		{
			this->transform.setTranslation(vel::helpers::functions::bulletToGlmVec3(this->rigidBody->getWorldTransform().getOrigin()));
			this->transform.setRotation(vel::helpers::functions::bulletToGlmQuat(this->rigidBody->getWorldTransform().getRotation()));
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
		return this->transform.getTranslation();
	}

	glm::vec3 Actor::getInterpolatedScale(float alpha)
	{
		if (this->previousTransform) // insure we have a value for previousTransform from which to interpolate
		{
			return Transform::interpolateScales(this->previousTransform.value(), this->transform, alpha);
		}
		return this->transform.getTranslation();
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
	}

	void Actor::setParentActorBone(vel::scene::armature::Bone* b)
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

    void Actor::setMeshIndex(size_t i)
    {
        this->meshIndex = i;
    }

    void Actor::setShaderIndex(size_t i)
    {
        this->shaderIndex = i;
    }

    void Actor::setTextureIndex(size_t i)
    {
        this->textureIndex = i;
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
	}

    const std::pair<size_t, size_t>& Actor::getRenderCommand() const
    {
        return this->renderCommand.value();
    }

    const std::string Actor::getName() const
    {
        return this->name;
    }

    const std::optional<size_t>& Actor::getShaderIndex() const
    {
        return this->shaderIndex;
    }

    const std::optional<size_t>& Actor::getMeshIndex() const
    {
        return this->meshIndex;
    }

    const std::optional<size_t>& Actor::getTextureIndex() const
    {
        return this->textureIndex;
    }

    void Actor::addRenderCommand(std::pair<size_t, size_t> cmd)
    {
        this->renderCommand = cmd;
    }

	const bool Actor::isAnimated() const
	{
		if (this->armature) 
		{
			return true;
		}
		return false;
	}

	void Actor::setArmature(vel::scene::armature::Armature* arm)
	{
		this->armature = arm;
	}

	vel::scene::armature::Armature* Actor::getArmature()
	{
		return this->armature;
	}

	vel::scene::mesh::Mesh& Actor::getMesh()
	{
		return App::get().getScene()->getMesh(this->meshIndex.value());
	}

}