#include <iostream>
#include <fstream>

#define GLM_FORCE_ALIGNED_GENTYPES
#include <glm/gtx/string_cast.hpp>
#include "imgui/imgui.h"


#include "vel/App.h"
#include "vel/scene/Scene.h"
#include "vel/scene/mesh/Vertex.h"
#include "vel/scene/material/Texture.h"
#include "vel/scene/AssetLoaderV2.h"

#include "dep/json.hpp"

using json = nlohmann::json;

namespace vel
{
	Scene::Scene() :
		loaded(false),
		animationTime(0.0)
	{
		// Set default container capacities. Capacities will never be smaller than these
		// values (reserving a smaller amount does not re-allocate)
		this->shaders.reserve(10);
		this->meshes.reserve(100);
		this->textures.reserve(100);
		this->materials.reserve(100);
		this->animations.reserve(100);
		this->renderables.reserve(100);
		this->armatures.reserve(100);
		this->stages.reserve(10);
	}
	
	Scene::~Scene()
	{
		App::get().getGPU()->wipe(this->shaders, this->meshes, this->textures);
	}

	/* Json Scene Loader
	--------------------------------------------------*/
	void Scene::loadSceneConfig(std::string path)
	{
		std::ifstream i(path);
		json j;
		i >> j;

		//std::cout << j["scene"]["materials"][0]["ao"] << "\n";

		//for (auto& el : j.items())
		//{
		//	std::cout << el.key() << " : " << el.value() << "\n";
		//}

		// load elements into scene
		for (auto& s : j["scene"]["shaders"])
			this->loadShader(s["name"], s["vert_path"], s["frag_path"]);

		for (auto& m : j["scene"]["meshes"])
			this->loadMesh(m);

		for (auto& t : j["scene"]["textures"])
			this->loadTexture(t["name"], t["type"], t["path"]);

		for (auto& m : j["scene"]["materials"])
		{
			Material mat;
			mat.name = m["name"];

			if (!m["albedo"].is_null())
				mat.albedo = this->getTextureIndex(m["albedo"]);
			if (!m["normal"].is_null())
				mat.normal = this->getTextureIndex(m["normal"]);
			if (!m["metallic"].is_null())
				mat.metalness = this->getTextureIndex(m["metallic"]);
			if (!m["roughness"].is_null())
				mat.roughness = this->getTextureIndex(m["roughness"]);
			if (!m["ao"].is_null())
				mat.ao = this->getTextureIndex(m["ao"]);

			this->addMaterial(mat);
		}

		for (auto& r : j["scene"]["renderables"])
		{
			this->addRenderable(
				r["name"],
				this->getShaderIndex(r["shader"]),
				this->getMeshIndex(r["mesh"]),
				this->getMaterialIndex(r["material"])
			);
		}

		// load stages into scene
		for (auto& s : j["stages"])
		{
			auto& stage = this->addStage();

			if (s["camera"]["type"] == "perspective")
				stage.addPerspectiveCamera(s["camera"]["near"], s["camera"]["far"], s["camera"]["fov"]);
			else if (s["camera"]["type"] == "orthographic")
				stage.addOrthographicCamera(s["camera"]["near"], s["camera"]["far"], s["camera"]["scale"]);
			else
				App::get().logger.die("Scene::loadSceneConfig(): config contains a camera type other than 'perspective' or 'orthographic'");

			stage.getCamera()->setPosition(glm::vec3(
				(float)s["camera"]["position"][0],
				(float)s["camera"]["position"][1],
				(float)s["camera"]["position"][2]
			));

			stage.getCamera()->setLookAt(glm::vec3(
				(float)s["camera"]["lookat"][0],
				(float)s["camera"]["lookat"][1],
				(float)s["camera"]["lookat"][2]
			));

			for (auto& a : s["actors"])
			{
				auto act = Actor(a["name"]);
				act.setDynamic(a["dynamic"]);
				act.setVisible(a["visible"]);
				act.addRenderable(this->getRenderable(a["renderable"]));

				if (!a["transform"].is_null())
				{
					auto trans = a["transform"]["translation"];
					auto rot = a["transform"]["rotation"];
					auto scale = a["transform"]["scale"];

					if (!trans.is_null())
					{
						act.getTransform().setTranslation(glm::vec3(
							(float)trans[0],
							(float)trans[1],
							(float)trans[2]
						));
					}

					if (!rot.is_null())
					{
						if (rot["type"] == "euler")
						{
							act.getTransform().setRotation((float)rot["val"]["angle"], glm::vec3(
								(float)rot["val"]["axis"][0],
								(float)rot["val"]["axis"][1],
								(float)rot["val"]["axis"][2]
							));
						}
						else if (rot["type"] == "quaternion")
						{
							act.getTransform().setRotation(glm::quat(
								(float)rot["val"][3],
								(float)rot["val"][0],
								(float)rot["val"][1],
								(float)rot["val"][2]
							));
						}
						else
						{
							App::get().logger.die("Scene::loadSceneConfig(): config contains an actor transform rotation type other than 'euler' or 'quaternion'");
						}
					}

					if (!scale.is_null())
					{
						act.getTransform().setScale(glm::vec3(
							(float)scale[0],
							(float)scale[1],
							(float)scale[2]
						));
					}
				}

				stage.addActor(act);
			}

			for (auto& a : s["armatures"])
			{
				std::vector<std::string> actorNames;

				for (auto& actName : a["actors"])
					actorNames.push_back(actName);

				stage.addArmature(this->getArmature(a["base"]), a["defaultAnimation"], actorNames);
			}
		}
	}


