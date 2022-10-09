#pragma once

#include <vector>
#include <string>

#include "vel/Shader.h"
#include "vel/Mesh.h"

#include "vel/ptrsac.h"

namespace vel
{
	class Actor;
	
    class Renderable
    {
    private:
		std::string					name;
		Shader*						shader;
		Mesh*						mesh;

    public:
									Renderable(std::string rn, Shader* shader, Mesh* mesh);
		const std::string&			getName();
		Shader*						getShader();
		Mesh*						getMesh();

		ptrsac<Actor*>				actors;

    };
}