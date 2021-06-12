#pragma once

#include <vector>
#include <string>

#include "vel/assets/Shader.h"
#include "vel/assets/mesh/Mesh.h"
#include "vel/assets/material/Material.h"

namespace vel
{
    class Renderable
    {
    private:
		std::string					name;
        std::vector<size_t>			actorIndexes;

		

		Shader*						shader;
		Mesh*						mesh;
		Material*					material;

		size_t						materialHasAlpha;

    public:
									Renderable(std::string rn, Shader* shader, Mesh* mesh, Material* material);

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