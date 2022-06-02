#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>

#include "vel/sac.h"
#include "vel/ptrsac.h"

#include "vel/Camera.h"
#include "vel/Mesh.h"
#include "vel/Stage.h"
#include "vel/Armature.h"
#include "vel/Animation.h"
#include "vel/Material.h"
#include "vel/Cubemap.h"
#include "vel/AssetTrackers.h"
#include "vel/CollisionWorld.h"
#include "vel/CollisionDebugDrawer.h"


namespace vel
{
	class Scene
	{
	private:
		sac<Camera>							cameras;
		Camera*								sceneCamera;
		Cubemap*							activeInfiniteCubemap;
		bool								drawSkybox;
		sac<Stage>							stages;
		ptrsac<CollisionWorld*> 			collisionWorlds;
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
		
		// TODO: some of these should probably be protected

		void								setSceneCamera(Camera* c);
		Camera*								getSceneCamera(); // get Camera assigned to this scene

		Camera*								addCamera(std::string name, Camera c);
		Camera*								getCamera(std::string name); // get Camera pointer from cameras sac

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

		CollisionWorld*						addCollisionWorld(std::string name, float gravity = -10.0f);
		CollisionWorld*						getCollisionWorld(std::string name);

	};

}