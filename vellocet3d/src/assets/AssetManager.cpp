#include <thread> 
#include <chrono>

#include "vel/assets/AssetManager.h"
#include "vel/assets/AssetLoaderV2.h"
#include "vel/App.h"

using namespace std::chrono_literals;

namespace vel
{

	AssetManager::AssetManager(){}
	AssetManager::~AssetManager() {}


	void AssetManager::sendAllToGpu()
	{
		for(auto& s : this->shadersThatNeedGpuLoad)
		{
			App::get().getGPU()->loadShader(s->ptr);
			s->gpuLoaded = true;
			this->shadersThatNeedGpuLoad.pop_front();
		}
		
		for(auto& m : this->meshesThatNeedGpuLoad)
		{
			App::get().getGPU()->loadMesh(m->ptr);
			m->gpuLoaded = true;
			this->meshesThatNeedGpuLoad.pop_front();
		}
		
		for(auto& t : this->texturesThatNeedGpuLoad)
		{
			App::get().getGPU()->loadTexture(t->ptr);
			t->gpuLoaded = true;
			this->texturesThatNeedGpuLoad.pop_front();
		}
	}

	void AssetManager::sendNextToGpu()
	{
		if (this->shadersThatNeedGpuLoad.size() > 0)
		{
			auto shaderTracker = this->shadersThatNeedGpuLoad.at(0);
			App::get().getGPU()->loadShader(shaderTracker->ptr);
			shaderTracker->gpuLoaded = true;
			this->shadersThatNeedGpuLoad.pop_front();
			return;
		}

		if (this->meshesThatNeedGpuLoad.size() > 0)
		{
			auto meshTracker = this->meshesThatNeedGpuLoad.at(0);
			App::get().getGPU()->loadMesh(meshTracker->ptr);
			meshTracker->gpuLoaded = true;
			this->meshesThatNeedGpuLoad.pop_front();
			return;
		}

		if (this->texturesThatNeedGpuLoad.size() > 0)
		{
			auto textureTracker = this->texturesThatNeedGpuLoad.at(0);
			App::get().getGPU()->loadTexture(textureTracker->ptr);
			textureTracker->gpuLoaded = true;
			this->texturesThatNeedGpuLoad.pop_front();
			return;
		}
	}

	/* Shaders
	--------------------------------------------------*/
	std::string AssetManager::loadShader(std::string name, std::string vertFile, std::string fragFile)
	{
		if (this->shaderTrackerMap.contains(name)) 
		{
#ifdef DEBUG_ASSET_MANAGEMENT
	std::cout << "Existing Shader, bypass reload: " << name << std::endl;
#endif
			
			// if usage count is not greater than zero then the asset is being deleted on the main thread
			// so we will need to re-create it. This could still potentially be a race condition, although
			// I would think it would be unlikely, putting a TODO here until we thoroughly test some 
			// real world use cases to verify
			if(this->shaderTrackerMap[name]->usageCount > 0)
			{
				this->shaderTrackerMap[name]->usageCount++;
				return name;
			}
			// if name exists in map, AND usage count IS EQUAL TO 0, then we must have called this method
			// from a thread other than the main thread. Therefore, let us poll until the name no longer 
			// exists within the map so we do not recreate the asset while it's being deleted
			else
			{
				std::this_thread::sleep_for(100ms);
				return this->loadShader(name, vertFile, fragFile);
			}			
		}
		
#ifdef DEBUG_ASSET_MANAGEMENT
	std::cout << "Loading new Shader: " << name << std::endl;
#endif		

		Shader s;
		s.name = name;
		s.vertFile = vertFile;
		s.fragFile = fragFile;

		auto it = this->shaders.insert(s);
		
		ShaderTracker t;
		t.ptr = &(*it);
		t.usageCount++;
		
		auto it2 = this->shaderTrackers.insert(t);
		auto tmpTracker = &(*it2);

		this->shadersThatNeedGpuLoad.push_back(tmpTracker);

		this->shaderTrackerMap[name] = tmpTracker;

		return name;
	}

	Shader* AssetManager::getShader(std::string name)
	{
		if(!this->shaderTrackerMap.contains(name))
			App::get().logger.log(("AssetManager::getShader(): Attempting to get shader that does not exist: " + name));

		return this->shaderTrackerMap[name]->ptr;
	}

	bool AssetManager::shaderIsGpuLoaded(std::string name)
	{
		if (!this->shaderTrackerMap.contains(name))
			App::get().logger.log(("AssetManager::shaderIsGpuLoaded(): Attempting to get shader that does not exist: " + name));

		return this->shaderTrackerMap[name]->gpuLoaded;
	}

