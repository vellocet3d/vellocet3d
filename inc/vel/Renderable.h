#pragma once

#include <vector>
#include <string>
#include <optional>

#include "vel/Shader.h"
#include "vel/Mesh.h"
#include "vel/Material.h"
#include "vel/MaterialAnimator.h"

#include "vel/ptrsac.h"

namespace vel
{
	class Actor;
	
    class Renderable
    {
    private:
		std::string						name;
		Shader*							shader;
		Mesh*							mesh;
		Material*						material;
		size_t							materialHasAlpha;

		//MaterialAnimator*				materialAnimator;
		//bool							materialAnimatorUpdatedByActor;


    public:
									Renderable(std::string rn, Shader* shader, Mesh* mesh, Material* material);
		const size_t&				getMaterialHasAlpha() const;
		const std::string&			getName();
		Shader*						getShader();
		Material*					getMaterial();
		Mesh*						getMesh();

		ptrsac<Actor*>				actors;

		//void								setMaterialAnimator(MaterialAnimator ma);
		//MaterialAnimator*					getMaterialAnimator();
		//void								setMaterialAnimatorUpdatedByActor(bool b);
		//bool								getMaterialAnimatorUpdatedByActor();

    };
}