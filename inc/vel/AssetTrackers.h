#pragma once

#include <string>


#include "vel/Shader.h"
#include "vel/Mesh.h"
#include "vel/Texture.h"
#include "vel/Material.h"
#include "vel/Renderable.h"
#include "vel/Cubemap.h"
#include "vel/Animation.h"
#include "vel/Armature.h"



namespace vel
{
	struct ShaderTracker{
		Shader* 		ptr = nullptr;
		bool 			gpuLoaded = false;
		size_t 			usageCount = 0;
	};
	
	struct MeshTracker{
		Mesh* 			ptr = nullptr;
		bool 			gpuLoaded = false;
		size_t 			usageCount = 0;
	};
	
	struct TextureTracker{
		Texture* 		ptr = nullptr;
		bool 			gpuLoaded = false;
		size_t 			usageCount = 0;
	};
    
    struct InfiniteCubemapTracker{
		Cubemap*	ptr = nullptr;
		bool 			gpuLoaded = false;
		size_t 			usageCount = 0;
	};
	
	struct MaterialTracker{
		Material* 		ptr = nullptr;
		size_t 			usageCount = 0;
	};
	
	struct RenderableTracker{
		Renderable* 	ptr = nullptr;
		size_t 			usageCount = 0;
	};
	
	struct AnimationTracker{
		Animation* 		ptr = nullptr;
		size_t 			usageCount = 0;
	};
	
	struct ArmatureTracker{
		Armature* 		ptr = nullptr;
		size_t 			usageCount = 0;
	};
}