	void AssetManager::removeShader(std::string name)
	{
		if (!this->shaderTrackerMap.contains(name))
			App::get().logger.log(("AssetManager::removeShader(): Attempting to remove shader that does not exist: " + name));
		

		auto t = this->shaderTrackerMap[name];
		t->usageCount--;
		if (t->usageCount == 0)
		{
			
#ifdef DEBUG_ASSET_MANAGEMENT
	std::cout << "Full remove Shader: " << name << std::endl;
#endif	
			
			if (!t->gpuLoaded)
				for (size_t i = 0; i < this->shadersThatNeedGpuLoad.size(); i++)
					if (this->shadersThatNeedGpuLoad.at(i) == t)
						this->shadersThatNeedGpuLoad.erase(this->shadersThatNeedGpuLoad.begin() + i);
			else
				App::get().getGPU()->clearShader(t->ptr);
			
			this->shaders.erase(this->shaders.get_iterator(t->ptr));
			this->shaderTrackers.erase(this->shaderTrackers.get_iterator(t));
			this->shaderTrackerMap.erase(name);			
		}
		
#ifdef DEBUG_ASSET_MANAGEMENT
	else
	{
		std::cout << "Decrement Shader usageCount, retain: " << name << std::endl;
	}
#endif	
		
	}

	/* Meshes
	--------------------------------------------------*/
	std::pair<std::vector<std::string>, std::string> AssetManager::loadMesh(std::string path)
	{
		auto al = AssetLoaderV2(this, path);
		al.load();

		std::pair<std::vector<std::string>, std::string> out;
		auto trackers = al.getTrackers();

		for (auto& mt : trackers.first)
			out.first.push_back(mt->ptr->getName());

		out.second = trackers.second == nullptr ? "" : trackers.second->ptr->getName();

		return out;
	}

	MeshTracker* AssetManager::addMesh(Mesh m)
	{
		// AssetLoader checks for existing mesh by name, therefore if we have
		// made it this far, assume that m is a new Mesh
		
		auto it = this->meshes.insert(m);
		
		MeshTracker t;
		t.ptr = &(*it);
		t.usageCount++;
		
		auto it2 = this->meshTrackers.insert(t);

		auto tmpTracker = &(*it2);

		this->meshesThatNeedGpuLoad.push_back(tmpTracker);

		this->meshTrackerMap[m.getName()] = tmpTracker;

		return tmpTracker;
	}

	MeshTracker* AssetManager::getMeshTracker(std::string name)
	{
		if(this->meshTrackerMap.contains(name))
		{
			auto t = this->meshTrackerMap[name];
			if(t->usageCount > 0)
			{
				t->usageCount++;
				return t;
			}
			else
			{
				std::this_thread::sleep_for(100ms);
				return this->getMeshTracker(name);
			}
		}
		
		return nullptr;
	}

	Mesh* AssetManager::getMesh(std::string name)
	{
		if (!this->meshTrackerMap.contains(name))
			App::get().logger.log(("AssetManager::getMesh(): Attempting to get mesh that does not exist: " + name));

		return this->meshTrackerMap[name]->ptr;	
	}

	bool AssetManager::meshIsGpuLoaded(std::string name)
	{
		return this->meshTrackerMap[name]->gpuLoaded;
	}
	
	void AssetManager::removeMesh(std::string name)
	{
		if (!this->meshTrackerMap.contains(name))
			App::get().logger.log(("AssetManager::removeMesh(): Attempting to remove mesh that does not exist: " + name));
		
		auto t = this->meshTrackerMap[name];
		t->usageCount--;
		if(t->usageCount == 0)
		{
#ifdef DEBUG_ASSET_MANAGEMENT
	std::cout << "Full remove Mesh: " << name << std::endl;
#endif
			if(!t->gpuLoaded)
				for (size_t i = 0; i < this->meshesThatNeedGpuLoad.size(); i++)
					if (this->meshesThatNeedGpuLoad.at(i) == t)
						this->meshesThatNeedGpuLoad.erase(this->meshesThatNeedGpuLoad.begin() + i);
			else
				App::get().getGPU()->clearMesh(t->ptr);
			
			this->meshes.erase(this->meshes.get_iterator(t->ptr));
			this->meshTrackers.erase(this->meshTrackers.get_iterator(t));
			this->meshTrackerMap.erase(name);
		}
#ifdef DEBUG_ASSET_MANAGEMENT
	else
	{
		std::cout << "Decrement Mesh usageCount, retain: " << name << std::endl;
	}
#endif
		
	}

