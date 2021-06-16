

#include "vel/assets/AssetManager.h"
#include "vel/assets/AssetLoaderV2.h"
#include "vel/App.h"


namespace vel
{

	AssetManager::AssetManager(){}
	AssetManager::~AssetManager() {}

	void AssetManager::runGarbageCollector()
	{
		//TODO: loop through all asset trackers and delete all assets that have a usageCount of 0
		// from main memory and gpu

		//TODO: figure out when we're going to call this, have member collectGarbage that I was doing
		// something with

	}

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

		this->checkForGpuLoads = false;
	}

	/* Shaders
	--------------------------------------------------*/
	ShaderTracker* AssetManager::loadShader(std::string name, std::string vertFile, std::string fragFile)
	{
		// first check if shader name already exists
		// if so, return the ShaderTracker* to that element
		for(auto& s : this->shaderTrackers)
		{
			if(s.ptr->name == name)
			{
				s.usageCount++;
				return &s;
			}
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
		this->checkForGpuLoads = true;

		return tmpTracker;
	}

	Shader* AssetManager::getShader(std::string name)
	{
		for (auto& s : this->shaders)
			if (s.name == name)
				return &s;

		App::get().logger.die("AssetManager::getShader(): CANNOT GET SHADER FOR NON EXISTING SHADER NAME");
	}

	/* Meshes
	--------------------------------------------------*/
	MeshTracker* AssetManager::getMeshTracker(std::string name)
	{
		for (auto& m : this->meshTrackers)
			if (m.ptr->getName() == name)
				return &m;
			
		return nullptr;
	}
	
	std::pair<std::vector<MeshTracker*>, ArmatureTracker*> AssetManager::loadMesh(std::string path)
	{
		auto al = AssetLoaderV2(this, path);
		al.load();
		
		return al.getTrackers();
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
		this->checkForGpuLoads = true;

		return tmpTracker;
		
		//App::get().getGPU()->loadMesh(m);
		//this->meshes.insert(m);
	}

	Mesh* AssetManager::getMesh(std::string name)
	{
		for (auto& m : this->meshes)
			if (m.getName() == name)
				return &m;

		App::get().logger.die("AssetManager::getMeshes(): CANNOT GET MESH FOR NON EXISTING MESH NAME");
	}

	const plf::colony<Mesh>& AssetManager::getMeshes() const
	{
		return this->meshes;
	}

	/* Textures
	--------------------------------------------------*/
	TextureTracker* AssetManager::loadTexture(std::string name, std::string type, std::string path, std::vector<std::string> mips)
	{
		for(auto& t : this->textureTrackers)
		{
			if(t.ptr->name == name)
			{
				t.usageCount++;
				return &t;
			}
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
		this->checkForGpuLoads = true;

		return tmpTracker;

		//App::get().getGPU()->loadTexture(texture);
	}

	Texture* AssetManager::getTexture(std::string textureName)
	{
		for (auto& t : this->textures)
			if (t.name == textureName)
				return &t;

		App::get().logger.die("AssetManager::getTexture(): CANNOT GET TEXTURE FOR NON EXISTING TEXTURE NAME");
	}

	/* Materials
	--------------------------------------------------*/
	MaterialTracker* AssetManager::addMaterial(Material m)
	{
		for(auto& t : this->materialTrackers)
		{
			if(t.ptr->name == m.name)
			{
				t.usageCount++;
				return &t;
			}
		}

		auto it = this->materials.insert(m);
		
		MaterialTracker t;
		t.ptr = &(*it);
		t.usageCount++;
		
		auto it2 = this->materialTrackers.insert(t);
		return &(*it2);
	}

	Material* AssetManager::getMaterial(std::string materialName)
	{
		for (auto& m : this->materials)
			if (m.name == materialName)
				return &m;

		App::get().logger.die("AssetManager::getMaterial(): CANNOT GET MATERIAL FOR NON EXISTING MATERIAL NAME");
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
	RenderableTracker* AssetManager::addRenderable(std::string name, Shader* shader, Mesh* mesh, Material* material)
	{
		for(auto& t : this->renderableTrackers)
		{
			if(t.ptr->getName() == name)
			{
				t.usageCount++;
				return &t;
			}
		}

		auto it = this->renderables.insert(Renderable(name, shader, mesh, material));
		
		RenderableTracker t;
		t.ptr = &(*it);
		t.usageCount++;
		
		auto it2 = this->renderableTrackers.insert(t);
		return &(*it2);
	}

	Renderable AssetManager::getRenderable(std::string name)
	{
		for (auto& r : this->renderables)
			if (r.getName() == name)
				return r;

		App::get().logger.die("AssetManager::getRenderable(): CANNOT GET RENDERABLE FOR NON EXISTING RENDERABLE NAME");
	}

	/* Armatures
	--------------------------------------------------*/
	ArmatureTracker* AssetManager::getArmatureTracker(std::string name)
	{
		for (auto& a : this->armatureTrackers)
			if (a.ptr->getName() == name)
				return &a;
			
		return nullptr;
	}
	
	ArmatureTracker* AssetManager::addArmature(Armature a)
	{
		auto it = this->armatures.insert(a);
		
		ArmatureTracker t;
		t.ptr = &(*it);
		t.usageCount++;
		
		auto it2 = this->armatureTrackers.insert(t);
		return &(*it2);
	}

	Armature AssetManager::getArmature(std::string name)
	{
		for (auto& a : this->armatures)
			if (a.getName() == name)
				return a;

		App::get().logger.die("AssetManager::getArmature(): Attempting to get armature by name that does not exist");
	}


}