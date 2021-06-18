

#include "vel/assets/AssetManager.h"
#include "vel/assets/AssetLoaderV2.h"
#include "vel/App.h"


namespace vel
{

	AssetManager::AssetManager(){}
	AssetManager::~AssetManager() {}

	void AssetManager::sendToGpu()
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
		// first check if shader name already exists
		// if so, return the ShaderTracker* to that element
		if (this->shaderTrackerMap.contains(name))
		{
			this->shaderTrackerMap[name]->usageCount++;
			return name;
		}
		
		// else, create the shader and ShaderTracker then return ShaderTracker*
		
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
			App::get().logger.die(("AssetManager::getShader(): Attempting to get shader that does not exist: " + name));

		return this->shaderTrackerMap[name]->ptr;
	}

	bool AssetManager::shaderIsGpuLoaded(std::string name)
	{
		if (!this->shaderTrackerMap.contains(name))
			App::get().logger.die(("AssetManager::shaderIsGpuLoaded(): Attempting to get shader that does not exist: " + name));

		return this->shaderTrackerMap[name]->gpuLoaded;
	}

	void AssetManager::removeShader(std::string name)
	{
		if (!this->shaderTrackerMap.contains(name))
			App::get().logger.die(("AssetManager::removeShader(): Attempting to remove shader that does not exist: " + name));

		//TODO: This wouldn't be thread safe would it, since the thread that loads scenes into main memory could potentially see a value
		// as valid, right before we remove it here

		auto t = this->shaderTrackerMap[name];
		t->usageCount--;
		if (t->usageCount == 0)
		{

		}
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
		if (!this->meshTrackerMap.contains(name))
			return nullptr;

		return this->meshTrackerMap[name];
	}

	Mesh* AssetManager::getMesh(std::string name)
	{
		if (!this->meshTrackerMap.contains(name))
			App::get().logger.die(("AssetManager::getMesh(): Attempting to get mesh that does not exist: " + name));

		return this->meshTrackerMap[name]->ptr;	
	}

	bool AssetManager::meshIsGpuLoaded(std::string name)
	{
		return this->meshTrackerMap[name]->gpuLoaded;
	}

	/* Textures
	--------------------------------------------------*/
	std::string AssetManager::loadTexture(std::string name, std::string type, std::string path, std::vector<std::string> mips)
	{
		if (this->textureTrackerMap.contains(name))
		{
			this->textureTrackerMap[name]->usageCount++;
			return name;
		}
		
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
			App::get().logger.die(("AssetManager::getTexture(): Attempting to get texture that does not exist: " + name));

		return this->textureTrackerMap[name]->ptr;		
	}

	bool AssetManager::textureIsGpuLoaded(std::string name)
	{
		return this->textureTrackerMap[name]->gpuLoaded;
	}

	/* Materials
	--------------------------------------------------*/
	std::string AssetManager::addMaterial(Material m)
	{
		if (this->materialTrackerMap.contains(m.name))
		{
			this->materialTrackerMap[m.name]->usageCount++;
			return m.name;
		}

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
			App::get().logger.die(("AssetManager::getMaterial(): Attempting to get material that does not exist: " + name));

		return this->materialTrackerMap[name]->ptr;		
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
			this->renderableTrackerMap[name]->usageCount++;
			return name;
		}

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
			App::get().logger.die(("AssetManager::getRenderable(): Attempting to get renderable that does not exist: " + name));

		return *this->renderableTrackerMap[name]->ptr;		
	}

	/* Armatures
	--------------------------------------------------*/
	ArmatureTracker* AssetManager::getArmatureTracker(std::string name)
	{
		if (!this->armatureTrackerMap.contains(name))
			return nullptr;

		return this->armatureTrackerMap[name];
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
			App::get().logger.die(("AssetManager::getArmature(): Attempting to get armature that does not exist: " + name));

		return *this->armatureTrackerMap[name]->ptr;		
	}


}