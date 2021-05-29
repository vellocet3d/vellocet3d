


#include "vel/scene/stage/RenderCommand.h"


namespace vel
{

    RenderCommand::RenderCommand(size_t sI, size_t mI, size_t tI, size_t ta) :
        shaderIndex(sI),
        meshIndex(mI),
        textureIndex(tI),
		textureHasAlpha(ta){}

	size_t RenderCommand::addActorIndex(size_t actorIndex)
	{
		size_t slotIndex = this->actorIndexes.size();
		this->actorIndexes.push_back(actorIndex);
		return slotIndex;
	}

    void RenderCommand::freeActorIndex(size_t actorIndex)
    {
		for (size_t i = 0; i < this->actorIndexes.size(); i++)
			if (this->actorIndexes.at(i) == actorIndex)
				this->actorIndexes.erase(this->actorIndexes.begin() + i); //TODO shifting vector, maybe revise in future
    }

    const size_t& RenderCommand::getShaderIndex() const
    {
        return this->shaderIndex;
    }

    const size_t& RenderCommand::getMeshIndex() const
    {
        return this->meshIndex;
    }

    const size_t& RenderCommand::getTextureIndex() const
    {
        return this->textureIndex;
    }
    
	const size_t& RenderCommand::getTextureHasAlpha() const
	{
		return this->textureHasAlpha;
	}

    const std::vector<size_t>& RenderCommand::getActorIndexes() const
    {
        return this->actorIndexes;
    }
}