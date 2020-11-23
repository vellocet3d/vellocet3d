

#include "vel/helpers/MultiTweener.h"




namespace vel::helpers
{
	/*
	Smoothly translate between a vector of glm::vec3 values
	*/
	MultiTweener::MultiTweener(std::vector<glm::vec3> vecs, float speed, bool repeat) :
		vecs(vecs),
		speed(speed),
		speedPerVec(speed * (float)vecs.size()),
		repeat(repeat)
	{

		size_t i = 0;
		for (auto& v : this->vecs)
		{
			i++;
			if (i < this->vecs.size())
			{
				this->tweens.push_back(Tweener(v, this->vecs[i], this->speedPerVec));
			}
		}

	};


	glm::vec3 MultiTweener::update(float dt)
	{
		for (auto& t : this->tweens)
		{
			if (t.isForwardComplete())
				continue;

			return t.updateForward(dt);
		}

		if (!this->repeat)
			return this->tweens[this->tweens.size() - 1].updateForward(dt);

		for (auto& t : this->tweens)
		{
			t.reset();
		}

		return this->update(dt);
	}

}