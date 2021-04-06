#include <iostream>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "vel/App.h"
#include "vel/scene/stage/Stage.h"
#include "vel/AssetLoader.h"





namespace vel::scene::stage
{

    Stage::Stage(vel::scene::Scene* parentScene) :
		parentScene(parentScene),
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

        this->renderCommands = std::vector<RenderCommand>();
        this->renderCommandsOrder = std::vector<size_t>();

    }

	vel::scene::armature::Armature* Stage::addArmature(vel::scene::armature::Armature a)
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

	void Stage::useCollisionDebugDrawer()
	{
		if (!this->collisionWorld)
		{
			return;
		}

		this->collisionDebuggingSwitch = true;

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

	vel::scene::Scene* Stage::getParentScene()
	{
		return this->parentScene;
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

	std::vector<size_t> Stage::loadActors(std::string filename, bool dynamic)
    {
        auto loader = AssetLoader(this->parentScene, this, filename, dynamic);
        return loader.loadActors();
    }

	std::vector<size_t> Stage::loadActors(std::string filename, bool dynamic, int shaderIndex)
    {
        auto loader = AssetLoader(this->parentScene, this, filename, dynamic);
        loader.findShaderId = [&](std::string actorName) {
            return shaderIndex;
        };
        return loader.loadActors();
    }

	std::vector<size_t> Stage::loadActors(std::string filename, bool dynamic,
        std::vector<std::pair<int, std::vector<std::string>>> actorShaderAssocs)
    {
        auto loader = AssetLoader(this->parentScene, this, filename, dynamic);
        loader.findShaderId = [&](std::string actorName) {
            
            for (auto& a : actorShaderAssocs)
            {
                for (auto& p : a.second)
                {
                    if (actorName.find(p) != std::string::npos)
                    {
                        return a.first;
                    }
                }
            }

            return 0; // if no shader assoc found, use default shader

        };
        return loader.loadActors();
    }

	void Stage::updateActorAnimations(double runTime)
	{
		for (auto& a : this->actors)
		{
			//if (!a.isDeleted() && a.isAnimated() && a.isVisible())
			if (!a.isDeleted() && a.isAnimated())
			{
				a.getArmature()->updateAnimation(runTime, a.getParentMatrix());
			}
		}
	}

	void Stage::applyTransformations()
	{
		for (auto& a : this->actors)
		{
			a.processTransform();
		}
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

        if (actor->getShaderIndex().has_value() && 
            actor->getMeshIndex().has_value() &&
			actor->getTextureIndex().has_value()) // If we are not in headless mode, AND this actor has a shader, mesh, and texture
        {
            size_t shaderIndex = actor->getShaderIndex().value();
			size_t meshRenderableIndex = this->parentScene->getMesh(actor->getMeshIndex().value()).getMeshRenderableIndex().value();
			size_t textureIndex = actor->getTextureIndex().value();
			size_t textureHasAlpha = actor->getTextureHasAlphaChannel() ? 1 : 0;

			//std::cout << actor->getName() << " " << textureHasAlpha << "\n";

            // Search the existing render commands to see if one exists for the given criteria
            std::optional<size_t> renderCommandIndex = this->renderCommandExists(shaderIndex, meshRenderableIndex, textureIndex);

            // If a render command does not exist for the given criteria, create one and get it's index
            if (!renderCommandIndex)
            {
                auto rc = RenderCommand(shaderIndex, meshRenderableIndex, textureIndex, textureHasAlpha);
                renderCommandIndex = this->addRenderCommand(rc);
            }

            // Add this actor's slot index to the actorIndexes container of it's RenderCommand
			// REVISED
			//size_t indexOfActorInRenderCommand = this->renderCommands->at(renderCommandIndex.value()).addActorIndex(slotIndex);
			this->renderCommands->at(renderCommandIndex.value()).addActorIndex(slotIndex);

            // Now add the renderCommandIndex AND indexOfActorInRenderCommandActorIndexes to the actor's 
            // std::optional<std::pair<int, int>> renderCommand member, which is used to quickly remove the 
            // actor from the render command if it is ever removed from the stage
			// REVISED: removed slot system from render command as it was not implemented correctly, and im not sure there was really
			// a reason for it in the first place, so the second value in the below pair, is now the value of the actor's index in the actors slot container
            actor->addRenderCommand(std::pair<size_t, size_t>(renderCommandIndex.value(), slotIndex));

        }

		return slotIndex;
    }

    Actor* Stage::getActor(std::string name)
    {
        for (auto& a : this->actors) 
        {
            if (a.getName() == name) 
            {
                return &a;
            }
        }
		return nullptr;
    }

    Actor* Stage::getActor(size_t index) 
    {
		if (this->actors.size() == 0 || !(index <= (this->actors.size() - 1)))
		{
			return nullptr;
		}
        return &this->actors.at(index);
    }

    void Stage::removeActor(std::string name) 
    {
        for (unsigned int i = 0; i < this->actors.size(); i++)
        {
            auto a = this->actors.at(i);
            if (a.getName() == name)
            {
                this->removeActor(i);
            }
        }
    }

    void Stage::removeActor(size_t index)
    {
        Actor& a = this->actors.at(index);

        // free actor slot in render command
        if (this->renderCommands)
        {
			//std::cout << a.getRenderCommand().first << ":" << a.getRenderCommand().second << "\n";

            this->renderCommands.value().at(a.getRenderCommand().first).freeActorIndex(a.getRenderCommand().second);
        }

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

    size_t Stage::addRenderCommand(RenderCommand rc) 
    {
        this->renderCommands->push_back(rc);
        size_t renderCommandIndex = this->renderCommands->size() - 1;

        // loop through render commands and sort order saving indexes
        // within this->renderCommandsOrder

        std::vector<std::pair<size_t, RenderCommand>> toSort;

        for (size_t i = 0; i < this->renderCommands->size(); i++) 
        {
            toSort.push_back(std::pair<size_t, RenderCommand>(i, this->renderCommands->at(i)));
        }

		// sort sharder
        std::sort(toSort.begin(), toSort.end(), [](auto &left, auto &right) {
            return left.second.getShaderIndex() < right.second.getShaderIndex();
        });

		// sort mesh
		std::sort(toSort.begin(), toSort.end(), [](auto &left, auto &right) {
			return left.second.getMeshIndex() < right.second.getMeshIndex();
		});

		// sort texture
		std::sort(toSort.begin(), toSort.end(), [](auto &left, auto &right) {
			return left.second.getTextureIndex() < right.second.getTextureIndex();
		});

		// sort texture alpha
		std::sort(toSort.begin(), toSort.end(), [](auto &left, auto &right) {
			return left.second.getTextureHasAlpha() < right.second.getTextureHasAlpha();
		});



        this->renderCommandsOrder->clear();

        for (auto& p : toSort) 
        {
            this->renderCommandsOrder->push_back(p.first);
        }

		//for debugging
		//std::cout << "-----------------------\n";
		//for (auto& rco : this->renderCommandsOrder.value())
		//{
		//	auto rc = this->renderCommands.value().at(rco);

		//	std::cout << "s:" << rc.getShaderIndex() << " m:" << rc.getMeshIndex() << " t:" << rc.getTextureIndex() << " a:" << rc.getTextureHasAlpha() << "\n";

		//}

        return renderCommandIndex;

    }

    std::optional<size_t> Stage::renderCommandExists(size_t sI, size_t mI, size_t tI)
    {
        for (unsigned int i = 0; i < this->renderCommands->size(); i++)
        {
            if ((this->renderCommands.value()[i].getShaderIndex() == sI) &&
                (this->renderCommands.value()[i].getMeshIndex() == mI) &&
                (this->renderCommands.value()[i].getTextureIndex() == tI))
            {
                return i;
            }
        }
        return std::nullopt;
    }

    const bool Stage::hasActorWithName(std::string name) const
    {
        for (auto& a : this->actors)
        {
            if (a.getName() == name)
            {
                return true;
            }
        }
        return false;
    }

    void Stage::printRenderCommands() const
    {
        std::cout << "RenderCommands\n";
        std::cout << "----------------------------------\n";
        for (auto& rc : this->renderCommands.value()) 
        {
            std::cout << "shaderIndex:" << rc.getShaderIndex() << " meshIndex:" << rc.getMeshIndex() << " textureIndex:" << rc.getTextureIndex() << "\n";
            std::cout << "actors:";
            for (auto& a : rc.getActorIndexes()) 
            {
                if (a != -1) 
                {
                    std::cout << this->actors.at(a).getName() << ",";
                }
            }
            std::cout << "\norder:";
            for (auto& o : this->renderCommandsOrder.value()) 
            {
                std::cout << o << ",";
            }
            std::cout << "\n------------------------------\n";
        }
    }

    RenderCommand& Stage::getRenderCommand(size_t index)
    {
        return this->renderCommands->at(index);
    }

    const std::optional<std::vector<size_t>>& Stage::getRenderCommandsOrder() const
    {
        return this->renderCommandsOrder;
    }

    std::optional<Camera>& Stage::getCamera()
    {
        return this->camera;
    }

}