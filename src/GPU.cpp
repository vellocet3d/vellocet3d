#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <glm/gtx/string_cast.hpp>
#include "stb_image/stb_image.h"

#include "vel/GPU.h"
#include "vel/Vertex.h"
#include "vel/functions.h"
#include "vel/App.h"
#include "vel/Log.h"

namespace vel
{
	GPU::GPU(Window* w) :
		window(w),
		activeShader(nullptr),
		activeMesh(nullptr),
		activeMaterial(nullptr)
	{
        this->enableDepthTest();
		this->enableBackfaceCulling();
		this->initBoneUBO();
		this->initTextureUBO();

	}

	GPU::~GPU(){}

	void GPU::enableBackfaceCulling()
	{
		glEnable(GL_CULL_FACE);
	}

	void GPU::disableBackfaceCulling()
	{
		glDisable(GL_CULL_FACE);
	}

	void GPU::resetActives()
	{
		this->activeShader = nullptr;
		this->activeMesh = nullptr;
		this->activeMaterial = nullptr;
	}

	void GPU::initTextureUBO()
	{
		
		const int MAX_SUPPORTED_TEXTURES = 1000;
		glGenBuffers(1, &this->texturesUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, this->texturesUBO);
		//glBufferData(GL_UNIFORM_BUFFER, MAX_SUPPORTED_TEXTURES * sizeof(GLuint64), NULL, GL_STATIC_DRAW);
		glBufferData(GL_UNIFORM_BUFFER, MAX_SUPPORTED_TEXTURES * sizeof(GLuint64) * 2, NULL, GL_STATIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, this->texturesUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		//std::cout << "textureUBO: " << this->texturesUBO << std::endl;
	}

	void GPU::updateTextureUBO(unsigned int index, GLuint64 dsaHandle)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, this->texturesUBO);

