#include <iostream>
#include <fstream>
#include <sstream>

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "STB_IMAGE/stb_image.h"

#include "vel/App.h"
#include "vel/scene/GPU.h"
#include "vel/scene/mesh/Vertex.h"

namespace vel::scene
{
    GPU::GPU() :
        shaderDirectory(App::get().config.SHADER_FILE_PATH),
        activeShaderIndex(-1),
        activeMeshRenderableIndex(-1),
        activeTextureIndex(-1),
		collisionDebugDrawer(std::move(std::make_unique<CollisionDebugDrawer>()))
    {
	
		//std::cout << "loading new GPU\n";

		// create default shaders
		this->loadShader("default", App::get().config.DEFAULT_VERTEX_SHADER, App::get().config.DEFAULT_FRAGMENT_SHADER);
		this->loadShader("default_skinned", App::get().config.DEFAULT_SKINNED_VERTEX_SHADER, App::get().config.DEFAULT_SKINNED_FRAGMENT_SHADER);
		this->loadShader("default_debug", App::get().config.DEFAULT_DEBUG_VERTEX_SHADER, App::get().config.DEFAULT_DEBUG_FRAGMENT_SHADER);

		// create default texture
		this->loadTexture("diffuse", ("data/models/default_texture.jpg"));
	
	}

	GPU::~GPU()
	{
		this->wipe();
	}

	CollisionDebugDrawer* GPU::getCollisionDebugDrawer()
	{
		return this->collisionDebugDrawer.get();
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
            vShaderFile.open(this->shaderDirectory + "/" + vertFile);
            fShaderFile.open(this->shaderDirectory + "/" + fragFile);

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

	size_t GPU::loadMesh(mesh::Mesh& m)
    {
        mesh::Renderable mr = mesh::Renderable();
        mr.indiceCount = (GLsizei)m.getIndices().size();

        // Generate and bind vertex attribute array
        glGenVertexArrays(1, &mr.VAO);
        glBindVertexArray(mr.VAO);

        // Generate and bind vertex buffer object
        glGenBuffers(1, &mr.VBO);
        glBindBuffer(GL_ARRAY_BUFFER, mr.VBO);
        glBufferData(GL_ARRAY_BUFFER, m.getVertices().size() * sizeof(mesh::Vertex), &m.getVertices()[0], GL_STATIC_DRAW);

        // Generate and bind element buffer object
        glGenBuffers(1, &mr.EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mr.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m.getIndices().size() * sizeof(unsigned int), &m.getIndices()[0], GL_STATIC_DRAW);

        // Assign vertex positions to location = 0
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(mesh::Vertex), (void*)0);

        // Assign vertex normals to location = 1
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(mesh::Vertex), (void*)offsetof(mesh::Vertex, normal));

        // Assign vertex texture coordinates to location = 2
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(mesh::Vertex), (void*)offsetof(mesh::Vertex, textureCoordinates));

		// Assign vertex bone ids to location = 3
		glEnableVertexAttribArray(3);
		glVertexAttribIPointer(3, 4, GL_INT, sizeof(mesh::Vertex), (void*)offsetof(mesh::Vertex, weights.ids));

		// Assign vertex weights to location = 4
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(mesh::Vertex), (void*)offsetof(mesh::Vertex, weights.weights));

        // Unbind the vertex array to prevent accidental operations
        glBindVertexArray(0);

        this->meshRenderables.push_back(mr);

        return this->meshRenderables.size() - 1;
    }

	size_t GPU::loadTexture(std::string type, std::string path)
    {
		mesh::Texture texture;
        texture.type = "diffuse";
        texture.path = path;

        glGenTextures(1, &texture.id);

        int width, height, nrComponents;
        //stbi_set_flip_vertically_on_load(true);
        unsigned char*	data = stbi_load(texture.path.c_str(), &width, &height, &nrComponents, 0);
        if (data) 
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, texture.id);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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

    const size_t GPU::getActiveMeshRenderableIndex() const
    {
        return this->activeMeshRenderableIndex;
    }

    const size_t GPU::getActiveTextureIndex() const
    {
        return this->activeTextureIndex;
    }

    const std::vector<mesh::Texture>& GPU::getTextures() const
    {
        return this->textures;
    }

	std::vector<std::string> GPU::getActiveShaderNames()
	{
		std::vector<std::string> names;
		for (auto& s : this->shaders)
		{
			names.push_back(s.name);
		}

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


    void GPU::useMeshRenderable(size_t meshRenderableIndex)
    {
        this->activeMeshRenderableIndex = meshRenderableIndex;
        glBindVertexArray(this->meshRenderables.at(meshRenderableIndex).VAO);
    }

    void GPU::useTexture(size_t textureIndex)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->textures.at(textureIndex).id);
        this->setShaderInt("texture1", 0);

        this->activeTextureIndex = textureIndex;
    }

    void GPU::drawMeshRenderable()
    {
        // The naive approach. But after researching how to optimize this for 3 days I decided to leave it alone
        // until there's an actual reason to (DRASTICALLY) complicate things.
        glDrawElements(GL_TRIANGLES, this->meshRenderables.at(this->activeMeshRenderableIndex).indiceCount, GL_UNSIGNED_INT, 0);
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

        for (auto& mr : this->meshRenderables)
        {
            glDeleteVertexArrays(1, &mr.VAO);
            glDeleteBuffers(1, &mr.VBO);
            glDeleteBuffers(1, &mr.EBO);
        }

        for (auto& t : this->textures)
        {
			glDeleteTextures(1, &t.id);
        }

        for (auto& s : this->shaders)
        {
			glDeleteProgram(s.id);
        }
    }
		
	

}