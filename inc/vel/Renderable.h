#pragma once

#include <vector>
#include <string>
#include <optional>

#include "vel/Shader.h"
#include "vel/Mesh.h"
#include "vel/Material.h"
#include "vel/MaterialAnimator.h"

#include "vel/ptrsac.h"

namespace vel
{
	class Actor;
	
    class Renderable
    {
    private:
		std::string						name;
		Shader*							shader;
		Mesh*							mesh;
		std::optional<Material>			material;



    public:
									Renderable(std::string rn, Shader* shader, Mesh* mesh, Material material);
									Renderable(std::string rn, Shader* shader, Mesh* mesh);
		const std::string&			getName();
		Shader*						getShader();
		std::optional<Material>&	getMaterial();
		Mesh*						getMesh();

		ptrsac<Actor*>				actors;

    };
}