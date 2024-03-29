#include <thread> 
#include <chrono>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#include "glad/glad.h"

#include "vel/AssetManager.h"
#include "vel/AssetLoaderV2.h"
#include "vel/Log.h"
#include "vel/functions.h"

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
        
        for(auto& h : this->infiniteCubemapsThatNeedGpuLoad)
		{
			this->gpu->loadInfiniteCubemap(h->ptr);
			h->gpuLoaded = true;
			this->infiniteCubemapsThatNeedGpuLoad.pop_front();
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
        
        if (this->infiniteCubemapsThatNeedGpuLoad.size() > 0)
		{
			auto hdrTracker = this->infiniteCubemapsThatNeedGpuLoad.at(0);
			this->gpu->loadInfiniteCubemap(hdrTracker->ptr);
			hdrTracker->gpuLoaded = true;
			this->infiniteCubemapsThatNeedGpuLoad.pop_front();
			return;
		}
	}

	/* Shaders
	--------------------------------------------------*/
	std::string AssetManager::loadShader(std::string name, std::string vertFile, std::string fragFile)
	{
		if (this->shaderTrackers.exists(name))
		{
#ifdef DEBUG_LOG
	Log::toCliAndFile("Existing Shader, bypass reload: " + name);
#endif
			
			// if usage count is not greater than zero then the asset is being deleted on the main thread
			// so we will need to re-create it. This could still potentially be a race condition, although
			// I would think it would be unlikely, putting a TODO here until we thoroughly test some 
			// real world use cases to verify
			if(this->shaderTrackers.get(name)->usageCount > 0)
			{
				this->shaderTrackers.get(name)->usageCount++;
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
	Log::toCliAndFile("Loading new Shader: " + name);
#endif		

		Shader s;
		s.name = name;
		s.vertFile = vertFile;
		s.fragFile = fragFile;

		auto shaderPtr = this->shaders.insert(name, s);
		
		ShaderTracker t;
		t.ptr = shaderPtr;
		t.usageCount++;
	
		this->shadersThatNeedGpuLoad.push_back(this->shaderTrackers.insert(name, t));

		return name;
	}

	Shader* AssetManager::getShader(std::string name)
	{
#ifdef DEBUG_LOG
if (!this->shaderTrackers.exists(name))
    Log::crash("AssetManager::getShader(): Attempting to get shader that does not exist: " + name);
#endif

		return this->shaderTrackers.get(name)->ptr;
	}

	bool AssetManager::shaderIsGpuLoaded(std::string name)
	{
#ifdef DEBUG_LOG
	if (!this->shaderTrackers.exists(name))
		Log::crash("AssetManager::shaderIsGpuLoaded(): Attempting to get shader that does not exist: " + name);
#endif

		return this->shaderTrackers.get(name)->gpuLoaded;
	}

	void AssetManager::removeShader(std::string name)
	{
#ifdef DEBUG_LOG
if (!this->shaderTrackers.exists(name))
    Log::crash("AssetManager::removeShader(): Attempting to remove shader that does not exist: " + name);
#endif

		auto t = this->shaderTrackers.get(name);
		t->usageCount--;
		if (t->usageCount == 0)
		{
			
#ifdef DEBUG_LOG
	Log::toCliAndFile("Full remove Shader: " + name);
#endif	
			
			if (!t->gpuLoaded)
				for (size_t i = 0; i < this->shadersThatNeedGpuLoad.size(); i++)
					if (this->shadersThatNeedGpuLoad.at(i) == t)
						this->shadersThatNeedGpuLoad.erase(this->shadersThatNeedGpuLoad.begin() + i);
			else
				this->gpu->clearShader(t->ptr);
			
			this->shaders.erase(name);
			this->shaderTrackers.erase(name);
		}
#ifdef DEBUG_LOG
	else
		Log::toCliAndFile("Decrement Shader usageCount, retain: " + name);
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
		
		auto meshPtr = this->meshes.insert(m.getName(), m);
		
		MeshTracker t;
		t.ptr = meshPtr;
		t.usageCount++;
		
		auto meshTrackerPtr = this->meshTrackers.insert(m.getName(), t);

		this->meshesThatNeedGpuLoad.push_back(meshTrackerPtr);

		return meshTrackerPtr;
	}

	MeshTracker* AssetManager::getMeshTracker(std::string name)
	{
		if(this->meshTrackers.exists(name))
		{
			auto t = this->meshTrackers.get(name);
			if(t->usageCount > 0)
			{
				t->usageCount++;
				return t;
			}
			else
			{
				std::this_thread::sleep_for(100ms); //tf? Guessing I did this as some form of ghetto thread-safety?
				return this->getMeshTracker(name);
			}
		}
		
		return nullptr;
	}

	Mesh* AssetManager::getMesh(std::string name)
	{
#ifdef DEBUG_LOG
if (!this->meshTrackers.exists(name))
    Log::crash("AssetManager::getMesh(): Attempting to get mesh that does not exist: " + name);
#endif
		return this->meshTrackers.get(name)->ptr;	
	}

	bool AssetManager::meshIsGpuLoaded(std::string name)
	{
		return this->meshTrackers.get(name)->gpuLoaded;
	}
	
	void AssetManager::removeMesh(std::string name)
	{
#ifdef DEBUG_LOG
	if (!this->meshTrackers.exists(name))
		Log::crash("AssetManager::removeMesh(): Attempting to remove mesh that does not exist: " + name);
#endif

		auto t = this->meshTrackers.get(name);
		t->usageCount--;
		if(t->usageCount == 0)
		{
#ifdef DEBUG_LOG
	Log::toCliAndFile("Full remove Mesh: " + name);
#endif
			if(!t->gpuLoaded)
				for (size_t i = 0; i < this->meshesThatNeedGpuLoad.size(); i++)
					if (this->meshesThatNeedGpuLoad.at(i) == t)
						this->meshesThatNeedGpuLoad.erase(this->meshesThatNeedGpuLoad.begin() + i);
			else
				this->gpu->clearMesh(t->ptr);
			
			this->meshes.erase(name);
			this->meshTrackers.erase(name);
		}
#ifdef DEBUG_LOG
	else
		Log::toCliAndFile("Decrement Mesh usageCount, retain: " + name);
#endif
		
	}

	/* Textures
	--------------------------------------------------*/
	std::string AssetManager::loadTexture(std::string name, std::string type, std::string path, std::vector<std::string> mips)
	{	
		if (this->textureTrackers.exists(name))
		{
#ifdef DEBUG_LOG
	Log::toCliAndFile("Existing Texture, bypass reload: " + name);
#endif
			auto t = this->textureTrackers.get(name);
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
	Log::toCliAndFile("Load new Texture: " + name);
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
		Log::crash("AssetManager::loadTexture(): Unable to load texture at path: " + path);
	if (texture.primaryImageData.width != texture.primaryImageData.height)
		Log::crash("AssetManager::loadTexture(): Texture not square: " + name);
	if (!isPowerOfTwo(texture.primaryImageData.width))
		Log::crash("AssetManager::loadTexture(): Texture not power of two: " + name);
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
			//Log::toCliAndFile("mip:" + m);
            ImageData id;
            id.data = stbi_load(m.c_str(), &id.width, &id.height, &id.nrComponents, 0);

#ifdef DEBUG_LOG
	if (!id.data)
		Log::crash("AssetManager::loadTexture(): Unable to load texture at path: " + m);
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

		auto texturePtr = this->textures.insert(texture.name, texture);
		
		TextureTracker t;
		t.ptr = texturePtr;
		t.usageCount++;

		this->texturesThatNeedGpuLoad.push_back(this->textureTrackers.insert(texture.name, t));

		return name;
	}

	Texture* AssetManager::getTexture(std::string name)
	{
#ifdef DEBUG_LOG
	if (!this->textureTrackers.exists(name))
		Log::crash("AssetManager::getTexture(): Attempting to get texture that does not exist: " + name);
#endif

		return this->textureTrackers.get(name)->ptr;		
	}

	bool AssetManager::textureIsGpuLoaded(std::string name)
	{
		return this->textureTrackers.get(name)->gpuLoaded;
	}
	
	void AssetManager::removeTexture(std::string name)
	{
#ifdef DEBUG_LOG
	if (!this->textureTrackers.exists(name))
		Log::crash("AssetManager::removeTexture(): Attempting to remove texture that does not exist: " + name);
#endif

		auto t = this->textureTrackers.get(name);
		t->usageCount--;
		if (t->usageCount == 0)
		{
#ifdef DEBUG_LOG
	Log::toCliAndFile("Full remove Texture: " + name);
#endif
			if (!t->gpuLoaded)
				for (size_t i = 0; i < this->texturesThatNeedGpuLoad.size(); i++)
					if (this->texturesThatNeedGpuLoad.at(i) == t)
						this->texturesThatNeedGpuLoad.erase(this->texturesThatNeedGpuLoad.begin() + i);
			else
				this->gpu->clearTexture(t->ptr);
			
			this->textures.erase(name);
			this->textureTrackers.erase(name);
		}
#ifdef DEBUG_LOG
	else
		Log::toCliAndFile("Decrement Texture usageCount, retain: " + name);
#endif	
	}

    /* HDRs
	--------------------------------------------------*/
    std::string AssetManager::loadInfiniteCubemap(std::string name, std::string path)
    {
        if (this->infiniteCubemapTrackers.exists(name))
		{
#ifdef DEBUG_LOG
	Log::toCliAndFile("Existing Cubemap, bypass reload: " + name);
#endif
			auto h = this->infiniteCubemapTrackers.get(name);
			if(h->usageCount > 0)
			{
				h->usageCount++;
				return name;
			}
			else
			{
				std::this_thread::sleep_for(100ms);
				return this->loadInfiniteCubemap(name, path);
			}			
		}
        
#ifdef DEBUG_LOG
	Log::toCliAndFile("Load new Cubemap: " + name);
#endif

        Cubemap hdr;
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
		Log::crash("AssetManager::loadInfiniteCubemap(): Unable to load hdr at path: " + path);
#endif

        if (hdr.primaryImageData.nrComponents == 1)
            hdr.primaryImageData.format = GL_RED;
        else if (hdr.primaryImageData.nrComponents == 3)
            hdr.primaryImageData.format = GL_RGB;
        else if (hdr.primaryImageData.nrComponents == 4)
            hdr.primaryImageData.format = GL_RGBA;
        
        
        auto hdrPtr = this->infiniteCubemaps.insert(hdr.name, hdr);
        
        InfiniteCubemapTracker t;
        t.ptr = hdrPtr;
		t.usageCount++;
        
		this->infiniteCubemapsThatNeedGpuLoad.push_back(this->infiniteCubemapTrackers.insert(hdr.name, t));

		return name;
    }
    
    Cubemap* AssetManager::getInfiniteCubemap(std::string name)
	{
#ifdef DEBUG_LOG
	if (!this->infiniteCubemapTrackers.exists(name))
		Log::crash("AssetManager::getInfiniteCubemap(): Attempting to get Cubemap that does not exist: " + name);
#endif

		return this->infiniteCubemapTrackers.get(name)->ptr;		
	}

	bool AssetManager::infiniteCubemapIsGpuLoaded(std::string name)
	{
		return this->infiniteCubemapTrackers.get(name)->gpuLoaded;
	}
    
    void AssetManager::removeInfiniteCubemap(std::string name)
	{
#ifdef DEBUG_LOG
	if (!this->infiniteCubemapTrackers.exists(name))
		Log::crash("AssetManager::removeInfiniteCubemap(): Attempting to remove Cubemap that does not exist: " + name);
#endif

		auto h = this->infiniteCubemapTrackers.get(name);
		h->usageCount--;
		if (h->usageCount == 0)
		{
#ifdef DEBUG_LOG
	Log::toCliAndFile("Full remove Cubemap: " + name);
#endif
			if (!h->gpuLoaded)
				for (size_t i = 0; i < this->infiniteCubemapsThatNeedGpuLoad.size(); i++)
					if (this->infiniteCubemapsThatNeedGpuLoad.at(i) == h)
						this->infiniteCubemapsThatNeedGpuLoad.erase(this->infiniteCubemapsThatNeedGpuLoad.begin() + i);
			else
				this->gpu->clearInfiniteCubemap(h->ptr);
			
			this->infiniteCubemaps.erase(name);
			this->infiniteCubemapTrackers.erase(name);
		}
#ifdef DEBUG_LOG
	else
		Log::toCliAndFile("Decrement Cubemap usageCount, retain: " + name);
#endif

    }

	/* Materials
	--------------------------------------------------*/
	std::string AssetManager::addMaterial(Material m)
	{
		if (this->materialTrackers.exists(m.name))
		{
#ifdef DEBUG_LOG
	Log::toCliAndFile("Existing Material, bypass reload: " + m.name);
#endif
			auto t = this->materialTrackers.get(m.name);
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
	Log::toCliAndFile("Loading new Material: " + m.name);
#endif		

		if (m.color.w < 1.0f)
		{
			m.hasAlphaChannel = true;
		}
		else
		{
			if (!m.hasAlphaChannel && m.diffuse && m.diffuse->alphaChannel)
				m.hasAlphaChannel = true;
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
		}

        // add default textures to material for those which are not provided
		// removed when we added renderModes, it's expected to provide a texture for every
		// element in a material by user, if they don't have one, they must specify that they
		// want to use default**
        //if(m.albedo == nullptr)
        //    m.albedo = this->getTexture("defaultAlbedo");
        //if(m.normal == nullptr)
        //    m.normal = this->getTexture("defaultNormal");
        //if(m.metallic == nullptr)
        //    m.metallic = this->getTexture("defaultMetallic");
        //if(m.roughness == nullptr)
        //    m.roughness = this->getTexture("defaultRoughness");
        //if(m.ao == nullptr)
        //    m.ao = this->getTexture("defaultAO");
        

		auto materialPtr = this->materials.insert(m.name, m);
		
		MaterialTracker t;
		t.ptr = materialPtr;
		t.usageCount++;

		this->materialTrackers.insert(m.name, t);

		return m.name;
	}

	Material* AssetManager::getMaterial(std::string name)
	{
#ifdef DEBUG_LOG
	if (!this->materialTrackers.exists(name))
		Log::crash("AssetManager::getMaterial(): Attempting to get material that does not exist: " + name);
#endif

		return this->materialTrackers.get(name)->ptr;
	}
	
	void AssetManager::removeMaterial(std::string name)
	{
#ifdef DEBUG_LOG
	if (!this->materialTrackers.exists(name))
		Log::crash("AssetManager::removeMaterial(): Attempting to remove material that does not exist: " + name);
#endif

		auto t = this->materialTrackers.get(name);
		t->usageCount--;
		if (t->usageCount == 0)
		{
#ifdef DEBUG_LOG
	Log::toCliAndFile("Full remove Material: " + name);
#endif
			this->materials.erase(name);
			this->materialTrackers.erase(name);
		}
#ifdef DEBUG_LOG
	else
		Log::toCliAndFile("Decrement Material usageCount, retain: " + name);
#endif	
	}

	/* Animations
	--------------------------------------------------*/	
	Animation* AssetManager::addAnimation(Animation a)
	{
		return this->animations.insert(a.name, a);
	}

	/* Renderables
	--------------------------------------------------*/
	std::string AssetManager::addRenderable(std::string name, Shader* shader, Mesh* mesh, Material* material)
	{
		if (this->renderableTrackers.exists(name))
		{
#ifdef DEBUG_LOG
	Log::toCliAndFile("Existing Renderable, bypass reload: " + name);
#endif
			auto t = this->renderableTrackers.get(name);
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
	Log::toCliAndFile("Loading new Renderable: " + name);
#endif	

		auto renderablePtr = this->renderables.insert(name, Renderable(name, shader, mesh, material));
		
		RenderableTracker t;
		t.ptr = renderablePtr;
		t.usageCount++;
		
		this->renderableTrackers.insert(name, t);

		return name;
	}

	Renderable AssetManager::getRenderable(std::string name)
	{
#ifdef DEBUG_LOG
	if (!this->renderableTrackers.exists(name))
		Log::crash("AssetManager::getRenderable(): Attempting to get renderable that does not exist: " + name);
#endif

		return *this->renderableTrackers.get(name)->ptr;
	}
	
	void AssetManager::removeRenderable(std::string name)
	{
#ifdef DEBUG_LOG
	if (!this->renderableTrackers.exists(name))
		Log::crash("AssetManager::removeRenderable(): Attempting to remove renderable that does not exist: " + name);
#endif
        
		auto t = this->renderableTrackers.get(name);
		t->usageCount--;
		if (t->usageCount == 0)
		{
#ifdef DEBUG_LOG
	Log::toCliAndFile("Full remove Renderable: " + name);
#endif
			this->renderables.erase(name);
			this->renderableTrackers.erase(name);
		}
#ifdef DEBUG_LOG
	else
		Log::toCliAndFile("Decrement Renderable usageCount, retain: " + name);
#endif	
	}

	/* Armatures
	--------------------------------------------------*/
	ArmatureTracker* AssetManager::getArmatureTracker(std::string name)
	{
		if(this->armatureTrackers.exists(name))
		{
			auto t = this->armatureTrackers.get(name);
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
		auto armaturePtr = this->armatures.insert(a.getName(), a);
		
		ArmatureTracker t;
		t.ptr = armaturePtr;
		t.usageCount++;
		
		return this->armatureTrackers.insert(a.getName(), t);
	}

	Armature AssetManager::getArmature(std::string name)
	{
#ifdef DEBUG_LOG
	if (!this->armatureTrackers.exists(name))
		Log::crash("AssetManager::getArmature(): Attempting to get armature that does not exist: " + name);
#endif

		return *this->armatureTrackers.get(name)->ptr;		
	}

	void AssetManager::removeArmature(std::string name)
	{
#ifdef DEBUG_LOG
	if (!this->armatureTrackers.exists(name))
		Log::crash("AssetManager::removeArmature(): Attempting to remove armature that does not exist: " + name);
#endif
        
		auto t = this->armatureTrackers.get(name);
		t->usageCount--;
		if(t->usageCount == 0)
		{
#ifdef DEBUG_LOG
	Log::toCliAndFile("Full remove Armature: " + name);
#endif

			// remove all animations used by this armature
			for (auto& animArmPair : t->ptr->getAnimations())
			{
				//for (auto& animOG : this->animations)
				for (auto iter = this->animations.getAll().begin(); iter != this->animations.getAll().end();)
				{
					auto animOG = *iter;

					if (animArmPair.second == animOG)
					{
						this->animations.erase(animOG);
						continue;
					}
					else
					{
						++iter;
					}
				}
			}
				
			
			// remove armature
			this->armatures.erase(name);
			this->armatureTrackers.erase(name);
		}
#ifdef DEBUG_LOG
	else
		Log::toCliAndFile("Decrement Armature usageCount, retain: " + name);
#endif	
	}

}