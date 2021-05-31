#pragma once

#include <vector>
#include <string>

#include "vel/Shader.h"
#include "vel/scene/mesh/Mesh.h"
#include "vel/scene/material/Material.h"

namespace vel
{
    class Renderable
    {
    private:
		std::string					name;
        std::vector<size_t>			actorIndexes;
		size_t						shaderIndex;
		size_t						meshIndex;
        size_t						materialIndex;
		size_t						materialHasAlpha;

		Shader*						shader;
		Mesh*						mesh;
		Material*					material;

    public:
									Renderable(std::string rn, size_t shaderIndex, size_t meshIndex, size_t materialIndex,
										Shader* shader, Mesh* mesh, Material* material, bool hasAlphaChannel = false);
        const size_t&				getShaderIndex() const;
		const size_t&				getMeshIndex() const;
        const size_t&				getMaterialIndex() const;
		const size_t&				getMaterialHasAlpha() const;
        const std::vector<size_t>&	getActorIndexes() const;
		size_t						addActorIndex(size_t actorIndex);
        void						freeActorIndex(size_t actorIndex);
		const std::string&			getName();

		Shader*						getShader();
		Material*					getMaterial();
		Mesh*						getMesh();


    };
}