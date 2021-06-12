

#include "vel/App.h"
#include "vel/assets/Renderable.h"


namespace vel
{

    Renderable::Renderable(std::string rn, Shader* shader, Mesh* mesh, Material* material) :
		name(rn),
		shader(shader),
		mesh(mesh),
		material(material),
		materialHasAlpha((material->hasAlphaChannel ? 1 : 0))
	{
		
	}

	size_t Renderable::addActorIndex(size_t actorIndex)
	{
		size_t slotIndex = this->actorIndexes.size();
		this->actorIndexes.push_back(actorIndex);
		return slotIndex;
	}

    void Renderable::freeActorIndex(size_t actorIndex)
    {
		for (size_t i = 0; i < this->actorIndexes.size(); i++)
			if (this->actorIndexes.at(i) == actorIndex)
				this->actorIndexes.erase(this->actorIndexes.begin() + i); //TODO shifting vector, maybe revise in future
    }

	const std::string& Renderable::getName()
	{
		return this->name;
	}
    
	const size_t& Renderable::getMaterialHasAlpha() const
	{
		return this->materialHasAlpha;
	}

    const std::vector<size_t>& Renderable::getActorIndexes() const
    {
        return this->actorIndexes;
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