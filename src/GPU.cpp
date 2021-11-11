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
        activeHdr(nullptr),
		activeMesh(nullptr),
		activeMaterial(nullptr),
        equirectangularToCubemapShader(nullptr),
        irradianceShader(nullptr),
        prefilterShader(nullptr),
        brdfShader(nullptr),
        backgroundShader(nullptr)
	{
        this->enableDepthTest();
        this->enableCubeMapTextures();       
        
        this->initQuad();
        this->initCube();

        glGenFramebuffers(1, &this->pbrCaptureFBO);
        glGenRenderbuffers(1, &this->pbrCaptureRBO);
	}

	GPU::~GPU(){}

	void GPU::resetActives()
	{
		this->activeShader = nullptr;
        this->activeHdr = nullptr;
		this->activeMesh = nullptr;
		this->activeMaterial = nullptr;
	}

    void GPU::initPbrShaders(Shader* equi, Shader* irr, Shader* pre, Shader* brdf, Shader* back)
    {
        this->equirectangularToCubemapShader = equi;
        this->irradianceShader = irr;
        this->prefilterShader = pre;
        this->brdfShader = brdf;
        this->backgroundShader = back;
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
		glDeleteTextures(1, &t->id);
	}
    
    void GPU::clearHdr(HDR* h)
    {
        glDeleteTextures(1, &h->hdrTexture);
        glDeleteTextures(1, &h->envCubemap);
        glDeleteTextures(1, &h->irradianceMap);
        glDeleteTextures(1, &h->prefilterMap);
        glDeleteTextures(1, &h->brdfLUTTexture);
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
			std::cout << s->vertFile << std::endl << s->fragFile << std::endl;
			
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

		// Assign vertex bone ids to location = 3 (and 4 for second array element)
		glEnableVertexAttribArray(3);
		glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, weights.ids));
		glEnableVertexAttribArray(4);
		glVertexAttribIPointer(4, 4, GL_INT, sizeof(Vertex), (void*)(offsetof(Vertex, weights.ids) + 16));

		// Assign vertex weights to location = 5 (and 6 for second array element)
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights.weights));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, weights.weights) + 16));

		// Unbind the vertex array to prevent accidental operations
		glBindVertexArray(0);

		m->setGpuMesh(gm);
	}

	void GPU::loadTexture(Texture* t)
	{
		glGenTextures(1, &t->id);
		glBindTexture(GL_TEXTURE_2D, t->id);
		glTexImage2D(
			GL_TEXTURE_2D, 
			0, 
			t->primaryImageData.format, 
			t->primaryImageData.width, 
			t->primaryImageData.height, 
			0, 
			t->primaryImageData.format, 
			GL_UNSIGNED_BYTE,
			t->primaryImageData.data
		);

		if (t->mips.size() == 0)
		{
#ifdef DEBUG_LOG
	Log::toCliAndFile("Generating mipmaps");
#endif
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		if (t->mips.size() > 0)
		{
#ifdef DEBUG_LOG
	Log::toCliAndFile("Loading pre-computed mipmaps");
#endif

			int mipcount = (int)t->mips.size();

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipcount);

			while (mipcount > 0)
			{
				glTexImage2D(
					GL_TEXTURE_2D, 
					mipcount, 
					t->mips.at((mipcount - 1)).format, 
					t->mips.at((mipcount - 1)).width, 
					t->mips.at((mipcount - 1)).height, 
					0, 
					t->mips.at((mipcount - 1)).format, 
					GL_UNSIGNED_BYTE, 
					t->mips.at((mipcount - 1)).data
				);

				stbi_image_free(t->mips.at((mipcount - 1)).data);
				mipcount--;
			}
		}
			
		stbi_image_free(t->primaryImageData.data);
	}

    void GPU::loadHdr(HDR* h)
    {		
        // pbr: reset framebuffers
        // ------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, this->pbrCaptureFBO);
        glBindRenderbuffer(GL_RENDERBUFFER, this->pbrCaptureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->pbrCaptureRBO);
		
        
        // pbr: load the HDR environment map
        // ---------------------------------
        glGenTextures(1, &h->hdrTexture);
        glBindTexture(GL_TEXTURE_2D, h->hdrTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, h->primaryImageData.width, h->primaryImageData.height, 0, GL_RGB, GL_FLOAT, h->primaryImageData.dataf); // note how we specify the texture's data value to be float

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(h->primaryImageData.dataf);
        
        
        // pbr: setup cubemap to render to and attach to framebuffer
        // ---------------------------------------------------------
        glGenTextures(1, &h->envCubemap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, h->envCubemap);
        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable pre-filter mipmap sampling (combatting visible dots artifact)
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
     
     
        // pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
        // ----------------------------------------------------------------------------------------------
        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 captureViews[] =
        {
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };
        
        
        // pbr: convert HDR equirectangular environment map to cubemap equivalent
        // ----------------------------------------------------------------------
		this->useShader(this->equirectangularToCubemapShader);
		this->setShaderInt("equirectangularMap", 0);
		this->setShaderMat4("projection", captureProjection);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, h->hdrTexture);

        glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
        glBindFramebuffer(GL_FRAMEBUFFER, this->pbrCaptureFBO);
        for (unsigned int i = 0; i < 6; ++i)
        {
			this->setShaderMat4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, h->envCubemap, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            this->drawCube();
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // then let OpenGL generate mipmaps from first mip face (combatting visible dots artifact)
        glBindTexture(GL_TEXTURE_CUBE_MAP, h->envCubemap);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        
        
        // pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance scale.
        // --------------------------------------------------------------------------------
        glGenTextures(1, &h->irradianceMap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, h->irradianceMap);
        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindFramebuffer(GL_FRAMEBUFFER, this->pbrCaptureFBO);
        glBindRenderbuffer(GL_RENDERBUFFER, this->pbrCaptureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
        
        
        // pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
        // -----------------------------------------------------------------------------
		this->useShader(this->irradianceShader);
		this->setShaderInt("environmentMap", 0);
		this->setShaderMat4("projection", captureProjection);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, h->envCubemap);

        glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
        glBindFramebuffer(GL_FRAMEBUFFER, this->pbrCaptureFBO);
        for (unsigned int i = 0; i < 6; ++i)
        {
			this->setShaderMat4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, h->irradianceMap, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            this->drawCube();
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        
        // pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
        // --------------------------------------------------------------------------------
        glGenTextures(1, &h->prefilterMap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, h->prefilterMap);
        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification filter to mip_linear 
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        
        
        // pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
        // ----------------------------------------------------------------------------------------------------
		this->useShader(this->prefilterShader);
		this->setShaderInt("environmentMap", 0);
		this->setShaderMat4("projection", captureProjection);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, h->envCubemap);

        glBindFramebuffer(GL_FRAMEBUFFER, this->pbrCaptureFBO);
        unsigned int maxMipLevels = 5;
        for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
        {
            // resize framebuffer according to mip-level size.
            unsigned int mipWidth = 128 * std::pow(0.5, mip);
            unsigned int mipHeight = 128 * std::pow(0.5, mip);
            glBindRenderbuffer(GL_RENDERBUFFER, this->pbrCaptureRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
            glViewport(0, 0, mipWidth, mipHeight);

            float roughness = (float)mip / (float)(maxMipLevels - 1);
			this->setShaderFloat("roughness", roughness);
            for (unsigned int i = 0; i < 6; ++i)
            {
				this->setShaderMat4("view", captureViews[i]);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, h->prefilterMap, mip);

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                this->drawCube();
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        
        // pbr: generate a 2D LUT from the BRDF equations used.
        // ----------------------------------------------------
        glGenTextures(1, &h->brdfLUTTexture);

        // pre-allocate enough memory for the LUT texture.
        glBindTexture(GL_TEXTURE_2D, h->brdfLUTTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
        // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
        glBindFramebuffer(GL_FRAMEBUFFER, this->pbrCaptureFBO);
        glBindRenderbuffer(GL_RENDERBUFFER, this->pbrCaptureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, h->brdfLUTTexture, 0);

        glViewport(0, 0, 512, 512);
		this->useShader(this->brdfShader);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        this->drawQuad();


        // re-bind OG framebuffer and reset viewport dimensions to screen dimensions
        // ---------------------------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, this->window->getScreenSize().x, this->window->getScreenSize().y);
    }

	const Shader* const GPU::getActiveShader() const
	{
		return this->activeShader;
	}

    const HDR* const GPU::getActiveHdr() const
	{
		return this->activeHdr;
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

    void GPU::useHdr(HDR* h)
    {
        this->activeHdr = h;
        
        // bind pre-computed IBL data
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, h->irradianceMap);
        this->setShaderInt("irradianceMap", 0);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, h->prefilterMap);
        this->setShaderInt("prefilterMap", 1);
        
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, h->brdfLUTTexture);
        this->setShaderInt("brdfLUT", 2);
    }

	void GPU::useMaterial(Material* m)
	{
		this->activeMaterial = m;

		this->setShaderVec4("color", m->color);

		//std::cout << glm::to_string(m->color) << std::endl;

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m->albedo->id);
		this->setShaderInt("albedoMap", 3);
        
        glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, m->normal->id);
		this->setShaderInt("normalMap", 4);
        
        glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, m->metallic->id);
		this->setShaderInt("metallicMap", 5);
        
        glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, m->roughness->id);
		this->setShaderInt("roughnessMap", 6);
        
        glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, m->ao->id);
		this->setShaderInt("aoMap", 7);
	}

	void GPU::drawGpuMesh()
	{
		// The naive approach. But after researching how to optimize this for 3 days I decided to leave it alone
		// until there's an actual reason to complicate things.
		glDrawElements(GL_TRIANGLES, this->activeMesh->getGpuMesh()->indiceCount, GL_UNSIGNED_INT, 0);
	}

    void GPU::enableCubeMapTextures()
    {
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
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
    
    void GPU::drawSkybox(glm::mat4 projectionMatrix, glm::mat4 viewMatrix, unsigned int cm)
    {
		this->useShader(this->backgroundShader);
		this->setShaderInt("environmentMap", 0);
		this->setShaderMat4("projection", projectionMatrix);
		this->setShaderMat4("view", viewMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cm);
        this->drawCube();
    }
    
    void GPU::initQuad()
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &this->quadVAO);
        glGenBuffers(1, &this->quadVBO);
        glBindVertexArray(this->quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, this->quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    
    void GPU::drawQuad()
    {
        glBindVertexArray(this->quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }
    
    void GPU::initCube()
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &this->cubeVAO);
        glGenBuffers(1, &this->cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER,this->cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    
    void GPU::drawCube()
    {
        glBindVertexArray(this->cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }
}