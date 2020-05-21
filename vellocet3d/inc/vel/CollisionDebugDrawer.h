#pragma once

#include <vector>

#include "btBulletCollisionCommon.h"


namespace vel
{
	class CollisionDebugDrawer : public btIDebugDraw 
	{

	private:
		int					debug_mode = 1; // default to DBG_DrawWireframe
		std::vector<float>	verts;
		unsigned int		VAO;
		unsigned int		VBO;

	public:
					CollisionDebugDrawer();
					~CollisionDebugDrawer();
		void		draw();
		void		drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;
		void		drawContactPoint(const btVector3 &, const btVector3 &, btScalar, int, const btVector3 &) override;
		void		reportErrorWarning(const char *) override;
		void		draw3dText(const btVector3 &, const char *) override;
		void		setDebugMode(int debug_mode) override;
		int			getDebugMode(void) const override;

	};
}