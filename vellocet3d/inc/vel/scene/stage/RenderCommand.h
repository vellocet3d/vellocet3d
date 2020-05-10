#pragma once

#include <vector>


namespace vel::scene::stage
{
    class RenderCommand
    {
    private:
        std::vector<size_t>			actorIndexes;
        std::vector<size_t>			freeSlots;
		size_t						shaderIndex;
		size_t						meshIndex;
        size_t						textureIndex;

    public:
									RenderCommand(size_t sI, size_t mI, size_t tI);
        const size_t&				getShaderIndex() const;
		const size_t&				getMeshIndex() const;
        const size_t&				getTextureIndex() const;
        const std::vector<size_t>&	getActorIndexes() const;
		size_t						addActorIndex(size_t actorIndex);
        void						freeActorIndex(size_t actorSlot);

    };
}