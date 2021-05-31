#include <iostream>

#define GLM_FORCE_ALIGNED_GENTYPES
#include <glm/gtx/string_cast.hpp>
#include "imgui/imgui.h"


#include "vel/App.h"
#include "vel/scene/Scene.h"
#include "vel/scene/mesh/Vertex.h"
#include "vel/scene/material/Texture.h"
#include "vel/scene/AssetLoaderV2.h"


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
			s.applyTransformations();
	}

	void Scene::processSensors()
	{
		for (auto& s : this->stages)
			if (s.getCollisionWorld())
				s.getCollisionWorld()->processSensors();
	}	

	size_t Scene::addAnimation(Animation a)
	{
		this->animations.push_back(a);
		return this->animations.size() - 1;
	}

	size_t Scene::loadShader(std::string name, std::string vertFile, std::string fragFile)
	{
		Shader s;
		s.name = name;
		s.vertFile = vertFile;
		s.fragFile = fragFile;

		App::get().getGPU()->loadShader(s);

		this->shaders.push_back(s);
		
		return this->shaders.size() - 1;
	}

	size_t Scene::loadTexture(std::string name, std::string type, std::string path, std::vector<std::string> mips)
	{
		Texture texture;
		texture.name = name;
		texture.type = type;
		texture.path = path;
		texture.mips = mips;

		App::get().getGPU()->loadTexture(texture);

		this->textures.push_back(texture);

		return this->textures.size() - 1;
	}

	Texture* Scene::getTexture(size_t textureIndex)
	{
		return &this->textures.at(textureIndex);
	}

	void Scene::loadMesh(std::string path) 
	{
		auto al = AssetLoaderV2(this, path);
		al.load();
	}

	size_t Scene::addMaterial(Material m)
	{
		this->materials.push_back(m);

		return this->materials.size() - 1;
	}

	Material*	Scene::getMaterial(size_t materialIndex)
	{
		return &this->materials.at(materialIndex);
	}

	Material* Scene::getMaterial(std::string materialName)
	{
		for (auto& m : this->materials)
			if (m.name == materialName)
				return &m;
	}

	Renderable Scene::getRenderable(std::string name)
	{
		for (auto& r : this->baseRenderables)
			if (r.getName() == name)
				return r;

		std::cout << "Scene: attempting to get renderable by name that does not exist\n";
		std::cin.get();
		exit(EXIT_FAILURE);
	}

	Armature* Scene::addArmature(Armature a)
	{
		this->baseArmatures.push_back(a);

		return &this->baseArmatures.back();
	}

	Stage& Scene::addStage()
	{
		this->stages.push_back(Stage());

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
		App::get().getGPU()->loadMesh(m);
		this->meshes.push_back(m);
		return this->meshes.size() - 1;
	}

	Mesh* Scene::getMesh(size_t index)
	{
		return &this->meshes.at(index);
	}

	Animation& Scene::getAnimation(size_t index)
	{
		return this->animations.at(index);
	}

	void Scene::updateAnimations(double delta)
	{
		this->animationTime += delta;
		for (auto& s : this->stages)
			s.updateActorAnimations(this->animationTime);
	}

	void Scene::stepPhysics(float delta)
	{
		for (auto& s : this->stages)
			s.stepPhysics(delta);
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

	size_t Scene::addRenderable(std::string name, size_t shaderIndex, size_t meshIndex, size_t materialIndex)
	{
		Renderable r = Renderable(name, shaderIndex, meshIndex, materialIndex, 
			&this->shaders.at(shaderIndex), &this->meshes.at(meshIndex), &this->materials.at(materialIndex),
			this->materials.at(materialIndex).hasAlphaChannel);

		this->baseRenderables.push_back(r);

		return this->baseRenderables.size() - 1;
	}

	size_t Scene::getTextureGpuId(std::string textureName)
	{
		for (int i = 0; i < this->textures.size(); i++)
			if (this->textures.at(i).name == textureName)
				return this->textures.at(i).id;

		std::cout << "CANNOT GET GPU ID FOR NON EXISTING TEXTURE NAME\n";
		std::cin.get();
		exit(EXIT_FAILURE);
	}

	size_t Scene::getMaterialIndex(std::string materialName)
	{
		for (int i = 0; i < this->materials.size(); i++)
			if (this->materials.at(i).name == materialName)
				return i;

		std::cout << "CANNOT GET MATERIAL INDEX FOR NON EXISTING MATERIAL NAME\n";
		std::cin.get();
		exit(EXIT_FAILURE);
	}

	size_t Scene::getShaderIndex(std::string shaderName)
	{
		for (int i = 0; i < this->shaders.size(); i++)
			if (this->shaders.at(i).name == shaderName)
				return i;

		std::cout << "CANNOT GET SHADER INDEX FOR NON EXISTING SHADER NAME\n";
		std::cin.get();
		exit(EXIT_FAILURE);
	}

	size_t Scene::getMeshIndex(std::string meshName)
	{
		for (int i = 0; i < this->meshes.size(); i++)
			if (this->meshes.at(i).getName() == meshName)
				return i;

		std::cout << "CANNOT GET MESH INDEX FOR NON EXISTING MESH NAME\n";
		std::cin.get();
		exit(EXIT_FAILURE);
	}

	Shader* Scene::getShader(size_t si)
	{
		return &this->shaders.at(si);
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

			// TODO - RETHINK HOW WE WANT TO HANDLE DEBUG DRAWING
			//// if debug drawer set, do debug draw
			//if (s.collisionDebugging())
			//{
			//	s.getCollisionWorld()->getDynamicsWorld()->debugDrawWorld(); // load vertices into associated CollisionDebugDrawer

			//	gpu->useShader(2);
			//	gpu->setShaderMat4("vp", s.getCamera()->getProjectionMatrix() * s.getCamera()->getViewMatrix());
			//	gpu->getCollisionDebugDrawer()->draw(); // draw all loaded vertices with a single call and clear

			//	//std::cout << "debug draw\n";
			//}

            //s.printRenderables();            

            // should always have a camera if we've made it this far
            //s.getCamera()->update(alpha);

			//std::cout << "----------------------------------\n";

			//std::cout << "gpu active before rco loop:" << gpu.getActiveShader() << "," << gpu.getActiveGpuMeshIndex() << "," << gpu.getActiveMaterialIndex() << "\n";


            for (auto& rco : s.getRenderablesOrder())
            {
                Renderable& rc = s.getRenderable(rco);

				//std::cout << "rc:" << rc.getShaderIndex() << "," << rc.getMeshIndex() << "," << rc.getTextureIndex() << "\n";

                if (rc.getShader() != gpu->getActiveShader())
                    gpu->useShader(rc.getShader());

                if (rc.getMesh() != gpu->getActiveMesh())
                    gpu->useMesh(rc.getMesh());

                if (rc.getMaterial() != gpu->getActiveMaterial())
                    gpu->useMaterial(rc.getMaterial());

                // gpu state has been set, now draw all actors which use this gpu state
                for (auto& ai : rc.getActorIndexes())
                {
                    Actor* a = s.getActor(ai);
					
					//std::cout << a->getName() << ":" << rc.getShaderIndex() << "-" << rc.getMeshIndex() << "-" << rc.getTextureIndex() << "\n";
					//std::cout << a->getName() << ":" << gpu->getActiveShader() << "-" << gpu->getActiveGpuMeshIndex() << "-" << gpu->getActiveMaterialIndex() << "\n";
					//std::cout << "--------------------------------\n";

					//std::cout << a->getName() << "\n";

                    if (!a->isDeleted() && a->isVisible())
                    {
						gpu->setShaderMat4("mvp", s.getCamera()->getProjectionMatrix() * s.getCamera()->getViewMatrix() * a->getWorldRenderMatrix(alpha));

						

						//std::cout << a->getName() << "\n";

						// TODO - FIX THIS WHEN WE CONTINUE WORK ON ARMATURES
						//// If this actor is animated, send the bone transforms of it's armature to the shader
						//if (a->isAnimated())
						//{
						//	auto& mesh = a->getMesh();
						//	auto armature = a->getArmature();

						//	//std::cout << a->getName() << "\n";
						//	//for(auto& b : armature->getBones())
						//	//	std::cout << b.name << "\n";

						//	
						//	size_t boneIndex = 0;
						//	for (auto& activeBone : a->getActiveBones().value())
						//	{
						//		glm::mat4 meshBoneTransform = mesh.getGlobalInverseMatrix() * armature->getBone(activeBone.first).getRenderMatrix(alpha) * mesh.getBone(boneIndex).offsetMatrix;

						//		//std::cout << glm::to_string(meshBoneTransform) << "\n";
						//		//std::cout << activeBone.second << ":" << armature->getBone(activeBone.first).name << "\n";

						//		gpu->setShaderMat4(activeBone.second, meshBoneTransform);

						//		boneIndex++;
						//	}
						//}

						

                        gpu->drawGpuMesh();
                    }
                }

            }
            
        }


    }

}