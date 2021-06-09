#pragma once

#include <vector>

#include "btBulletCollisionCommon.h"
#include "glm/glm.hpp"

#include "vel/Shader.h"

namespace vel
{
	struct BulletDebugDrawData
	{
		glm::vec3 position;
		glm::vec3 color;
	};

	class CollisionDebugDrawer : public btIDebugDraw
	{

	private:
		int										debug_mode = 1; // default to DBG_DrawWireframe
																// 2 = AABB
		std::vector<BulletDebugDrawData> 		verts;
		Shader*									shaderProgram;

	public:
		CollisionDebugDrawer();
		~CollisionDebugDrawer();
		void		drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;
		void		drawContactPoint(const btVector3 &, const btVector3 &, btScalar, int, const btVector3 &) override;
		void		reportErrorWarning(const char *) override;
		void		draw3dText(const btVector3 &, const char *) override;
		void		setDebugMode(int debug_mode) override;
		int			getDebugMode(void) const override;
		void		setShaderProgram(Shader* s);
		Shader*		getShaderProgram();

		std::vector<BulletDebugDrawData>& getVerts();

	};
}