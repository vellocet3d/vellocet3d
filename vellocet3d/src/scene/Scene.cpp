#include <iostream>
#include <fstream>

#define GLM_FORCE_ALIGNED_GENTYPES
#include <glm/gtx/string_cast.hpp>
#include "imgui/imgui.h"


#include "vel/App.h"
#include "vel/scene/Scene.h"
#include "vel/assets/mesh/Vertex.h"
#include "vel/assets/material/Texture.h"
#include "vel/assets/AssetLoaderV2.h"
#include "vel/dep/json.hpp"

using json = nlohmann::json;

namespace vel
{
	Scene::Scene() :
		mainMemoryloaded(false),
		swapWhenLoaded(false),
		animationTime(0.0)
	{

	}
	
	void Scene::setName(std::string n)
	{
		this->name = n;
	}

	Scene::~Scene()
	{
		this->freeAssets();
	}

	void Scene::freeAssets()
	{
#ifdef DEBUG_ASSET_MANAGEMENT
	std::cout << "Freeing assets for scene: " << this->name << std::endl;
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
	}

	/* Json Scene Loader
	--------------------------------------------------*/
	void Scene::loadSceneConfig(std::string path)
	{
		std::ifstream i(path);
		json j;
		i >> j;
		
		// just a usage example
		//std::cout << j["scene"]["materials"][0]["ao"] << "\n";
		//for (auto& el : j.items())
		//	std::cout << el.key() << " : " << el.value() << "\n";

#ifdef DEBUG_ASSET_MANAGEMENT
	std::cout << "Loading Scene via configuration file: " << path << std::endl;
#endif

		//load elements into scene
		for (auto& s : j["scene"]["shaders"])
			this->loadShader(s["name"], s["vert_path"], s["frag_path"]);

		for (auto& m : j["scene"]["meshes"])
			this->loadMesh(m);

		for (auto& t : j["scene"]["textures"])
			this->loadTexture(t["name"], t["type"], t["path"]);
		// TODO: need mipmap logic here

		for (auto& m : j["scene"]["materials"])
		{
			Material mat;
			mat.name = m["name"];

			if (!m["albedo"].is_null())
				mat.albedo = this->getTexture(m["albedo"]);
			if (!m["normal"].is_null())
				mat.normal = this->getTexture(m["normal"]);
			if (!m["metallic"].is_null())
				mat.metalness = this->getTexture(m["metallic"]);
			if (!m["roughness"].is_null())
				mat.roughness = this->getTexture(m["roughness"]);
			if (!m["ao"].is_null())
				mat.ao = this->getTexture(m["ao"]);

			this->addMaterial(mat);
		}

		for (auto& r : j["scene"]["renderables"])
		{
			this->addRenderable(
				r["name"],
				this->getShader(r["shader"]),
				this->getMesh(r["mesh"]),
				this->getMaterial(r["material"])
			);
		}

		//load stages into scene
		for (auto& s : j["stages"])
		{
			auto stage = this->addStage(s["name"]);

			if (s["camera"]["type"] == "perspective")
				stage->addPerspectiveCamera(s["camera"]["near"], s["camera"]["far"], s["camera"]["fov"]);
			else if (s["camera"]["type"] == "orthographic")
				stage->addOrthographicCamera(s["camera"]["near"], s["camera"]["far"], s["camera"]["scale"]);
			else
				App::get().logger.log("Scene::loadSceneConfig(): config contains a camera type other than 'perspective' or 'orthographic'");

			stage->getCamera()->setPosition(glm::vec3(
				(float)s["camera"]["position"][0],
				(float)s["camera"]["position"][1],
				(float)s["camera"]["position"][2]
			));

			stage->getCamera()->setLookAt(glm::vec3(
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
							App::get().logger.log("Scene::loadSceneConfig(): config contains an actor transform rotation type other than 'euler' or 'quaternion'");
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

				stage->addActor(act);
			}

			for (auto& a : s["armatures"])
			{
				std::vector<std::string> actorNames;

				for (auto& actName : a["actors"])
					actorNames.push_back(actName);

				stage->addArmature(this->getArmature(a["base"]), a["defaultAnimation"], actorNames);
			}
		}
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

	void Scene::loadShader(std::string name, std::string vertFile, std::string fragFile)
	{
		this->shadersInUse.push_back(App::get().getAssetManager().loadShader(name, vertFile, fragFile));
	}
	
	void Scene::loadMesh(std::string path)
	{
		auto tts = App::get().getAssetManager().loadMesh(path);
		for(auto& t : tts.first)
			this->meshesInUse.push_back(t);
		
		if(tts.second != "")
			this->armaturesInUse.push_back(tts.second);
	}
	
	void Scene::loadTexture(std::string name, std::string type, std::string path, std::vector<std::string> mips)
	{
		this->texturesInUse.push_back(App::get().getAssetManager().loadTexture(name, type, path, mips));
	}
	
	void Scene::addMaterial(Material m)
	{
		this->materialsInUse.push_back(App::get().getAssetManager().addMaterial(m));
	}
	
	void Scene::addRenderable(std::string name, Shader* shader, Mesh* mesh, Material* material)
	{
		this->renderablesInUse.push_back(App::get().getAssetManager().addRenderable(name, shader, mesh, material));
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

	Material* Scene::getMaterial(std::string name)
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
	void Scene::updateAnimations(double delta)
	{
		this->animationTime += delta;
		for (auto& s : this->stages.getAll())
			s.updateActorAnimations(this->animationTime);
	}
	
	std::string Scene::getName()
	{
		return this->name;
	}

	void Scene::stepPhysics(float delta)
	{
		for (auto& s : this->stages.getAll())
			s.stepPhysics(delta);
	}

	void Scene::postPhysics(float delta) {}

	void Scene::applyTransformations()
	{
		for (auto& s : this->stages.getAll())
			s.applyTransformations();
	}

	void Scene::processSensors()
	{
		for (auto& s : this->stages.getAll())
			if (s.getCollisionWorld())
				s.getCollisionWorld()->processSensors();
	}

	void Scene::draw(float alpha)
	{
		//TODO: random thought: if we are not sorting renderables, we will need some way of
		// pushing renderables with transparent materials to the last rendered elements. Might do this in the draw loop,
		// when renderable changes, check if it has transparent property, and if so, push it to a separate queue to be
		// rendered after everything else.

		GPU* gpu = App::get().getGPU(); // for convenience

		gpu->enableBlend();

		for (auto& s : this->stages.getAll())
		{
			if (!s.isVisible())
				continue;


			// clear depth buffer if flag set in stage
			if (s.getClearDepthBuffer())
				gpu->clearDepthBuffer();

			// should always have a camera if we've made it this far
			s.getCamera()->update();


			// if debug drawer set, do debug draw
			if (s.getCollisionWorld() && s.getCollisionWorld()->getDebugDrawer())
			{
				//std::cout << "yeet001\n";
				s.getCollisionWorld()->getDynamicsWorld()->debugDrawWorld(); // load vertices into associated CollisionDebugDrawer
				gpu->useShader(s.getCollisionWorld()->getDebugDrawer()->getShaderProgram());
				gpu->setShaderMat4("vp", s.getCamera()->getProjectionMatrix() * s.getCamera()->getViewMatrix());
				gpu->debugDrawCollisionWorld(s.getCollisionWorld()->getDebugDrawer()); // draw all loaded vertices with a single call and clear
				//gpu->resetActives(); // force reset active gpu objects since debug draw binds it's own VAO
			}

			gpu->resetActives(); // moving this here so that we reset gpu actives every tick fixes runtime bug, BUT WHY? If I remember correctly, the only reason
								// we were doing this was because of the debugdrawer??? For some reason, loading another scene into the gpu while one is loaded,
								// jacks with this. WAIT...probably because when we gpu load things, we're altering the state in order to load those elements...duh,
								// so we WOULD need to reset actives so that the gpu knows to reset state to what we're suppose to be drawing
			
			for(auto& r : s.getRenderables())
			{
				// Only do state switches if we need to. We used to sort renderables when they were added,
				// but after the massive refactor we removed this feature as everytime a renderable was added
				// we had to sort on every renderable, which if you have 1000s of renderables would be a significant
				// performance hit. So now gpu state switches are hit or missed. All actors that use a specific renderable
				// do not require any state change, but when switching between renderables, if the next renderable in the
				// queue uses a different mesh than the current renderable, the gpu state would be switched, then if the renderable
				// after that used the mesh from the first renderable it would have to switch back again, instead of these renderables
				// being sorted so that everything is hit in order minimizing state switches.
				if (r.getShader() != gpu->getActiveShader())
					gpu->useShader(r.getShader());
				if (r.getMesh() != gpu->getActiveMesh())
					gpu->useMesh(r.getMesh());
				if (r.getMaterial() != gpu->getActiveMaterial())
					gpu->useMaterial(r.getMaterial());

				for (auto& a : r.actors.getAll())
				{
					if (a->isVisible()) // shouldn't need to check a->isDeleted() since using colony
					{
						gpu->setShaderMat4("mvp", s.getCamera()->getProjectionMatrix() * s.getCamera()->getViewMatrix() * a->getWorldRenderMatrix(alpha));

						if (a->isAnimated())// If this actor is animated, send the bone transforms of it's armature to the shader
						{
							auto mesh = a->getMesh();
							auto armature = a->getArmature();

							size_t boneIndex = 0;
							for (auto& activeBone : a->getActiveBones())
							{
								glm::mat4 meshBoneTransform = mesh->getGlobalInverseMatrix() * armature->getBone(activeBone.first).getRenderMatrix(alpha) * mesh->getBone(boneIndex).offsetMatrix;
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