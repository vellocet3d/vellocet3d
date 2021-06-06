#pragma once

#include <vector>
#include <string>
#include <optional>
#include <memory>

#include "glm/glm.hpp"

#include "vel/scene/stage/Actor.h"
#include "vel/scene/stage/Camera.h"
#include "vel/scene/Renderable.h"
#include "vel/scene/stage/CollisionWorld.h"
#include "vel/CollisionDebugDrawer.h"
#include "vel/scene/stage/Sensor.h"
#include "vel/GPU.h"


namespace vel
{
	class Scene;

	class Stage
	{
	private:

		bool											visible;
		std::optional<Camera>							camera;
		std::vector<Actor>								actors;
		std::vector<size_t>								actorFreeSlots;
		std::vector<Armature>							armatures;
		std::vector<Renderable>							renderables;
		std::vector<size_t>								renderablesOrder;
		std::optional<size_t>							renderableExists(const std::string& rn);
		size_t											addRenderable(Renderable rc);
		std::optional<std::unique_ptr<CollisionWorld>>	collisionWorld;
		bool											collisionDebuggingSwitch;
		bool											clearDepthBuffer;





	public:
		Stage();

		void										updateActorAnimations(double runTime);
		size_t										addActor(Actor a);
		void										removeActor(std::string name);
		void										removeActor(size_t index);
		Actor*										getActor(size_t index);
		Actor*										getActor(std::string name);
		std::vector<Actor>&							getActors();
		const size_t								getActorSize() const;

		void										printRenderables() const;
		Renderable&									getRenderable(size_t index);
		std::optional<Renderable>&					getRenderable(std::string name);
		const std::vector<size_t>&					getRenderablesOrder() const;


		std::optional<Camera>&						getCamera();
		const bool									hasActorWithName(std::string name) const;
		void										setActorCapacity(size_t size);
		void										setArmatureCapacity(size_t size);
		void										show();
		void										hide();
		void										addPerspectiveCamera(float nearPlane, float farPlane, float fov);
		void										addOrthographicCamera(float nearPlane, float farPlane, float scale);
		const bool									isVisible();
		void										parentActorToActor(std::string childName, std::string parentName);
		void										parentActorToActorBone(std::string childName, std::string parentName, std::string parentBoneName);
		void										setClearDepthBuffer(bool b);
		bool										getClearDepthBuffer();


		void										applyTransformations();



		void										setCollisionWorld(float gravity = -10.0f);
		CollisionWorld*								getCollisionWorld();

		void										stepPhysics(float delta);
		void										useCollisionDebugDrawer(int debugMode = 1);
		bool										collisionDebugging();


		Armature*									addArmature(Armature a, std::string defaultAnimation, std::vector<std::string> actors);

		void debugListNumberOfBonesPerArmature();
		void debugActiveNumberOfBonesPerActor();

	};
}