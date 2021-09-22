#include <iostream>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "vel/App.h"
#include "vel/Stage.h"






namespace vel
{

	Stage::Stage(std::string name) :
		visible(true),
		collisionWorld(nullptr),
		clearDepthBuffer(false),
		name(name),
        activeHdr(nullptr),
		drawHdr(true),
		IBLCamera(nullptr)
	{}

	Stage::~Stage()
	{
		delete this->collisionWorld;
	}

	const std::string& Stage::getName() const
	{
		return this->name;
	}

	Armature* Stage::getArmature(std::string armatureName)
	{
		return this->armatures.get(armatureName);
	}

	void Stage::setIBLCamera(Camera* c)
	{
		this->IBLCamera = c;
	}

	Camera* Stage::getIBLCamera()
	{
		return this->IBLCamera;
	}

    void Stage::setActiveHdr(HDR* h)
    {
        this->activeHdr = h;
    }
    
    HDR* Stage::getActiveHdr()
    {
        return this->activeHdr;
    }

	void Stage::setDrawHdr(bool b)
	{
		this->drawHdr = b;
	}

	bool Stage::getDrawHdr()
	{
		return this->drawHdr;
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

	plf::colony<Actor>& Stage::getActors()
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

	void Stage::addPerspectiveCamera(float nearPlane, float farPlane, float fov)
	{
		this->camera = Camera(
			CameraType::PERSPECTIVE,
			nearPlane,
			farPlane,
			fov
		);
	}

	void Stage::addOrthographicCamera(float nearPlane, float farPlane, float scale)
	{
		this->camera = Camera(
			CameraType::ORTHOGRAPHIC,
			nearPlane,
			farPlane,
			scale
		);
	}

	void Stage::updateActorAnimations(double runTime)
	{
		for (auto& a : this->actors.getAll())
			if (!a.isDeleted() && a.isAnimated())
				a.getArmature()->updateAnimation(runTime);
	}

	void Stage::applyTransformations()
	{
		for (auto& a : this->actors.getAll())
			a.processTransform();
	}

	Actor* Stage::addActor(Actor a)
	{
		auto actor = this->actors.insert(a.getName(), a);

		if (actor->getTempRenderable())
		{
			auto& tempRenderable = actor->getTempRenderable().value();

			if(!this->renderables.exists(tempRenderable.getName()))
				this->renderables.insert(tempRenderable.getName(), tempRenderable);

			auto actorStageRenderable = this->renderables.get(tempRenderable.getName());

			actor->setStageRenderable(actorStageRenderable);
			actor->clearTempRenderable();
			
			actorStageRenderable->actors.insert(actor->getName(), actor);
		}

		return actor;
	}

	Actor* Stage::getActor(std::string name)
	{
		return this->actors.get(name);
	}

	void Stage::removeActor(std::string name)
	{
		Actor* a = this->actors.get(name);		

		// free actor slot in renderable
		if(a->getStageRenderable())
			a->getStageRenderable().value()->actors.erase(a->getName());

		// remove all sensors associated with this actor
		for(auto& s : a->getContactSensors())
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

	plf::colony<Renderable>& Stage::getRenderables()
	{
		return this->renderables.getAll();
	}

	std::optional<Camera>& Stage::getCamera()
	{
		return this->camera;
	}

}