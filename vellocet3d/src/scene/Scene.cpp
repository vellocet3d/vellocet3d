#include <iostream>

#define GLM_FORCE_ALIGNED_GENTYPES
#include <glm/gtx/string_cast.hpp>
#include "imgui/imgui.h"


#include "vel/App.h"
#include "vel/scene/Scene.h"
#include "vel/scene/mesh/Vertex.h"
#include "vel/scene/mesh/Texture.h"


namespace vel
{
	Scene::Scene() :
		loaded(false),
		animationTime(0.0)
	{
		this->stages.reserve(10); // can't see ever needing more than 10 stages (naive, but good enough until it's not)

		this->animations.reserve(100); // TODO: threw this in here as temporary fix for now

		//std::cout << "scene constructing - Context:" << this->getGPU().value().getOpenGLContext() << "\n";

	}

	Scene::~Scene()
	{
		//std::cout << "scene destructing - Context:" << this->getGPU().value().getOpenGLContext() << "\n";
	}

	void Scene::applyTransformations()
	{
		for (auto& s : this->stages)
		{
			s.applyTransformations();
		}
	}

	void Scene::processSensors()
	{
		for (auto& s : this->stages)
		{
			if (s.getCollisionWorld())
			{
				s.getCollisionWorld()->processSensors();
			}
		}
	}	

	size_t Scene::addAnimation(Animation a)
	{
		this->animations.push_back(a);
		return this->animations.size() - 1;
	}

	size_t Scene::addShader(std::string name, std::string vertName, std::string fragName)
	{
		return App::get().getGPU()->loadShader(name, vertName, fragName);
	}

	Stage& Scene::addStage()
	{
		this->stages.push_back(Stage(this));

		return this->getStage(this->stages.size() - 1);
	}

	Stage& Scene::getStage(size_t index)
	{
		return this->stages.at(index);
	}

	const std::vector<Mesh>& Scene::getMeshes() const
	{
		return this->meshes;
	}

	const std::vector<Animation>& Scene::getAnimations() const
	{
		return this->animations;
	}

	size_t Scene::addMesh(Mesh m)
	{
		m.setMeshRenderableIndex(App::get().getGPU()->loadMesh(m));
		this->meshes.push_back(m);
		return this->meshes.size() - 1;
	}

	Mesh& Scene::getMesh(size_t index)
	{
		return this->meshes.at(index);
	}

	Animation& Scene::getAnimation(size_t index)
	{
		return this->animations.at(index);
	}

	void Scene::updateAnimations(double delta)
	{
		this->animationTime += delta;
		for (auto& s : this->stages)
		{
			s.updateActorAnimations(this->animationTime);
		}
	}

	void Scene::stepPhysics(float delta)
	{
		for (auto& s : this->stages)
		{
			s.stepPhysics(delta);
		}
	}

	void Scene::postPhysics(float delta){}

	void Scene::swap(Scene* sceneIn)
	{
		this->swapping = true;
		this->sceneToSwap = sceneIn;
		this->showLoadingIcon();
	}

	void Scene::showLoadingIcon()
	{
		auto screenX = (float)App::get().getScreenSize().x;
		auto screenY = (float)App::get().getScreenSize().y;

		// Set window size
		ImGui::SetNextWindowSize(ImVec2(140.0f, 51.0f));

		// Center window
		ImGui::SetNextWindowPos(
			ImVec2((screenX * 0.5f) - 70.0f, (screenY * 0.5f) - 26.0f),
			ImGuiCond_Always
		);

		// Create "loading" window
		ImGui::Begin("Loading", NULL, ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoResize
		);

		ImGui::PushFont(App::get().getImguiFont("Teko-35"));

		ImGui::Text("We Be Load'n...");

		ImGui::PopFont();

		ImGui::End();
	}

	// Use this when running into vertex bone buffer size issues
	void Scene::debugVertexBones()
	{
		int maxAttemptedVertexBoneAllocations = 0;

		for (auto& m : this->meshes)
			for (auto& v : m.getVertices())
				if (v.attemptedVertexWeightAdditions > maxAttemptedVertexBoneAllocations)
					maxAttemptedVertexBoneAllocations = v.attemptedVertexWeightAdditions;

		std::cout << maxAttemptedVertexBoneAllocations << "\n";
	}

