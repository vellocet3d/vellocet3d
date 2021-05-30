#pragma once

#include <optional>
#include <string>

#include "glm/glm.hpp"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

#include "vel/scene/armature/Armature.h"
#include "vel/scene/armature/ArmatureBone.h"
#include "vel/scene/mesh/Mesh.h"
#include "vel/scene/stage/Transform.h"
#include "vel/scene/Renderable.h"


namespace vel
{
	class Stage;

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
		std::optional<ArmatureBone*>					parentActorBone;
		std::vector<Actor*>								childActors;
		Armature*										armature;
		std::optional<std::vector<std::pair<size_t, std::string>>> activeBones; // the bones from the armature that are actually used by the mesh, 
																				// the glue between an armature and a mesh (index is mesh bone index, value is armature bone index)
		
		std::vector<Renderable>							tempRenderables;
		std::vector<size_t>								parentRenderableIndexes;
		std::optional<size_t>							containerIndex;

		
		btRigidBody*									rigidBody;
		btPairCachingGhostObject*						ghostObject;
		bool											manualTransform;
		//Stage*											parentStage;

		
		

    public:
														//Actor(std::string name, Transform t, Stage* parentStage);
														Actor(std::string name);
		Actor											cleanCopy(std::string newName);
		void											setDynamic(bool dynamic);

        const std::string								getName() const;
		void											setName(std::string newName);

		void											addRenderable(Renderable r);
		std::vector<Renderable>&						getRenderables();
		void											clearTempRenderables();
        void											addParentRenderableIndex(size_t ri);
        const std::vector<size_t>&						getParentRenderableIndexes() const;

		void											setContainerIndex(size_t ci);
		const std::optional<size_t>&					getContainerIndex() const;

        void											setDeleted(bool d);
        const bool										isDeleted() const;
		void											setVisible(bool v);
		const bool										isVisible() const;
		const bool										isAnimated() const;
		const bool										isDynamic() const;
		void											setArmature(Armature* arm);
		Armature*										getArmature();

		const std::optional<std::vector<std::pair<size_t, std::string>>>& getActiveBones() const;
		void											setActiveBones(std::vector<std::pair<size_t, std::string>> activeBones);
		void											setParentActor(Actor* a);
		void											setParentActorBone(ArmatureBone* b);
		void											addChildActor(Actor* a);
		Transform&										getTransform();
		std::optional<Transform>&						getPreviousTransform();
		void											updatePreviousTransform();
		void											clearPreviousTransform();
		std::optional<glm::mat4>						getParentMatrix();
		glm::mat4										getWorldMatrix();
		glm::mat4										getWorldRenderMatrix(float alpha); // contains logic for interpolation
		glm::vec3										getInterpolatedTranslation(float alpha);
		glm::quat										getInterpolatedRotation(float alpha);
		glm::vec3										getInterpolatedScale(float alpha);

		void											setRigidBody(btRigidBody* rb);
		btRigidBody*									getRigidBody();
		void											setGhostObject(btPairCachingGhostObject* go);
		btPairCachingGhostObject*						getGhostObject();
		void											setManualTransform(bool mt);
		bool											getManualTransform();

		
		void											processTransform();
		void											setTextureHasAlphaChannel(bool in);
		bool											getTextureHasAlphaChannel();

		void											removeParentActor(bool calledFromRemoveChildActor = false);
		void											removeChildActor(Actor* a, bool calledFromRemoveParentActor = false);



    };
}