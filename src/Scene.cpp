#include <iostream>
#include <fstream>

#define GLM_FORCE_ALIGNED_GENTYPES
#include <glm/gtx/string_cast.hpp>
#include "imgui/imgui.h"


#include "vel/App.h"
#include "vel/Scene.h"
#include "vel/Vertex.h"
#include "vel/Texture.h"
#include "vel/AssetLoaderV2.h"
#include "nlohmann/json.hpp"
#include "vel/CollisionObjectTemplate.h"
#include "vel/functions.h"
#include "vel/Log.h"

using json = nlohmann::json;

namespace vel
{
	Scene::Scene() :
		mainMemoryloaded(false),
		swapWhenLoaded(false),
		animationTime(0.0),
		fixedAnimationTime(0.0)
	{
		this->transparentActors.reserve(1000); // reserve space for 1000 transparent actors (won't reallocate until that limit reached)
	}
	
	Scene::~Scene()
	{
		this->freeAssets();
	}

	std::vector<Stage*>& Scene::getStages()
	{
		return this->stages.getAll();
	}

	void Scene::setName(std::string n)
	{
		this->name = n;
	}

	void Scene::addCamera(Camera c)
	{
		this->camerasInUse.push_back(App::get().getAssetManager().addCamera(c));
	}
	Camera* Scene::getCamera(std::string name)
	{
		return App::get().getAssetManager().getCamera(name);
	}


	void Scene::freeAssets()
	{
#ifdef DEBUG_LOG
	Log::toCliAndFile("Freeing assets for scene: " + this->name);
#endif
		for(auto& name : this->armaturesInUse)
			App::get().getAssetManager().removeArmature(name);
        
		for(auto& name : this->renderablesInUse)
			App::get().getAssetManager().removeRenderable(name);
		
		for(auto& name : this->materialsInUse)
			App::get().getAssetManager().removeMaterial(name);
		
		for(auto& name : this->texturesInUse)
			App::get().getAssetManager().removeTexture(name);
		
		for(auto& name : this->meshesInUse)
			App::get().getAssetManager().removeMesh(name);
        
		for(auto& name : this->shadersInUse)
			App::get().getAssetManager().removeShader(name);

		for (auto cw : this->collisionWorlds.getAll())
			delete cw;

	}

	bool Scene::isFullyLoaded()
	{
		if (!this->mainMemoryloaded)
			return false;

		// there is for sure a better way to handle this, but for the time being
		// it is what it is until I at least get a working build again

		for (auto& s : this->shadersInUse)
			if (!App::get().getAssetManager().shaderIsGpuLoaded(s))
				return false;
		
		for (auto& m : this->meshesInUse)
			if (!App::get().getAssetManager().meshIsGpuLoaded(m))
				return false;

		for (auto& t : this->texturesInUse)
			if (!App::get().getAssetManager().textureIsGpuLoaded(t))
				return false;

		return true;
	}

	CollisionWorld* Scene::addCollisionWorld(std::string name, float gravity)
	{
		// for some reason CollisionWorld has to be a pointer or bullet has read access violation issues
		// delete in destructor
		CollisionWorld* cw = new CollisionWorld(gravity);
		this->collisionWorlds.insert(name, cw);

		return cw;
	}

	CollisionWorld* Scene::getCollisionWorld(std::string name)
	{
		return this->collisionWorlds.get(name);
	}

	void Scene::loadShader(std::string name, std::string vertFile, std::string fragFile)
	{
		this->shadersInUse.push_back(App::get().getAssetManager().loadShader(name, vertFile, fragFile));
	}
	
	void Scene::loadMesh(std::string path)
	{
		auto tts = App::get().getAssetManager().loadMesh(path);

		for (auto& t : tts.first)
			this->meshesInUse.push_back(t);
			
		if(tts.second != "")
			this->armaturesInUse.push_back(tts.second);
	}
	
	void Scene::loadTexture(std::string name, std::string path)
	{
		this->texturesInUse.push_back(App::get().getAssetManager().loadTexture(name, path));
	}
	
	void Scene::addMaterial(Material m)
	{
		this->materialsInUse.push_back(App::get().getAssetManager().addMaterial(m));
	}
	
	void Scene::addRenderable(std::string name, Shader* shader, Mesh* mesh, Material material)
	{
		this->renderablesInUse.push_back(App::get().getAssetManager().addRenderable(name, shader, mesh, material));
	}

	void Scene::addRenderable(std::string name, Shader* shader, Mesh* mesh)
	{
		this->renderablesInUse.push_back(App::get().getAssetManager().addRenderable(name, shader, mesh));
	}

	Shader* Scene::getShader(std::string name)
	{
		return App::get().getAssetManager().getShader(name);
	}

	Mesh* Scene::getMesh(std::string name)
	{
		return App::get().getAssetManager().getMesh(name);
	}

	Texture* Scene::getTexture(std::string name)
	{
		return App::get().getAssetManager().getTexture(name);
	}

