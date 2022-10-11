#pragma once

#include <string>
#include <unordered_map>

//#include "plf_colony/plf_colony.h"
//#include "robin_hood/robin_hood.h"

#include "sac.h"

#include "vel/Shader.h"
#include "vel/Mesh.h"
#include "vel/Texture.h"
#include "vel/Material.h"
#include "vel/Renderable.h"
#include "vel/Animation.h"
#include "vel/Armature.h"

#include "vel/AssetTrackers.h"

namespace vel
{
	class GPU;

	class AssetManager
	{
	private:
		GPU*												gpu;

		sac<Shader>											shaders;
		sac<ShaderTracker>									shaderTrackers;
		std::deque<ShaderTracker*>							shadersThatNeedGpuLoad;
		
		sac<Mesh>											meshes;
		sac<MeshTracker>									meshTrackers;
		std::deque<MeshTracker*>							meshesThatNeedGpuLoad;

		sac<Texture>										textures;
		sac<TextureTracker> 								textureTrackers;
		std::deque<TextureTracker*>							texturesThatNeedGpuLoad;
		

		sac<Camera>											cameras;
		sac<CameraTracker> 									cameraTrackers;

		sac<Material>										materials;
		sac<MaterialTracker> 								materialTrackers;

		sac<Renderable>										renderables;
		sac<RenderableTracker> 								renderableTrackers;

		sac<Armature>										armatures;
		sac<ArmatureTracker>								armatureTrackers;

		sac<Animation>										animations;
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
        

		std::string					addMaterial(Material m);
		Material*					getMaterial(std::string name);
		void						removeMaterial(std::string name);

		std::string					addCamera(Camera c);
		Camera*						getCamera(std::string name);
		void						removeCamera(std::string name);

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