	/* Shaders
	--------------------------------------------------*/
	void Scene::setShaderCapacity(size_t cap)
	{
		this->shaders.reserve(cap);
	}

	size_t Scene::loadShader(std::string name, std::string vertFile, std::string fragFile)
	{
		if (this->shaders.size() == this->shaders.capacity())
			App::get().logger.die("Scene::loadShader(): Attempting to add shader after shaders capacity has been reached");

		Shader s;
		s.name = name;
		s.vertFile = vertFile;
		s.fragFile = fragFile;

		App::get().getGPU()->loadShader(s);

		this->shaders.push_back(s);

		return this->shaders.size() - 1;
	}

	size_t Scene::getShaderIndex(std::string shaderName)
	{
		for (int i = 0; i < this->shaders.size(); i++)
			if (this->shaders.at(i).name == shaderName)
				return i;

		App::get().logger.die("Scene::getShaderIndex(): CANNOT GET SHADER INDEX FOR NON EXISTING SHADER NAME");
	}

	Shader* Scene::getShader(size_t si)
	{
		return &this->shaders.at(si);
	}

	Shader* Scene::getShader(std::string shaderName)
	{
		for (auto& s : this->shaders)
			if (s.name == shaderName)
				return &s;

		App::get().logger.die("Scene::getShader(): CANNOT GET SHADER FOR NON EXISTING SHADER NAME");
	}

	/* Meshes
	--------------------------------------------------*/
	void Scene::setMeshCapacity(size_t cap)
	{
		this->meshes.reserve(cap);
	}

	void Scene::loadMesh(std::string path)
	{
		auto al = AssetLoaderV2(this, path);
		al.load();
	}

	Mesh* Scene::addMesh(Mesh m)
	{
		if (this->meshes.size() == this->meshes.capacity())
			App::get().logger.die("Scene::addMesh(): Attempting to add mesh after meshes capacity has been reached");

		App::get().getGPU()->loadMesh(m);

		this->meshes.push_back(m);

		return &this->meshes.at((this->meshes.size() - 1));
	}

	const std::vector<Mesh>& Scene::getMeshes() const
	{
		return this->meshes;
	}

	size_t Scene::getMeshIndex(std::string meshName)
	{
		for (int i = 0; i < this->meshes.size(); i++)
			if (this->meshes.at(i).getName() == meshName)
				return i;

		App::get().logger.die("CANNOT GET MESH INDEX FOR NON EXISTING MESH NAME");
	}

	Mesh* Scene::getMesh(size_t index)
	{
		return &this->meshes.at(index);
	}


	/* Textures
	--------------------------------------------------*/
	void Scene::setTextureCapacity(size_t cap)
	{
		this->textures.reserve(cap);
	}

