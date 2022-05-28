#include <iostream>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "vel/App.h"
#include "vel/Stage.h"






namespace vel
{

	Stage::Stage(std::string name) :
		renderMode(RenderMode::STATIC_DIFFUSE),
		visible(true),
		collisionWorld(nullptr),
		clearDepthBuffer(false),
		name(name),
		useSceneSpaceLighting(true)
	{}

	Stage::~Stage()
	{
		delete this->collisionWorld;
	}

	void Stage::setRenderMode(RenderMode rm)
	{
		this->renderMode = rm;
	}

	RenderMode Stage::getRenderMode()
	{
		return this->renderMode;
	}

	const std::string& Stage::getName() const
	{
		return this->name;
	}

	Armature* Stage::getArmature(std::string armatureName)
	{
		return this->armatures.get(armatureName);
	}

	void Stage::setUseSceneSpaceLighting(bool b)
	{
		this->useSceneSpaceLighting = b;
	}

	bool Stage::getUseSceneSpaceLighting()
	{
		return this->useSceneSpaceLighting;
	}

	Armature* Stage::addArmature(Armature a, std::string defaultAnimation, std::vector<std::string> actorsIn)
	{
		Armature* sa = this->armatures.insert(a.getName(), a);
		sa->playAnimation(defaultAnimation);

		for (auto& actorName : actorsIn)
		{
			auto act = this->actors.get(actorName);
			act->setArmature(sa);

			std::vector<std::pair<size_t, std::string>> activeBones;
			size_t index = 0;
			for (auto& meshBone : act->getMesh()->getBones())
			{
				activeBones.push_back(std::pair<size_t, std::string>(act->getArmature()->getBoneIndex(meshBone.name), "bones[" + std::to_string(index) + "]"));
				index++;
			}

			act->setActiveBones(activeBones);
		}

		return sa;
	}

	bool Stage::getClearDepthBuffer()
	{
		return this->clearDepthBuffer;
	}

	void Stage::setClearDepthBuffer(bool b)
	{
		this->clearDepthBuffer = b;
	}

	void Stage::stepPhysics(float delta)
	{
		if (this->collisionWorld)
			this->collisionWorld->getDynamicsWorld()->stepSimulation(delta, 0);
	}

	CollisionWorld* Stage::getCollisionWorld()
	{
		return this->collisionWorld;
	}

	void Stage::setCollisionWorld(float gravity)
	{
		// for some reason this has to be a pointer or bullet has read access violation issues
		// delete in destructor
		this->collisionWorld = new CollisionWorld(gravity); 
	}

	std::vector<Actor*>& Stage::getActors()
	{
		return this->actors.getAll();
	}

	const bool Stage::isVisible()
	{
		return this->visible;
	}

	void Stage::hide()
	{
		this->visible = false;
	}

	void Stage::show()
	{
		this->visible = true;
	}

	void Stage::addCamera(CameraType ct, float nearPlane, float farPlane, float fovOrScale)
	{
		this->camera = Camera(
			ct,
			nearPlane,
			farPlane,
			fovOrScale
		);
	}

	void Stage::updateFixedArmatureAnimations(double runTime)
	{
		// TODO: This needs to update Armatures regardless of actors since if
		// 30 actors use the same armature, this will update the armature 30 times
		//for (auto a : this->actors.getAll())
		//	if (!a->isDeleted() && a->isAnimated())
		//		a->getArmature()->updateAnimation(runTime);

		for (auto a : this->armatures.getAll())
			if(a->getShouldInterpolate())
				a->updateAnimation(runTime);
	}

	void Stage::updateArmatureAnimations(double runTime)
	{
		for (auto a : this->armatures.getAll())
			if (!a->getShouldInterpolate())
				a->updateAnimation(runTime);
	}

	void Stage::applyTransformations()
	{
		for (auto a : this->actors.getAll())
			a->processTransform();
	}

	Actor* Stage::addActor(Actor a)
	{
		auto actor = this->actors.insert(a.getName(), a);
		
		if (actor->getTempRenderable())
		{
			auto& tempRenderable = actor->getTempRenderable().value();

			if (!this->renderables.exists(tempRenderable.getName()))
				this->renderables.insert(tempRenderable.getName(), tempRenderable);
				
			auto actorStageRenderable = this->renderables.get(tempRenderable.getName());

			actor->setStageRenderable(actorStageRenderable);
			
			actor->clearTempRenderable();
			
			actorStageRenderable->actors.insert(actor->getName(), actor);
		}
		// if adding an actor that does not have a tempRenderable pointer, BUT DOES have a 
		// stageRenderable value, then we assume that this actor was derived from an existing
		// actor, and we simply need to add it to that existing stageRenderable.
		else if(!actor->getTempRenderable().has_value() && actor->getStageRenderable().has_value())
		{
			actor->getStageRenderable().value()->actors.insert(actor->getName(), actor);
		}

		return actor;
	}

	Actor* Stage::getActor(std::string name)
	{
		return this->actors.get(name);
	}

	void Stage::_removeActor(Actor* a)
	{
		// free actor slot in renderable
		if(a->getStageRenderable())
			a->getStageRenderable().value()->actors.erase(a->getName());

		// remove all sensors associated with this actor
		for(auto s : a->getContactSensors())
			this->collisionWorld->removeSensor(s);
		
		a->clearContactSensors();
		
		// remove rigidbody and ghost objects if they exist
		auto arb = a->getRigidBody();
		if (arb != nullptr)
		{
			this->collisionWorld->removeRigidBody(arb);
			a->setRigidBody(nullptr);
		}

		auto ago = a->getGhostObject();
		if (ago != nullptr)
		{
			this->collisionWorld->removeGhostObject(ago);
			a->setGhostObject(nullptr);
		}

		// mark actor as deleted (since it's value will persist in memory) and "remove" from sac
		a->setDeleted(true);
		this->actors.erase(a->getName());
	}

	void Stage::removeActor(Actor* a)
	{
		this->_removeActor(a);
	}

	void Stage::removeActor(std::string name)
	{
		this->_removeActor(this->actors.get(name));
	}

	std::vector<Renderable*>& Stage::getRenderables()
	{
		return this->renderables.getAll();
	}

	std::optional<Camera>& Stage::getCamera()
	{
		return this->camera;
	}

}