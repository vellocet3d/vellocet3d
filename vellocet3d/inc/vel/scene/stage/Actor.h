#pragma once

#include <optional>
#include <string>

#include "glm/glm.hpp"

#include "vel/scene/armature/Armature.h"
#include "vel/scene/mesh/Mesh.h"
#include "vel/scene/stage/Transform.h"


namespace vel::scene::stage
{
    class Actor
    {
    private:
        bool											deleted;
		bool											visible;
        std::string										name;
		bool											dynamic;
        Transform										transform;
		std::optional<Transform>						previousTransform;
		std::optional<Actor*>							parentActor;
		std::optional<vel::scene::armature::Bone*>		parentActorBone;
		std::vector<Actor*>								childActors;
		std::optional<vel::scene::armature::Armature>	armature;
		std::optional<std::vector<std::pair<size_t, std::string>>> activeBones; // the bones from the armature that are actually used by the mesh, 
																				// the glue between an armature and a mesh (index is mesh bone index, value is armature bone index)
		std::optional<std::pair<size_t, size_t>>		renderCommand;
        std::optional<size_t>							shaderIndex;
        std::optional<size_t>							meshIndex;
        std::optional<size_t>							textureIndex;							
		

    public:
														Actor(std::string name, Transform t);
		Actor											cleanCopy(std::string newName);
		void											setDynamic(bool dynamic);
        void											setMeshIndex(size_t i);
        void											setShaderIndex(size_t i);
        void											setTextureIndex(size_t i);
        const std::string								getName() const;
		void											setName(std::string newName);
        const std::optional<size_t>&					getShaderIndex() const;
        const std::optional<size_t>&					getMeshIndex() const;
        const std::optional<size_t>&					getTextureIndex() const;
        void											addRenderCommand(std::pair<size_t, size_t> cmd);
        const std::pair<size_t, size_t>&				getRenderCommand() const;
        void											setDeleted(bool d);
        const bool										isDeleted() const;
		void											setVisible(bool v);
		const bool										isVisible() const;
		const bool										isAnimated() const;
		const bool										isDynamic() const;
		void											setArmature(vel::scene::armature::Armature& arm);
		vel::scene::armature::Armature&					getArmature();
		vel::scene::mesh::Mesh&							getMesh();
		void											animate(std::string animationName);
		const std::optional<std::vector<std::pair<size_t, std::string>>>& getActiveBones() const;
		void											setActiveBones(std::vector<std::pair<size_t, std::string>> activeBones);
		void											setParentActor(Actor* a);
		void											setParentActorBone(vel::scene::armature::Bone* b);
		void											addChildActor(Actor* a);
		Transform&										getTransform();
		std::optional<Transform>&						getPreviousTransform();
		void											updatePreviousTransform();
		void											translate(glm::vec3 translation);
		void											rotate(float angle, glm::vec3 axis);
		void											rotate(glm::quat rotation);
		void											scale(glm::vec3 scale);
		std::optional<glm::mat4>						getParentMatrix();
		glm::mat4										getWorldMatrix();
		glm::mat4										getWorldRenderMatrix(float alpha); // contains logic for interpolation

		const glm::vec3&								getTranslation();
		const glm::quat&								getRotation();
		const glm::vec3&								getScale();

		// The below methods (upon implementing) will allow for the retrieval of this actor's translation/rotation/scale
		// taking into account it's parent's transform, ie it's world translation/rotation/scale
		//glm::vec3										getWorldTranslation();
		//glm::quat										getWorldRotation();
		//glm::vec3										getWorldScale();

    };
}