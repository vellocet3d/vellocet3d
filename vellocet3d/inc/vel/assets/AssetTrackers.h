#pragma once

#include <string>


#include "vel/assets/Shader.h"
#include "vel/assets/mesh/Mesh.h"
#include "vel/assets/material/Texture.h"
#include "vel/assets/material/Material.h"
#include "vel/assets/Renderable.h"
#include "vel/assets/animation/Animation.h"
#include "vel/assets/armature/Armature.h"



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