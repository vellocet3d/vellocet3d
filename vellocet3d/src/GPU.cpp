#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "STB_IMAGE/stb_image.h"

#include "vel/GPU.h"
#include "vel/scene/mesh/Vertex.h"
#include "vel/helpers/functions.h"

namespace vel
{
    GPU::GPU() :
        activeShaderIndex(-1),
        activeGpuMeshIndex(-1),
        activeMaterialIndex(-1)
    {
		// create collision debug drawer
		this->collisionDebugDrawer = CollisionDebugDrawer();
	
	}

	GPU::~GPU()
	{
		this->wipe();
	}

	const Texture&	GPU::getTexture(size_t i) const
	{
		return this->textures.at(i);
	}

	CollisionDebugDrawer* GPU::getCollisionDebugDrawer()
	{
		//return this->collisionDebugDrawer.get();
		if (this->collisionDebugDrawer)
			return &this->collisionDebugDrawer.value();
		else
			return nullptr;
	}

    size_t GPU::loadShader(const std::string name, const std::string vertFile, const std::string fragFile)
    {
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
            // open files
            vShaderFile.open(vertFile);
            fShaderFile.open(fragFile);

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

        auto shader = Shader();
        shader.name = name;
        shader.id = id;

        this->shaders.push_back(shader);

        return this->shaders.size() - 1;
    }

	size_t GPU::loadMesh(Mesh& m)
    {
        GpuMesh gm = GpuMesh();
        gm.indiceCount = (GLsizei)m.getIndices().size();

        // Generate and bind vertex attribute array
        glGenVertexArrays(1, &gm.VAO);
        glBindVertexArray(gm.VAO);

        // Generate and bind vertex buffer object
        glGenBuffers(1, &gm.VBO);
        glBindBuffer(GL_ARRAY_BUFFER, gm.VBO);
        glBufferData(GL_ARRAY_BUFFER, m.getVertices().size() * sizeof(Vertex), &m.getVertices()[0], GL_STATIC_DRAW);

        // Generate and bind element buffer object
        glGenBuffers(1, &gm.EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gm.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m.getIndices().size() * sizeof(unsigned int), &m.getIndices()[0], GL_STATIC_DRAW);

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

        this->gpuMeshes.push_back(gm);

        return this->gpuMeshes.size() - 1;
    }

