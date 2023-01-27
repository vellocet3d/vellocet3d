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
#include "vel/AssetTrackers.h"
#include "vel/CollisionWorld.h"
#include "vel/CollisionDebugDrawer.h"
#include "vel/FontBitmap.h"
#include "vel/FontGlyphInfo.h"


namespace vel
{
	class Scene
	{
	private:
		sac<Stage>							stages;
		ptrsac<CollisionWorld*> 			collisionWorlds;
		double								fixedAnimationTime;
		double								animationTime;

		std::vector<std::string>			camerasInUse;
		std::vector<std::string>			shadersInUse;
		std::vector<std::string>			meshesInUse;
		std::vector<std::string> 			texturesInUse;
		std::vector<std::string> 			fontBitmapsInUse;
		std::vector<std::string> 			materialsInUse;
		std::vector<std::string> 			renderablesInUse;
		std::vector<std::string>			armaturesInUse;
		
		void								freeAssets();
		void								drawActor(Actor* a, float alphaTime);
		std::vector<std::pair<float, Actor*>> transparentActors;

		glm::vec3							cameraPosition;
		glm::mat4							cameraProjectionMatrix;
		glm::mat4							cameraViewMatrix;

		std::string							name = "";


		// these used to be protected before we started working on level builders where we needed to be able to generate
		// and add things to scenes from outside of a child scene
		//TODO: can these be set back to protected?
	//protected:
	public:
		
		

		void								loadShader(std::string name, std::string vertFile, std::string fragFile);
		void								loadMesh(std::string path);
		void								loadTexture(std::string name, std::string path);
		void								loadFontBitmap(FontBitmap fb);

		void								addCamera(Camera m);
		void								addMaterial(Material m);
		void								addRenderable(std::string name, Shader* shader, Mesh* mesh, Material material);
		void								addRenderable(std::string name, Shader* shader, Mesh* mesh);
		Stage*								addStage(std::string name);

		Shader*								getShader(std::string name);
		Mesh*								getMesh(std::string name);
		Texture*							getTexture(std::string name);
		FontBitmap*							getFontBitmap(std::string name);
		Camera*								getCamera(std::string name);
		Material							getMaterial(std::string name);
		Renderable							getRenderable(std::string name);
		Armature							getArmature(std::string name);
		
		Stage*								getStage(std::string name);
		std::vector<Stage*>&				getStages();


	//public:
		Scene();
		~Scene();
		virtual void						load() = 0;
		virtual void						innerLoop(float deltaTime) = 0;
		virtual void						outerLoop(float frameTime, float renderLerpInterval) = 0;
		virtual void						postPhysics(float deltaTime);

		// TODO: why are these public? probably being lazy and didn't want to write getters/setters
		bool								mainMemoryloaded;
		bool								swapWhenLoaded;
		//////////////
		
		void								setName(std::string n);
		std::string							getName();
		bool								isFullyLoaded();


		void								updateFixedAnimations(double runTime);
		void								updateAnimations(double frameTime);
		void								draw(float alpha);
		void								stepPhysics(float delta);
		void								updatePreviousTransforms();

		CollisionWorld*						addCollisionWorld(std::string name, float gravity = -10.0f);
		CollisionWorld*						getCollisionWorld(std::string name);

		void								clearAllRenderTargetBuffers();

		void								updateMaterialAnimations(double frameTime);

		
		Mesh								generateTextActorMesh(TextActor* ta);
		void								updateTextActors();

		TextActor*							addTextActor(Stage* stage, std::string name, std::string theText, FontBitmap* fb, 
												TextActorAlignment alignment = TextActorAlignment::LEFT_ALIGN, 
												glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), bool queue = false);

		

	};

}