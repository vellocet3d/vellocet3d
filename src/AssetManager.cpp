#include <thread> 
#include <chrono>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#include "glad/glad.h"

#include "vel/AssetManager.h"
#include "vel/AssetLoaderV2.h"
#include "vel/Log.h"

using namespace std::chrono_literals;

namespace vel
{

	AssetManager::AssetManager(GPU* gpu) :
		gpu(gpu)
	{}
	AssetManager::~AssetManager(){}


	void AssetManager::sendAllToGpu()
	{
		for(auto& s : this->shadersThatNeedGpuLoad)
		{
			this->gpu->loadShader(s->ptr);
			s->gpuLoaded = true;
			this->shadersThatNeedGpuLoad.pop_front();
		}
		
		for(auto& m : this->meshesThatNeedGpuLoad)
		{
			this->gpu->loadMesh(m->ptr);
			m->gpuLoaded = true;
			this->meshesThatNeedGpuLoad.pop_front();
		}
		
		for(auto& t : this->texturesThatNeedGpuLoad)
		{
			this->gpu->loadTexture(t->ptr);
			t->gpuLoaded = true;
			this->texturesThatNeedGpuLoad.pop_front();
		}
        
        for(auto& h : this->hdrsThatNeedGpuLoad)
		{
			this->gpu->loadHdr(h->ptr);
			h->gpuLoaded = true;
			this->hdrsThatNeedGpuLoad.pop_front();
		}        
	}

	void AssetManager::sendNextToGpu()
	{
		if (this->shadersThatNeedGpuLoad.size() > 0)
		{
			auto shaderTracker = this->shadersThatNeedGpuLoad.at(0);
			this->gpu->loadShader(shaderTracker->ptr);
			shaderTracker->gpuLoaded = true;
			this->shadersThatNeedGpuLoad.pop_front();
			return;
		}

		if (this->meshesThatNeedGpuLoad.size() > 0)
		{
			auto meshTracker = this->meshesThatNeedGpuLoad.at(0);
			this->gpu->loadMesh(meshTracker->ptr);
			meshTracker->gpuLoaded = true;
			this->meshesThatNeedGpuLoad.pop_front();
			return;
		}

		if (this->texturesThatNeedGpuLoad.size() > 0)
		{
			auto textureTracker = this->texturesThatNeedGpuLoad.at(0);
			this->gpu->loadTexture(textureTracker->ptr);
			textureTracker->gpuLoaded = true;
			this->texturesThatNeedGpuLoad.pop_front();
			return;
		}
        
        if (this->hdrsThatNeedGpuLoad.size() > 0)
		{
			auto hdrTracker = this->hdrsThatNeedGpuLoad.at(0);
			this->gpu->loadHdr(hdrTracker->ptr);
			hdrTracker->gpuLoaded = true;
			this->hdrsThatNeedGpuLoad.pop_front();
			return;
		}
	}

