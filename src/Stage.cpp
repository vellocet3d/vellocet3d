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
		clearDepthBuffer(false),
		name(name)
	{}

	Stage::~Stage()
	{
		
	}

	const std::string& Stage::getName() const
	{
		return this->name;
	}

	Armature* Stage::getArmature(std::string armatureName)
	{
		return this->armatures.get(armatureName);
	}

	Armature* Stage::addArmature(Armature a, std::string defaultAnimation, std::vector<std::string> actorsIn)
	{
		Armature* sa = this->armatures.insert(a.getName(), a);
		sa->playAnimation(defaultAnimation);

		for (auto& actorName : actorsIn)
		{
			auto act = this->actors.get(actorName);
			act->setArmature(sa);

			std::vector<std::pair<size_t, unsigned int>> activeBones;
			unsigned int index = 0;
			for (auto& meshBone : act->getMesh()->getBones())
			{
				// associate the index of the armature bone with the index of the mesh bone used for transformation
				activeBones.push_back(std::pair<size_t, unsigned int>(act->getArmature()->getBoneIndex(meshBone.name), index));
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

	void Stage::addCamera(Camera* c)
	{
		this->cameras.push_back(c);
	}

	Camera* Stage::getCamera(std::string name)
	{
		for (auto c : this->cameras)
			if (c->getName() == name)
				return c;

		return nullptr;
	}

	std::vector<Camera*>& Stage::getCameras()
	{
		return this->cameras;
	}

	void Stage::updateFixedArmatureAnimations(double runTime)
	{
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

	void Stage::updatePreviousTransforms()
	{
		for (auto a : this->actors.getAll())
			if (a->isDynamic())
				a->updatePreviousTransform();
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

			// new renderable, so check if it has an animated material and if so add pointer to it to animatedMaterials
			if (actorStageRenderable->getMaterial().has_value() && actorStageRenderable->getMaterial()->getMaterialAnimator().has_value())
				this->animatedMaterials.insert(actorStageRenderable->getName() + "_" + actorStageRenderable->getMaterial()->getName(), &actorStageRenderable->getMaterial().value());
		}
		// if adding an actor that does not have a tempRenderable pointer, BUT DOES have a 
		// stageRenderable value, then we assume that this actor was derived from an existing
		// actor, and we simply need to add it to that existing stageRenderable.
		else if(!actor->getTempRenderable().has_value() && actor->getStageRenderable().has_value())
		{
			actor->getStageRenderable().value()->actors.insert(actor->getName(), actor);
		}

		// if actor has an animated material, add it to animatedMaterials
		if (actor->getMaterial().has_value() && actor->getMaterial()->getMaterialAnimator().has_value())
			this->animatedMaterials.insert(actor->getName() + "_" + actor->getMaterial()->getName(), &actor->getMaterial().value());

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

		// remove pointer to animated material if actor had a unique animated material
		if (a->getMaterial().has_value() && a->getMaterial()->getMaterialAnimator().has_value())
			this->animatedMaterials.erase(&a->getMaterial().value());

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

	TextActor* Stage::addTextActor(TextActor ta)
	{
		return this->textActors.insert(ta.name, ta);
	}

	TextActor* Stage::getTextActor(std::string name)
	{
		return this->textActors.get(name);
	}

	std::vector<TextActor*>& Stage::getTextActors()
	{
		return this->textActors.getAll();
	}

	void Stage::removeTextActor(TextActor* ta)
	{
		this->_removeActor(ta->actor);
		this->textActors.erase(ta);
	}

	void Stage::removeTextActor(std::string name)
	{
		this->removeTextActor(this->textActors.get(name));
	}

	std::vector<Renderable*>& Stage::getRenderables()
	{
		return this->renderables.getAll();
	}

}