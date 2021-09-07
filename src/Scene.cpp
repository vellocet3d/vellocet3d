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
		animationTime(0.0)
	{
		this->sortedTransparentActors.reserve(1000); // reserve space for 1000 transparent actors (won't reallocate until that limit reached)
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
		
        for(auto& name : this->hdrsInUse)
			App::get().getAssetManager().removeHdr(name);
        
		for(auto& name : this->shadersInUse)
			App::get().getAssetManager().removeShader(name);
	}

	/* Json Scene Loader
	--------------------------------------------------*/
	void Scene::loadStageConfig(std::string path)
	{
		std::ifstream i(path);
		json j;
		i >> j;
		
		// just a usage example
		//std::cout << j["scene"]["materials"][0]["ao"] << "\n";
		//for (auto& el : j.items())
		//	std::cout << el.key() << " : " << el.value() << "\n";

#ifdef DEBUG_LOG
	Log::toCliAndFile("Loading Stage via configuration file: " + path);
#endif

		//load elements into scene
		for (auto& s : j["shaders"])
			this->loadShader(s["name"], s["vert_path"], s["frag_path"]);

        for (auto& h : j["hdrs"])
			this->loadHdr(h["name"], h["path"]);

		for (auto& m : j["meshes"])
			this->loadMesh(m);

		for (auto& t : j["textures"])
		{
			std::vector<std::string> mips;
			for (auto& mip : t["mips"])
				mips.push_back(mip);

			this->loadTexture(t["name"], t["type"], t["path"], mips);
		}

		for (auto& m : j["materials"])
		{
			Material mat;
			mat.name = m["name"];

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

		for (auto& r : j["renderables"])
		{
			this->addRenderable(
				r["name"],
				this->getShader(r["shader"]),
				this->getMesh(r["mesh"]),
				this->getMaterial(r["material"])
			);
		}

		//load stage

		auto stage = this->addStage(j["name"]);

		if (j.contains("clearDepthBuffer") && !j["clearDepthBuffer"].is_null() && j["clearDepthBuffer"] != "" && j["clearDepthBuffer"])
			stage->setClearDepthBuffer(true);

        if (j.contains("activeHdr") && !j["activeHdr"].is_null() && j["activeHdr"] != "")
            stage->setActiveHdr(this->getHdr(j["activeHdr"]));
        else
			stage->setActiveHdr(this->getHdr("defaultHdr"));

		if (j.contains("drawHdr") && j["drawHdr"] != "" && !j["drawHdr"].is_null())
			stage->setDrawHdr(j["drawHdr"]);

		if (j["camera"]["type"] == "perspective")
			stage->addPerspectiveCamera(j["camera"]["near"], j["camera"]["far"], j["camera"]["fov"]);
		else if (j["camera"]["type"] == "orthographic")
			stage->addOrthographicCamera(j["camera"]["near"], j["camera"]["far"], j["camera"]["scale"]);
#ifdef DEBUG_LOG
	else
		Log::crash("Scene::loadStageConfig(): config contains a camera type other than 'perspective' or 'orthographic'");
#endif

		stage->getCamera()->setPosition(glm::vec3(
			(float)j["camera"]["position"][0],
			(float)j["camera"]["position"][1],
			(float)j["camera"]["position"][2]
		));

		stage->getCamera()->setLookAt(glm::vec3(
			(float)j["camera"]["lookat"][0],
			(float)j["camera"]["lookat"][1],
			(float)j["camera"]["lookat"][2]
		));
            
		if (j.contains("useCollisionWorld") && !j["useCollisionWorld"].is_null())
        {
            stage->setCollisionWorld();

			if (j["debug"])
				stage->getCollisionWorld()->useDebugDrawer(this->getShader("defaultDebug"));					
                
            for(auto& cs : j["collisionShapes"])
            {
                if(cs["type"] == "btCylinderShape")
                {
                    btCollisionShape* btcs = new btCylinderShape(btVector3(cs["dimensions"][0], cs["dimensions"][1], cs["dimensions"][2]));
                    stage->getCollisionWorld()->addCollisionShape(cs["name"], btcs);
                }
					
				if(cs["type"] == "btCapsuleShape")
				{
					btCollisionShape* btcs = new btCapsuleShape(cs["dimensions"][0], cs["dimensions"][1]); // total height is height+2*radius
					stage->getCollisionWorld()->addCollisionShape(cs["name"], btcs);
				}
					
				// Add support for more shapes as needed
            }
				
			for(auto& co : j["collisionObjects"])
			{
				// Add support for more properties as needed
				CollisionObjectTemplate cot;
				cot.type = co["type"];
				cot.name = co["name"];
				cot.collisionShape = stage->getCollisionWorld()->getCollisionShape(co["collisionShape"]);
				if(co.contains("mass") && !co["mass"].is_null())
					cot.mass = co["mass"];
				if(co.contains("friction") && !co["friction"].is_null())
					cot.friction = co["friction"];
				if(co.contains("restitution") && !co["restitution"].is_null())
					cot.restitution = co["restitution"];
				if(co.contains("linearDamping") && !co["linearDamping"].is_null())
					cot.linearDamping = co["linearDamping"];
				if(co.contains("angularFactor") && !co["angularFactor"].is_null())
					cot.angularFactor = btVector3(co["angularFactor"][0], co["angularFactor"][1], co["angularFactor"][2]);
				if(co.contains("activationState") && !co["activationState"].is_null())
					cot.activationState = co["activationState"];
				if(co.contains("gravity") && !co["gravity"].is_null())
					cot.gravity = btVector3(co["gravity"][0], co["gravity"][1], co["gravity"][2]);
					
				stage->getCollisionWorld()->addCollisionObjectTemplate(cot.name, cot);
			}
				
        }

		for (auto& a : j["actors"])
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
				
				
			if(a.contains("collisionObject") && !a["collisionObject"].is_null())
			{	
				if((a["collisionObject"] != "GENERATE_STATIC_RIGIDBODY") && (a["collisionObject"] != "GENERATE_STATIC_GHOST"))
				{
					auto& cot = stage->getCollisionWorld()->getCollisionObjectTemplate(a["collisionObject"]);
					auto actorTranslation = pActor->getTransform().getTranslation();
						
					btTransform theTransform;
					theTransform.setIdentity();
					theTransform.setOrigin(btVector3(actorTranslation.x, actorTranslation.y, actorTranslation.z));
					theTransform.setRotation(vel::glmToBulletQuat(pActor->getTransform().getRotation()));
						
					if(cot.type == "rigidBody")
					{
						btVector3 theInertia(0.0f, 0.0f, 0.0f);
						cot.collisionShape->calculateLocalInertia(cot.mass.value(), theInertia);
							
						btDefaultMotionState* theMotionState = new btDefaultMotionState(theTransform);
						btRigidBody::btRigidBodyConstructionInfo theBodyInfo(cot.mass.value(), theMotionState, cot.collisionShape, theInertia);
							
						// Add support for more properties as needed
						if(cot.friction)
							theBodyInfo.m_friction = cot.friction.value();
						if(cot.restitution)
							theBodyInfo.m_restitution = cot.restitution.value();
						if(cot.linearDamping)
							theBodyInfo.m_linearDamping = cot.linearDamping.value();
							
						btRigidBody* theRigidBody = new btRigidBody(theBodyInfo);
						if(cot.gravity)
							theRigidBody->setGravity(cot.gravity.value());
						if(cot.angularFactor)
							theRigidBody->setAngularFactor(cot.angularFactor.value());
						if(cot.activationState)
							theRigidBody->setActivationState(cot.activationState.value());

						theRigidBody->setUserPointer(pActor);
						stage->getCollisionWorld()->getDynamicsWorld()->addRigidBody(theRigidBody);
							
						pActor->setRigidBody(theRigidBody);
					}
					else if(cot.type == "ghostObject")
					{
						btPairCachingGhostObject* theGhostObject = new btPairCachingGhostObject();
						theGhostObject->setCollisionShape(cot.collisionShape);
						theGhostObject->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
						theGhostObject->setUserPointer(pActor);
							
						stage->getCollisionWorld()->getDynamicsWorld()->addCollisionObject(theGhostObject,
						btBroadphaseProxy::KinematicFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
							
						pActor->setGhostObject(theGhostObject);
					}
				}
				else
				{
					if(a["collisionObject"] == "GENERATE_STATIC_RIGIDBODY")
					{
						stage->getCollisionWorld()->addStaticCollisionBody(pActor);
					}
					else if(a["collisionObject"] == "GENERATE_STATIC_GHOST")
					{
						btPairCachingGhostObject* theGhostObject = new btPairCachingGhostObject();
						theGhostObject->setCollisionShape(stage->getCollisionWorld()->collisionShapeFromActor(pActor));
						theGhostObject->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
						theGhostObject->setUserPointer(pActor);
							
						stage->getCollisionWorld()->getDynamicsWorld()->addCollisionObject(theGhostObject,
						btBroadphaseProxy::KinematicFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
							
						pActor->setGhostObject(theGhostObject);
					}
				}
					
			}
		}

		for (auto& a : j["armatures"])
		{
			std::vector<std::string> actorNames;

			for (auto& actName : a["actors"])
				actorNames.push_back(actName);

			stage->addArmature(this->getArmature(a["base"]), a["defaultAnimation"], actorNames);
		}

		for (auto& p : j["parenting"])
		{
			auto parent = stage->getActor(p["parent"]);
			for (auto& c : p["children"])
				stage->getActor(c)->setParentActor(parent);
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
    
    void Scene::loadHdr(std::string name, std::string path)
    {
        this->hdrsInUse.push_back(App::get().getAssetManager().loadHdr(name, path));
    }
	
	void Scene::addMaterial(Material m)
	{
		if (!m.hasAlphaChannel && m.albedo && m.albedo->alphaChannel)
			m.hasAlphaChannel = true;
		if (!m.hasAlphaChannel && m.normal && m.normal->alphaChannel)
			m.hasAlphaChannel = true;
		if (!m.hasAlphaChannel && m.metallic && m.metallic->alphaChannel)
			m.hasAlphaChannel = true;
		if (!m.hasAlphaChannel && m.roughness && m.roughness->alphaChannel)
			m.hasAlphaChannel = true;
		if (!m.hasAlphaChannel && m.ao && m.ao->alphaChannel)
			m.hasAlphaChannel = true;

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
    
    HDR* Scene::getHdr(std::string name)
    {
        return App::get().getAssetManager().getHdr(name);
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
		//Log::toCli("----------------------------------------------------");
		//Log::toCli("NEW RENDER PASS");
		//Log::toCli("----------------------------------------------------");

		auto gpu = App::get().getGPU(); // for convenience	
		
        gpu->disableBlend(); // disable blending for opaque objects for performance (was fine before, but this should be even better)

		for (auto& s : this->stages.getAll())
		{
			if (!s.isVisible())
				continue;

			// clear depth buffer if flag set in stage
			if (s.getClearDepthBuffer())
				gpu->clearDepthBuffer();


			// should always have a camera if we've made it this far
			s.getCamera()->update();
			this->cameraPosition = s.getCamera()->getPosition();
			this->cameraProjectionMatrix = s.getCamera()->getProjectionMatrix();
			this->cameraViewMatrix = s.getCamera()->getViewMatrix();

			this->IBLCameraPosition = s.getIBLCamera() == nullptr ? this->cameraPosition : s.getIBLCamera()->getPosition();
			//this->IBLCameraPosition = this->cameraPosition;
			this->IBLOffsetMatrix = s.getIBLCamera() == nullptr ? glm::mat4(1.0f) : glm::inverse(s.getIBLCamera()->getViewMatrix());
			//this->IBLOffsetMatrix = s.getIBLCamera() == nullptr ? glm::mat4(1.0f) : s.getIBLCamera()->getViewMatrix();
			//this->IBLOffsetMatrix = glm::mat4(1.0f);


			// if debug drawer set, do debug draw
			if (s.getCollisionWorld() != nullptr && s.getCollisionWorld()->getDebugDrawer() != nullptr)
			{
				s.getCollisionWorld()->getDynamicsWorld()->debugDrawWorld(); // load vertices into associated CollisionDebugDrawer
				gpu->useShader(s.getCollisionWorld()->getDebugDrawer()->getShaderProgram());
				gpu->setShaderMat4("vp", s.getCamera()->getProjectionMatrix() * s.getCamera()->getViewMatrix());
				gpu->debugDrawCollisionWorld(s.getCollisionWorld()->getDebugDrawer()); // draw all loaded vertices with a single call and clear
			}


			// when we gpu load things, we're altering the state in order to load those elements,
			// so we need to reset actives so that the gpu knows to reset state to what we're suppose to be drawing
			gpu->resetActives(); 
			
			auto& stageRenderables = s.getRenderables();
			plf::colony<plf::colony<Renderable>::iterator> transparentRenderables;
			
			for (plf::colony<Renderable>::iterator it = stageRenderables.begin(); it != stageRenderables.end(); ++it)
			{
				if (it->getMaterialHasAlpha())
				{
					transparentRenderables.insert(it);
					continue;
				}

				// DRAW OPAQUES

				// Only do state switches if we need to. We used to sort renderables when they were added,
				// but after the massive refactor we removed this feature as everytime a renderable was added
				// we had to sort on every renderable, which if you have 1000s of renderables would be a significant
				// performance hit. So now gpu state switches are hit or miss. All actors that use a specific renderable
				// do not require any state change, but when switching between renderables, if the next renderable in the
				// queue uses a different mesh than the current renderable, the gpu state would be switched, then if the renderable
				// after that used the mesh from the first renderable it would have to switch back again, instead of these renderables
				// being sorted so that everything is hit in order minimizing state switches.
				auto r = *it;
				if (r.getShader() != gpu->getActiveShader())
					gpu->useShader(r.getShader());
				if (s.getActiveHdr() != gpu->getActiveHdr())
					gpu->useHdr(s.getActiveHdr());
				if (r.getMesh() != gpu->getActiveMesh())
					gpu->useMesh(r.getMesh());
				if (r.getMaterial() != gpu->getActiveMaterial())
					gpu->useMaterial(r.getMaterial());			

				for (auto& a : r.actors.getAll())
					this->drawActor(a, alpha);
			}


			// Draw cubemap skybox (must be done before transparents)
			if (s.getDrawHdr())
			{
				gpu->drawSkybox(this->cameraProjectionMatrix, this->cameraViewMatrix, s.getActiveHdr()->envCubemap);
				gpu->resetActives();
			}
			


			// DRAW TRANSPARENTS

			// TODO: not proud of this, but it get's the job done for the time being (can't use map since there's a chance keys could be the same,
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
				auto r = it->second->getStageRenderable().value();
				if (r->getShader() != gpu->getActiveShader())
					gpu->useShader(r->getShader());
                if (s.getActiveHdr() != gpu->getActiveHdr())
					gpu->useHdr(s.getActiveHdr());
				if (r->getMesh() != gpu->getActiveMesh())
					gpu->useMesh(r->getMesh());
				if (r->getMaterial() != gpu->getActiveMaterial())
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
			//gpu->setShaderVec3("camPos", this->cameraPosition);
			gpu->setShaderVec3("camPos", this->IBLCameraPosition);
			gpu->setShaderMat4("iblOffset", this->IBLOffsetMatrix);

            gpu->setShaderMat4("projection", this->cameraProjectionMatrix);
            gpu->setShaderMat4("view", this->cameraViewMatrix);
            gpu->setShaderMat4("model", a->getWorldRenderMatrix(alphaTime));

			// If this actor is animated, send the bone transforms of it's armature to the shader
			if (a->isAnimated())
			{
				auto mesh = a->getMesh();
				auto armature = a->getArmature();

				size_t boneIndex = 0;
				for (auto& activeBone : a->getActiveBones())
				{
					glm::mat4 meshBoneTransform = mesh->getGlobalInverseMatrix() * armature->getBone(activeBone.first).getRenderMatrix(alphaTime) * mesh->getBone(boneIndex).offsetMatrix;
					gpu->setShaderMat4(activeBone.second, meshBoneTransform);
					boneIndex++;
				}
			}

			gpu->drawGpuMesh();
		}
	}
}