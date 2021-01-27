#include <iostream>

#define GLM_FORCE_ALIGNED_GENTYPES
#include <glm/gtx/string_cast.hpp>

#include "vel/App.h"
#include "vel/scene/Scene.h"
#include "vel/scene/mesh/Vertex.h"
#include "vel/scene/mesh/Texture.h"


namespace vel::scene
{
	Scene::Scene() :
		headless(App::get().config.HEADLESS),
		loaded(false),
		animationTime(0.0),
		gpu(!this->headless ? std::make_optional<GPU>(GPU()) : std::nullopt)
	{
		this->stages.reserve(10); // can't see ever needing more than 10 stages (naive, but good enough until it's not)

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
		return this->gpu->loadShader(name, vertName, fragName);
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

	std::optional<GPU>& Scene::getGPU()
	{
		return this->gpu;
	}

	size_t Scene::addMesh(Mesh m)
	{
		if (!this->headless)
		{
			m.setMeshRenderableIndex(this->gpu->loadMesh(m));
		}
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

    void Scene::draw(float alpha)
    {
        GPU& gpu = this->gpu.value(); // for convenience

        for (auto& s : this->stages)
        {
			if (!s.isVisible())
			{
				continue;
			}

			// clear depth buffer if flag set in stage
			if (s.getClearDepthBuffer())
			{
				gpu.clearDepthBuffer();
			}

			// if debug drawer set, do debug draw
			if (s.collisionDebugging())
			{
				s.getCollisionWorld()->getDynamicsWorld()->debugDrawWorld(); // load vertices into associated CollisionDebugDrawer

				gpu.useShader(2);
				gpu.setShaderMat4("vp", s.getCamera()->getProjectionMatrix() * s.getCamera()->getViewMatrix());
				gpu.getCollisionDebugDrawer()->draw(); // draw all loaded vertices with a single call and clear
			}

            //s.printRenderCommands();            

            // should always have a camera if we've made it this far
            s.getCamera()->update(alpha);

			//std::cout << "----------------------------------\n";

            for (auto& rco : s.getRenderCommandsOrder().value())
            {
                RenderCommand& rc = s.getRenderCommand(rco);

                if (rc.getShaderIndex() != gpu.getActiveShaderIndex())
                {
                    gpu.useShader(rc.getShaderIndex());
                }

                if (rc.getMeshIndex() != gpu.getActiveMeshRenderableIndex())
                {
                    gpu.useMeshRenderable(rc.getMeshIndex());
                }

                if (rc.getTextureIndex() != gpu.getActiveTextureIndex())
                {
                    gpu.useTexture(rc.getTextureIndex());
                }

                // gpu state has been set, now draw all actors which use this gpu state
                for (auto& ai : rc.getActorIndexes())
                {
                    Actor* a = s.getActor(ai);

					//std::cout << a->getName() << ":" << rc.getShaderIndex() << "-" << rc.getMeshIndex() << "-" << rc.getTextureIndex() << "\n";

                    if (!a->isDeleted() && a->isVisible())
                    {
						gpu.setShaderMat4("mvp", s.getCamera()->getProjectionMatrix() * s.getCamera()->getViewMatrix() * a->getWorldRenderMatrix(alpha));

						// If this actor is animated, send the bone transforms of it's armature to the shader
						if (a->isAnimated())
						{
							auto& mesh = a->getMesh();
							auto armature = a->getArmature();
							
							size_t boneIndex = 0;
							for (auto& activeBone : a->getActiveBones().value())
							{
								glm::mat4 meshBoneTransform = mesh.getGlobalInverseMatrix() * armature->getBone(activeBone.first).getRenderMatrix(alpha) * mesh.getBone(boneIndex).offsetMatrix;

								gpu.setShaderMat4(activeBone.second, meshBoneTransform);

								boneIndex++;
							}
						}

                        gpu.drawMeshRenderable();
                    }
                }

            }
            
        }


    }

}