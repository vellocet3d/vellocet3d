#include "vel/Material.h"

namespace vel
{

	Material::Material(std::string name) :
		name(name),
		color(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)),
		hasAlphaChannel(false)
	{

	}

	void Material::setColor(glm::vec4 c)
	{
		this->color = c;
	}

	void Material::addTexture(Texture* t)
	{
		this->textures.push_back(t);
	}

	void Material::addAnimatedTexture(Texture* t, float fps)
	{
		if (!this->materialAnimator.has_value())
			this->materialAnimator = MaterialAnimator();

		this->addTexture(t);
		this->materialAnimator->addTextureAnimator(t->frames.size(), fps);
	}

	void Material::setHasAlphaChannel(bool b)
	{
		this->hasAlphaChannel = b;
	}

	void Material::pauseAnimatedTextureAfterCycles(unsigned int textureId, unsigned int cycles)
	{
		if (!this->materialAnimator.has_value())
			return;

		this->materialAnimator.value().setTextureAnimatorPauseAfterCycles(textureId, cycles);
	}

	void Material::setAnimatedTexturePause(unsigned int textureId, bool isPaused)
	{
		if (!this->materialAnimator.has_value())
			return;

		this->materialAnimator.value().setTextureAnimatorPause(textureId, isPaused);
	}

	bool Material::getAnimatedTexturePause(unsigned int textureId)
	{
		if (!this->materialAnimator.has_value())
			return true;

		return this->materialAnimator.value().getTexturePaused(textureId);
	}

	void Material::setAnimatedTextureReverse(unsigned int textureId, bool reverse)
	{
		if (!this->materialAnimator.has_value())
			return;

		this->materialAnimator.value().setAnimatedTextureReverse(textureId, reverse);
	}

	bool Material::getAnimatedTextureReversed(unsigned int textureId)
	{
		if (!this->materialAnimator.has_value())
			return true;

		return this->materialAnimator.value().getAnimatedTextureReversed(textureId);
	}

	std::string& Material::getName()
	{
		return this->name;
	}

	glm::vec4& Material::getColor()
	{
		return this->color;
	}

	std::vector<Texture*>& Material::getTextures()
	{
		return this->textures;
	}

	std::optional<MaterialAnimator>& Material::getMaterialAnimator()
	{
		return this->materialAnimator;
	}

	bool Material::getHasAlphaChannel()
	{
		return this->hasAlphaChannel;
	}

}