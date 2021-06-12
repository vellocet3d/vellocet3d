#pragma once

#include <string>

#include "dep/plf_colony.h"


#include "vel/assets/Shader.h"
#include "vel/assets/mesh/Mesh.h"
#include "vel/assets/material/Texture.h"
#include "vel/assets/material/Material.h"
#include "vel/assets/Renderable.h"
#include "vel/assets/animation/Animation.h"
#include "vel/assets/armature/Armature.h"



namespace vel
{
	class AssetManager
	{
	private:
		plf::colony<Shader>			shaders;
		plf::colony<Mesh>			meshes;
		plf::colony<Texture>		textures;
		plf::colony<Material>		materials;
		plf::colony<Renderable>		renderables;
		plf::colony<Animation>		animations;
		plf::colony<Armature>		armatures;

	public:
		AssetManager();
		~AssetManager();

		void						loadShader(std::string name, std::string vertFile, std::string fragFile);
		Shader*						getShader(std::string name);

		void						loadMesh(std::string path);
		void						addMesh(Mesh m);
		const plf::colony<Mesh>&	getMeshes() const;
		Mesh*						getMeshes(std::string name);

		void						loadTexture(std::string name, std::string type, std::string path, std::vector<std::string> mips = std::vector<std::string>());
		Texture*					getTexture(std::string textureName);

		void						addMaterial(Material m);
		Material*					getMaterial(std::string materialName);

		Animation*					addAnimation(Animation a);
		Animation*					getAnimation(std::string animationName);
		plf::colony<Animation>&		getAnimations();

		void						addRenderable(std::string name, Shader* shader, Mesh* mesh, Material* material);
		Renderable					getRenderable(std::string name);

		Armature*					addArmature(Armature a);
		Armature					getArmature(std::string name);

	};

}