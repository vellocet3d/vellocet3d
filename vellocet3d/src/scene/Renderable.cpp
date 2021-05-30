

#include "vel/App.h"
#include "vel/scene/Renderable.h"


namespace vel
{

    Renderable::Renderable(std::string rn, size_t shaderIndex, size_t meshIndex, size_t materialIndex, bool hasAlphaChannel) :
		name(rn),
        shaderIndex(shaderIndex),
        meshIndex(meshIndex),
        materialIndex(materialIndex),
		materialHasAlpha((hasAlphaChannel ? 1 : 0)),
		shader(App::get().getScene()->getShader(shaderIndex)),
		mesh(App::get().getScene()->getMesh(meshIndex)),
		material(App::get().getScene()->getMaterial(materialIndex))
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

    const size_t& Renderable::getShaderIndex() const
    {
        return this->shaderIndex;
    }

    const size_t& Renderable::getMeshIndex() const
    {
        return this->meshIndex;
    }

    const size_t& Renderable::getMaterialIndex() const
    {
        return this->materialIndex;
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