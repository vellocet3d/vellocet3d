

#include "vel/assets/AssetManager.h"
#include "vel/assets/AssetLoaderV2.h"
#include "vel/App.h"


namespace vel
{

	AssetManager::AssetManager(){}
	AssetManager::~AssetManager() {}


	/* Shaders
	--------------------------------------------------*/
	void AssetManager::loadShader(std::string name, std::string vertFile, std::string fragFile)
	{
		Shader s;
		s.name = name;
		s.vertFile = vertFile;
		s.fragFile = fragFile;

		App::get().getGPU()->loadShader(s);

		this->shaders.insert(s);
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
	void AssetManager::loadMesh(std::string path)
	{
		auto al = AssetLoaderV2(this, path);
		al.load();
	}

	void AssetManager::addMesh(Mesh m)
	{
		App::get().getGPU()->loadMesh(m);

		this->meshes.insert(m);
	}

	Mesh* AssetManager::getMeshes(std::string name)
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
	void AssetManager::loadTexture(std::string name, std::string type, std::string path, std::vector<std::string> mips)
	{
		Texture texture;
		texture.name = name;
		texture.type = type;
		texture.path = path;
		texture.mips = mips;

		App::get().getGPU()->loadTexture(texture);

		this->textures.insert(texture);
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
	void AssetManager::addMaterial(Material m)
	{
		this->materials.insert(m);
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

	Animation* AssetManager::getAnimation(std::string animationName)
	{
		for (auto& a : this->animations)
			if (a.name == animationName)
				return &a;

		App::get().logger.die("AssetManager::getAnimation(): CANNOT GET ANIMATION FOR NON EXISTING ANIMATION NAME");
	}

	plf::colony<Animation>& AssetManager::getAnimations()
	{
		return this->animations;
	}

	/* Renderables
	--------------------------------------------------*/
	void AssetManager::addRenderable(std::string name, Shader* shader, Mesh* mesh, Material* material)
	{
		this->renderables.insert(Renderable(name, shader, mesh, material));
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
	Armature* AssetManager::addArmature(Armature a)
	{
		auto it = this->armatures.insert(a);
		return &(*it);
	}

	Armature AssetManager::getArmature(std::string name)
	{
		for (auto& a : this->armatures)
			if (a.getName() == name)
				return a;

		App::get().logger.die("AssetManager::getArmature(): Attempting to get armature by name that does not exist");
	}


}