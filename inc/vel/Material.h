#pragma once

#include <string>
#include <optional>
#include <vector>

#include "glm/glm.hpp"

#include "vel/Texture.h"
#include "vel/MaterialAnimator.h"


namespace vel
{
	class Material
	{
	private:
		std::string							name;
		glm::vec4							color;
		std::vector<Texture*>				textures;
		bool								hasAlphaChannel;

		std::optional<MaterialAnimator>		materialAnimator;

	public:
		Material(std::string name);

		void								setColor(glm::vec4 c);
		void								addTexture(Texture* t);
		void								addAnimatedTexture(Texture* t, float fps);
		void								setHasAlphaChannel(bool b);

		void								pauseAnimatedTextureAfterCycles(unsigned int textureId, unsigned int cycles);
		void								setAnimatedTexturePause(unsigned int textureId, bool isPaused);
		bool								getAnimatedTexturePause(unsigned int textureId);

		std::string&						getName();
		glm::vec4&							getColor();
		std::vector<Texture*>&				getTextures();
		std::optional<MaterialAnimator>&	getMaterialAnimator();
		bool								getHasAlphaChannel();
	};
}