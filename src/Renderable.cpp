

#include "vel/App.h"
#include "vel/Renderable.h"


namespace vel
{

    Renderable::Renderable(std::string rn, Shader* shader, Mesh* mesh) :
		name(rn),
		shader(shader),
		mesh(mesh){}

	const std::string& Renderable::getName()
	{
		return this->name;
	}

	Shader*	Renderable::getShader()
	{
		return this->shader;
	}

	Mesh* Renderable::getMesh()
	{
		return this->mesh;
	}


}