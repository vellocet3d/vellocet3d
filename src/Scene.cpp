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
		camera(Camera(CameraType::PERSPECTIVE, 0.1f, 250.0f, 60.0f)),
		activeInfiniteCubemap(nullptr),
		drawSkybox(false),
		collisionWorld(nullptr),
		mainMemoryloaded(false),
		swapWhenLoaded(false),
		animationTime(0.0),
		fixedAnimationTime(0.0)
	{
		this->sortedTransparentActors.reserve(1000); // reserve space for 1000 transparent actors (won't reallocate until that limit reached)

		this->camera.setPosition(glm::vec3(0.0f, 2.0f, 0.0f));
		this->camera.setLookAt(glm::vec3(0.0f, 0.0f, -1.0f));
	}
	
	Scene::~Scene()
	{
		this->freeAssets();
	}

	Camera* Scene::getCamera()
	{
		return &this->camera;
	}

	void Scene::setName(std::string n)
	{
		this->name = n;
	}

	void Scene::setActiveInfiniteCubemap(Cubemap* c)
	{
		this->activeInfiniteCubemap = c;
	}

	Cubemap* Scene::getActiveInfiniteCubemap()
	{
		return this->activeInfiniteCubemap;
	}

	void Scene::setDrawSkybox(bool b)
	{
		this->drawSkybox = b;
	}

	bool Scene::getDrawSkybox()
	{
		return this->drawSkybox;
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
		
        for(auto& name : this->infiniteCubemapsInUse)
			App::get().getAssetManager().removeInfiniteCubemap(name);
        
		for(auto& name : this->shadersInUse)
			App::get().getAssetManager().removeShader(name);


		delete this->collisionWorld;

	}

	/* Json Scene Loader
	--------------------------------------------------*/
	void Scene::loadConfigFile(std::string path)
	{
		std::ifstream i(path);
		json j;
		i >> j;
		
		// just a usage example
		//std::cout << j["scene"]["materials"][0]["ao"] << "\n";
		//for (auto& el : j.items())
		//	std::cout << el.key() << " : " << el.value() << "\n";

#ifdef DEBUG_LOG
	Log::toCliAndFile("Loading Scene via configuration file: " + path);
#endif

		// Load user defined shaders
		for (auto& s : j["shaders"])
			this->loadShader(s["name"], s["vert_path"], s["frag_path"]);
		
		// Load infinitely distant hdr images (cubemaps with computed data for IBL)
		for (auto& h : j["infiniteCubemaps"])
			this->loadInfiniteCubemap(h["name"], h["path"]);

		// Set active infinite cubemap if parameter provided
		if (j.contains("activeInfiniteCubemap") && !j["activeInfiniteCubemap"].is_null() && j["activeInfiniteCubemap"] != "")
			this->setActiveInfiniteCubemap(this->getInfiniteCubemap(j["activeInfiniteCubemap"]));

		// Set whether or not the scene should draw a skybox (which would use the previously set "activeInfiniteCubemap")
		if (j.contains("drawSkybox") && j["drawSkybox"] != "" && !j["drawSkybox"].is_null())
			this->setDrawSkybox(j["drawSkybox"]);

		// Update camera
		if (j.contains("camera") && j["camera"] != "" && !j["camera"].is_null())
		{
			if (j["camera"]["type"] == "perspective")
				this->camera = Camera(CameraType::PERSPECTIVE, j["camera"]["near"], j["camera"]["far"], j["camera"]["fov"]);
			else if (j["camera"]["type"] == "orthographic")
				this->camera = Camera(CameraType::ORTHOGRAPHIC, j["camera"]["near"], j["camera"]["far"], j["camera"]["scale"]);
#ifdef DEBUG_LOG
			else
				Log::crash("Scene::loadConfigFile(): config contains a camera type other than 'perspective' or 'orthographic'");
#endif

			this->camera.setPosition(glm::vec3(
				(float)j["camera"]["position"][0],
				(float)j["camera"]["position"][1],
				(float)j["camera"]["position"][2]
			));

			this->camera.setLookAt(glm::vec3(
				(float)j["camera"]["lookat"][0],
				(float)j["camera"]["lookat"][1],
				(float)j["camera"]["lookat"][2]
			));

		}

		// Load meshes from .fbx files, will also load in associated armatures
		for (auto& m : j["meshes"])
			this->loadMesh(m);
			

		// Load textures and their associated mip chains
		for (auto& t : j["textures"])
		{
			std::vector<std::string> mips;
			for (auto& mip : t["mips"])
				mips.push_back(mip);

			this->loadTexture(t["name"], t["type"], t["path"], mips);
		}

		// Load materials which are created from previously loaded textures
		for (auto& m : j["materials"])
		{
			Material mat;
			mat.name = m["name"];

			if (m.contains("color") && m["color"] != "" && !m["color"].is_null())
			{
				mat.color = glm::vec4(
					(float)m["color"][0],
					(float)m["color"][1],
					(float)m["color"][2],
					(float)m["color"][3]
				);
			}

			if (m.contains("diffuse") && m["diffuse"] != "" && !m["diffuse"].is_null())
				mat.diffuse = this->getTexture(m["diffuse"]);

			if (m.contains("albedo") && m["albedo"] != "" && !m["albedo"].is_null())
				mat.albedo = this->getTexture(m["albedo"]);

			if (m.contains("normal") && m["normal"] != "" && !m["normal"].is_null())
				mat.normal = this->getTexture(m["normal"]);	

			if (m.contains("metallic") && m["metallic"] != "" && !m["metallic"].is_null())
				mat.metallic = this->getTexture(m["metallic"]);

			if (m.contains("roughness") && m["roughness"] != "" && !m["roughness"].is_null())
				mat.roughness = this->getTexture(m["roughness"]);

			if (m.contains("ao") && m["ao"] != "" && !m["ao"].is_null())
				mat.ao = this->getTexture(m["ao"]);

			this->addMaterial(mat);
		}

		// Load renderables
		for (auto& r : j["renderables"])
		{
			this->addRenderable(
				r["name"],
				this->getShader(r["shader"]),
				this->getMesh(r["mesh"]),
				this->getMaterial(r["material"])
			);
		}

		// Load collision world
		if (j.contains("collisionWorld") && j["collisionWorld"] != "" && !j["collisionWorld"].is_null())
		{
			auto cw = this->addCollisionWorld(j["collisionWorld"]["gravity"]);
			cw->setIsActive(j["collisionWorld"]["active"]);

			if (j["collisionWorld"]["debug"])
				cw->useDebugDrawer(this->getShader("defaultDebug"));

			for (auto& cs : j["collisionWorld"]["collisionShapes"])
			{
				if (cs["type"] == "btCylinderShape")
				{
					btCollisionShape* btcs = new btCylinderShape(btVector3(cs["dimensions"][0], cs["dimensions"][1], cs["dimensions"][2]));
					cw->addCollisionShape(cs["name"], btcs);
				}

				if (cs["type"] == "btCapsuleShape")
				{
					btCollisionShape* btcs = new btCapsuleShape(cs["dimensions"][0], cs["dimensions"][1]); // total height is height+2*radius
					cw->addCollisionShape(cs["name"], btcs);
				}

				// Add support for more shapes as needed
			}

			for (auto& co : j["collisionWorld"]["collisionObjects"])
			{
				// Add support for more properties as needed
				CollisionObjectTemplate cot;
				cot.type = co["type"];
				cot.name = co["name"];
				cot.collisionShape = cw->getCollisionShape(co["collisionShape"]);
				if (co.contains("mass") && !co["mass"].is_null())
					cot.mass = co["mass"];
				if (co.contains("friction") && !co["friction"].is_null())
					cot.friction = co["friction"];
				if (co.contains("restitution") && !co["restitution"].is_null())
					cot.restitution = co["restitution"];
				if (co.contains("linearDamping") && !co["linearDamping"].is_null())
					cot.linearDamping = co["linearDamping"];
				if (co.contains("angularFactor") && !co["angularFactor"].is_null())
					cot.angularFactor = btVector3(co["angularFactor"][0], co["angularFactor"][1], co["angularFactor"][2]);
				if (co.contains("activationState") && !co["activationState"].is_null())
					cot.activationState = co["activationState"];
				if (co.contains("gravity") && !co["gravity"].is_null())
					cot.gravity = btVector3(co["gravity"][0], co["gravity"][1], co["gravity"][2]);

				cw->addCollisionObjectTemplate(cot.name, cot);
			}
		}
		

		// Load stages
		for (auto& s : j["stages"])
		{
			auto stage = this->addStage(s["name"]);

			if (s.contains("renderMode"))
			{
				if (s["renderMode"] == "RGBA")
					stage->setRenderMode(RenderMode::RGBA);
				else if (s["renderMode"] == "STATIC_DIFFUSE")
					stage->setRenderMode(RenderMode::STATIC_DIFFUSE);
				else if (s["renderMode"] == "PBR")
					stage->setRenderMode(RenderMode::PBR);
				else if (s["renderMode"] == "PBR_IBL")
					stage->setRenderMode(RenderMode::PBR_IBL);
			}


			if (s.contains("clearDepthBuffer") && !s["clearDepthBuffer"].is_null() && s["clearDepthBuffer"] != "" && s["clearDepthBuffer"])
				stage->setClearDepthBuffer(true);

			if (s.contains("camera") && s["camera"] != "" && !s["camera"].is_null())
			{
				if (s["camera"]["type"] == "perspective")
					stage->addCamera(CameraType::PERSPECTIVE, s["camera"]["near"], s["camera"]["far"], s["camera"]["fov"]);
				else if (j["camera"]["type"] == "orthographic")
					stage->addCamera(CameraType::ORTHOGRAPHIC, s["camera"]["near"], s["camera"]["far"], s["camera"]["scale"]);
#ifdef DEBUG_LOG
				else
					Log::crash("Scene::loadConfigFile(): config contains a camera type other than 'perspective' or 'orthographic'");
#endif

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
			}

			if (s.contains("useSceneCameraPositionForLighting") && !s["useSceneCameraPositionForLighting"].is_null())
				stage->setUseSceneCameraPositionForLighting(s["useSceneCameraPositionForLighting"]);
			
			if (s.contains("activeInfiniteCubemap") && !s["activeInfiniteCubemap"].is_null() && s["activeInfiniteCubemap"] != "")
				stage->setActiveInfiniteCubemap(this->getInfiniteCubemap(s["activeInfiniteCubemap"]));


			for (auto& a : s["actors"])
			{
				auto act = Actor(a["name"]);
				act.setDynamic(a["dynamic"]);
				act.setVisible(a["visible"]);
				act.setAutoTransform(a["autoTransform"]);
				act.addRenderable(this->getRenderable(a["renderable"]));//TODO: what if headless?

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
#ifdef DEBUG_LOG
						else
							Log::crash("Scene::loadSceneConfig(): config contains an actor transform rotation type other than 'euler' or 'quaternion'");
#endif

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


				// must get final memory address of actor in order to pass userptr to rididbody or ghostobject, so we add actor to stage
				// here, meaning everything above this line would be properties that the actor MUST HAVE before being added to stage,
				// although at this time a Renderable is all that it needs to have before being added.
				Actor* pActor = stage->addActor(act);


				if (this->collisionWorld != nullptr && a.contains("collisionObject") && !a["collisionObject"].is_null())
				{
					if ((a["collisionObject"] != "GENERATE_STATIC_RIGIDBODY") && (a["collisionObject"] != "GENERATE_STATIC_GHOST"))
					{
						auto& cot = this->collisionWorld->getCollisionObjectTemplate(a["collisionObject"]);
						auto actorTranslation = pActor->getTransform().getTranslation();

						btTransform theTransform;
						theTransform.setIdentity();
						theTransform.setOrigin(btVector3(actorTranslation.x, actorTranslation.y, actorTranslation.z));
						theTransform.setRotation(vel::glmToBulletQuat(pActor->getTransform().getRotation()));

						if (cot.type == "rigidBody")
						{
							btVector3 theInertia(0.0f, 0.0f, 0.0f);
							cot.collisionShape->calculateLocalInertia(cot.mass.value(), theInertia);

							btDefaultMotionState* theMotionState = new btDefaultMotionState(theTransform);
							btRigidBody::btRigidBodyConstructionInfo theBodyInfo(cot.mass.value(), theMotionState, cot.collisionShape, theInertia);

							// Add support for more properties as needed
							if (cot.friction)
								theBodyInfo.m_friction = cot.friction.value();
							if (cot.restitution)
								theBodyInfo.m_restitution = cot.restitution.value();
							if (cot.linearDamping)
								theBodyInfo.m_linearDamping = cot.linearDamping.value();

							btRigidBody* theRigidBody = new btRigidBody(theBodyInfo);
							if (cot.gravity)
								theRigidBody->setGravity(cot.gravity.value());
							if (cot.angularFactor)
								theRigidBody->setAngularFactor(cot.angularFactor.value());
							if (cot.activationState)
								theRigidBody->setActivationState(cot.activationState.value());

							theRigidBody->setUserPointer(pActor);
							this->collisionWorld->getDynamicsWorld()->addRigidBody(theRigidBody);

							pActor->setRigidBody(theRigidBody);
						}
						else if (cot.type == "ghostObject")
						{
							btPairCachingGhostObject* theGhostObject = new btPairCachingGhostObject();
							theGhostObject->setCollisionShape(cot.collisionShape);
							theGhostObject->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
							theGhostObject->setUserPointer(pActor);

							this->collisionWorld->getDynamicsWorld()->addCollisionObject(theGhostObject,
								btBroadphaseProxy::KinematicFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);

							pActor->setGhostObject(theGhostObject);
						}
					}
					else
					{
						if (a["collisionObject"] == "GENERATE_STATIC_RIGIDBODY")
						{
							this->collisionWorld->addStaticCollisionBody(pActor);
						}
						else if (a["collisionObject"] == "GENERATE_STATIC_GHOST")
						{
							btPairCachingGhostObject* theGhostObject = new btPairCachingGhostObject();
							theGhostObject->setCollisionShape(this->collisionWorld->collisionShapeFromActor(pActor));
							theGhostObject->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
							theGhostObject->setUserPointer(pActor);

							this->collisionWorld->getDynamicsWorld()->addCollisionObject(theGhostObject,
								btBroadphaseProxy::KinematicFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);

							pActor->setGhostObject(theGhostObject);
						}
					}

				}

			// end of actor foreach
			}

			for (auto& a : s["armatures"])
			{
				std::vector<std::string> actorNames;

				for (auto& actName : a["actors"])
					actorNames.push_back(actName);

				auto arm = stage->addArmature(this->getArmature(a["base"]), a["defaultAnimation"], actorNames);

				if (a.contains("shouldInterpolate") && !a["shouldInterpolate"].is_null())
					arm->setShouldInterpolate(a["shouldInterpolate"]);


				if (!a["transform"].is_null())
				{
					auto trans = a["transform"]["translation"];
					auto rot = a["transform"]["rotation"];
					auto scale = a["transform"]["scale"];

					if (!trans.is_null())
					{
						arm->getTransform().setTranslation(glm::vec3(
							(float)trans[0],
							(float)trans[1],
							(float)trans[2]
						));
					}

					if (!rot.is_null())
					{
						if (rot["type"] == "euler")
						{
							arm->getTransform().setRotation((float)rot["val"]["angle"], glm::vec3(
								(float)rot["val"]["axis"][0],
								(float)rot["val"]["axis"][1],
								(float)rot["val"]["axis"][2]
							));
						}
						else if (rot["type"] == "quaternion")
						{
							arm->getTransform().setRotation(glm::quat(
								(float)rot["val"][3],
								(float)rot["val"][0],
								(float)rot["val"][1],
								(float)rot["val"][2]
							));
						}
#ifdef DEBUG_LOG
						else
							Log::crash("Scene::loadSceneConfig(): config contains an armature transform rotation type other than 'euler' or 'quaternion'");
#endif

					}

					if (!scale.is_null())
					{
						arm->getTransform().setScale(glm::vec3(
							(float)scale[0],
							(float)scale[1],
							(float)scale[2]
						));
					}
				}

			// end of armatures foreach
			}

			for (auto& p : s["parenting"])
			{
				auto parent = stage->getActor(p["parent"]);
				for (auto& c : p["children"])
					stage->getActor(c)->setParentActor(parent);
			}

		// end stage foreach
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

	CollisionWorld* Scene::addCollisionWorld(float gravity)
	{
		// for some reason CollisionWorld has to be a pointer or bullet has read access violation issues
		// delete in destructor
		this->collisionWorld = new CollisionWorld(gravity);

		return this->collisionWorld;
	}

	CollisionWorld* Scene::getCollisionWorld()
	{
		return this->collisionWorld;
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
    
    void Scene::loadInfiniteCubemap(std::string name, std::string path)
    {
        this->infiniteCubemapsInUse.push_back(App::get().getAssetManager().loadInfiniteCubemap(name, path));
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
    
    Cubemap* Scene::getInfiniteCubemap(std::string name)
    {
        return App::get().getAssetManager().getInfiniteCubemap(name);
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
		return this->stages.insert(name, Stage(this, name));
	}

	Stage* Scene::getStage(std::string name)
	{
		return this->stages.get(name);
	}


	/* Misc
	--------------------------------------------------*/
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
		if(this->collisionWorld != nullptr && this->collisionWorld->getIsActive())
			this->collisionWorld->getDynamicsWorld()->stepSimulation(delta, 0);
	}

	void Scene::postPhysics(float delta) {}

	void Scene::applyTransformations()
	{
		for (auto& s : this->stages.getAll())
			s->applyTransformations();
	}

	void Scene::processSensors()
	{
		if (this->collisionWorld != nullptr && this->collisionWorld->getIsActive())
			this->collisionWorld->processSensors();
	}

	void Scene::draw(float alpha)
	{
		//Log::toCli("----------------------------------------------------");
		//Log::toCli("NEW RENDER PASS");
		//Log::toCli("----------------------------------------------------");

		auto gpu = App::get().getGPU(); // for convenience

        gpu->disableBlend(); // disable blending for opaque objects

		// set scene camera values
		this->camera.update();
		this->cameraPosition = this->camera.getPosition();
		this->cameraProjectionMatrix = this->camera.getProjectionMatrix();
		this->cameraViewMatrix = this->camera.getViewMatrix();
		// these are for applying lighting to objects that are in screen space as if they were in world space, for example
		// first person arms / weapons (allows us to use the view matrix of one camera only for lighting), set to scene camera defaults here
		// only relevant for when stage has it's own camera AND useSceneCameraPositionForLighting is set to true
		this->renderCameraPosition = this->cameraPosition;
		this->renderCameraOffset = glm::mat4(1.0f);

		// draw cubemap skybox if we should
		if (this->getDrawSkybox() && this->getActiveInfiniteCubemap() != nullptr)
			gpu->drawSkybox(this->cameraProjectionMatrix, this->cameraViewMatrix, this->getActiveInfiniteCubemap()->envCubemap);

		// debug draw collision world
#ifdef DEBUG_LOG
		if(this->collisionWorld != nullptr && this->collisionWorld->getIsActive() && this->collisionWorld->getDebugDrawer() != nullptr)
		{
			this->collisionWorld->getDynamicsWorld()->debugDrawWorld(); // load vertices into associated CollisionDebugDrawer
			gpu->useShader(this->collisionWorld->getDebugDrawer()->getShaderProgram());
			gpu->setShaderMat4("vp", this->camera.getProjectionMatrix() * this->camera.getViewMatrix());
			gpu->debugDrawCollisionWorld(this->collisionWorld->getDebugDrawer()); // draw all loaded vertices with a single call and clear
		}
#endif
		
		// loop through all stages
		for (auto s : this->stages.getAll())
		{
			if (!s->isVisible())
				continue;

			// send render mode used for this stage to gpu
			gpu->setCurrentRenderMode(s->getRenderMode());

			// clear depth buffer if flag set in stage
			if (s->getClearDepthBuffer())
				gpu->clearDepthBuffer();

			// if stage has camera, use stage camera values
			if (s->getCamera().has_value())
			{
				s->getCamera()->update();
				this->cameraPosition = s->getCamera()->getPosition();
				this->cameraProjectionMatrix = s->getCamera()->getProjectionMatrix();
				this->cameraViewMatrix = s->getCamera()->getViewMatrix();

				// these are for applying lighting to objects that are in screen space as if they were in world space, for example
				// first person arms / weapons (allows us to use the view matrix of one camera only for lighting)
				this->renderCameraPosition = s->getUseSceneCameraPositionForLighting() == false ? this->cameraPosition : this->camera.getPosition();
				this->renderCameraOffset = s->getUseSceneCameraPositionForLighting() == false ? glm::mat4(1.0f) : glm::inverse(this->camera.getViewMatrix());
			}


			std::vector<Renderable*> transparentRenderables;
			
			for(auto r : s->getRenderables())
			{
				if (r->getMaterialHasAlpha())
				{
					transparentRenderables.push_back(r); //TODO this reallocates on every insert
					continue;
				}

				// DRAW OPAQUES

				// RESET GPU STATE FOR THIS RENDERABLE
				gpu->useShader(r->getShader());

				if(s->getRenderMode() == RenderMode::PBR_IBL)
					if(s->getActiveInfiniteCubemap() != nullptr)
						gpu->useIBL(s->getActiveInfiniteCubemap());
					else if(this->activeInfiniteCubemap != nullptr)
						gpu->useIBL(this->activeInfiniteCubemap);
					else
						gpu->useIBL(App::get().getAssetManager().getInfiniteCubemap("defaultCubemap"));

				gpu->useMesh(r->getMesh());
				gpu->useMaterial(r->getMaterial());
					
				for (auto a : r->actors.getAll())
					this->drawActor(a, alpha);
			}


			// DRAW TRANSPARENTS/TRANSLUCENTS

			// TODO: not proud of this, but it gets the job done for the time being (can't use map since there's a chance keys could be the same,
			// not that using map would make this anymore acceptable for a performance crucial application)
            gpu->enableBlend();
			this->sortedTransparentActors.clear();
			for (auto& r : transparentRenderables)
			{
				for (auto a : r->actors.getAll())
				{
					float dist = glm::length(this->cameraPosition - a->getTransform().getTranslation());
					this->sortedTransparentActors.push_back(std::pair<float, Actor*>(dist, a));
				}
			}
				
			std::sort(sortedTransparentActors.begin(), sortedTransparentActors.end(), [](auto &left, auto &right) {
				return left.first < right.first;
			});

			for (std::vector<std::pair<float, Actor*>>::reverse_iterator it = sortedTransparentActors.rbegin(); it != sortedTransparentActors.rend(); ++it)
			{
				// Reset gpu state for this ACTOR and draw
				auto r = it->second->getStageRenderable().value();

				gpu->useShader(r->getShader());

				if (s->getRenderMode() == RenderMode::PBR_IBL)
					if (s->getActiveInfiniteCubemap() != nullptr)
						gpu->useIBL(s->getActiveInfiniteCubemap());
					else if (this->activeInfiniteCubemap != nullptr)
						gpu->useIBL(this->activeInfiniteCubemap);
					else
						gpu->useIBL(App::get().getAssetManager().getInfiniteCubemap("defaultCubemap"));

				gpu->useMesh(r->getMesh());
				gpu->useMaterial(r->getMaterial());

				this->drawActor(it->second, alpha);
			}
            
		}

	}

	void Scene::drawActor(Actor* a, float alphaTime)
	{
		auto gpu = App::get().getGPU();

		if (a->isVisible())
		{
			gpu->setShaderVec3("camPos", this->renderCameraPosition);
			gpu->setShaderMat4("camOffset", this->renderCameraOffset);

            gpu->setShaderMat4("projection", this->cameraProjectionMatrix);
            gpu->setShaderMat4("view", this->cameraViewMatrix);
            gpu->setShaderMat4("model", a->getWorldRenderMatrix(alphaTime));

			// If this actor is animated, send the bone transforms of it's armature to the shader
			if (a->isAnimated())
			{
				auto mesh = a->getMesh();
				auto armature = a->getArmature();

				size_t boneIndex = 0;
				if (armature->getShouldInterpolate())
				{
					for (auto& activeBone : a->getActiveBones())
					{
						//std::cout << activeBone.second << std::endl;

						// global inverse matrix does not seem to make any difference
						//glm::mat4 meshBoneTransform = mesh->getGlobalInverseMatrix() * armature->getBone(activeBone.first).getRenderMatrixInterpolated(alphaTime) * mesh->getBone(boneIndex).offsetMatrix;
						glm::mat4 meshBoneTransform = armature->getBone(activeBone.first).getRenderMatrixInterpolated(alphaTime) * mesh->getBone(boneIndex).offsetMatrix;
						gpu->setShaderMat4(activeBone.second, meshBoneTransform);
						boneIndex++;
					}
				}
				else
				{
					for (auto& activeBone : a->getActiveBones())
					{
						//glm::mat4 meshBoneTransform = mesh->getGlobalInverseMatrix() * armature->getBone(activeBone.first).getRenderMatrix() * mesh->getBone(boneIndex).offsetMatrix;
						glm::mat4 meshBoneTransform = armature->getBone(activeBone.first).getRenderMatrix() * mesh->getBone(boneIndex).offsetMatrix;
						gpu->setShaderMat4(activeBone.second, meshBoneTransform);
						boneIndex++;
					}
				}

				
			}

			gpu->drawGpuMesh();
		}
	}
}