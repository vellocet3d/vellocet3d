#pragma once

#include <vector>
#include <string>
#include <optional>
#include <memory>

#include "vel/scene/stage/Actor.h"
#include "vel/scene/stage/Camera.h"
#include "vel/scene/stage/RenderCommand.h"
#include "vel/scene/stage/Controller.h"


namespace vel::scene::stage
{
    class Stage
    {
    private:
        const bool                                  headless;
        bool                                        visible;
        std::optional<Camera>                       camera;
        std::vector<Actor>                          actors;
        std::vector<size_t>                         actorFreeSlots;
        std::optional<std::vector<RenderCommand>>   renderCommands;
        std::optional<std::vector<size_t>>          renderCommandsOrder;
		std::optional<size_t>                       renderCommandExists(size_t sI, size_t mI, size_t tI);
        size_t                                      addRenderCommand(RenderCommand rc);
		std::vector<std::unique_ptr<Controller>>	controllers;

    public:
													Stage(bool headless);
        void										loadActors(std::string filename, bool dynamic = false);
        void										loadActors(std::string filename, bool dynamic, int shaderIndex);
        void										loadActors(std::string filename, bool dynamic, std::vector<std::pair<int, std::vector<std::string>>> actorShaderAssocs);
		void										updateActorAnimations(double runTime);
        void										addActor(Actor a);
        void										removeActor(std::string name);
        void										removeActor(size_t index);
        Actor*										getActor(size_t index);
        Actor*										getActor(std::string name);
        const size_t								getActorSize() const;
        void										printRenderCommands() const;
        RenderCommand&								getRenderCommand(size_t index);
        const std::optional<std::vector<size_t>>&	getRenderCommandsOrder() const;
        std::optional<Camera>&						getCamera();
        const bool									hasActorWithName(std::string name) const;
        void										setActorContainerSize(unsigned int size);
        void										show();
        void										hide();
        void										addPerspectiveCamera(bool fixed, float nearPlane, float farPlane, float fov);
        void										addOrthographicCamera(bool fixed, float nearPlane, float farPlane, float scale);
		const bool									isVisible();
		void										parentActorToActor(std::string childName, std::string parentName);
		void										parentActorToActorBone(std::string childName, std::string parentName, std::string parentBoneName);
		void										addController(Controller* controller);
		void										executeControllers(float deltaTime);
		void										savePreviousTransforms();

    };
}