	void Scene::debugListNumberOfBonesPerArmature()
	{
		for (auto& s : this->stages)
			s.debugListNumberOfBonesPerArmature();
	}

	void Scene::debugActiveNumberOfBonesPerActor()
	{
		for (auto& s : this->stages)
			s.debugActiveNumberOfBonesPerActor();
	}

    void Scene::draw(float alpha)
    {
		//std::cout << "new draw iteration---------------------------------------\n";

        GPU* gpu = App::get().getGPU(); // for convenience

		gpu->enableBlend();

        for (auto& s : this->stages)
        {
			if (!s.isVisible())
				continue;


			// clear depth buffer if flag set in stage
			if (s.getClearDepthBuffer())
				gpu->clearDepthBuffer();

			// should always have a camera if we've made it this far
			s.getCamera()->update();

			// if debug drawer set, do debug draw
			if (s.collisionDebugging())
			{
				s.getCollisionWorld()->getDynamicsWorld()->debugDrawWorld(); // load vertices into associated CollisionDebugDrawer

				gpu->useShader(2);
				gpu->setShaderMat4("vp", s.getCamera()->getProjectionMatrix() * s.getCamera()->getViewMatrix());
				gpu->getCollisionDebugDrawer()->draw(); // draw all loaded vertices with a single call and clear

				//std::cout << "debug draw\n";
			}

            //s.printRenderCommands();            

            // should always have a camera if we've made it this far
            //s.getCamera()->update(alpha);

			//std::cout << "----------------------------------\n";

			//std::cout << "gpu active before rco loop:" << gpu.getActiveShaderIndex() << "," << gpu.getActiveMeshRenderableIndex() << "," << gpu.getActiveTextureIndex() << "\n";

            for (auto& rco : s.getRenderCommandsOrder().value())
            {
                RenderCommand& rc = s.getRenderCommand(rco);

				//std::cout << "rc:" << rc.getShaderIndex() << "," << rc.getMeshIndex() << "," << rc.getTextureIndex() << "\n";

                if (rc.getShaderIndex() != gpu->getActiveShaderIndex())
                    gpu->useShader(rc.getShaderIndex());

                if (rc.getMeshIndex() != gpu->getActiveMeshRenderableIndex())
                    gpu->useMeshRenderable(rc.getMeshIndex());

                if (rc.getTextureIndex() != gpu->getActiveTextureIndex())
                    gpu->useTexture(rc.getTextureIndex());

                // gpu state has been set, now draw all actors which use this gpu state
                for (auto& ai : rc.getActorIndexes())
                {
                    Actor* a = s.getActor(ai);

					
					//std::cout << a->getName() << ":" << rc.getShaderIndex() << "-" << rc.getMeshIndex() << "-" << rc.getTextureIndex() << "\n";
					//std::cout << a->getName() << ":" << gpu->getActiveShaderIndex() << "-" << gpu->getActiveMeshRenderableIndex() << "-" << gpu->getActiveTextureIndex() << "\n";
					//std::cout << "--------------------------------\n";

					//std::cout << a->getName() << "\n";

                    if (!a->isDeleted() && a->isVisible())
                    {
						gpu->setShaderMat4("mvp", s.getCamera()->getProjectionMatrix() * s.getCamera()->getViewMatrix() * a->getWorldRenderMatrix(alpha));

						//std::cout << a->getName() << "\n";

						// If this actor is animated, send the bone transforms of it's armature to the shader
						if (a->isAnimated())
						{
							auto& mesh = a->getMesh();
							auto armature = a->getArmature();

							//std::cout << a->getName() << "\n";
							//for(auto& b : armature->getBones())
							//	std::cout << b.name << "\n";

							
							size_t boneIndex = 0;
							for (auto& activeBone : a->getActiveBones().value())
							{
								glm::mat4 meshBoneTransform = mesh.getGlobalInverseMatrix() * armature->getBone(activeBone.first).getRenderMatrix(alpha) * mesh.getBone(boneIndex).offsetMatrix;

								//std::cout << glm::to_string(meshBoneTransform) << "\n";
								//std::cout << activeBone.second << ":" << armature->getBone(activeBone.first).name << "\n";

								gpu->setShaderMat4(activeBone.second, meshBoneTransform);

								boneIndex++;
							}
						}

						

                        gpu->drawMeshRenderable();
                    }
                }

            }
            
        }


    }

}