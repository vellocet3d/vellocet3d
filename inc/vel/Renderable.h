#pragma once

#include <vector>
#include <string>

#include "vel/Shader.h"
#include "vel/Mesh.h"
#include "vel/Material.h"

#include "vel/sac.h"

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