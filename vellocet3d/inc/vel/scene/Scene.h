#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>

#include "dep/plf_colony.h"

#include "vel/assets/mesh/Mesh.h"
#include "vel/scene/stage/Stage.h"
#include "vel/assets/armature/Armature.h"
#include "vel/assets/animation/Animation.h"
#include "vel/assets/material/Material.h"

#include "vel/assets/AssetTrackers.h"

namespace vel
{
	class Scene
	{
	private:
		std::vector<Stage>					stages;
		double								animationTime;


		std::vector<std::string>			shadersInUse;
		std::vector<std::string>			meshesInUse;
		std::vector<std::string> 			texturesInUse;
		std::vector<std::string> 			materialsInUse;
		std::vector<std::string> 			renderablesInUse;
		std::vector<std::string>			armaturesInUse;
		
		void								freeAssets();

	protected:
		std::string							name = "default";
		

		void								loadShader(std::string name, std::string vertFile, std::string fragFile);
		void								loadMesh(std::string path);
		void								loadTexture(std::string name, std::string type, std::string path, std::vector<std::string> mips = std::vector<std::string>());
		//void								loadSceneConfig(std::string path);

		void								addMaterial(Material m);
		void								addRenderable(std::string name, Shader* shader, Mesh* mesh, Material* material);
		Stage&								addStage();

		Shader*								getShader(std::string n);
		Mesh*								getMesh(std::string n);
		Texture*							getTexture(std::string n);
		Material*							getMaterial(std::string n);

		Renderable							getRenderable(std::string name);
		Armature							getArmature(std::string name);
		Stage&								getStage(size_t index);


	public:
		Scene();
		~Scene();
		virtual void						load() = 0;
		virtual void						innerLoop(float deltaTime) = 0;
		virtual void						outerLoop(float frameTime, float renderLerpInterval) = 0;
		virtual void						postPhysics(float deltaTime);

		bool								mainMemoryloaded;
		bool								swapWhenLoaded;
		std::string							getName();
		bool								isFullyLoaded();
		


		void								updateAnimations(double runTime);
		void								draw(float alpha);
		void								stepPhysics(float delta);
		void								applyTransformations();
		void								processSensors();

	};

}