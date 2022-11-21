#pragma once

#include <optional>
#include <string>

#include "glm/glm.hpp"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

#include "vel/Armature.h"
#include "vel/ArmatureBone.h"
#include "vel/Mesh.h"
#include "vel/Transform.h"
#include "vel/Renderable.h"
#include "vel/Material.h"


namespace vel
{
	class	Stage;
	class	CollisionWorld;

	class Actor
	{
	private:
		bool											deleted;
		bool											visible;
		std::string										name;
		bool											dynamic;
		Transform										transform;
		std::optional<Transform>						previousTransform;
		Actor*											parentActor;
		ArmatureBone*									parentArmatureBone;
		std::vector<Actor*>								childActors;
		Armature*										armature;
		std::vector<std::pair<size_t, unsigned int>>	activeBones; // the bones from the armature that are actually used by the mesh, 
																	// the glue between an armature and a mesh (index is mesh bone index, value is armature bone index)
																	// TODO: could this be part of Renderable instead...?
																				
		std::optional<Renderable>						tempRenderable;
		std::optional<Renderable*>						stageRenderable;

		Mesh*											mesh; // pointer to mesh used by this Actor independant of renderable. required for headless mode since there will be no renderable instance
		
		glm::vec4										color; // overwrites material color property, defaults to 1.0

		std::optional<Material>							material; // overwrites renderable material
		
		void*											userPointer;

	public:
		Actor(std::string name);
		Actor											cleanCopy(std::string newName);
		void											setDynamic(bool dynamic);

		const std::string								getName() const;
		void											setName(std::string newName);

		void											addRenderable(Renderable r);
		void											addMesh(Mesh* m);
		std::optional<Renderable>&						getTempRenderable();
		void											clearTempRenderable();
		void											setStageRenderable(Renderable* r);
		std::optional<Renderable*>						getStageRenderable(); //TODO: tf is this an optional pointer for???



		void											setDeleted(bool d);
		const bool										isDeleted() const;
		void											setVisible(bool v);
		const bool										isVisible() const;
		const bool										isAnimated() const;
		const bool										isDynamic() const;
		void											setArmature(Armature* arm);
		Armature*										getArmature();

		const std::vector<std::pair<size_t, unsigned int>>& getActiveBones() const;
		void											setActiveBones(std::vector<std::pair<size_t, unsigned int>> activeBones);
		void											setParentActor(Actor* a);
		void											setParentArmatureBone(ArmatureBone* b);
		void											addChildActor(Actor* a);
		Transform&										getTransform();
		std::optional<Transform>&						getPreviousTransform();
		void											updatePreviousTransform();
		void											clearPreviousTransform();
		glm::mat4										getWorldMatrix();
		glm::mat4										getWorldRenderMatrix(float alpha); // contains logic for interpolation
		glm::vec3										getInterpolatedTranslation(float alpha);
		glm::quat										getInterpolatedRotation(float alpha);
		glm::vec3										getInterpolatedScale(float alpha);

		void											removeParentActor(bool calledFromRemoveChildActor = false);
		void											removeChildActor(Actor* a, bool calledFromRemoveParentActor = false);

		Mesh*											getMesh();

		void											setColor(glm::vec4 c);
		const glm::vec4&								getColor();

		void											setMaterial(Material m);
		std::optional<Material>&						getMaterial();

		void*											getUserPointer();
		void											setUserPointer(void* p);

	};
}