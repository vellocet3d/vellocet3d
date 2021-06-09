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
		std::vector<Shader>					shaders;
		std::vector<Mesh>					meshes;
		std::vector<Texture>				textures;
		std::vector<Material>				materials;
		std::vector<Animation>				animations;

		std::vector<Renderable>				renderables;
		std::vector<Armature>				armatures;

		std::vector<Stage>					stages;

		double								animationTime;


	protected:
		void								setShaderCapacity(size_t cap);
		void								setMeshCapacity(size_t cap);
		void								setTextureCapacity(size_t cap);
		void								setMaterialCapacity(size_t cap);
		void								setAnimationCapacity(size_t cap);
		void								setRenderableCapacity(size_t cap);
		void								setArmatureCapacity(size_t cap);
		void								setStageCapacity(size_t cap);

		size_t								loadShader(std::string name, std::string vertFile, std::string fragFile);
		void								loadMesh(std::string path);
		size_t								loadTexture(std::string name, std::string type, std::string path, std::vector<std::string> mips = std::vector<std::string>());
		void								loadSceneConfig(std::string path);
		size_t								addMaterial(Material m);
		size_t								addRenderable(std::string name, size_t defaultShaderIndex, size_t crateMeshIndex, size_t crateMaterialIndex);
		Renderable							getRenderable(std::string name);
		Armature							getArmature(std::string name);
		Stage&								addStage();
		Stage&								getStage(size_t index);




	public:
		Scene();
		~Scene();
		bool								loaded;
		const std::vector<Mesh>&			getMeshes() const;
		size_t								addMesh(Mesh m);
		Mesh*								getMesh(size_t index);
		Animation*							addAnimation(Animation a);
		std::vector<Animation>&				getAnimations(); //TODO - I really REALLY need to go back through this whole project and properly use const
		Animation&							getAnimation(size_t index); //TODO
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



		Shader*								getShader(size_t si);
		Shader*								getShader(std::string shaderName);
		Texture*							getTexture(size_t textureIndex);
		Material*							getMaterial(size_t materialIndex);
		Material*							getMaterial(std::string materialName);


		size_t								getShaderIndex(std::string shaderName);
		size_t								getTextureIndex(std::string textureName);
		size_t								getMaterialIndex(std::string materialName);
		size_t								getMeshIndex(std::string meshName);


		Armature*							addArmature(Armature a);


	};

}