		//glBufferSubData(GL_UNIFORM_BUFFER, sizeof(GLuint64) * index, sizeof(GLuint64), (void*)&dsaHandle);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(GLuint64) * index * 2, sizeof(GLuint64) * 2, (void*)&dsaHandle);

		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}



	void GPU::initBoneUBO()
	{
		const int MAX_SUPPORTED_BONES = 200;
		glGenBuffers(1, &this->bonesUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, this->bonesUBO);
		glBufferData(GL_UNIFORM_BUFFER, MAX_SUPPORTED_BONES * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, this->bonesUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		//std::cout << "bonesUBO: " << this->bonesUBO << std::endl;
	}

	void GPU::updateBonesUBO(std::vector<std::pair<unsigned int, glm::mat4>> boneData)
	{
		
		glBindBuffer(GL_UNIFORM_BUFFER, this->bonesUBO);

		for (auto& bd : boneData)
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * bd.first, sizeof(glm::mat4), glm::value_ptr(bd.second));

		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void GPU::clearShader(Shader* s)
	{
		glDeleteProgram(s->id);
	}

	void GPU::clearMesh(Mesh* m)
	{
		if (m->getGpuMesh())
		{
			glDeleteVertexArrays(1, &m->getGpuMesh().value().VAO);
			glDeleteBuffers(1, &m->getGpuMesh().value().VBO);
			glDeleteBuffers(1, &m->getGpuMesh().value().EBO);
		}
	}

	void GPU::clearTexture(Texture* t)
	{
		glMakeTextureHandleNonResidentARB(t->dsaHandle);
		glDeleteTextures(1, &t->id);
	}

	void GPU::clearRenderTarget(RenderTarget* rt)
	{
		glMakeTextureHandleNonResidentARB(rt->FBOTextureDSAHandle);
		glDeleteTextures(1, &rt->FBOTexture);
		glDeleteRenderbuffers(1, &rt->RBO);
		glDeleteFramebuffers(1, &rt->FBO);
	}

	void GPU::loadShader(Shader* s)
	{
		//TODO: Totally missed moving this file read out of this class and into assetmanager
		
		unsigned int id;

		// 1. retrieve the vertex/fragment source code from filePath
		std::string vertexCode;
		std::string fragmentCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;

		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try
		{
			//std::cout << s->vertFile << std::endl << s->fragFile << std::endl;
			
			// open files
			vShaderFile.open(s->vertFile);
			fShaderFile.open(s->fragFile);

			std::stringstream vShaderStream, fShaderStream;

			// read file's buffer contents into streams
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();

			// close file handlers
			vShaderFile.close();
			fShaderFile.close();

			// convert stream into string
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();

		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ\n";
			std::cin.get();
			exit(EXIT_FAILURE);
		}

		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragmentCode.c_str();

		// 2. compile shaders
		int success;
		char infoLog[512];


		// vertex Shader
		unsigned int vertex;
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);

		// if compile errors, log and exit
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << "\n";
			std::cin.get();
			exit(EXIT_FAILURE);
		};


		// fragment Shader
		unsigned int fragment;
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);

		// if compile errors, log and exit
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << "\n";
			std::cin.get();
			exit(EXIT_FAILURE);
		};


		// shader Program
		id = glCreateProgram();
		glAttachShader(id, vertex);
		glAttachShader(id, fragment);
		glLinkProgram(id);

		// if linking errors, log and exit
		glGetProgramiv(id, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(id, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << "\n";
			std::cin.get();
			exit(EXIT_FAILURE);
		}

		// delete the shaders as they're linked into our program now and no longer necessary
		glDeleteShader(vertex);
		glDeleteShader(fragment);


		s->id = id;
	}

	RenderTarget GPU::createRenderTarget(unsigned int width, unsigned int height)
	{
		RenderTarget rt;
		rt.resolution = glm::ivec2(width, height);
		glGenFramebuffers(1, &rt.FBO);
		glGenTextures(1, &rt.FBOTexture);
		glGenRenderbuffers(1, &rt.RBO);

		// obtain texture's DSA handle
		rt.FBOTextureDSAHandle = glGetTextureHandleARB(rt.FBOTexture);

		// set texture's DSA handle as resident so it can be accessed in shaders
		glMakeTextureHandleResidentARB(rt.FBOTextureDSAHandle);

		this->updateRenderTarget(&rt);

		return rt;
	}

	void GPU::updateRenderTarget(RenderTarget* rt)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, rt->FBO);

		glBindTexture(GL_TEXTURE_2D, rt->FBOTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, rt->resolution.x, rt->resolution.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rt->FBOTexture, 0);

		glBindRenderbuffer(GL_RENDERBUFFER, rt->RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, rt->resolution.x, rt->resolution.y);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rt->RBO);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
			std::cin.get();
			exit(EXIT_FAILURE);
		}
			
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void GPU::loadMesh(Mesh* m)
	{
		GpuMesh gm = GpuMesh();
		gm.indiceCount = (GLsizei)m->getIndices().size();

		// Generate and bind vertex attribute array
		glGenVertexArrays(1, &gm.VAO);
		glBindVertexArray(gm.VAO);

		// Generate and bind vertex buffer object
		glGenBuffers(1, &gm.VBO);
		glBindBuffer(GL_ARRAY_BUFFER, gm.VBO);
		glBufferData(GL_ARRAY_BUFFER, m->getVertices().size() * sizeof(Vertex), &m->getVertices()[0], GL_STATIC_DRAW);

		// Generate and bind element buffer object
		glGenBuffers(1, &gm.EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gm.EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->getIndices().size() * sizeof(unsigned int), &m->getIndices()[0], GL_STATIC_DRAW);

		// Assign vertex positions to location = 0
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

		// Assign vertex normals to location = 1
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

		// Assign vertex texture coordinates to location = 2
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, textureCoordinates));

		// Assign texture id to location = 3
		glEnableVertexAttribArray(3);
		glVertexAttribIPointer(3, 1, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, textureId));

		// Assign vertex bone ids to location = 4 (and 5 for second array element)
		glEnableVertexAttribArray(4);
		glVertexAttribIPointer(4, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, weights.ids));
		glEnableVertexAttribArray(5);
		glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)(offsetof(Vertex, weights.ids) + 16));

		// Assign vertex weights to location = 6 (and 7 for second array element)
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights.weights));
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, weights.weights) + 16));

		// Unbind the vertex array to prevent accidental operations
		glBindVertexArray(0);

		m->setGpuMesh(gm);
	}

	void GPU::loadTexture(Texture* t)
	{
		//// create a texture buffer and bind it to context
		glGenTextures(1, &t->id);
		glBindTexture(GL_TEXTURE_2D, t->id);

		////// define properties of this texture
		//glTexStorage2D(
		//	GL_TEXTURE_2D,
		//	6,
		//	//t->primaryImageData.format,
		//	t->primaryImageData.sizedFormat,
		//	t->primaryImageData.width,
		//	t->primaryImageData.height
		//);

		////// fill texture with data
		//glTexSubImage2D(
		//	GL_TEXTURE_2D,
		//	0,
		//	0,
		//	0,
		//	t->primaryImageData.width,
		//	t->primaryImageData.height,
		//	t->primaryImageData.format,
		//	GL_UNSIGNED_BYTE,
		//	t->primaryImageData.data
		//);

		glTexImage2D(
			GL_TEXTURE_2D, 
			0, 
			t->primaryImageData.sizedFormat, 
			t->primaryImageData.width, 
			t->primaryImageData.height, 
			0, 
			t->primaryImageData.format, 
			GL_UNSIGNED_BYTE,
			t->primaryImageData.data
		);

		//// auto generate mipmap levels for texture
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		// obtain texture's DSA handle
		t->dsaHandle = glGetTextureHandleARB(t->id);

		// set texture's DSA handle as resident so it can be accessed in shaders
		glMakeTextureHandleResidentARB(t->dsaHandle);

		// send handle to texturesUBO inserting this handle at this texture's dsaIdIndex
		//this->updateTextureUBO(t->dsaIdIndex, handle);




			
		stbi_image_free(t->primaryImageData.data);
	}

	const Shader* const GPU::getActiveShader() const
	{
		return this->activeShader;
	}

	const Mesh* const GPU::getActiveMesh() const
	{
		return this->activeMesh;
	}

	const Material* const GPU::getActiveMaterial() const
	{
		return this->activeMaterial;
	}

	void GPU::useShader(Shader* s)
	{
		this->activeShader = s;
		glUseProgram(s->id);
	}

	void GPU::setShaderBool(const std::string& name, bool value) const
	{
		if (!this->activeShader->uniformLocations.count(name) == 1)
			this->activeShader->uniformLocations[name] = glGetUniformLocation(this->activeShader->id, name.c_str());

		glUniform1i(this->activeShader->uniformLocations[name], (int)value);
	}

	void GPU::setShaderInt(const std::string& name, int value) const
	{
		if (!this->activeShader->uniformLocations.count(name) == 1)
			this->activeShader->uniformLocations[name] = glGetUniformLocation(this->activeShader->id, name.c_str());

		glUniform1i(this->activeShader->uniformLocations[name], value);
	}

	void GPU::setShaderFloat(const std::string& name, float value) const
	{
		if (!this->activeShader->uniformLocations.count(name) == 1)
			this->activeShader->uniformLocations[name] = glGetUniformLocation(this->activeShader->id, name.c_str());

		glUniform1f(this->activeShader->uniformLocations[name], value);
	}

	void GPU::setShaderMat4(const std::string& name, glm::mat4 value) const
	{
		if (!this->activeShader->uniformLocations.count(name) == 1)
			this->activeShader->uniformLocations[name] = glGetUniformLocation(this->activeShader->id, name.c_str());

		glUniformMatrix4fv(this->activeShader->uniformLocations[name], 1, GL_FALSE, glm::value_ptr(value));
	}

	void GPU::setShaderVec3(const std::string &name, glm::vec3 value) const
	{
		if (!this->activeShader->uniformLocations.count(name) == 1)
			this->activeShader->uniformLocations[name] = glGetUniformLocation(this->activeShader->id, name.c_str());

		glUniform3fv(this->activeShader->uniformLocations[name], 1, &value[0]);
	}

	void GPU::setShaderVec4(const std::string &name, glm::vec4 value) const
	{
		if (!this->activeShader->uniformLocations.count(name) == 1)
			this->activeShader->uniformLocations[name] = glGetUniformLocation(this->activeShader->id, name.c_str());

		glUniform4fv(this->activeShader->uniformLocations[name], 1, &value[0]);
	}


	void GPU::useMesh(Mesh* m)
	{
		this->activeMesh = m;
		glBindVertexArray(m->getGpuMesh()->VAO);
	}

	void GPU::useMaterial(Material* m)
	{
		this->activeMaterial = m;

		this->setShaderVec4("color", m->color);

		for (unsigned int i = 0; i < m->textures.size(); i++)
		{
			this->updateTextureUBO(i, m->textures.at(i)->dsaHandle);
		}
		
	}

	void GPU::drawGpuMesh()
	{
		// The naive approach. But after researching how to optimize this for 3 days I decided to leave it alone
		// until there's an actual reason to complicate things.
		//
		// TODO: Coming back to this years later, we should definitely refactor so that we don't have to swap
		// buffer objects for each different object, just pack all of our vertex data into the same buffer
		// and draw elements of that object individually using glDrawElementsBaseVertex()... I think that's the right
		// method, need to confirm... doing this will greatly reduce the number of state switches, especially since
		// we now use DSA textures
		glDrawElements(GL_TRIANGLES, this->activeMesh->getGpuMesh()->indiceCount, GL_UNSIGNED_INT, 0);
	}

	void GPU::enableDepthTest()
	{
		glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
	}
    
    void GPU::enableBlend()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
    
    void GPU::disableBlend()
    {
        glDisable(GL_BLEND);
    }

	void GPU::clearDepthBuffer()
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void GPU::clearBuffers(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void GPU::drawLinesOnly()
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	void GPU::finish()
	{
		glFinish();
	}

	void GPU::debugDrawCollisionWorld(CollisionDebugDrawer* cdd)
	{
		if (cdd->getVerts().size() > 0)
		{
			unsigned int VAO, VBO;
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);

			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);

			glBufferData(GL_ARRAY_BUFFER, cdd->getVerts().size() * sizeof(BulletDebugDrawData), &cdd->getVerts()[0], GL_STATIC_DRAW);

			// Assign vertex positions to location = 0
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BulletDebugDrawData), (void*)0);

			// Assign vertex color to location = 1
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(BulletDebugDrawData), (void*)offsetof(BulletDebugDrawData, color));

			//glDrawArrays(GL_LINES, 0, (GLsizei)this->verts.size() / 3);
			glDrawArrays(GL_LINES, 0, (GLsizei)cdd->getVerts().size());

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
			cdd->getVerts().clear();

			glDeleteVertexArrays(1, &VAO);
			glDeleteBuffers(1, &VBO);
		}
	}

}