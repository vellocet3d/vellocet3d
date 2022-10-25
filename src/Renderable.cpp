

#include "vel/App.h"
#include "vel/Renderable.h"


namespace vel
{

    Renderable::Renderable(std::string rn, Shader* shader, Mesh* mesh, Material material) :
		name(rn),
		shader(shader),
		mesh(mesh),
		material(material)
	{}

	Renderable::Renderable(std::string rn, Shader* shader, Mesh* mesh) :
		name(rn),
		shader(shader),
		mesh(mesh)
	{}

	const std::string& Renderable::getName()
	{
		return this->name;
	}

	Shader*	Renderable::getShader()
	{
		return this->shader;
	}

	std::optional<Material>& Renderable::getMaterial()
	{
		return this->material;
	}

	Mesh* Renderable::getMesh()
	{
		return this->mesh;
	}


}