	/* Textures
	--------------------------------------------------*/
	std::string AssetManager::loadTexture(std::string name, std::string type, std::string path, std::vector<std::string> mips)
	{	
		if (this->textureTrackerMap.contains(name)) 
		{
#ifdef DEBUG_ASSET_MANAGEMENT
	std::cout << "Existing Texture, bypass reload: " << name << std::endl;
#endif
			auto t = this->textureTrackerMap[name];
			if(t->usageCount > 0)
			{
				t->usageCount++;
				return name;
			}
			else
			{
				std::this_thread::sleep_for(100ms);
				return this->loadTexture(name, type, path, mips);
			}			
		}

#ifdef DEBUG_ASSET_MANAGEMENT
	std::cout << "Load new Texture: " << name << std::endl;
#endif

		Texture texture;
		texture.name = name;
		texture.type = type;
		texture.path = path;
		texture.mips = mips;

		auto it = this->textures.insert(texture);
		
		TextureTracker t;
		t.ptr = &(*it);
		t.usageCount++;
		
		auto it2 = this->textureTrackers.insert(t);

		auto tmpTracker = &(*it2);

		this->texturesThatNeedGpuLoad.push_back(tmpTracker);

		this->textureTrackerMap[name] = tmpTracker;

		return name;
	}

	Texture* AssetManager::getTexture(std::string name)
	{
		if (!this->textureTrackerMap.contains(name))
			App::get().logger.log(("AssetManager::getTexture(): Attempting to get texture that does not exist: " + name));

		return this->textureTrackerMap[name]->ptr;		
	}

	bool AssetManager::textureIsGpuLoaded(std::string name)
	{
		return this->textureTrackerMap[name]->gpuLoaded;
	}
	
	void AssetManager::removeTexture(std::string name)
	{
		if (!this->textureTrackerMap.contains(name))
			App::get().logger.log(("AssetManager::removeTexture(): Attempting to remove texture that does not exist: " + name));

		auto t = this->textureTrackerMap[name];
		t->usageCount--;
		if (t->usageCount == 0)
		{
#ifdef DEBUG_ASSET_MANAGEMENT
	std::cout << "Full remove Texture: " << name << std::endl;
#endif
			if (!t->gpuLoaded)
				for (size_t i = 0; i < this->texturesThatNeedGpuLoad.size(); i++)
					if (this->texturesThatNeedGpuLoad.at(i) == t)
						this->texturesThatNeedGpuLoad.erase(this->texturesThatNeedGpuLoad.begin() + i);
			else
				App::get().getGPU()->clearTexture(t->ptr);
			
			this->textures.erase(this->textures.get_iterator(t->ptr));
			this->textureTrackers.erase(this->textureTrackers.get_iterator(t));
			this->textureTrackerMap.erase(name);
		}
#ifdef DEBUG_ASSET_MANAGEMENT
	else
	{
		std::cout << "Decrement Texture usageCount, retain: " << name << std::endl;
	}
#endif	
	}

	/* Materials
	--------------------------------------------------*/
	std::string AssetManager::addMaterial(Material m)
	{
		if (this->materialTrackerMap.contains(m.name)) 
		{
#ifdef DEBUG_ASSET_MANAGEMENT
	std::cout << "Existing Material, bypass reload: " << m.name << std::endl;
#endif
			auto t = this->materialTrackerMap[m.name];
			if(t->usageCount > 0)
			{
				t->usageCount++;
				return m.name;
			}
			else
			{
				std::this_thread::sleep_for(100ms);
				return this->addMaterial(m);
			}			
		}

#ifdef DEBUG_ASSET_MANAGEMENT
	std::cout << "Loading new Material: " << m.name << std::endl;
#endif		

		auto it = this->materials.insert(m);
		
		MaterialTracker t;
		t.ptr = &(*it);
		t.usageCount++;
		
		auto it2 = this->materialTrackers.insert(t);

		this->materialTrackerMap[m.name] = &(*it2);

		return m.name;
	}

	Material* AssetManager::getMaterial(std::string name)
	{
		if (!this->materialTrackerMap.contains(name))
			App::get().logger.log(("AssetManager::getMaterial(): Attempting to get material that does not exist: " + name));

		return this->materialTrackerMap[name]->ptr;		
	}
	
	void AssetManager::removeMaterial(std::string name)
	{
		if (!this->materialTrackerMap.contains(name))
			App::get().logger.log(("AssetManager::removeMaterial(): Attempting to remove material that does not exist: " + name));
		
		auto t = this->materialTrackerMap[name];
		t->usageCount--;
		if (t->usageCount == 0)
		{
#ifdef DEBUG_ASSET_MANAGEMENT
	std::cout << "Full remove Material: " << name << std::endl;
#endif
			this->materials.erase(this->materials.get_iterator(t->ptr));
			this->materialTrackers.erase(this->materialTrackers.get_iterator(t));
			this->materialTrackerMap.erase(name);
		}
#ifdef DEBUG_ASSET_MANAGEMENT
	else
	{
		std::cout << "Decrement Material usageCount, retain: " << name << std::endl;
	}
#endif	
	}

