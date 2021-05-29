#pragma once

#include <vector>
#include <string>
#include <memory>
#include <optional>

#include "glm/glm.hpp"


#include "vel/Shader.h"
#include "vel/scene/mesh/Mesh.h"
#include "vel/scene/material/Texture.h"
#include "vel/CollisionDebugDrawer.h"

struct GLFWusercontext;

namespace vel
{
    class GPU
    {
    private:
        std::vector<Shader>					shaders;
        std::vector<Texture>				textures;
		std::vector<GpuMesh>				gpuMeshes;
        size_t								activeShaderIndex;
		size_t								activeGpuMeshIndex;
		size_t								activeTextureIndex;
		//std::unique_ptr<CollisionDebugDrawer> collisionDebugDrawer;
		std::optional<CollisionDebugDrawer> collisionDebugDrawer;
		

    public:
											GPU();
											~GPU();
											GPU(GPU&&) = default;
        void								enableDepthTest();
        void								clearBuffers(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 0.0f);
        void								drawLinesOnly();
        const size_t						getActiveShaderIndex() const;
        const size_t						getActiveGpuMeshIndex() const;
        const size_t						getActiveTextureIndex() const;
		size_t								loadShader(const std::string name, const std::string vertFile, const std::string fragFile);
		size_t								loadMesh(Mesh& m);
		size_t								loadTexture(Texture texture);
        void								useTexture(size_t textureIndex);
        const std::vector<Texture>&			getTextures() const;
        void								useShader(size_t shaderIndex);
        void								setShaderBool(const std::string &name, bool value) const;
        void								setShaderInt(const std::string &name, int value) const;
        void								setShaderFloat(const std::string &name, float value) const;
        void								setShaderMat4(const std::string &name, glm::mat4 value) const;
        void								useGpuMesh(size_t gpuMeshIndex);
        void								drawGpuMesh();
		CollisionDebugDrawer*				getCollisionDebugDrawer();
		void								clearDepthBuffer();
		std::vector<std::string>			getActiveShaderNames(); // added this to assist debugging
	
		const Texture&						getTexture(size_t i) const;

		void								wipe();
		void								finish();
		void								enableBlend();

    };
}