	Material Scene::getMaterial(std::string name)
	{
		return App::get().getAssetManager().getMaterial(name);
	}

	Renderable Scene::getRenderable(std::string name)
	{
		return App::get().getAssetManager().getRenderable(name);
	}

	Armature Scene::getArmature(std::string name)
	{
		return App::get().getAssetManager().getArmature(name);
	}

	/* Stages
	--------------------------------------------------*/
	Stage* Scene::addStage(std::string name)
	{
		return this->stages.insert(name, Stage(name));
	}

	Stage* Scene::getStage(std::string name)
	{
		return this->stages.get(name);
	}


	/* Misc
	--------------------------------------------------*/
	void Scene::updateMaterialAnimations(double frameTime)
	{
		for (auto& s : this->stages.getAll())
			for (auto am : s->animatedMaterials.getAll())
				am->materialAnimator->update(frameTime);
	}

	void Scene::updateFixedAnimations(double delta)
	{
		this->fixedAnimationTime += delta;
		for (auto& s : this->stages.getAll())
			s->updateFixedArmatureAnimations(this->fixedAnimationTime);
	}

	void Scene::updateAnimations(double delta)
	{
		this->animationTime += delta;
		for (auto& s : this->stages.getAll())
			s->updateArmatureAnimations(this->animationTime);
	}
	
	std::string Scene::getName()
	{
		return this->name;
	}

	void Scene::stepPhysics(float delta)
	{
		for(auto cw : this->collisionWorlds.getAll())
			if(cw->getIsActive())
				cw->getDynamicsWorld()->stepSimulation(delta, 0);
	}

	void Scene::postPhysics(float delta) {}

	void Scene::applyTransformations()
	{
		for (auto& s : this->stages.getAll())
			s->applyTransformations();
	}

	void Scene::processSensors()
	{
		for (auto cw : this->collisionWorlds.getAll())
			if (cw->getIsActive())
				cw->processSensors();
	}

	void Scene::draw(float alpha)
	{
		//Log::toCli("----------------------------------------------------");
		//Log::toCli("NEW RENDER PASS");
		//Log::toCli("----------------------------------------------------");

		auto gpu = App::get().getGPU(); // for convenience

		gpu->enableDepthTest(); // insure depth buffer is active
		gpu->enableBackfaceCulling(); // insure backface culling is occurring
        gpu->disableBlend(); // disable blending for opaque objects
			
		
		// loop through all stages
		for (auto s : this->stages.getAll())
		{
			if (!s->isVisible())
				continue;

			if (s->getClearDepthBuffer())
				gpu->clearDepthBuffer();

			for (auto c : s->getCameras())
			{
				// update stage camera (view/projection matrices), update scene's camera data to this stage's camera data
				c->update();
				this->cameraPosition = c->getPosition();
				this->cameraProjectionMatrix = c->getProjectionMatrix();
				this->cameraViewMatrix = c->getViewMatrix();


				// setup gl state to render to framebuffer
				gpu->setRenderTarget(c->getRenderTarget()->FBO, true); // should always write to depth buffer here
				gpu->updateViewportSize(c->getViewportSize().x, c->getViewportSize().y);

				//std::cout << "VPSIZE:" << c->getViewportSize().x << "," << c->getViewportSize().y << std::endl;

				
				// loop through all renderables and build a vector of actors which use an alpha channel, and draw all
				// completely opaque actors
				this->transparentActors.clear();

				for (auto r : s->getRenderables())
				{
					//if (r->getMaterialHasAlpha())
					if(r->getMaterial().has_value() && r->getMaterial()->hasAlphaChannel)
					{
						for (auto a : r->actors.getAll())
						{
							float dist = glm::length(this->cameraPosition - a->getTransform().getTranslation());
							this->transparentActors.push_back(std::pair<float, Actor*>(dist, a));
						}
						continue;
					}

					// DRAW OPAQUES

					// RESET GPU STATE FOR THIS RENDERABLE
					gpu->useShader(r->getShader());
					gpu->useMesh(r->getMesh());

					if(r->getMaterial().has_value())
						gpu->useMaterial(&r->getMaterial().value());

					for (auto a : r->actors.getAll())
					{
						// if this actor has a color with a transparent component less than fully opaque, push it onto
						// transparentActors queue and continue the loop without drawing the actor
						if ((a->getColor().w < 1.0f) || (a->getMaterial().has_value() && a->getMaterial()->hasAlphaChannel))
						{
							float dist = glm::length(this->cameraPosition - a->getTransform().getTranslation());
							this->transparentActors.push_back(std::pair<float, Actor*>(dist, a));
							continue;
						}

						this->drawActor(a, alpha);
					}
				}

				// DRAW TRANSPARENTS/TRANSLUCENTS
				gpu->enableBlend2();

				// not proud of this, but it gets the job done for the time being, loop through all transparent actors and sort by their distance
				// from the current camera position
				std::sort(transparentActors.begin(), transparentActors.end(), [](auto &left, auto &right) {
					return left.first < right.first;
				});

				// Draw all transparent/translucent actors
				for (std::vector<std::pair<float, Actor*>>::reverse_iterator it = transparentActors.rbegin(); it != transparentActors.rend(); ++it)
				{
					// Reset gpu state for this ACTOR and draw
					auto r = it->second->getStageRenderable().value();

					// TODO: have to reset entire state for each actor since they have to be rendered in order from
					// distance to camera...need to come up with a better solution to this

					gpu->useShader(r->getShader());
					gpu->useMesh(r->getMesh());

					if (r->getMaterial().has_value())
						gpu->useMaterial(&r->getMaterial().value());

					this->drawActor(it->second, alpha);
				}

				// clear buffers for this render target
				//gpu->clearBuffers(0.0f, 0.0f, 0.0f, 1.0f);


			} // end for each camera

		} // end for each stage


		// all stage camera's framebuffers are now updated, loop through each stage camera and check if it should display it's contents to
		// screen, if so do so...

		// now bind back to default framebuffer and draw a quad plane with the attached framebuffer texture
		// disable depth test so screen-space quad isn't discarded due to depth test. ????
		gpu->setRenderTarget(0, false);
		gpu->enableBlend2();
		for (auto s : this->stages.getAll())
		{
			if (!s->isVisible())
				continue;

			for (auto c : s->getCameras())
			{
				if (c->isFinalRenderCam())
				{
					gpu->updateViewportSize(c->getViewportSize().x, c->getViewportSize().y); // TODO: can probably do this outside of this loop since this render is the screen size
					gpu->drawScreen(c->getRenderTarget()->texture.frames.at(0).dsaHandle);
				}
			}
		}

		// clear buffers for this render target
		//gpu->clearBuffers(0.0f, 0.0f, 0.0f, 1.0f);



		// moving collision debug draw event as final thing as it draws directly to the screen buffer, and I don't want to have to think about updating
		// it right now
#ifdef DEBUG_LOG
		for (auto cw : this->collisionWorlds.getAll())
		{
			if (cw->getIsActive() && cw->getDebugDrawer() != nullptr)
			{
				cw->getDynamicsWorld()->debugDrawWorld(); // load vertices into associated CollisionDebugDrawer
				gpu->useShader(cw->getDebugDrawer()->getShaderProgram());
				gpu->setShaderMat4("vp", cw->getCamera()->getProjectionMatrix() * cw->getCamera()->getViewMatrix());
				gpu->debugDrawCollisionWorld(cw->getDebugDrawer()); // draw all loaded vertices with a single call and clear
			}
		}
#endif


	}

