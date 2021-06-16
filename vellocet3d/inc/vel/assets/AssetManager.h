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

#include "vel/assets/AssetTrackers.h"

namespace vel
{
	class AssetManager
	{
	private:
		plf::colony<Shader>				shaders;
		plf::colony<ShaderTracker>		shaderTrackers;
		std::deque<ShaderTracker*>		shadersThatNeedGpuLoad;
		
		plf::colony<Mesh>				meshes;
		plf::colony<MeshTracker>		meshTrackers;
		std::deque<MeshTracker*>		meshesThatNeedGpuLoad;
		
		plf::colony<Texture>			textures;
		plf::colony<TextureTracker> 	textureTrackers;
		std::deque<TextureTracker*>		texturesThatNeedGpuLoad;
		
		plf::colony<Material>			materials;
		plf::colony<MaterialTracker> 	materialTrackers;
		
		plf::colony<Renderable>			renderables;
		plf::colony<RenderableTracker> 	renderableTrackers;
		
		plf::colony<Armature>			armatures;
		plf::colony<ArmatureTracker>	armatureTrackers;
		
		plf::colony<Animation>			animations;
		// since animations are tracked within an Armature, and they are only associated with a single armatureTrackers
		// we shouldn't need to track them, just account for them when adding/removing an armature
		//plf::colony<AnimationTracker>	animationTrackers;
		

	public:
		AssetManager();
		~AssetManager();

		bool						collectGarbage = false;
		bool						checkForGpuLoads = false;
		void						sendToGpu();

		ShaderTracker*				loadShader(std::string name, std::string vertFile, std::string fragFile);
		Shader*						getShader(std::string name);

		std::pair<std::vector<MeshTracker*>, ArmatureTracker*> loadMesh(std::string path);
		MeshTracker*				addMesh(Mesh m);
		const plf::colony<Mesh>&	getMeshes() const;
		Mesh*						getMesh(std::string name);

		TextureTracker*				loadTexture(std::string name, std::string type, std::string path, std::vector<std::string> mips = std::vector<std::string>());
		Texture*					getTexture(std::string textureName);

		MaterialTracker*			addMaterial(Material m);
		Material*					getMaterial(std::string materialName);

		Animation*					addAnimation(Animation a);

		RenderableTracker*			addRenderable(std::string name, Shader* shader, Mesh* mesh, Material* material);
		Renderable					getRenderable(std::string name);

		ArmatureTracker*			addArmature(Armature a);
		Armature					getArmature(std::string name);

		MeshTracker*				getMeshTracker(std::string name);
		ArmatureTracker*			getArmatureTracker(std::string name);

		void						runGarbageCollector();

	};

}