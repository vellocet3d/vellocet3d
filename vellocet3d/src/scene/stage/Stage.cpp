#include <iostream>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "vel/App.h"
#include "vel/scene/stage/Stage.h"
#include "vel/scene/AssetLoader.h"




namespace vel::scene::stage
{

    Stage::Stage(bool headless) :
        headless(headless),
        visible(true),
		controllers(std::vector<std::unique_ptr<Controller>>()),
		outerLoopControllers(std::vector<std::unique_ptr<Controller>>()),
		collisionDebuggingSwitch(false)
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
        this->actors.reserve(1000);

        if (!this->headless)
        {
            this->renderCommands = std::vector<RenderCommand>();
            this->renderCommandsOrder = std::vector<size_t>();
        }

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

		this->getCollisionWorld()->dynamicsWorld->setDebugDrawer(App::get().getGPU()->getCollisionDebugDrawer());
	}

	void Stage::stepPhysics(float delta)
	{
		if (this->collisionWorld)
		{
			this->collisionWorld.value()->dynamicsWorld->stepSimulation(delta, 0);
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
		this->collisionWorld = std::make_unique<CollisionWorld>(gravity);
	}

	std::vector<Actor>& Stage::getActors()
	{
		return this->actors;
	}

	void Stage::executeOuterLoopControllers(float frameTime, float alphaTime)
	{
		for (auto& c : this->outerLoopControllers)
		{
			c->setDeltaTime(frameTime);
			c->setAlphaTime(alphaTime);
			c->logic();
		}
	}

	void Stage::executeControllers(float deltaTime)
	{
		for (auto& c : this->controllers)
		{
			c->setDeltaTime(deltaTime);
			c->logic();
		}
	}

	void Stage::addController(Controller* controller, bool forOuterLoop)
	{
		if (!forOuterLoop)
		{
			this->controllers.push_back(std::move(std::unique_ptr<Controller>(controller)));
			return;
		}
		this->outerLoopControllers.push_back(std::move(std::unique_ptr<Controller>(controller)));
	}

	void Stage::parentActorToActorBone(std::string childName, std::string parentName, std::string parentBoneName)
	{
		auto childActor = this->getActor(childName);
		auto parentActor = this->getActor(parentName);

		childActor->setParentActor(parentActor);
		childActor->setParentActorBone(parentActor->getArmature().getBone(parentBoneName));
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

    void Stage::loadActors(std::string filename, bool dynamic)
    {
        auto loader = AssetLoader(this, filename, dynamic);
        loader.loadActors();
    }

    void Stage::loadActors(std::string filename, bool dynamic, int shaderIndex)
    {
        auto loader = AssetLoader(this, filename, dynamic);
        loader.findShaderId = [&](std::string actorName) {
            return shaderIndex;
        };
        loader.loadActors();
    }

    void Stage::loadActors(std::string filename, bool dynamic,
        std::vector<std::pair<int, std::vector<std::string>>> actorShaderAssocs)
    {
        auto loader = AssetLoader(this, filename, dynamic);
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
        loader.loadActors();
    }

	void Stage::updateActorAnimations(double runTime)
	{
		for (auto& a : this->actors)
		{
			if (!a.isDeleted() && a.isAnimated())
			{
				a.getArmature().updateCurrentAnimation(runTime, a.getParentMatrix());
			}
		}
	}

	void Stage::savePreviousTransforms()
	{
		for (auto& a : this->actors)
		{
			a.updatePreviousTransform();
		}
	}

    const size_t Stage::getActorSize() const
    {
        return this->actors.size();
    }

    void Stage::addActor(Actor a)
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

        if (!this->headless && 
            actor->getShaderIndex().has_value() && 
            actor->getMeshIndex().has_value() &&
			actor->getTextureIndex().has_value()) // If we are not in headless mode, AND this actor has a shader, mesh, and texture
        {
            size_t shaderIndex = actor->getShaderIndex().value();
			size_t meshRenderableIndex = App::get().getScene()->getMesh(actor->getMeshIndex().value()).getMeshRenderableIndex().value();
			size_t textureIndex = actor->getTextureIndex().value();

            // Search the existing render commands to see if one exists for the given criteria
            std::optional<size_t> renderCommandIndex = this->renderCommandExists(shaderIndex, meshRenderableIndex, textureIndex);

            // If a render command does not exist for the given criteria, create one and get it's index
            if (!renderCommandIndex)
            {
                auto rc = RenderCommand(shaderIndex, meshRenderableIndex, textureIndex);
                renderCommandIndex = this->addRenderCommand(rc);
            }

            // Add this actor's slot index to the actorIndexes container of it's RenderCommand
			size_t indexOfActorInRenderCommand = this->renderCommands->at(renderCommandIndex.value()).addActorIndex(slotIndex);

            // Now add the renderCommandIndex AND indexOfActorInRenderCommandActorIndexes to the actor's 
            // std::optional<std::pair<int, int>> renderCommand member, which is used to quickly remove the 
            // actor from the render command if it is ever removed from the stage
            actor->addRenderCommand(std::pair<size_t, size_t>(renderCommandIndex.value(), indexOfActorInRenderCommand));

        }
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
            this->renderCommands.value().at(a.getRenderCommand().first).freeActorIndex(a.getRenderCommand().second);
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

        std::vector<std::pair<size_t, size_t>> toSort;

        for (size_t i = 0; i < this->renderCommands->size(); i++) 
        {
            std::string cmdString = std::to_string(this->renderCommands->at(i).getShaderIndex()) +
                std::to_string(this->renderCommands->at(i).getMeshIndex()) +
                std::to_string(this->renderCommands->at(i).getTextureIndex());

            toSort.push_back(std::pair<size_t, size_t>(i, std::stoi(cmdString)));
        }

        std::sort(toSort.begin(), toSort.end(), [](auto &left, auto &right) {
            return left.second < right.second;
        });

        this->renderCommandsOrder->clear();

        for (auto& p : toSort) 
        {
            this->renderCommandsOrder->push_back(p.first);
        }

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