#include <thread> 
#include <chrono>
#include <iostream>
#include <fstream>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_headers/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_headers/stb_truetype.h"

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
		// should never be called without having a gpu member, but check here just in case for now
		if (this->gpu == nullptr)
			return;

		// TODO: whoa whoa whoa....how is this working if I'm poping elements off the containers while I'm
		// looping through them...this is probably a horrible runtime bug that I just haven't hit yet...
		// simple fix is to just clear the containers outside of their loops...lets do that...
		//
		// DONE, but leaving this comment here until it's been tested
		for (auto& s : this->shadersThatNeedGpuLoad)
		{
			this->gpu->loadShader(s->ptr);
			s->gpuLoaded = true;
			//this->shadersThatNeedGpuLoad.pop_front();
		}
		this->shadersThatNeedGpuLoad.clear();

		for (auto& c : this->camerasThatNeedGpuLoad)
		{
			c->ptr->setRenderTarget(this->gpu->createRenderTarget(c->ptr->getViewportSize().x, c->ptr->getViewportSize().y));
			c->gpuLoaded = true;

			c->ptr->getRenderTarget()->texture.name = c->ptr->getName() + "_RT";
			c->ptr->getRenderTarget()->texture.alphaChannel = false;

			//this->shadersThatNeedGpuLoad.pop_front();
		}
		this->shadersThatNeedGpuLoad.clear();

		for (auto& t : this->texturesThatNeedGpuLoad)
		{
			this->gpu->loadTexture(t->ptr);
			t->gpuLoaded = true;
			//this->texturesThatNeedGpuLoad.pop_front();
		}
		this->texturesThatNeedGpuLoad.clear();

		for (auto& fb : this->fontBitmapsThatNeedGpuLoad)
		{
			this->gpu->loadFontBitmapTexture(fb->ptr);
			fb->gpuLoaded = true;
			//this->fontBitmapsThatNeedGpuLoad.pop_front();
		}
		this->fontBitmapsThatNeedGpuLoad.clear();

		for (auto& m : this->meshesThatNeedGpuLoad)
		{
			this->gpu->loadMesh(m->ptr);
			m->gpuLoaded = true;
			//this->meshesThatNeedGpuLoad.pop_front();
		}
		this->meshesThatNeedGpuLoad.clear();
	}

	void AssetManager::sendNextToGpu()
	{
		// should never be called without having a gpu member, but check here just in case for now
		if (this->gpu == nullptr)
			return;

		if (this->shadersThatNeedGpuLoad.size() > 0)
		{
			auto shaderTracker = this->shadersThatNeedGpuLoad.at(0);
			this->gpu->loadShader(shaderTracker->ptr);
			shaderTracker->gpuLoaded = true;
			this->shadersThatNeedGpuLoad.pop_front();
			return;
		}

		if (this->camerasThatNeedGpuLoad.size() > 0)
		{
			auto cameraTracker = this->camerasThatNeedGpuLoad.at(0);
			cameraTracker->ptr->setRenderTarget(this->gpu->createRenderTarget(
				cameraTracker->ptr->getViewportSize().x, cameraTracker->ptr->getViewportSize().y));
			cameraTracker->gpuLoaded = true;
			this->camerasThatNeedGpuLoad.pop_front();
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

		if (this->fontBitmapsThatNeedGpuLoad.size() > 0)
		{
			auto fbTracker = this->fontBitmapsThatNeedGpuLoad.at(0);
			this->gpu->loadFontBitmapTexture(fbTracker->ptr);
			fbTracker->gpuLoaded = true;
			this->fontBitmapsThatNeedGpuLoad.pop_front();
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
			
			this->shaderTrackers.get(name)->usageCount++;
			return name;
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

	MeshTracker* AssetManager::addMesh(Mesh& m, bool queue)
	{
		// AssetLoader checks for existing mesh by name, therefore if we have
		// made it this far, assume that m is a new Mesh
		
		auto meshPtr = this->meshes.insert(m.getName(), m);
		
		MeshTracker t;
		t.ptr = meshPtr;
		t.usageCount++;
		
		auto meshTrackerPtr = this->meshTrackers.insert(m.getName(), t);

		if (this->gpu != nullptr)
		{
			if (queue)
			{
				this->meshesThatNeedGpuLoad.push_back(meshTrackerPtr);
			}
			else
			{
				this->gpu->loadMesh(meshTrackerPtr->ptr);
				meshTrackerPtr->gpuLoaded = true;
			}
		}

		return meshTrackerPtr;
	}

	void AssetManager::updateMesh(Mesh* m)
	{
		MeshTracker* mt = this->getMeshTracker(m->getName());
		*mt->ptr = *m;
		this->gpu->updateMesh(mt->ptr);
	}
	
	MeshTracker* AssetManager::getMeshTracker(std::string name)
	{
		if(this->meshTrackers.exists(name))
		{
			auto t = this->meshTrackers.get(name);
			t->usageCount++;
			return t;
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
			if (this->gpu != nullptr)
			{
				if (!t->gpuLoaded)
					for (size_t i = 0; i < this->meshesThatNeedGpuLoad.size(); i++)
						if (this->meshesThatNeedGpuLoad.at(i) == t)
							this->meshesThatNeedGpuLoad.erase(this->meshesThatNeedGpuLoad.begin() + i);
						else
							this->gpu->clearMesh(t->ptr);
			}

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
	TextureData AssetManager::generateTextureData(std::string path)
	{
		TextureData td;
		td.primaryImageData.data = stbi_load(
			path.c_str(),
			&td.primaryImageData.width,
			&td.primaryImageData.height,
			&td.primaryImageData.nrComponents,
			0
		);

#ifdef DEBUG_LOG
		if (!td.primaryImageData.data)
			Log::crash("AssetManager::loadTexture(): Unable to load texture at path: " + path);
		if (td.primaryImageData.width != td.primaryImageData.height)
			Log::crash("AssetManager::loadTexture(): Texture not square: " + path);
		if (!isPowerOfTwo(td.primaryImageData.width))
			Log::crash("AssetManager::loadTexture(): Texture not power of two: " + path);
#endif

		if (td.primaryImageData.nrComponents == 1)
		{
			td.alphaChannel = false;
			td.primaryImageData.sizedFormat = GL_R8; // TODO IDK if 8 bit is right for this
			td.primaryImageData.format = GL_RED;
		}
		else if (td.primaryImageData.nrComponents == 3)
		{
			td.alphaChannel = false;
			td.primaryImageData.sizedFormat = GL_RGB8; // TODO IDK if 8 bit is right for this
			td.primaryImageData.format = GL_RGB;
		}
		else if (td.primaryImageData.nrComponents == 4)
		{
			td.alphaChannel = true;
			td.primaryImageData.sizedFormat = GL_RGBA8; // TODO IDK if 8 bit is right for this
			td.primaryImageData.format = GL_RGBA;
		}

		return td;
	}

	std::string AssetManager::loadTexture(std::string name, std::string path)
	{	
		if (this->textureTrackers.exists(name))
		{
#ifdef DEBUG_LOG
	Log::toCliAndFile("Existing Texture, bypass reload: " + name);
#endif
			this->textureTrackers.get(name)->usageCount++;
			return name;
		}

#ifdef DEBUG_LOG
	Log::toCliAndFile("Load new Texture: " + name);
#endif
		Texture texture;
		texture.name = name;

		// Determine if path is a directory or file, if directory then load each file in the directory as
		// a texture frame
		if (std::filesystem::is_directory(path))
			for (const auto & entry : std::filesystem::directory_iterator(path))
				texture.frames.push_back(this->generateTextureData(entry.path().string()));
		else
			texture.frames.push_back(this->generateTextureData(path));

		// loop over all frames and if any of them have alpha channel, set alpha channel member of texture to true
		texture.alphaChannel = false;
		for (auto& f : texture.frames)
		{
			if (f.alphaChannel)
			{
				texture.alphaChannel = true;
				break;
			}
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
			{
				for (size_t i = 0; i < this->texturesThatNeedGpuLoad.size(); i++)
				{
					if (this->texturesThatNeedGpuLoad.at(i) == t)
					{
						this->texturesThatNeedGpuLoad.erase(this->texturesThatNeedGpuLoad.begin() + i);
					}
				}
			}		
			else
			{
				this->gpu->clearTexture(t->ptr);
			}
				
			
			this->textures.erase(name);
			this->textureTrackers.erase(name);
		}
#ifdef DEBUG_LOG
	else
		Log::toCliAndFile("Decrement Texture usageCount, retain: " + name);
#endif	
	}

	/* FontBitmaps
	--------------------------------------------------*/
	std::string	AssetManager::loadFontBitmap(FontBitmap fb)
	{
		if (this->fontBitmapTrackers.exists(fb.fontName))
		{
#ifdef DEBUG_LOG
			Log::toCliAndFile("Existing FontBitmap, bypass reload: " + fb.fontName);
#endif
			this->fontBitmapTrackers.get(fb.fontName)->usageCount++;
			return fb.fontName;
		}

#ifdef DEBUG_LOG
		Log::toCliAndFile("Load new FontBitmap: " + fb.fontName);
#endif

		// Read in bytes from file on disc
		std::ifstream file(fb.fontPath, std::ios::binary | std::ios::ate);
		if (!file.is_open())
			Log::crash("Failed to open file: " + fb.fontPath);

		const auto size = file.tellg();
		file.seekg(0, std::ios::beg);
		auto bytes = std::vector<uint8_t>(size);
		file.read(reinterpret_cast<char *>(&bytes[0]), size);
		file.close();

		// Build texture data using read in bytes
		auto fontData = bytes;
		//fb.data = std::make_shared<uint8_t[]>(fb.textureWidth * fb.textureHeight);
		//fb.data = new unsigned char(fb.textureWidth * fb.textureHeight);
		//fb.charInfo = new fb_packedchar;

		//fb.data = std::make_unique<unsigned char[]>(fb.textureWidth * fb.textureHeight);
		//fb.charInfo = std::make_unique<fb_packedchar[]>(fb.charCount);

		auto tmpData = std::make_unique<unsigned char[]>(fb.textureWidth * fb.textureHeight);
		auto tmpCharInfo = std::make_unique<fb_packedchar[]>(fb.charCount);

		fb.data = tmpData.release();
		fb.charInfo = tmpCharInfo.release();




		stbtt_pack_context context;
		if (!stbtt_PackBegin(&context, fb.data, fb.textureWidth, fb.textureHeight, 0, 1, nullptr))
			Log::crash("Failed to initialize font");

		stbtt_PackSetOversampling(&context, fb.oversampleX, fb.oversampleY);
		if (!stbtt_PackFontRange(&context, fontData.data(), 0, fb.fontSize, fb.firstChar, fb.charCount, (stbtt_packedchar*)fb.charInfo))
			Log::crash("Failed to pack font");

		stbtt_PackEnd(&context);

		FontBitmap* pFb = this->fontBitmaps.insert(fb.fontName, fb);

		FontBitmapTracker fbt;
		fbt.ptr = pFb;
		fbt.usageCount++;

		this->fontBitmapsThatNeedGpuLoad.push_back(this->fontBitmapTrackers.insert(fb.fontName, fbt));

		return fb.fontName;
	}

	FontBitmap* AssetManager::getFontBitmap(std::string name)
	{
#ifdef DEBUG_LOG
		if (!this->fontBitmapTrackers.exists(name))
			Log::crash("AssetManager::getFontBitmap(): Attempting to get font bitmap that does not exist: " + name);
#endif

		return this->fontBitmapTrackers.get(name)->ptr;
	}

	bool AssetManager::fontBitmapIsGpuLoaded(std::string name)
	{
		return this->fontBitmapTrackers.get(name)->gpuLoaded;
	}

	void AssetManager::removeFontBitmap(std::string name)
	{
#ifdef DEBUG_LOG
		if (!this->fontBitmapTrackers.exists(name))
			Log::crash("AssetManager::removeFontBitmap(): Attempting to remove font bitmap that does not exist: " + name);
#endif

		auto fbt = this->fontBitmapTrackers.get(name);
		fbt->usageCount--;
		if (fbt->usageCount == 0)
		{
#ifdef DEBUG_LOG
			Log::toCliAndFile("Full remove FontBitmap: " + name);
#endif
			if (!fbt->gpuLoaded)
			{
				for (size_t i = 0; i < this->fontBitmapsThatNeedGpuLoad.size(); i++)
				{
					if (this->fontBitmapsThatNeedGpuLoad.at(i) == fbt)
					{
						this->fontBitmapsThatNeedGpuLoad.erase(this->fontBitmapsThatNeedGpuLoad.begin() + i);
					}
				}
			}
			else
			{
				this->gpu->clearTexture(&fbt->ptr->texture);
			}


			this->fontBitmaps.erase(name);
			this->fontBitmapTrackers.erase(name);
		}
#ifdef DEBUG_LOG
		else
			Log::toCliAndFile("Decrement FontBitmap usageCount, retain: " + name);
#endif	
	}

	FontGlyphInfo AssetManager::getFontGlyphInfo(uint32_t character, float offsetX, float offsetY, FontBitmap* fb)
	{
		stbtt_aligned_quad quad;

		stbtt_GetPackedQuad((stbtt_packedchar*)fb->charInfo, fb->textureWidth, fb->textureHeight,
			character - fb->firstChar, &offsetX, &offsetY, &quad, 1);
		const auto xmin = quad.x0;
		const auto xmax = quad.x1;
		const auto ymin = -quad.y1;
		const auto ymax = -quad.y0;


		FontGlyphInfo info{};
		info.offsetX = offsetX;
		info.offsetY = offsetY;
		info.positions[0] = { xmin, ymin, 0 };
		info.positions[1] = { xmin, ymax, 0 };
		info.positions[2] = { xmax, ymax, 0 };
		info.positions[3] = { xmax, ymin, 0 };
		info.uvs[0] = { quad.s0, quad.t1 };
		info.uvs[1] = { quad.s0, quad.t0 };
		info.uvs[2] = { quad.s1, quad.t0 };
		info.uvs[3] = { quad.s1, quad.t1 };

		return info;
	}

	/* Cameras
	--------------------------------------------------*/
	std::string AssetManager::addCamera(Camera c)
	{
		if (this->cameraTrackers.exists(c.getName()))
		{
#ifdef DEBUG_LOG
			Log::toCliAndFile("Existing Camera, bypass reload: " + c.getName());
#endif
			this->cameraTrackers.get(c.getName())->usageCount++;
			return c.getName();
		}

#ifdef DEBUG_LOG
		Log::toCliAndFile("Loading new Camera: " + c.getName());
#endif		

		auto cameraPtr = this->cameras.insert(c.getName(), c);

		CameraTracker t;
		t.ptr = cameraPtr;
		t.usageCount++;

		this->camerasThatNeedGpuLoad.push_back(this->cameraTrackers.insert(c.getName(), t));

		return c.getName();
	}

	Camera* AssetManager::getCamera(std::string name)
	{
#ifdef DEBUG_LOG
		if (!this->cameraTrackers.exists(name))
			Log::crash("AssetManager::getCamera(): Attempting to get camera that does not exist: " + name);
#endif

		return this->cameraTrackers.get(name)->ptr;
	}

	bool AssetManager::cameraIsGpuLoaded(std::string name)
	{
#ifdef DEBUG_LOG
		if (!this->cameraTrackers.exists(name))
			Log::crash("AssetManager::cameraIsGpuLoaded(): Attempting to get camera that does not exist: " + name);
#endif

		return this->cameraTrackers.get(name)->gpuLoaded;
	}

	void AssetManager::removeCamera(std::string name)
	{
#ifdef DEBUG_LOG
		if (!this->cameraTrackers.exists(name))
			Log::crash("AssetManager::removeCamera(): Attempting to remove camera that does not exist: " + name);
#endif

		auto t = this->cameraTrackers.get(name);
		t->usageCount--;
		if (t->usageCount == 0)
		{
#ifdef DEBUG_LOG
			Log::toCliAndFile("Full remove Camera: " + name);
#endif
			this->gpu->clearRenderTarget(t->ptr->getRenderTarget());
			this->cameras.erase(name);
			this->cameraTrackers.erase(name);
		}
#ifdef DEBUG_LOG
		else
			Log::toCliAndFile("Decrement Camera usageCount, retain: " + name);
#endif	
	}

	/* Materials
	--------------------------------------------------*/
	std::string AssetManager::addMaterial(Material m)
	{
		if (this->materialTrackers.exists(m.getName()))
		{
#ifdef DEBUG_LOG
	Log::toCliAndFile("Existing Material, bypass reload: " + m.getName());
#endif
			this->materialTrackers.get(m.getName())->usageCount++;
			return m.getName();
		}

#ifdef DEBUG_LOG
	Log::toCliAndFile("Loading new Material: " + m.getName());
#endif		

		//// assign default texture to material if a material is not provided at this time
		//if (m.diffuse == nullptr)
		//	m.diffuse = this->getTexture("__default__");

		if (m.getColor().w < 1.0f)
		{
			m.setHasAlphaChannel(true);
		}
		else
		{
			//if (!m.hasAlphaChannel && m.diffuse && m.diffuse->alphaChannel)
			//	m.hasAlphaChannel = true;
			for (auto& t : m.getTextures())
			{
				if (t->alphaChannel)
					m.setHasAlphaChannel(true);
			}
		}
			

		auto materialPtr = this->materials.insert(m.getName(), m);
		
		MaterialTracker t;
		t.ptr = materialPtr;
		t.usageCount++;

		this->materialTrackers.insert(m.getName(), t);

		return m.getName();
	}

	Material AssetManager::getMaterial(std::string name)
	{
#ifdef DEBUG_LOG
	if (!this->materialTrackers.exists(name))
		Log::crash("AssetManager::getMaterial(): Attempting to get material that does not exist: " + name);
#endif

		return *this->materialTrackers.get(name)->ptr;
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
	std::string AssetManager::addRenderable(std::string name, Shader* shader, Mesh* mesh)
	{
		if (this->renderableTrackers.exists(name))
		{
#ifdef DEBUG_LOG
			Log::toCliAndFile("Existing Renderable, bypass reload: " + name);
#endif
			this->renderableTrackers.get(name)->usageCount++;
			return name;
		}

#ifdef DEBUG_LOG
		Log::toCliAndFile("Loading new Renderable: " + name);
#endif	

		auto renderablePtr = this->renderables.insert(name, Renderable(name, shader, mesh));

		RenderableTracker t;
		t.ptr = renderablePtr;
		t.usageCount++;

		this->renderableTrackers.insert(name, t);

		return name;
	}

	std::string AssetManager::addRenderable(std::string name, Shader* shader, Mesh* mesh, Material material)
	{
		if (this->renderableTrackers.exists(name))
		{
#ifdef DEBUG_LOG
	Log::toCliAndFile("Existing Renderable, bypass reload: " + name);
#endif
			this->renderableTrackers.get(name)->usageCount++;
			return name;		
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
			t->usageCount++;
			return t;
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