	/* Shaders
	--------------------------------------------------*/
	std::string AssetManager::loadShader(std::string name, std::string vertFile, std::string fragFile)
	{
		if (this->shaderTrackerMap.contains(name)) 
		{
#ifdef DEBUG_LOG
    std::string msg = std::string("Existing Shader, bypass reload: ") + name;
    Log::toCli(msg);
    Log::toFile(msg);
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
		
#ifdef DEBUG_LOG
    std::string msg = std::string("Loading new Shader: ") + name;
    Log::toCli(msg);
    Log::toFile(msg);
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
#ifdef DEBUG_LOG
if (!this->shaderTrackerMap.contains(name))
{
    std::string msg = std::string("AssetManager::getShader(): Attempting to get shader that does not exist: ") + name;
    Log::crash(msg);
}
#endif

		return this->shaderTrackerMap[name]->ptr;
	}

	bool AssetManager::shaderIsGpuLoaded(std::string name)
	{
#ifdef DEBUG_LOG
if (!this->shaderTrackerMap.contains(name))
{
    std::string msg = std::string("AssetManager::shaderIsGpuLoaded(): Attempting to get shader that does not exist: ") + name;
    Log::crash(msg);
}
#endif

		return this->shaderTrackerMap[name]->gpuLoaded;
	}

	void AssetManager::removeShader(std::string name)
	{
#ifdef DEBUG_LOG
if (!this->shaderTrackerMap.contains(name))
{
    std::string msg = std::string("AssetManager::removeShader(): Attempting to remove shader that does not exist: ") + name;
    Log::crash(msg);
}
#endif

		auto t = this->shaderTrackerMap[name];
		t->usageCount--;
		if (t->usageCount == 0)
		{
			
#ifdef DEBUG_LOG
    std::string msg = std::string("Full remove Shader: ") + name;
    Log::toCli(msg);
    Log::toFile(msg);
#endif	
			
			if (!t->gpuLoaded)
				for (size_t i = 0; i < this->shadersThatNeedGpuLoad.size(); i++)
					if (this->shadersThatNeedGpuLoad.at(i) == t)
						this->shadersThatNeedGpuLoad.erase(this->shadersThatNeedGpuLoad.begin() + i);
			else
				this->gpu->clearShader(t->ptr);
			
			this->shaders.erase(this->shaders.get_iterator(t->ptr));
			this->shaderTrackers.erase(this->shaderTrackers.get_iterator(t));
			this->shaderTrackerMap.erase(name);			
		}
		
#ifdef DEBUG_LOG
	else
	{
        std::string msg = std::string("Decrement Shader usageCount, retain: ") + name;
        Log::toCli(msg);
        Log::toFile(msg);
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
#ifdef DEBUG_LOG
if (!this->meshTrackerMap.contains(name))
{
    std::string msg = std::string("AssetManager::getMesh(): Attempting to get mesh that does not exist: ") + name;
    Log::crash(msg);
}
#endif
		return this->meshTrackerMap[name]->ptr;	
	}

	bool AssetManager::meshIsGpuLoaded(std::string name)
	{
		return this->meshTrackerMap[name]->gpuLoaded;
	}
	
	void AssetManager::removeMesh(std::string name)
	{
#ifdef DEBUG_LOG
if (!this->meshTrackerMap.contains(name))
{
    std::string msg = std::string("AssetManager::removeMesh(): Attempting to remove mesh that does not exist: ") + name;
    Log::crash(msg);
}
#endif

		auto t = this->meshTrackerMap[name];
		t->usageCount--;
		if(t->usageCount == 0)
		{
#ifdef DEBUG_LOG
    std::string msg = std::string("Full remove Mesh: ") + name;
    Log::toCli(msg);
    Log::toFile(msg);
#endif
			if(!t->gpuLoaded)
				for (size_t i = 0; i < this->meshesThatNeedGpuLoad.size(); i++)
					if (this->meshesThatNeedGpuLoad.at(i) == t)
						this->meshesThatNeedGpuLoad.erase(this->meshesThatNeedGpuLoad.begin() + i);
			else
				this->gpu->clearMesh(t->ptr);
			
			this->meshes.erase(this->meshes.get_iterator(t->ptr));
			this->meshTrackers.erase(this->meshTrackers.get_iterator(t));
			this->meshTrackerMap.erase(name);
		}
#ifdef DEBUG_LOG
	else
	{
        std::string msg = std::string("Decrement Mesh usageCount, retain: ") + name;
        Log::toCli(msg);
        Log::toFile(msg);
	}
#endif
		
	}

	/* Textures
	--------------------------------------------------*/
	std::string AssetManager::loadTexture(std::string name, std::string type, std::string path, std::vector<std::string> mips)
	{	
		if (this->textureTrackerMap.contains(name)) 
		{
#ifdef DEBUG_LOG
    std::string msg = std::string("Existing Texture, bypass reload: ") + name;
    Log::toCli(msg);
    Log::toFile(msg);
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

#ifdef DEBUG_LOG
    std::string msg = std::string("Load new Texture: ") + name;
    Log::toCli(msg);
    Log::toFile(msg);
#endif

		Texture texture;
		texture.name = name;
		texture.type = type;
		texture.primaryImageData.data = stbi_load(
			path.c_str(), 
			&texture.primaryImageData.width, 
			&texture.primaryImageData.height, 
			&texture.primaryImageData.nrComponents, 
			0
		);

#ifdef DEBUG_LOG
if (!texture.primaryImageData.data)
{
    std::string msg = std::string("AssetManager::loadTexture(): Unable to load texture at path: ") + path;
    Log::crash(msg);
}
#endif

        if (texture.primaryImageData.nrComponents == 1)
        {
            texture.alphaChannel = false;
            texture.primaryImageData.format = GL_RED;
        }
        else if (texture.primaryImageData.nrComponents == 3)
        {
            texture.alphaChannel = false;
            texture.primaryImageData.format = GL_RGB;
        }
        else if (texture.primaryImageData.nrComponents == 4)
        {
            texture.alphaChannel = true;
            texture.primaryImageData.format = GL_RGBA;
        }

        for (auto& m : mips)
        {
            ImageData id;
            id.data = stbi_load(m.c_str(), &id.width, &id.height, &id.nrComponents, 0);

#ifdef DEBUG_LOG
if (!id.data)
{
    std::string msg = std::string("AssetManager::loadTexture(): Unable to load texture at path: ") + m;
    Log::crash(msg);
}
#endif

            if (id.nrComponents == 1)
                id.format = GL_RED;
            else if (id.nrComponents == 3)
                id.format = GL_RGB;
            else if (id.nrComponents == 4)
                id.format = GL_RGBA;

            texture.mips.push_back(id);
            
        }
		

		/////////////////////////////////////////

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
#ifdef DEBUG_LOG
if (!this->textureTrackerMap.contains(name))
{
    std::string msg = std::string("AssetManager::getTexture(): Attempting to get texture that does not exist: ") + name;
    Log::crash(msg);
}
#endif

		return this->textureTrackerMap[name]->ptr;		
	}

	bool AssetManager::textureIsGpuLoaded(std::string name)
	{
		return this->textureTrackerMap[name]->gpuLoaded;
	}
	
	void AssetManager::removeTexture(std::string name)
	{
#ifdef DEBUG_LOG
if (!this->textureTrackerMap.contains(name))
{
    std::string msg = std::string("AssetManager::removeTexture(): Attempting to remove texture that does not exist: ") + name;
    Log::crash(msg);
}
#endif

		auto t = this->textureTrackerMap[name];
		t->usageCount--;
		if (t->usageCount == 0)
		{
#ifdef DEBUG_LOG
    std::string msg = std::string("Full remove Texture: ") + name;
    Log::toCli(msg);
    Log::toFile(msg);
#endif
			if (!t->gpuLoaded)
				for (size_t i = 0; i < this->texturesThatNeedGpuLoad.size(); i++)
					if (this->texturesThatNeedGpuLoad.at(i) == t)
						this->texturesThatNeedGpuLoad.erase(this->texturesThatNeedGpuLoad.begin() + i);
			else
				this->gpu->clearTexture(t->ptr);
			
			this->textures.erase(this->textures.get_iterator(t->ptr));
			this->textureTrackers.erase(this->textureTrackers.get_iterator(t));
			this->textureTrackerMap.erase(name);
		}
#ifdef DEBUG_LOG
	else
	{
        std::string msg = std::string("Decrement Texture usageCount, retain: ") + name;
        Log::toCli(msg);
        Log::toFile(msg);
	}
#endif	
	}

    /* HDRs
	--------------------------------------------------*/
    std::string AssetManager::loadHdr(std::string name, std::string path)
    {
        if (this->hdrTrackerMap.contains(name)) 
		{
#ifdef DEBUG_LOG
    std::string msg = std::string("Existing HDR, bypass reload: ") + name;
    Log::toCli(msg);
    Log::toFile(msg);
#endif
			auto h = this->hdrTrackerMap[name];
			if(h->usageCount > 0)
			{
				h->usageCount++;
				return name;
			}
			else
			{
				std::this_thread::sleep_for(100ms);
				return this->loadHdr(name, path);
			}			
		}
        
#ifdef DEBUG_LOG
    std::string msg = std::string("Load new HDR: ") + name;
    Log::toCli(msg);
    Log::toFile(msg);
#endif

        HDR hdr;
        hdr.name = name;
        stbi_set_flip_vertically_on_load(true);
        hdr.primaryImageData.dataf = stbi_loadf(
			path.c_str(), 
			&hdr.primaryImageData.width, 
			&hdr.primaryImageData.height, 
			&hdr.primaryImageData.nrComponents, 
			0
		);
		stbi_set_flip_vertically_on_load(false);
        
        
#ifdef DEBUG_LOG
if (!hdr.primaryImageData.dataf)
{
    std::string msg = std::string("AssetManager::loadHdr(): Unable to load hdr at path: ") + path;
    Log::crash(msg);
}
#endif

        if (hdr.primaryImageData.nrComponents == 1)
            hdr.primaryImageData.format = GL_RED;
        else if (hdr.primaryImageData.nrComponents == 3)
            hdr.primaryImageData.format = GL_RGB;
        else if (hdr.primaryImageData.nrComponents == 4)
            hdr.primaryImageData.format = GL_RGBA;
        
        
        auto it = this->hdrs.insert(hdr);
        
        HDRTracker t;
        t.ptr = &(*it);
		t.usageCount++;
        
        auto it2 = this->hdrTrackers.insert(t);

		auto tmpTracker = &(*it2);

		this->hdrsThatNeedGpuLoad.push_back(tmpTracker);

		this->hdrTrackerMap[name] = tmpTracker;

		return name;
    }
    
    HDR* AssetManager::getHdr(std::string name)
	{
#ifdef DEBUG_LOG
if (!this->hdrTrackerMap.contains(name))
{
    std::string msg = std::string("AssetManager::getHdr(): Attempting to get HDR that does not exist: ") + name;
    Log::crash(msg);
}
#endif

		return this->hdrTrackerMap[name]->ptr;		
	}

	bool AssetManager::hdrIsGpuLoaded(std::string name)
	{
		return this->hdrTrackerMap[name]->gpuLoaded;
	}
    
    void AssetManager::removeHdr(std::string name)
	{
#ifdef DEBUG_LOG
if (!this->hdrTrackerMap.contains(name))
{
    std::string msg = std::string("AssetManager::removeHdr(): Attempting to remove HDR that does not exist: ") + name;
    Log::crash(msg);
}
#endif

		auto h = this->hdrTrackerMap[name];
		h->usageCount--;
		if (h->usageCount == 0)
		{
#ifdef DEBUG_LOG
    std::string msg = std::string("Full remove HDR: ") + name;
    Log::toCli(msg);
    Log::toFile(msg);
#endif
			if (!h->gpuLoaded)
				for (size_t i = 0; i < this->hdrsThatNeedGpuLoad.size(); i++)
					if (this->hdrsThatNeedGpuLoad.at(i) == h)
						this->hdrsThatNeedGpuLoad.erase(this->hdrsThatNeedGpuLoad.begin() + i);
			else
				this->gpu->clearHdr(h->ptr);
			
			this->hdrs.erase(this->hdrs.get_iterator(h->ptr));
			this->hdrTrackers.erase(this->hdrTrackers.get_iterator(h));
			this->hdrTrackerMap.erase(name);
		}
#ifdef DEBUG_LOG
	else
	{
        std::string msg = std::string("Decrement HDR usageCount, retain: ") + name;
        Log::toCli(msg);
        Log::toFile(msg);
	}
#endif

    }

	/* Materials
	--------------------------------------------------*/
	std::string AssetManager::addMaterial(Material m)
	{
		if (this->materialTrackerMap.contains(m.name)) 
		{
#ifdef DEBUG_LOG
    std::string msg = std::string("Existing Material, bypass reload: ") + m.name;
    Log::toCli(msg);
    Log::toFile(msg);
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

#ifdef DEBUG_LOG
    std::string msg = std::string("Loading new Material: ") + m.name;
    Log::toCli(msg);
    Log::toFile(msg);
#endif		

        // add default textures to material for those which are not provided
        if(m.albedo == nullptr)
            m.albedo = this->getTexture("defaultAlbedo");
        if(m.normal == nullptr)
            m.normal = this->getTexture("defaultNormal");
        if(m.metallic == nullptr)
            m.metallic = this->getTexture("defaultMetallic");
        if(m.roughness == nullptr)
            m.roughness = this->getTexture("defaultRoughness");
        if(m.ao == nullptr)
            m.ao = this->getTexture("defaultAO");
        

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
#ifdef DEBUG_LOG
if (!this->materialTrackerMap.contains(name))
{
    std::string msg = std::string("AssetManager::getMaterial(): Attempting to get material that does not exist: ") + name;
    Log::crash(msg);
}
#endif

		return this->materialTrackerMap[name]->ptr;		
	}
	
	void AssetManager::removeMaterial(std::string name)
	{
#ifdef DEBUG_LOG
if (!this->materialTrackerMap.contains(name))
{
    std::string msg = std::string("AssetManager::removeMaterial(): Attempting to remove material that does not exist: ") + name;
    Log::crash(msg);
}
#endif

		auto t = this->materialTrackerMap[name];
		t->usageCount--;
		if (t->usageCount == 0)
		{
#ifdef DEBUG_LOG
    std::string msg = std::string("Full remove Material: ") + name;
    Log::toCli(msg);
    Log::toFile(msg);
#endif
			this->materials.erase(this->materials.get_iterator(t->ptr));
			this->materialTrackers.erase(this->materialTrackers.get_iterator(t));
			this->materialTrackerMap.erase(name);
		}
#ifdef DEBUG_LOG
	else
	{
        std::string msg = std::string("Decrement Material usageCount, retain: ") + name;
        Log::toCli(msg);
        Log::toFile(msg);
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
#ifdef DEBUG_LOG
    std::string msg = std::string("Existing Renderable, bypass reload: ") + name;
    Log::toCli(msg);
    Log::toFile(msg);
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

#ifdef DEBUG_LOG
    std::string msg = std::string("Loading new Renderable: ") + name;
    Log::toCli(msg);
    Log::toFile(msg);
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
#ifdef DEBUG_LOG
if (!this->renderableTrackerMap.contains(name))
{
    std::string msg = std::string("AssetManager::getRenderable(): Attempting to get renderable that does not exist: ") + name;
    Log::crash(msg);
}
#endif

		return *this->renderableTrackerMap[name]->ptr;		
	}
	
	void AssetManager::removeRenderable(std::string name)
	{
#ifdef DEBUG_LOG
if (!this->renderableTrackerMap.contains(name))
{
    std::string msg = std::string("AssetManager::removeRenderable(): Attempting to remove renderable that does not exist: ") + name;
    Log::crash(msg);
}
#endif
        
		auto t = this->renderableTrackerMap[name];
		t->usageCount--;
		if (t->usageCount == 0)
		{
#ifdef DEBUG_LOG
    std::string msg = std::string("Full remove Renderable: ") + name;
    Log::toCli(msg);
    Log::toFile(msg);
#endif
			this->renderables.erase(this->renderables.get_iterator(t->ptr));
			this->renderableTrackers.erase(this->renderableTrackers.get_iterator(t));
			this->renderableTrackerMap.erase(name);
		}
#ifdef DEBUG_LOG
	else
	{
        std::string msg = std::string("Decrement Renderable usageCount, retain: ") + name;
        Log::toCli(msg);
        Log::toFile(msg);
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
#ifdef DEBUG_LOG
if (!this->armatureTrackerMap.contains(name))
{
    std::string msg = std::string("AssetManager::getArmature(): Attempting to get armature that does not exist: ") + name;
    Log::crash(msg);
}
#endif

		return *this->armatureTrackerMap[name]->ptr;		
	}

	void AssetManager::removeArmature(std::string name)
	{
#ifdef DEBUG_LOG
if (!this->armatureTrackerMap.contains(name))
{
    std::string msg = std::string("AssetManager::removeArmature(): Attempting to remove armature that does not exist: ") + name;
    Log::crash(msg);
}
#endif
        
		auto t = this->armatureTrackerMap[name];
		t->usageCount--;
		if(t->usageCount == 0)
		{
#ifdef DEBUG_LOG
    std::string msg = std::string("Full remove Armature: ") + name;
    Log::toCli(msg);
    Log::toFile(msg);
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
#ifdef DEBUG_LOG
	else
	{
        std::string msg = std::string("Decrement Armature usageCount, retain: ") + name;
        Log::toCli(msg);
        Log::toFile(msg);
	}
#endif	
	}

}