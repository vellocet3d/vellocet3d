#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>

#include "vel/scene/mesh/Mesh.h"
#include "vel/scene/stage/Stage.h"
#include "vel/scene/armature/Armature.h"
#include "vel/scene/animation/Animation.h"
#include "vel/scene/material/Material.h"



namespace vel
{
	class Scene
	{
	private:
		std::vector<Stage>					stages;
		std::vector<Mesh>					meshes;
		std::vector<Material>				materials;
		std::vector<Animation>				animations;
		double								animationTime;
		

	protected:
		size_t								loadShader(std::string name, std::string vertName, std::string fragName);
		size_t								loadMesh(std::string path);
		size_t								loadTexture(std::string name, std::string type, std::string path, std::vector<std::string> mips);
		size_t								addMaterial(Material m);
		Stage&								addStage();
		Stage&								getStage(size_t index);


	public:
											Scene();
											~Scene();
		bool								loaded;
		const std::vector<Mesh>&			getMeshes() const;
		size_t								addMesh(Mesh m);
		Mesh&								getMesh(size_t index);
		size_t								addAnimation(Animation a);
		const std::vector<Animation>&		getAnimations() const;
		Animation&							getAnimation(size_t index);
		void								updateAnimations(double runTime);
		void								draw(float alpha);
		void								stepPhysics(float delta);
		void								applyTransformations();
		void								processSensors();

		virtual void						load() = 0;
		virtual void						innerLoop(float deltaTime) = 0;
		virtual void						outerLoop(float frameTime, float renderLerpInterval) = 0;
		virtual void						postPhysics(float deltaTime);

		bool								swapping = false;
		void								swap(Scene* scene);
		Scene*								sceneToSwap = nullptr;
		virtual void						showLoadingIcon();
		
		void debugVertexBones();
		void debugListNumberOfBonesPerArmature();
		void debugActiveNumberOfBonesPerActor();

		const Material&						getMaterial(size_t materialIndex);
		const Material&						getMaterial(std::string materialName);

	};

}