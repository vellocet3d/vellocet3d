#include <iostream>

#include "glad/glad.h"

#include "vel/CollisionDebugDrawer.h"



namespace vel
{
	CollisionDebugDrawer::CollisionDebugDrawer(){};
	CollisionDebugDrawer::~CollisionDebugDrawer(){};

	void CollisionDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
	{
		auto colorVec = glm::vec3(color.getX(), color.getY(), color.getZ());

		BulletDebugDrawData data;
		data.position = glm::vec3(from.getX(), from.getY(), from.getZ());
		data.color = colorVec;
		this->verts.push_back(data);

		data.position = glm::vec3(to.getX(), to.getY(), to.getZ());
		this->verts.push_back(data);
	}

	void CollisionDebugDrawer::drawContactPoint(const btVector3&, const btVector3&, btScalar, int, const btVector3&) {}
	void CollisionDebugDrawer::reportErrorWarning(const char*) {}
	void CollisionDebugDrawer::draw3dText(const btVector3&, const char *) {}

	void CollisionDebugDrawer::setDebugMode(int debug_mode)
	{
		this->debug_mode = debug_mode;
	}

	int CollisionDebugDrawer::getDebugMode(void) const
	{
		return this->debug_mode;
	}

	std::vector<BulletDebugDrawData>& CollisionDebugDrawer::getVerts()
	{
		return this->verts;
	}

	void CollisionDebugDrawer::setShaderProgram(Shader* s)
	{
		this->shaderProgram = s;
	}

	Shader* CollisionDebugDrawer::getShaderProgram()
	{
		return this->shaderProgram;
	}

}