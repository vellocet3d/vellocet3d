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

	/* Json Scene Loader
	* NOTE: This is the old version before we wrote gui scenebuilder
	* retaining until I'm done implementing that process as this will
	* change to load in json files with different structures
	--------------------------------------------------*/
	void Scene::loadConfigFile(std::string path)
	{
//		std::ifstream i(path);
//		json j;
//		i >> j;
//		
//		// just a usage example
//		//std::cout << j["scene"]["materials"][0]["ao"] << "\n";
//		//for (auto& el : j.items())
//		//	std::cout << el.key() << " : " << el.value() << "\n";
//
//#ifdef DEBUG_LOG
//	Log::toCliAndFile("Loading Scene via configuration file: " + path);
//#endif
//
//		// Load user defined shaders
//		for (auto& s : j["shaders"])
//			this->loadShader(s["name"], s["vert_path"], s["frag_path"]);
//		
//		/* Broke this when refactored for AssetManager to manage cameras
//		// Add cameras to scene
//		for (auto& c : j["cameras"])
//		{
//			Camera* tmpCamPtr;
//			if (c["type"] == "perspective")
//				tmpCamPtr = this->cameras.insert(Camera(c["name"], CameraType::PERSPECTIVE, c["near"], c["far"], c["fovOrScale"]));
//			else if (c["type"] == "orthographic")
//				tmpCamPtr = this->cameras.insert(Camera(c["name"], CameraType::ORTHOGRAPHIC, c["near"], c["far"], c["fovOrScale"]));
//#ifdef DEBUG_LOG
//			else
//				Log::crash("Scene::loadConfigFile(): config contains a camera type other than 'perspective' or 'orthographic'");
//#endif
//
//			tmpCamPtr->setPosition(glm::vec3(
//				(float)c["position"][0],
//				(float)c["position"][1],
//				(float)c["position"][2]
//			));
//
//			tmpCamPtr->setLookAt(glm::vec3(
//				(float)c["lookat"][0],
//				(float)c["lookat"][1],
//				(float)c["lookat"][2]
//			));
//		}
//
//		// Update sceneCamera if provided
//		if (j.contains("sceneCamera") && !j["sceneCamera"].is_null() && j["sceneCamera"] != "")
//			this->sceneCamera = this->cameras.get(j["sceneCamera"]);
//		*/
//
//
//		// Load meshes from .fbx files, will also load in associated armatures
//		//for (auto& m : j["meshes"])
//		//	this->loadMesh(m);
//			
//
//		// Load textures and their associated mip chains
//		for (auto& t : j["textures"])
//		{
//			std::vector<std::string> mips;
//			for (auto& mip : t["mips"])
//				mips.push_back(mip);
//
//			this->loadTexture(t["name"], t["type"], t["path"], mips);
//		}
//
//		// Load materials which are created from previously loaded textures
//		//for (auto& m : j["materials"])
//		//{
//		//	Material mat;
//		//	mat.name = m["name"];
//
//		//	if (m.contains("color") && m["color"] != "" && !m["color"].is_null())
//		//	{
//		//		mat.color = glm::vec4(
//		//			(float)m["color"][0],
//		//			(float)m["color"][1],
//		//			(float)m["color"][2],
//		//			(float)m["color"][3]
//		//		);
//		//	}
//
//		//	if (m.contains("diffuse") && m["diffuse"] != "" && !m["diffuse"].is_null())
//		//		mat.diffuse = this->getTexture(m["diffuse"]);
//
//		//	this->addMaterial(mat);
//		//}
//
//		// Load renderables
//		for (auto& r : j["renderables"])
//		{
//			this->addRenderable(
//				r["name"],
//				this->getShader(r["shader"]),
//				this->getMesh(r["mesh"]),
//				this->getMaterial(r["material"])
//			);
//		}
//
//		// Load collision worlds
//		for(auto& cwj : j["collisionWorlds"])
//		{
//			auto cw = this->addCollisionWorld(cwj["name"], cwj["gravity"]);
//			cw->setIsActive(cwj["active"]);
//			
//			if (cwj["debug"])
//			{
//				cw->useDebugDrawer(this->getShader("defaultDebug"));
//				cw->setCamera(this->getCamera(cwj["debugCamera"]));
//			}
//
//			for (auto& cs : cwj["collisionShapes"])
//			{
//				if (cs["type"] == "btCylinderShape")
//				{
//					btCollisionShape* btcs = new btCylinderShape(btVector3(cs["dimensions"][0], cs["dimensions"][1], cs["dimensions"][2]));
//					cw->addCollisionShape(cs["name"], btcs);
//				}
//
//				if (cs["type"] == "btCapsuleShape")
//				{
//					btCollisionShape* btcs = new btCapsuleShape(cs["dimensions"][0], cs["dimensions"][1]); // total height is height+2*radius
//					cw->addCollisionShape(cs["name"], btcs);
//				}
//
//				// Add support for more shapes as needed
//			}
//
//			for (auto& co : cwj["collisionObjects"])
//			{
//				// Add support for more properties as needed
//				CollisionObjectTemplate cot;
//				cot.type = co["type"];
//				cot.name = co["name"];
//				cot.collisionShape = cw->getCollisionShape(co["collisionShape"]);
//				if (co.contains("mass") && !co["mass"].is_null())
//					cot.mass = co["mass"];
//				if (co.contains("friction") && !co["friction"].is_null())
//					cot.friction = co["friction"];
//				if (co.contains("restitution") && !co["restitution"].is_null())
//					cot.restitution = co["restitution"];
//				if (co.contains("linearDamping") && !co["linearDamping"].is_null())
//					cot.linearDamping = co["linearDamping"];
//				if (co.contains("angularFactor") && !co["angularFactor"].is_null())
//					cot.angularFactor = btVector3(co["angularFactor"][0], co["angularFactor"][1], co["angularFactor"][2]);
//				if (co.contains("activationState") && !co["activationState"].is_null())
//					cot.activationState = co["activationState"];
//				if (co.contains("gravity") && !co["gravity"].is_null())
//					cot.gravity = btVector3(co["gravity"][0], co["gravity"][1], co["gravity"][2]);
//
//				cw->addCollisionObjectTemplate(cot.name, cot);
//			}
//		}
//		
//
//		// Load stages
//		for (auto& s : j["stages"])
//		{
//			auto stage = this->addStage(s["name"]);
//
//
//			if (s.contains("clearDepthBuffer") && !s["clearDepthBuffer"].is_null() && s["clearDepthBuffer"] != "" && s["clearDepthBuffer"])
//				stage->setClearDepthBuffer(true);
//
//			/* broke this when refactor for asset manager to track cameras
//			if (s.contains("camera") && s["camera"] != "" && !s["camera"].is_null())
//				stage->setCamera(this->cameras.get(s["camera"]));
//			*/
//
//
//
//
//			for (auto& a : s["actors"])
//			{
//				auto act = Actor(a["name"]);
//				act.setDynamic(a["dynamic"]);
//				act.setVisible(a["visible"]);
//				act.setAutoTransform(a["autoTransform"]);
//				act.addRenderable(this->getRenderable(a["renderable"]));//TODO: what if headless?
//
//				if (!a["transform"].is_null())
//				{
//					auto trans = a["transform"]["translation"];
//					auto rot = a["transform"]["rotation"];
//					auto scale = a["transform"]["scale"];
//
//					if (!trans.is_null())
//					{
//						act.getTransform().setTranslation(glm::vec3(
//							(float)trans[0],
//							(float)trans[1],
//							(float)trans[2]
//						));
//					}
//
//					if (!rot.is_null())
//					{
//						if (rot["type"] == "euler")
//						{
//							act.getTransform().setRotation((float)rot["val"]["angle"], glm::vec3(
//								(float)rot["val"]["axis"][0],
//								(float)rot["val"]["axis"][1],
//								(float)rot["val"]["axis"][2]
//							));
//						}
//						else if (rot["type"] == "quaternion")
//						{
//							act.getTransform().setRotation(glm::quat(
//								(float)rot["val"][3],
//								(float)rot["val"][0],
//								(float)rot["val"][1],
//								(float)rot["val"][2]
//							));
//						}
//#ifdef DEBUG_LOG
//						else
//							Log::crash("Scene::loadSceneConfig(): config contains an actor transform rotation type other than 'euler' or 'quaternion'");
//#endif
//
//					}
//
//					if (!scale.is_null())
//					{
//						act.getTransform().setScale(glm::vec3(
//							(float)scale[0],
//							(float)scale[1],
//							(float)scale[2]
//						));
//					}
//				}
//
//
//				// must get final memory address of actor in order to pass userptr to rididbody or ghostobject, so we add actor to stage
//				// here, meaning everything above this line would be properties that the actor MUST HAVE before being added to stage,
//				// although at this time a Renderable is all that it needs to have before being added.
//				Actor* pActor = stage->addActor(act);
//
//
//				if (a.contains("collisionWorld") && !a["collisionWorld"].is_null() && a.contains("collisionObject") && !a["collisionObject"].is_null())
//				{
//					pActor->setCollisionWorld(this->getCollisionWorld(a["collisionWorld"]));
//					
//					if ((a["collisionObject"] != "GENERATE_STATIC_RIGIDBODY") && (a["collisionObject"] != "GENERATE_STATIC_GHOST"))
//					{
//						auto& cot = pActor->getCollisionWorld()->getCollisionObjectTemplate(a["collisionObject"]);
//						auto actorTranslation = pActor->getTransform().getTranslation();
//
//						btTransform theTransform;
//						theTransform.setIdentity();
//						theTransform.setOrigin(btVector3(actorTranslation.x, actorTranslation.y, actorTranslation.z));
//						theTransform.setRotation(vel::glmToBulletQuat(pActor->getTransform().getRotation()));
//
//						if (cot.type == "rigidBody")
//						{
//							btVector3 theInertia(0.0f, 0.0f, 0.0f);
//							cot.collisionShape->calculateLocalInertia(cot.mass.value(), theInertia);
//
//							btDefaultMotionState* theMotionState = new btDefaultMotionState(theTransform);
//							btRigidBody::btRigidBodyConstructionInfo theBodyInfo(cot.mass.value(), theMotionState, cot.collisionShape, theInertia);
//
//							// Add support for more properties as needed
//							if (cot.friction)
//								theBodyInfo.m_friction = cot.friction.value();
//							if (cot.restitution)
//								theBodyInfo.m_restitution = cot.restitution.value();
//							if (cot.linearDamping)
//								theBodyInfo.m_linearDamping = cot.linearDamping.value();
//
//							btRigidBody* theRigidBody = new btRigidBody(theBodyInfo);
//							if (cot.gravity)
//								theRigidBody->setGravity(cot.gravity.value());
//							if (cot.angularFactor)
//								theRigidBody->setAngularFactor(cot.angularFactor.value());
//							if (cot.activationState)
//								theRigidBody->setActivationState(cot.activationState.value());
//
//							theRigidBody->setUserPointer(pActor);
//							pActor->getCollisionWorld()->getDynamicsWorld()->addRigidBody(theRigidBody);
//
//							pActor->setRigidBody(theRigidBody);
//						}
//						else if (cot.type == "ghostObject")
//						{
//							btPairCachingGhostObject* theGhostObject = new btPairCachingGhostObject();
//							theGhostObject->setCollisionShape(cot.collisionShape);
//							theGhostObject->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
//							theGhostObject->setUserPointer(pActor);
//
//							pActor->getCollisionWorld()->getDynamicsWorld()->addCollisionObject(theGhostObject,
//								btBroadphaseProxy::KinematicFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
//
//							pActor->setGhostObject(theGhostObject);
//						}
//					}
//					else
//					{
//						if (a["collisionObject"] == "GENERATE_STATIC_RIGIDBODY")
//						{
//							pActor->getCollisionWorld()->addStaticCollisionBody(pActor);
//						}
//						else if (a["collisionObject"] == "GENERATE_STATIC_GHOST")
//						{
//							btPairCachingGhostObject* theGhostObject = new btPairCachingGhostObject();
//							theGhostObject->setCollisionShape(pActor->getCollisionWorld()->collisionShapeFromActor(pActor));
//							theGhostObject->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
//							theGhostObject->setUserPointer(pActor);
//
//							pActor->getCollisionWorld()->getDynamicsWorld()->addCollisionObject(theGhostObject,
//								btBroadphaseProxy::KinematicFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
//
//							pActor->setGhostObject(theGhostObject);
//						}
//					}
//
//				}
//
//			// end of actor foreach
//			}
//
//			for (auto& a : s["armatures"])
//			{
//				std::vector<std::string> actorNames;
//
//				for (auto& actName : a["actors"])
//					actorNames.push_back(actName);
//
//				auto arm = stage->addArmature(this->getArmature(a["base"]), a["defaultAnimation"], actorNames);
//
//				if (a.contains("shouldInterpolate") && !a["shouldInterpolate"].is_null())
//					arm->setShouldInterpolate(a["shouldInterpolate"]);
//
//
//				if (!a["transform"].is_null())
//				{
//					auto trans = a["transform"]["translation"];
//					auto rot = a["transform"]["rotation"];
//					auto scale = a["transform"]["scale"];
//
//					if (!trans.is_null())
//					{
//						arm->getTransform().setTranslation(glm::vec3(
//							(float)trans[0],
//							(float)trans[1],
//							(float)trans[2]
//						));
//					}
//
//					if (!rot.is_null())
//					{
//						if (rot["type"] == "euler")
//						{
//							arm->getTransform().setRotation((float)rot["val"]["angle"], glm::vec3(
//								(float)rot["val"]["axis"][0],
//								(float)rot["val"]["axis"][1],
//								(float)rot["val"]["axis"][2]
//							));
//						}
//						else if (rot["type"] == "quaternion")
//						{
//							arm->getTransform().setRotation(glm::quat(
//								(float)rot["val"][3],
//								(float)rot["val"][0],
//								(float)rot["val"][1],
//								(float)rot["val"][2]
//							));
//						}
//#ifdef DEBUG_LOG
//						else
//							Log::crash("Scene::loadSceneConfig(): config contains an armature transform rotation type other than 'euler' or 'quaternion'");
//#endif
//
//					}
//
//					if (!scale.is_null())
//					{
//						arm->getTransform().setScale(glm::vec3(
//							(float)scale[0],
//							(float)scale[1],
//							(float)scale[2]
//						));
//					}
//				}
//
//			// end of armatures foreach
//			}
//
//			for (auto& p : s["parenting"])
//			{
//				auto parent = stage->getActor(p["parent"]);
//				for (auto& c : p["children"])
//					stage->getActor(c)->setParentActor(parent);
//			}
//
//		// end stage foreach
//		}
		
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
					if (r->getMaterialHasAlpha())
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
					gpu->useMaterial(r->getMaterial());

					for (auto a : r->actors.getAll())
					{
						// if this actor has a color with a transparent component less than fully opaque, push it onto
						// transparentActors queue and continue the loop without drawing the actor
						if (a->getColor().w < 1.0f)
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

					gpu->useShader(r->getShader());
					gpu->useMesh(r->getMesh());
					gpu->useMaterial(r->getMaterial());

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
					gpu->drawScreen(c->getRenderTarget()->texture.dsaHandle);
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