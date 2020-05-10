


#include "vel/scene/stage/RenderCommand.h"


namespace vel::scene::stage
{

    RenderCommand::RenderCommand(size_t sI, size_t mI, size_t tI) :
        shaderIndex(sI),
        meshIndex(mI),
        textureIndex(tI) {}

    size_t RenderCommand::addActorIndex(size_t actorIndex)
    {
		size_t slotIndex;

        if (this->freeSlots.size() > 0)
        {
            slotIndex = this->freeSlots.back();
            this->freeSlots.pop_back();
            this->actorIndexes.at(slotIndex) = actorIndex;
        }
        else
        {
            slotIndex = this->actorIndexes.size();
            this->actorIndexes.push_back(actorIndex);
        }

        return slotIndex;
    }

    void RenderCommand::freeActorIndex(size_t actorSlot)
    {
        this->freeSlots.push_back(actorSlot);
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
    
    const std::vector<size_t>& RenderCommand::getActorIndexes() const
    {
        return this->actorIndexes;
    }
}