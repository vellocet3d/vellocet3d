#include <iostream>

#include "glad/glad.h"

#include "vel/scene/CollisionDebugDrawer.h"



namespace vel::scene
{
	CollisionDebugDrawer::CollisionDebugDrawer() 
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
	};

	CollisionDebugDrawer::~CollisionDebugDrawer() 
	{
		//std::cout << "debugDrawerDestructor\n";
	};

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

	void CollisionDebugDrawer::draw() 
	{
		if (this->verts.size() > 0)
		{
			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);

			glBufferData(GL_ARRAY_BUFFER, this->verts.size() * sizeof(BulletDebugDrawData), &this->verts[0], GL_STATIC_DRAW);

			// Assign vertex positions to location = 0
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BulletDebugDrawData), (void*)0);

			// Assign vertex color to location = 1
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(BulletDebugDrawData), (void*)offsetof(BulletDebugDrawData, color));

			//glDrawArrays(GL_LINES, 0, (GLsizei)this->verts.size() / 3);
			glDrawArrays(GL_LINES, 0, (GLsizei)this->verts.size());

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
			this->verts.clear();
		}
		
	}

}