

#include "vel/App.h"
#include "vel/Renderable.h"


namespace vel
{

    Renderable::Renderable(std::string rn, Shader* shader, Mesh* mesh, Material* material) :
		name(rn),
		shader(shader),
		mesh(mesh),
		material(material),
		materialHasAlpha((material->hasAlphaChannel ? 1 : 0)){}

	const std::string& Renderable::getName()
	{
		return this->name;
	}
    
	const size_t& Renderable::getMaterialHasAlpha() const
	{
		return this->materialHasAlpha;
	}

	Shader*	Renderable::getShader()
	{
		return this->shader;
	}

	Material* Renderable::getMaterial()
	{
		return this->material;
	}

	Mesh* Renderable::getMesh()
	{
		return this->mesh;
	}

}