	void Scene::drawActor(Actor* a, float alphaTime)
	{
		auto gpu = App::get().getGPU();

		if (a->isVisible())
		{
			if (a->getMaterial().has_value())
				gpu->useMaterial(&a->getMaterial().value());

			gpu->setShaderVec4("color", a->getColor());
			gpu->setShaderMat4("mvp", this->cameraProjectionMatrix * this->cameraViewMatrix * a->getWorldRenderMatrix(alphaTime));

			// If this actor is animated, send the bone transforms of it's armature to the shader
			if (a->isAnimated())
			{
				auto mesh = a->getMesh();
				auto armature = a->getArmature();
				bool armInterp = armature->getShouldInterpolate();

				size_t boneIndex = 0;
				std::vector<std::pair<unsigned int, glm::mat4>> boneData;

				glm::mat4 meshBoneTransform;
				for (auto& activeBone : a->getActiveBones())
				{
					if(armInterp)
						// global inverse matrix does not seem to make any difference
						//meshBoneTransform = mesh->getGlobalInverseMatrix() * armature->getBone(activeBone.first).getRenderMatrixInterpolated(alphaTime) * mesh->getBone(boneIndex).offsetMatrix;
						meshBoneTransform = armature->getBone(activeBone.first).getRenderMatrixInterpolated(alphaTime) * mesh->getBone(boneIndex).offsetMatrix;
					else
						meshBoneTransform = armature->getBone(activeBone.first).getRenderMatrix() * mesh->getBone(boneIndex).offsetMatrix;

					boneData.push_back(std::pair<unsigned int, glm::mat4>(activeBone.second, meshBoneTransform));
					boneIndex++;
				}

				gpu->updateBonesUBO(boneData);
			}

			gpu->drawGpuMesh();
		}
	}

	void Scene::clearAllRenderTargetBuffers()
	{
		auto gpu = App::get().getGPU();

		for (auto s : this->stages.getAll())
		{
			for (auto c : s->getCameras())
			{
				gpu->setRenderTarget(c->getRenderTarget()->FBO, true);
				gpu->clearBuffers(0.0f, 0.0f, 0.0f, 0.0f);
			}
		}

		// clear default screen buffer
		gpu->setRenderTarget(0, false);
		//gpu->clearBuffers(0.0f, 0.0f, 0.0f, 0.1f);
		gpu->clearBuffers(1.0f, 1.0f, 0.0f, 1.0f);

	}

// END VEL NAMESPACE
}