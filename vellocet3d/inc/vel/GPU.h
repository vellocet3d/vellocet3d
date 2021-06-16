#pragma once

#include <vector>
#include <string>
#include <memory>
#include <optional>

#include "glm/glm.hpp"


#include "vel/assets/Shader.h"
#include "vel/assets/mesh/Mesh.h"
#include "vel/assets/material/Texture.h"
#include "vel/assets/material/Material.h"
#include "vel/scene/stage/collision/CollisionDebugDrawer.h"

struct GLFWusercontext;

namespace vel
{
	class GPU
	{
	private:
		Shader*								activeShader;
		Mesh*								activeMesh;
		Material*							activeMaterial;

	public:
		GPU();
		~GPU();
		GPU(GPU&&) = default;
		void								enableDepthTest();
		void								clearBuffers(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 0.0f);
		void								drawLinesOnly();

		const Shader* const					getActiveShader() const;
		const Mesh*	const					getActiveMesh() const;
		const Material*	const				getActiveMaterial() const;

		void								loadShader(Shader* s);
		void								loadMesh(Mesh* m);
		void								loadTexture(Texture* t);


		void								useShader(Shader* s);
		void								useMaterial(Material* m);
		void								useMesh(Mesh* m);

		void								setShaderBool(const std::string &name, bool value) const;
		void								setShaderInt(const std::string &name, int value) const;
		void								setShaderFloat(const std::string &name, float value) const;
		void								setShaderMat4(const std::string &name, glm::mat4 value) const;

		void								drawGpuMesh();
		void								clearDepthBuffer();

		void								wipe(std::vector<Shader>& shaders, std::vector<Mesh>& meshes, std::vector<Texture>& textures);
		void								finish();
		void								enableBlend();

		void								debugDrawCollisionWorld(CollisionDebugDrawer* cdd);

	};
}