	/* Animations
	--------------------------------------------------*/	
	Animation* AssetManager::addAnimation(Animation a)
	{
		auto it = this->animations.insert(a);
		return &(*it);
	}

	/* Renderables
	--------------------------------------------------*/
	std::string AssetManager::addRenderable(std::string name, Shader* shader, Mesh* mesh, Material* material)
	{
		if (this->renderableTrackerMap.contains(name)) 
		{
#ifdef DEBUG_ASSET_MANAGEMENT
	std::cout << "Existing Renderable, bypass reload: " << name << std::endl;
#endif
			auto t = this->renderableTrackerMap[name];
			if(t->usageCount > 0)
			{
				t->usageCount++;
				return name;
			}
			else
			{
				std::this_thread::sleep_for(100ms);
				return this->addRenderable(name, shader, mesh, material);
			}			
		}

#ifdef DEBUG_ASSET_MANAGEMENT
	std::cout << "Loading new Renderable: " << name << std::endl;
#endif	

		auto it = this->renderables.insert(Renderable(name, shader, mesh, material));
		
		RenderableTracker t;
		t.ptr = &(*it);
		t.usageCount++;
		
		auto it2 = this->renderableTrackers.insert(t);

		this->renderableTrackerMap[name] = &(*it2);

		return name;
	}

	Renderable AssetManager::getRenderable(std::string name)
	{
		if (!this->renderableTrackerMap.contains(name))
			App::get().logger.log(("AssetManager::getRenderable(): Attempting to get renderable that does not exist: " + name));

		return *this->renderableTrackerMap[name]->ptr;		
	}
	
	void AssetManager::removeRenderable(std::string name)
	{
		if (!this->renderableTrackerMap.contains(name))
			App::get().logger.log(("AssetManager::removeRenderable(): Attempting to remove renderable that does not exist: " + name));
		
		auto t = this->renderableTrackerMap[name];
		t->usageCount--;
		if (t->usageCount == 0)
		{
#ifdef DEBUG_ASSET_MANAGEMENT
	std::cout << "Full remove Renderable: " << name << std::endl;
#endif
			this->renderables.erase(this->renderables.get_iterator(t->ptr));
			this->renderableTrackers.erase(this->renderableTrackers.get_iterator(t));
			this->renderableTrackerMap.erase(name);
		}
#ifdef DEBUG_ASSET_MANAGEMENT
	else
	{
		std::cout << "Decrement Renderable usageCount, retain: " << name << std::endl;
	}
#endif	
	}

	/* Armatures
	--------------------------------------------------*/
	ArmatureTracker* AssetManager::getArmatureTracker(std::string name)
	{
		if(this->armatureTrackerMap.contains(name))
		{
			auto t = this->armatureTrackerMap[name];
			if(t->usageCount > 0)
			{
				t->usageCount++;
				return t;
			}
			else
			{
				std::this_thread::sleep_for(100ms);
				return this->getArmatureTracker(name);
			}
		}
		
		return nullptr;
	}
	
	ArmatureTracker* AssetManager::addArmature(Armature a)
	{
		auto it = this->armatures.insert(a);
		
		ArmatureTracker t;
		t.ptr = &(*it);
		t.usageCount++;
		
		auto it2 = this->armatureTrackers.insert(t);

		this->armatureTrackerMap[a.getName()] = &(*it2);

		return &(*it2);
	}

	Armature AssetManager::getArmature(std::string name)
	{
		if (!this->armatureTrackerMap.contains(name))
			App::get().logger.log(("AssetManager::getArmature(): Attempting to get armature that does not exist: " + name));

		return *this->armatureTrackerMap[name]->ptr;		
	}

	void AssetManager::removeArmature(std::string name)
	{
		if (!this->armatureTrackerMap.contains(name))
			App::get().logger.log(("AssetManager::removeArmature(): Attempting to remove armature that does not exist: " + name));
		
		auto t = this->armatureTrackerMap[name];
		t->usageCount--;
		if(t->usageCount == 0)
		{
#ifdef DEBUG_ASSET_MANAGEMENT
	std::cout << "Full remove Armature: " << name << std::endl;
#endif
			// remove all animations used by this armature
			for(auto& animArmPair : t->ptr->getAnimations())
				for(auto& animOG : this->animations)
					if(animArmPair.second == &animOG)
						this->animations.erase(this->animations.get_iterator(animArmPair.second));
			
			// remove armature
			this->armatures.erase(this->armatures.get_iterator(t->ptr));
			this->armatureTrackers.erase(this->armatureTrackers.get_iterator(t));
			this->armatureTrackerMap.erase(name);
		}
#ifdef DEBUG_ASSET_MANAGEMENT
	else
	{
		std::cout << "Decrement Armature usageCount, retain: " << name << std::endl;
	}
#endif	
	}

}