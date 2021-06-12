#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>

#include "vel/assets/mesh/Mesh.h"
#include "vel/scene/stage/Stage.h"
#include "vel/assets/armature/Armature.h"
#include "vel/assets/animation/Animation.h"
#include "vel/assets/material/Material.h"



namespace vel
{
	class Scene
	{
	private:
		std::vector<Stage>					stages;

		double								animationTime;


	protected:
		void								loadSceneConfig(std::string path);
		Stage&								addStage();
		Stage&								getStage(size_t index);


	public:
		Scene();
		~Scene();
		virtual void						load() = 0;
		virtual void						innerLoop(float deltaTime) = 0;
		virtual void						outerLoop(float frameTime, float renderLerpInterval) = 0;
		virtual void						postPhysics(float deltaTime);

		bool								loaded;
		


		void								updateAnimations(double runTime);
		void								draw(float alpha);
		void								stepPhysics(float delta);
		void								applyTransformations();
		void								processSensors();

	};

}