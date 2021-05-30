#pragma once

#include <vector>
#include <string>
#include <memory>
#include <optional>

#include "glm/glm.hpp"


#include "vel/Shader.h"
#include "vel/scene/mesh/Mesh.h"
#include "vel/scene/material/Texture.h"
#include "vel/scene/material/Material.h"
#include "vel/CollisionDebugDrawer.h"

struct GLFWusercontext;

namespace vel
{
    class GPU
    {
    private:
        Shader*								activeShader;
		Mesh*								activeMesh;
		Material*							activeMaterial;

		std::optional<CollisionDebugDrawer> collisionDebugDrawer;
		
    public:
											GPU();
											~GPU();
											GPU(GPU&&) = default;
        void								enableDepthTest();
        void								clearBuffers(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 0.0f);
        void								drawLinesOnly();

        const Shader*						getActiveShader() const;
        const Mesh*							getActiveMesh() const;
        const Material*						getActiveMaterial() const;

		void								loadShader(Shader& s);
		void								loadMesh(Mesh& m);
		void								loadTexture(Texture& t);

        
        void								useShader(Shader* s);
		void								useMaterial(Material* m);
		void								useMesh(Mesh* m);

        void								setShaderBool(const std::string &name, bool value) const;
        void								setShaderInt(const std::string &name, int value) const;
        void								setShaderFloat(const std::string &name, float value) const;
        void								setShaderMat4(const std::string &name, glm::mat4 value) const;
        
        void								drawGpuMesh();
		CollisionDebugDrawer*				getCollisionDebugDrawer();
		void								clearDepthBuffer();

		void								wipe();
		void								finish();
		void								enableBlend();

    };
}