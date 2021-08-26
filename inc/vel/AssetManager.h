#pragma once

#include <string>

#include "plf_colony/plf_colony.h"
#include "robin_hood/robin_hood.h"


#include "vel/Shader.h"
#include "vel/Mesh.h"
#include "vel/Texture.h"
#include "vel/Material.h"
#include "vel/Renderable.h"
#include "vel/HDR.h"
#include "vel/Animation.h"
#include "vel/Armature.h"

#include "vel/AssetTrackers.h"

namespace vel
{
	class GPU;

	class AssetManager
	{
	private:
		GPU*							gpu;

		plf::colony<Shader>				shaders;
		plf::colony<ShaderTracker>		shaderTrackers;
		std::deque<ShaderTracker*>		shadersThatNeedGpuLoad;
		robin_hood::unordered_node_map<std::string, ShaderTracker*> shaderTrackerMap;
		
		plf::colony<Mesh>				meshes;
		plf::colony<MeshTracker>		meshTrackers;
		std::deque<MeshTracker*>		meshesThatNeedGpuLoad;
		robin_hood::unordered_node_map<std::string, MeshTracker*> meshTrackerMap;
		
		plf::colony<Texture>			textures;
		plf::colony<TextureTracker> 	textureTrackers;
		std::deque<TextureTracker*>		texturesThatNeedGpuLoad;
		robin_hood::unordered_node_map<std::string, TextureTracker*> textureTrackerMap;
        
        plf::colony<HDR>			    hdrs;
		plf::colony<HDRTracker> 	    hdrTrackers;
		std::deque<HDRTracker*>		    hdrsThatNeedGpuLoad;
		robin_hood::unordered_node_map<std::string, HDRTracker*> hdrTrackerMap;
		
		plf::colony<Material>			materials;
		plf::colony<MaterialTracker> 	materialTrackers;
		robin_hood::unordered_node_map<std::string, MaterialTracker*> materialTrackerMap;
		
		plf::colony<Renderable>			renderables;
		plf::colony<RenderableTracker> 	renderableTrackers;
		robin_hood::unordered_node_map<std::string, RenderableTracker*> renderableTrackerMap;
		
		plf::colony<Armature>			armatures;
		plf::colony<ArmatureTracker>	armatureTrackers;
		robin_hood::unordered_node_map<std::string, ArmatureTracker*> armatureTrackerMap;
		
		plf::colony<Animation>			animations;
		// since animations are tracked within an Armature, and they are only associated with a single armatureTracker
		// we shouldn't need to track them, just account for them when adding/removing an armature

	public:
		AssetManager(GPU* gpu);
		~AssetManager();


		void						sendNextToGpu();
		void						sendAllToGpu();

		std::string					loadShader(std::string name, std::string vertFile, std::string fragFile);
		Shader*						getShader(std::string name);
		bool						shaderIsGpuLoaded(std::string name);
		void						removeShader(std::string name);

		std::pair<std::vector<std::string>, std::string> loadMesh(std::string path);
		MeshTracker*				addMesh(Mesh m);
		Mesh*						getMesh(std::string name);
		bool						meshIsGpuLoaded(std::string name);
		void						removeMesh(std::string name);

		std::string					loadTexture(std::string name, std::string type, std::string path, std::vector<std::string> mips = std::vector<std::string>());
		Texture*					getTexture(std::string name);
		bool						textureIsGpuLoaded(std::string name);
		void						removeTexture(std::string name);
        
        
        std::string                 loadHdr(std::string name, std::string path);
        HDR*					    getHdr(std::string name);
		bool						hdrIsGpuLoaded(std::string name);
		void						removeHdr(std::string name);
        

		std::string					addMaterial(Material m);
		Material*					getMaterial(std::string name);
		void						removeMaterial(std::string name);

		Animation*					addAnimation(Animation a);

		std::string					addRenderable(std::string name, Shader* shader, Mesh* mesh, Material* material);
		Renderable					getRenderable(std::string name);
		void						removeRenderable(std::string name);

		ArmatureTracker*			addArmature(Armature a);
		Armature					getArmature(std::string name);
		void						removeArmature(std::string name);

		MeshTracker*				getMeshTracker(std::string name);
		ArmatureTracker*			getArmatureTracker(std::string name);

	};

}