	size_t Scene::loadTexture(std::string name, std::string type, std::string path, std::vector<std::string> mips)
	{
		if (this->textures.size() == this->textures.capacity())
			App::get().logger.die("Scene::loadTexture(): Attempting to add texture after textures capacity has been reached");

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

	size_t Scene::getTextureIndex(std::string textureName)
	{
		for (size_t i = 0; i < this->textures.size(); i++)
			if (this->textures.at(i).name == textureName)
				return i;

		App::get().logger.die("Scene::getTextureIndex(): Attempting to get texture index for non-existing texture name");
	}

	/* Materials
	--------------------------------------------------*/
	void Scene::setMaterialCapacity(size_t cap)
	{
		this->materials.reserve(cap);
	}

	size_t Scene::addMaterial(Material m)
	{
		if (this->materials.size() == this->materials.capacity())
			App::get().logger.die("Scene::addMaterial(): Attempting to add material after materials capacity has been reached");

		// convert texture container indexes into gpu indexes
		if (m.albedo)
			m.albedo = this->textures.at(m.albedo.value()).id;
		if (m.normal)
			m.normal = this->textures.at(m.normal.value()).id;
		if (m.metalness)
			m.metalness = this->textures.at(m.metalness.value()).id;
		if (m.roughness)
			m.roughness = this->textures.at(m.roughness.value()).id;
		if (m.ao)
			m.ao = this->textures.at(m.ao.value()).id;

		this->materials.push_back(m);

		return this->materials.size() - 1;
	}

	Material* Scene::getMaterial(size_t materialIndex)
	{
		return &this->materials.at(materialIndex);
	}

	size_t Scene::getMaterialIndex(std::string materialName)
	{
		for (int i = 0; i < this->materials.size(); i++)
			if (this->materials.at(i).name == materialName)
				return i;

		App::get().logger.die("CANNOT GET MATERIAL INDEX FOR NON EXISTING MATERIAL NAME");
	}

	Material* Scene::getMaterial(std::string materialName)
	{
		for (auto& m : this->materials)
			if (m.name == materialName)
				return &m;
	}

	/* Animations
	--------------------------------------------------*/
	void Scene::setAnimationCapacity(size_t cap)
	{
		this->animations.reserve(cap);
	}

	Animation* Scene::addAnimation(Animation a)
	{
		if (this->animations.size() == this->animations.capacity())
			App::get().logger.die("Scene::addAnimation(): Attempting to add animation after animations capacity has been reached");

		this->animations.push_back(a);
		return &this->animations.at((this->animations.size() - 1));
	}

	std::vector<Animation>& Scene::getAnimations()
	{
		return this->animations;
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

	/* Renderables
	--------------------------------------------------*/
	void Scene::setRenderableCapacity(size_t cap)
	{
		this->renderables.reserve(cap);
	}

	size_t Scene::addRenderable(std::string name, size_t shaderIndex, size_t meshIndex, size_t materialIndex)
	{
		if (this->renderables.size() == this->renderables.capacity())
			App::get().logger.die("Scene::addRenderable(): Attempting to add renderable after renderables capacity has been reached");

		Renderable r = Renderable(name, shaderIndex, meshIndex, materialIndex,
			&this->shaders.at(shaderIndex), &this->meshes.at(meshIndex), &this->materials.at(materialIndex),
			this->materials.at(materialIndex).hasAlphaChannel);

		this->renderables.push_back(r);

		return this->renderables.size() - 1;
	}

	Renderable Scene::getRenderable(std::string name)
	{
		for (auto& r : this->renderables)
			if (r.getName() == name)
				return r;

		App::get().logger.die("Scene: attempting to get renderable by name that does not exist");
	}


	/* Armatures
	--------------------------------------------------*/
	void Scene::setArmatureCapacity(size_t cap)
	{
		this->armatures.reserve(cap);
	}

	Armature* Scene::addArmature(Armature a)
	{
		if (this->armatures.size() == this->armatures.capacity())
			App::get().logger.die("Scene::addArmature(): Attempting to add armature after armatures capacity has been reached");

		this->armatures.push_back(a);

		return &this->armatures.back();
	}

	Armature Scene::getArmature(std::string name)
	{
		for (auto& a : this->armatures)
			if (a.getName() == name)
				return a;

		App::get().logger.die("Scene::getArmature(): Attempting to get armature by name that does not exist");
	}

	/* Stages
	--------------------------------------------------*/
	void Scene::setStageCapacity(size_t cap)
	{
		this->stages.reserve(cap);
	}

	Stage& Scene::addStage()
	{
		if (this->stages.size() == this->stages.capacity())
			App::get().logger.die("Scene::addStage(): Attempting to add stage after stages capacity has been reached");

		this->stages.push_back(Stage());

		return this->getStage(this->stages.size() - 1);
	}

	Stage& Scene::getStage(size_t index)
	{
		return this->stages.at(index);
	}


	/* Misc
	--------------------------------------------------*/
	void Scene::stepPhysics(float delta)
	{
		for (auto& s : this->stages)
			s.stepPhysics(delta);
	}

	void Scene::postPhysics(float delta) {}

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
			if (s.getCollisionWorld()->getDebugDrawer() != nullptr)
			{
				s.getCollisionWorld()->getDynamicsWorld()->debugDrawWorld(); // load vertices into associated CollisionDebugDrawer

				gpu->useShader(s.getCollisionWorld()->getDebugDrawer()->getShaderProgram());
				gpu->setShaderMat4("vp", s.getCamera()->getProjectionMatrix() * s.getCamera()->getViewMatrix());
				gpu->debugDrawCollisionWorld(s.getCollisionWorld()->getDebugDrawer()); // draw all loaded vertices with a single call and clear

				//std::cout << "debug draw\n";
			}


			//s.printRenderables();            

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

						// If this actor is animated, send the bone transforms of it's armature to the shader
						if (a->isAnimated())
						{
							auto mesh = s.getRenderable(a->getRenderableIndex()).getMesh();
							auto armature = a->getArmature();

							//std::cout << a->getName() << "\n";
							//for(auto& b : armature->getBones())
							//	std::cout << b.name << "\n";


							size_t boneIndex = 0;
							for (auto& activeBone : a->getActiveBones())
							{
								glm::mat4 meshBoneTransform = mesh->getGlobalInverseMatrix() * armature->getBone(activeBone.first).getRenderMatrix(alpha) * mesh->getBone(boneIndex).offsetMatrix;

								//std::cout << glm::to_string(meshBoneTransform) << "\n";
								//std::cout << activeBone.second << ":" << armature->getBone(activeBone.first).name << "\n";

								gpu->setShaderMat4(activeBone.second, meshBoneTransform);

								boneIndex++;
							}
						}



						gpu->drawGpuMesh();
					}
				}

			}

		}


	}

}