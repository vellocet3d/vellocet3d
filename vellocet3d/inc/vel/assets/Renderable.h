#pragma once

#include <vector>
#include <string>

#include "vel/assets/Shader.h"
#include "vel/assets/mesh/Mesh.h"
#include "vel/assets/material/Material.h"

#include "vel/dep/sac.h"

namespace vel
{
	class Actor;
	
    class Renderable
    {
    private:
		std::string					name;		
		Shader*						shader;
		Mesh*						mesh;
		Material*					material;
		size_t						materialHasAlpha;

    public:
									Renderable(std::string rn, Shader* shader, Mesh* mesh, Material* material);
		const size_t&				getMaterialHasAlpha() const;
		const std::string&			getName();
		Shader*						getShader();
		Material*					getMaterial();
		Mesh*						getMesh();

		sac<Actor*>					actors;

    };
}