#pragma once

#include <vector>
#include <string>
#include <optional>
#include <memory>

#include "glm/glm.hpp"

#include "vel/scene/stage/Actor.h"
#include "vel/scene/stage/Camera.h"
#include "vel/scene/stage/RenderCommand.h"
#include "vel/scene/stage/CollisionWorld.h"
#include "vel/CollisionDebugDrawer.h"
#include "vel/scene/stage/Sensor.h"
#include "vel/GPU.h"

namespace vel::scene
{
	class Scene;
}

namespace vel::scene::stage
{
    class Stage
    {
    private:
        
        bool											visible;
        std::optional<Camera>							camera;
        std::vector<Actor>								actors;
        std::vector<size_t>								actorFreeSlots;
		std::vector<vel::scene::armature::Armature>		armatures;
        std::optional<std::vector<RenderCommand>>		renderCommands;
        std::optional<std::vector<size_t>>				renderCommandsOrder;
		std::optional<size_t>							renderCommandExists(size_t sI, size_t mI, size_t tI);
        size_t											addRenderCommand(RenderCommand rc);
		std::optional<std::unique_ptr<CollisionWorld>>	collisionWorld;
		bool											collisionDebuggingSwitch;
		bool											clearDepthBuffer;

		vel::scene::Scene*								parentScene;
		GPU*											sceneGPU;
		


    public:
													Stage(vel::scene::Scene* parentScene);
		std::vector<size_t>							loadActors(std::string filename, bool dynamic = false);
		std::vector<size_t>							loadActors(std::string filename, bool dynamic, int shaderIndex);
		std::vector<size_t>							loadActors(std::string filename, bool dynamic, std::vector<std::pair<int, std::vector<std::string>>> actorShaderAssocs);
		void										updateActorAnimations(double runTime);
        size_t										addActor(Actor a);
        void										removeActor(std::string name);
        void										removeActor(size_t index);
        Actor*										getActor(size_t index);
        Actor*										getActor(std::string name);
		std::vector<Actor>&							getActors();
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
		void										setClearDepthBuffer(bool b);
		bool										getClearDepthBuffer();
		

		void										applyTransformations();
		


		void										setCollisionWorld(float gravity = -10.0f);
		CollisionWorld*								getCollisionWorld();

		void										stepPhysics(float delta);
		void										useCollisionDebugDrawer();
		bool										collisionDebugging();

		GPU*										getSceneGPU();
		vel::scene::Scene*							getParentScene();

		vel::scene::armature::Armature*				addArmature(vel::scene::armature::Armature a);

    };
}