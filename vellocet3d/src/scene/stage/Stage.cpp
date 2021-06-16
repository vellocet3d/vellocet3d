#include <iostream>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "vel/App.h"
#include "vel/scene/stage/Stage.h"






namespace vel
{

	Stage::Stage() :
		visible(true),
		clearDepthBuffer(false)
	{
		// Reserve default amount of space for each container. Can be updated via
		// setActorContainerSize() and setArmatureContainerSize()...BUT those calls
		// MUST be done before adding any actors or armatures to the stage.
		this->actors.reserve(100);
		this->armatures.reserve(100);

	}

	Armature* Stage::addArmature(Armature a, std::string defaultAnimation, std::vector<std::string> actors)
	{
		if (this->armatures.size() == this->armatures.capacity())
			App::get().logger.die("Scene::addArmature(): Attempting to add armature after armatures capacity has been reached");

		this->armatures.push_back(a);

		Armature* sa = &this->armatures.back();
		sa->playAnimation(defaultAnimation);

		for (auto& actorName : actors)
		{
			auto act = this->getActor(actorName);
			act->setArmature(sa);

			std::vector<std::pair<size_t, std::string>> activeBones;
			size_t index = 0;
			for (auto& meshBone : this->renderables.at(act->getRenderableIndex()).getMesh()->getBones())
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
			this->collisionWorld.value()->getDynamicsWorld()->stepSimulation(delta, 0);
	}

	CollisionWorld* Stage::getCollisionWorld()
	{
		if (!this->collisionWorld)
			return nullptr;

		return this->collisionWorld.value().get();
	}

	void Stage::setCollisionWorld(float gravity)
	{
		this->collisionWorld = std::make_unique<CollisionWorld>(gravity);
	}

	std::vector<Actor>& Stage::getActors()
	{
		return this->actors;
	}

	void Stage::parentActorToActorBone(std::string childName, std::string parentName, std::string parentBoneName)
	{
		auto childActor = this->getActor(childName);
		auto parentActor = this->getActor(parentName);

		childActor->setParentActor(parentActor);
		childActor->setParentActorBone(parentActor->getArmature()->getBone(parentBoneName));
		parentActor->addChildActor(childActor);
	}

	void Stage::parentActorToActor(std::string childName, std::string parentName)
	{
		auto childActor = this->getActor(childName);
		auto parentActor = this->getActor(parentName);

		childActor->setParentActor(parentActor);
		parentActor->addChildActor(childActor);
	}

	const bool Stage::isVisible()
	{
		return this->visible;
	}

	void Stage::setActorCapacity(size_t size)
	{
		this->actors.reserve(size);
	}

	void Stage::setArmatureCapacity(size_t size)
	{
		this->armatures.reserve(size);
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
		for (auto& a : this->actors)
			if (!a.isDeleted() && a.isAnimated())
				a.getArmature()->updateAnimation(runTime, a.getParentMatrix());
	}

	void Stage::applyTransformations()
	{
		for (auto& a : this->actors)
			a.processTransform();
	}

	const size_t Stage::getActorSize() const
	{
		return this->actors.size();
	}

	size_t Stage::addActor(Actor a)
	{
		if (this->actors.size() == this->actors.capacity() && this->actorFreeSlots.size() == 0)
			App::get().logger.die("Stage::addActor(): Attempting to add actor after actors capacity has been reached");

		size_t slotIndex;

		if (this->actorFreeSlots.size() > 0)
		{
			slotIndex = this->actorFreeSlots.back();
			this->actorFreeSlots.pop_back();
			this->actors.at(slotIndex) = a;
		}
		else
		{
			slotIndex = this->actors.size();
			this->actors.push_back(a);
		}

		auto actor = &this->actors.at(slotIndex);
		actor->setContainerIndex(slotIndex);

		if (actor->getTempRenderable())
		{
			auto& tempRenderable = actor->getTempRenderable().value();

			auto parentRenderableIndex = this->renderableExists(tempRenderable.getName());

			if (!parentRenderableIndex) // add renderable to stage if we do not already have an instance
				parentRenderableIndex = this->addRenderable(tempRenderable);

			actor->setRenderableIndex(parentRenderableIndex.value());
			this->getRenderable(parentRenderableIndex.value()).addActorIndex(slotIndex);

			actor->clearTempRenderable();
		}

		return slotIndex;
	}

	Actor* Stage::getActor(std::string name)
	{
		for (auto& a : this->actors)
			if (a.getName() == name)
				return &a;

		return nullptr;
	}

	Actor* Stage::getActor(size_t index)
	{
		if (this->actors.size() == 0 || !(index <= (this->actors.size() - 1)))
			return nullptr;

		return &this->actors.at(index);
	}

	void Stage::removeActor(std::string name)
	{
		for (unsigned int i = 0; i < this->actors.size(); i++)
		{
			auto a = this->actors.at(i);

			if (a.getName() == name)
				this->removeActor(i);
		}
	}

	void Stage::removeActor(size_t index)
	{
		Actor& a = this->actors.at(index);

		// free actor slot in render command
		this->renderables.at(a.getRenderableIndex()).freeActorIndex(a.getContainerIndex());


		// TODO: need to add logic for removing ghostObjects as well, AND remove all sensors which use either the
		// rigidBody or ghostObject of this actor

		// if this actor has a rigid body, remove it from the collision world and clear the pointer
		auto arb = a.getRigidBody();
		if (arb != nullptr)
		{
			this->collisionWorld.value()->removeRigidBody(arb);
			a.setRigidBody(nullptr);
		}

		auto ago = a.getGhostObject();
		if (ago != nullptr)
		{
			this->collisionWorld.value()->removeGhostObject(ago);
			a.setGhostObject(nullptr);
		}

		a.setDeleted(true);
		this->actorFreeSlots.push_back(index);
	}

	size_t Stage::addRenderable(Renderable rc)
	{
		this->renderables.push_back(rc);
		size_t renderableIndex = this->renderables.size() - 1;

		// loop through renderables and sort order saving indexes
		// within this->renderablesOrder

		std::vector<std::pair<size_t, Renderable>> toSort;

		for (size_t i = 0; i < this->renderables.size(); i++)
			toSort.push_back(std::pair<size_t, Renderable>(i, this->renderables.at(i)));

		// sort sharder
		std::sort(toSort.begin(), toSort.end(), [](auto &left, auto &right) {
			return left.second.getShader() < right.second.getShader();
		});

		// sort mesh
		std::sort(toSort.begin(), toSort.end(), [](auto &left, auto &right) {
			return left.second.getMesh() < right.second.getMesh();
		});

		// sort material
		std::sort(toSort.begin(), toSort.end(), [](auto &left, auto &right) {
			return left.second.getMaterial() < right.second.getMaterial();
		});

		// sort texture alpha
		std::sort(toSort.begin(), toSort.end(), [](auto &left, auto &right) {
			return left.second.getMaterialHasAlpha() < right.second.getMaterialHasAlpha();
		});


		this->renderablesOrder.clear();

		for (auto& p : toSort)
			this->renderablesOrder.push_back(p.first);


		return renderableIndex;
	}

	std::optional<size_t> Stage::renderableExists(const std::string& rn)
	{
		for (unsigned int i = 0; i < this->renderables.size(); i++)
			if (this->renderables[i].getName() == rn)
				return i;

		return std::nullopt;
	}

	const bool Stage::hasActorWithName(std::string name) const
	{
		for (auto& a : this->actors)
			if (a.getName() == name)
				return true;

		return false;
	}

	void Stage::printRenderables() const
	{
		//std::cout << "Renderables\n";
		//std::cout << "----------------------------------\n";
		//for (auto& rc : this->renderables)
		//{
		//	std::cout << "shaderIndex:" << rc.getShaderIndex() << " meshIndex:" << rc.getMeshIndex() << " materialIndex:" << rc.getMaterialIndex() << "\n";
		//	std::cout << "actors:";

		//	for (auto& a : rc.getActorIndexes())
		//		if (a != -1)
		//			std::cout << this->actors.at(a).getName() << ",";

		//	std::cout << "\norder:";

		//	for (auto& o : this->renderablesOrder)
		//		std::cout << o << ",";

		//	std::cout << "\n------------------------------\n";
		//}
	}

	Renderable& Stage::getRenderable(size_t index)
	{
		return this->renderables.at(index);
	}

	std::optional<Renderable>& Stage::getRenderable(std::string name)
	{
		auto rr = std::optional<Renderable>();
		for (auto& r : this->renderables)
		{
			if (r.getName() == name)
			{
				rr = r;
				return rr;
			}
		}
		return rr;
	}

	const std::vector<size_t>& Stage::getRenderablesOrder() const
	{
		return this->renderablesOrder;
	}

	std::optional<Camera>& Stage::getCamera()
	{
		return this->camera;
	}

	void Stage::debugListNumberOfBonesPerArmature()
	{
		for (auto& a : this->armatures)
			std::cout << a.getName() << ":" << a.getBones().size() << "\n";
	}

	void Stage::debugActiveNumberOfBonesPerActor()
	{
		for (auto& a : this->actors)
			std::cout << a.getName() << ":" << a.getActiveBones().size() << "\n";
	}

}