	size_t GPU::loadTexture(Texture texture)
    {
        glGenTextures(1, &texture.id);

        int width, height, nrComponents;
        unsigned char*	data = stbi_load(texture.path.c_str(), &width, &height, &nrComponents, 0);
        if (data) 
        {
            GLenum format;
			if (nrComponents == 1)
			{
				texture.alphaChannel = false;
				format = GL_RED;
			} 
			else if (nrComponents == 3)
			{
				texture.alphaChannel = false;
				format = GL_RGB;
			}
			else if (nrComponents == 4)
			{
				texture.alphaChannel = true;
				format = GL_RGBA;
			}
                
			//std::cout << texture.filename << ":" << texture.alphaChannel << "\n";

            glBindTexture(GL_TEXTURE_2D, texture.id);

			// load the base level texture
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

			if (texture.mips.size() == 0)
				glGenerateMipmap(GL_TEXTURE_2D);

			// set texture parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			if (texture.mips.size() == 0)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			}
			else
			{
				int mipcount = texture.mips.size();

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipcount);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

				while (mipcount > 0)
				{
					int mip_width, mip_height, mip_nrComponents;
					unsigned char*	mip_data = stbi_load(texture.mips.at(mipcount).c_str(), &mip_width, &mip_height, &mip_nrComponents, 0);

					if (mip_data)
					{
						GLenum mip_format;
						if (mip_nrComponents == 1)
							mip_format = GL_RED;
						else if (mip_nrComponents == 3)
							mip_format = GL_RGB;
						else if (mip_nrComponents == 4)
							mip_format = GL_RGBA;

						//std::cout << full_mip_file_path << ":" << mip_nrComponents << "\n";

						glTexImage2D(GL_TEXTURE_2D, mipcount, mip_format, mip_width, mip_height, 0, mip_format, GL_UNSIGNED_BYTE, mip_data);

						stbi_image_free(mip_data);

						mipcount--;
					}
					else
					{
						stbi_image_free(mip_data);
						std::cout << "Texture failed to load at path: " << texture.mips.at(mipcount) << "\n";
						std::cin.get();
						exit(EXIT_FAILURE);
					}
				}
				
			}

            stbi_image_free(data);

            this->textures.push_back(texture);

            return this->textures.size() - 1;
        }
        else
        {
            stbi_image_free(data);
            std::cout << "Texture failed to load at path: " << texture.path << "\n";
            std::cin.get();
            exit(EXIT_FAILURE);
        }
    }

    const size_t GPU::getActiveShaderIndex() const
    {
        return this->activeShaderIndex;
    }

    const size_t GPU::getActiveGpuMeshIndex() const
    {
        return this->activeGpuMeshIndex;
    }

    const size_t GPU::getActiveMaterialIndex() const
    {
        return this->activeMaterialIndex;
    }

    const std::vector<Texture>& GPU::getTextures() const
    {
        return this->textures;
    }

	std::vector<std::string> GPU::getActiveShaderNames()
	{
		std::vector<std::string> names;
		for (auto& s : this->shaders)
			names.push_back(s.name);

		return names;
	}

    void GPU::useShader(size_t shaderIndex)
    {
        this->activeShaderIndex = shaderIndex;
        glUseProgram(this->shaders.at(shaderIndex).id);
    }

    void GPU::setShaderBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(
            this->shaders.at(this->activeShaderIndex).id, 
            name.c_str()), 
            (int)value);
    }

    void GPU::setShaderInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(
            this->shaders.at(this->activeShaderIndex).id, 
            name.c_str()), 
            value);
    }

    void GPU::setShaderFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(
            this->shaders.at(this->activeShaderIndex).id, 
            name.c_str()), 
            value);
    }

    void GPU::setShaderMat4(const std::string& name, glm::mat4 value) const
    {
        glUniformMatrix4fv(glGetUniformLocation(
            this->shaders.at(this->activeShaderIndex).id, 
            name.c_str()), 
            1,
            GL_FALSE, glm::value_ptr(value));
    }


    void GPU::useGpuMesh(size_t gpuMeshIndex)
    {
        this->activeGpuMeshIndex = gpuMeshIndex;
        glBindVertexArray(this->gpuMeshes.at(gpuMeshIndex).VAO);
    }

    void GPU::useMaterial(size_t materialIndex)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->textures.at(textureIndex).id);
        this->setShaderInt("texture1", 0);

        this->activeMaterialIndex = textureIndex;
    }

    void GPU::drawGpuMesh()
    {
        // The naive approach. But after researching how to optimize this for 3 days I decided to leave it alone
        // until there's an actual reason to (DRASTICALLY) complicate things.
        glDrawElements(GL_TRIANGLES, this->gpuMeshes.at(this->activeGpuMeshIndex).indiceCount, GL_UNSIGNED_INT, 0);
    }

    void GPU::enableDepthTest()
    {
        glEnable(GL_DEPTH_TEST);
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

    void GPU::wipe()
    {
		//std::cout << "wiping gpu data for previous GPU instance\n";

        for (auto& mr : this->gpuMeshes)
        {
            glDeleteVertexArrays(1, &mr.VAO);
            glDeleteBuffers(1, &mr.VBO);
            glDeleteBuffers(1, &mr.EBO);
        }

        for (auto& t : this->textures)
			glDeleteTextures(1, &t.id);

        for (auto& s : this->shaders)
			glDeleteProgram(s.id);
    }
		
	void GPU::finish()
	{
		glFinish();
	}
	
	void GPU::enableBlend()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

}