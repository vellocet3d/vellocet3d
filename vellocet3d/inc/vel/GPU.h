#pragma once

#include <vector>
#include <string>
#include <memory>
#include <optional>

#include "glm/glm.hpp"


#include "vel/Shader.h"
#include "vel/scene/mesh/Mesh.h"
#include "vel/scene/mesh/Texture.h"
#include "vel/CollisionDebugDrawer.h"

struct GLFWusercontext;

namespace vel
{
    class GPU
    {
    private:
        std::string							shaderDirectory;
        std::vector<Shader>					shaders;
        std::vector<scene::mesh::Texture>			textures;
		std::vector<scene::mesh::Renderable>		meshRenderables;
        size_t								activeShaderIndex;
		size_t								activeMeshRenderableIndex;
		size_t								activeTextureIndex;
		//std::unique_ptr<CollisionDebugDrawer> collisionDebugDrawer;
		std::optional<CollisionDebugDrawer> collisionDebugDrawer;
		

    public:
											GPU(std::string shaderDirectory, std::string dvs, std::string dfs, std::string dsvs, std::string dsfs,
												std::string ddvs, std::string ddfs);
											~GPU();
											GPU(GPU&&) = default;
        void								enableDepthTest();
        void								clearBuffers(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 0.0f);
        void								drawLinesOnly();
        const size_t						getActiveShaderIndex() const;
        const size_t						getActiveMeshRenderableIndex() const;
        const size_t						getActiveTextureIndex() const;
		size_t								loadShader(const std::string name, const std::string vertFile, const std::string fragFile);
		size_t								loadMesh(scene::mesh::Mesh& m);
		size_t								loadTexture(std::string type, std::string dir, std::string filename);
        void								useTexture(size_t textureIndex);
        const std::vector<scene::mesh::Texture>&	getTextures() const;
        void								useShader(size_t shaderIndex);
        void								setShaderBool(const std::string &name, bool value) const;
        void								setShaderInt(const std::string &name, int value) const;
        void								setShaderFloat(const std::string &name, float value) const;
        void								setShaderMat4(const std::string &name, glm::mat4 value) const;
        void								useMeshRenderable(size_t meshRenderableIndex);
        void								drawMeshRenderable();
		CollisionDebugDrawer*				getCollisionDebugDrawer();
		void								clearDepthBuffer();
		std::vector<std::string>			getActiveShaderNames(); // added this to assist debugging
		

		void								primeGPU();


		void								wipe();
		void								finish();
		void								enableBlend();

    };
}