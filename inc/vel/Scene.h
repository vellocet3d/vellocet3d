#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>

#include "vel/sac.h"

#include "vel/Camera.h"
#include "vel/Mesh.h"
#include "vel/Stage.h"
#include "vel/Armature.h"
#include "vel/Animation.h"
#include "vel/Material.h"
#include "vel/Cubemap.h"
#include "vel/AssetTrackers.h"

namespace vel
{
	class Scene
	{
	private:
		Camera								camera;
		Cubemap*							activeInfiniteCubemap;
		bool								drawSkybox;
		sac<Stage>							stages;
		double								fixedAnimationTime;
		double								animationTime;


		std::vector<std::string>			shadersInUse;
		std::vector<std::string>			meshesInUse;
		std::vector<std::string> 			texturesInUse;
        std::vector<std::string>            infiniteCubemapsInUse;
		std::vector<std::string> 			materialsInUse;
		std::vector<std::string> 			renderablesInUse;
		std::vector<std::string>			armaturesInUse;
		
		void								freeAssets();
		void								drawActor(Actor* a, float alphaTime);
		std::vector<std::pair<float, Actor*>> sortedTransparentActors;

		glm::vec3							cameraPosition;
		glm::mat4							cameraProjectionMatrix;
		glm::mat4							cameraViewMatrix;

		glm::vec3							renderCameraPosition;
		glm::mat4							renderCameraOffset;

		std::string							name = "";

	protected:
		
		

		void								loadShader(std::string name, std::string vertFile, std::string fragFile);
		void								loadMesh(std::string path);
		void								loadTexture(std::string name, std::string type, std::string path, std::vector<std::string> mips = std::vector<std::string>());
		void                                loadInfiniteCubemap(std::string name, std::string path);
        void								loadConfigFile(std::string path);

		void								addMaterial(Material m);
		void								addRenderable(std::string name, Shader* shader, Mesh* mesh, Material* material);
		Stage*								addStage(std::string name);

		Shader*								getShader(std::string name);
        Cubemap*							getInfiniteCubemap(std::string name);
		Mesh*								getMesh(std::string name);
		Texture*							getTexture(std::string name);
		Material*							getMaterial(std::string name);
		Renderable							getRenderable(std::string name);
		Armature							getArmature(std::string name);
		
		Stage*								getStage(std::string name);


	public:
		Scene();
		~Scene();
		virtual void						load() = 0;
		virtual void						innerLoop(float deltaTime) = 0;
		virtual void						outerLoop(float frameTime, float renderLerpInterval) = 0;
		virtual void						postPhysics(float deltaTime);

		// TODO: why are these public?
		bool								mainMemoryloaded;
		bool								swapWhenLoaded;
		//////////////
		
		void								setName(std::string n);
		std::string							getName();
		bool								isFullyLoaded();
		
		Camera*								getCamera();

		void 								setActiveInfiniteCubemap(Cubemap* c);
		Cubemap* 							getActiveInfiniteCubemap();
		void								setDrawSkybox(bool b);
		bool								getDrawSkybox();

		void								updateFixedAnimations(double runTime);
		void								updateAnimations(double frameTime);
		void								draw(float alpha);
		void								stepPhysics(float delta);
		void								applyTransformations();
		void								processSensors();

	};

}