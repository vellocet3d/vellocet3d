#include <iostream>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "vel/App.h"
#include "vel/scene/stage/Stage.h"
#include "vel/AssetLoader.h"





namespace vel
{

    Stage::Stage() :
        visible(true),
		collisionDebuggingSwitch(false),
		clearDepthBuffer(false)
    {
        // Set default actors container to 1000 slots. If more space
        // is required, call setActorContainerSize before adding
        // actors to the stage. If this is not done and actors
        // are added past the reserved limit, it will result in
        // undefined behavior (as push_back will reallocate
        // all data to a new block every time it is called)
		//
		// Yes, this is a naive approach, but for the time being it
		// solves the problem at hand and allows me to move forward.
		// A more elegant solution should be put into place once
		// time allows
		//
		// 6ish months later this really isn't that bad of a way to handle
		// this...it's straight forward, works, and is sufficiently performant.
		//
		// TODO: simply add some logic to check if we are close to filling
		// up the reserve, and if so, allocate an additional 1000 blocks and
		// go about our day....yeah, that'd be a great way to invalidate all
		// of our pointers...wow...so yeah, you will need to implement multiple
		// 1000 block vectors, OR just make this value user defined and allow
		// user to provide value which application will not exceed?
        this->actors.reserve(1000);

		this->armatures.reserve(100);

        this->renderables = std::vector<Renderable>();
        this->renderablesOrder = std::vector<size_t>();

    }

	Armature* Stage::addArmature(Armature a)
	{
		this->armatures.push_back(a);
		
		return &this->armatures.back();
	}

	bool Stage::getClearDepthBuffer()
	{
		return this->clearDepthBuffer;
	}

	void Stage::setClearDepthBuffer(bool b)
	{
		this->clearDepthBuffer = b;
	}

	bool Stage::collisionDebugging()
	{
		return this->collisionDebuggingSwitch;
	}

	void Stage::useCollisionDebugDrawer(int debugMode)
	{
		if (!this->collisionWorld)
		{
			return;
		}

		this->collisionDebuggingSwitch = true;

		App::get().getGPU()->getCollisionDebugDrawer()->setDebugMode(debugMode);

		this->getCollisionWorld()->getDynamicsWorld()->setDebugDrawer(App::get().getGPU()->getCollisionDebugDrawer());
	}

	void Stage::stepPhysics(float delta)
	{
		if (this->collisionWorld)
		{
			this->collisionWorld.value()->getDynamicsWorld()->stepSimulation(delta, 0);
		}
	}

	CollisionWorld* Stage::getCollisionWorld()
	{
		if (!this->collisionWorld)
		{
			return nullptr;
		}
		return this->collisionWorld.value().get();
	}

	void Stage::setCollisionWorld(float gravity)
	{
		this->collisionWorld = std::make_unique<CollisionWorld>(this, gravity);
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

    void Stage::setActorContainerSize(unsigned int size)
    {
        this->actors.reserve(size);
    }

    void Stage::hide()
    {
        this->visible = false;
    }

    void Stage::show()
    {
        this->visible = true;
    }

    void Stage::addPerspectiveCamera(bool fixed, float nearPlane, float farPlane, float fov)
    {
        this->camera = Camera(
            CameraType::PERSPECTIVE,
            fixed,
            nearPlane,
            farPlane,
            fov
        );
    }

    void Stage::addOrthographicCamera(bool fixed, float nearPlane, float farPlane, float scale)
    {
        this->camera = Camera(
            CameraType::ORTHOGRAPHIC,
            fixed,
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

		//TODO renderable things
		if (actor->getRenderables().size() > 0)
		{
			for (auto& tempRenderable : actor->getRenderables())
			{
				auto parentRenderableIndex = this->renderableExists(tempRenderable.getName());

				if (!parentRenderableIndex) // add renderable to stage if we do not already have an instance
					parentRenderableIndex = this->addRenderable(tempRenderable);

				actor->addParentRenderableIndex(parentRenderableIndex.value());
				this->getRenderable(parentRenderableIndex.value()).addActorIndex(slotIndex);
			}

			actor->clearTempRenderables();
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
		for (auto& ar : a.getParentRenderableIndexes())
			this->renderables.at(ar).freeActorIndex(a.getContainerIndex().value());
		

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
            return left.second.getShaderIndex() < right.second.getShaderIndex();
        });

		// sort mesh
		std::sort(toSort.begin(), toSort.end(), [](auto &left, auto &right) {
			return left.second.getMeshIndex() < right.second.getMeshIndex();
		});

		// sort material
		std::sort(toSort.begin(), toSort.end(), [](auto &left, auto &right) {
			return left.second.getMaterialIndex() < right.second.getMaterialIndex();
		});

		// sort texture alpha
		std::sort(toSort.begin(), toSort.end(), [](auto &left, auto &right) {
			return left.second.getMaterialHasAlpha() < right.second.getMaterialHasAlpha();
		});



        this->renderablesOrder.clear();

        for (auto& p : toSort) 
            this->renderablesOrder.push_back(p.first);

		//for debugging
		//std::cout << "-----------------------\n";
		//for (auto& rco : this->renderablesOrder.value())
		//{
		//	auto rc = this->renderables.value().at(rco);

		//	std::cout << "s:" << rc.getShaderIndex() << " m:" << rc.getMeshIndex() << " t:" << rc.getTextureIndex() << " a:" << rc.getTextureHasAlpha() << "\n";

		//}

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
        std::cout << "Renderables\n";
        std::cout << "----------------------------------\n";
        for (auto& rc : this->renderables) 
        {
            std::cout << "shaderIndex:" << rc.getShaderIndex() << " meshIndex:" << rc.getMeshIndex() << " materialIndex:" << rc.getMaterialIndex() << "\n";
            std::cout << "actors:";

            for (auto& a : rc.getActorIndexes()) 
                if (a != -1) 
                    std::cout << this->actors.at(a).getName() << ",";
                
            std::cout << "\norder:";

            for (auto& o : this->renderablesOrder) 
                std::cout << o << ",";

            std::cout << "\n------------------------------\n";
        }
    }

    Renderable& Stage::getRenderable(size_t index)
    {
        return this->renderables.at(index);
    }

	std::optional<Renderable&> Stage::getRenderable(std::string name)
	{
		auto rr = std::optional<Renderable&>();
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

    const std::optional<std::vector<size_t>>& Stage::getRenderablesOrder() const
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
			std::cout << a.getName() << ":" << a.getActiveBones().value().size() << "\n";
	}

}