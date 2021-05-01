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
    GPU::GPU(std::string shaderDirectory, std::string dvs, std::string dfs, std::string dsvs, std::string dsfs,
		std::string ddvs, std::string ddfs, bool autoGenerateMipmaps) :
        shaderDirectory(shaderDirectory),
        activeShaderIndex(-1),
        activeMeshRenderableIndex(-1),
        activeTextureIndex(-1),
		autoGenerateMipmaps(autoGenerateMipmaps)
    {
		// create collision debug drawer
		this->collisionDebugDrawer = CollisionDebugDrawer();

		// create default shaders
		this->loadShader("default", dvs, dfs);
		this->loadShader("default_skinned", dsvs, dsfs);
		this->loadShader("default_debug", ddvs, ddfs);

		// create default texture
		this->loadTexture("diffuse", "data/models", "default_texture.jpg");
	
	}

	GPU::~GPU()
	{
		this->wipe();
	}

	const scene::mesh::Texture&	GPU::getTexture(size_t i) const
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

	size_t GPU::loadMesh(scene::mesh::Mesh& m)
    {
        scene::mesh::Renderable mr = scene::mesh::Renderable();
        mr.indiceCount = (GLsizei)m.getIndices().size();

        // Generate and bind vertex attribute array
        glGenVertexArrays(1, &mr.VAO);
        glBindVertexArray(mr.VAO);

        // Generate and bind vertex buffer object
        glGenBuffers(1, &mr.VBO);
        glBindBuffer(GL_ARRAY_BUFFER, mr.VBO);
        glBufferData(GL_ARRAY_BUFFER, m.getVertices().size() * sizeof(scene::mesh::Vertex), &m.getVertices()[0], GL_STATIC_DRAW);

        // Generate and bind element buffer object
        glGenBuffers(1, &mr.EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mr.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m.getIndices().size() * sizeof(unsigned int), &m.getIndices()[0], GL_STATIC_DRAW);

        // Assign vertex positions to location = 0
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(scene::mesh::Vertex), (void*)0);

        // Assign vertex normals to location = 1
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(scene::mesh::Vertex), (void*)offsetof(scene::mesh::Vertex, normal));

        // Assign vertex texture coordinates to location = 2
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(scene::mesh::Vertex), (void*)offsetof(scene::mesh::Vertex, textureCoordinates));

		// Assign vertex bone ids to location = 3 (and 4 for second array element)
		glEnableVertexAttribArray(3);
		glVertexAttribIPointer(3, 4, GL_INT, sizeof(scene::mesh::Vertex), (void*)offsetof(scene::mesh::Vertex, weights.ids));
		glEnableVertexAttribArray(4);
		glVertexAttribIPointer(4, 4, GL_INT, sizeof(scene::mesh::Vertex), (void*)(offsetof(scene::mesh::Vertex, weights.ids) + 16));

		// Assign vertex weights to location = 5 (and 6 for second array element)
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(scene::mesh::Vertex), (void*)offsetof(scene::mesh::Vertex, weights.weights));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(scene::mesh::Vertex), (void*)(offsetof(scene::mesh::Vertex, weights.weights) + 16));

        // Unbind the vertex array to prevent accidental operations
        glBindVertexArray(0);

        this->meshRenderables.push_back(mr);

        return this->meshRenderables.size() - 1;
    }

	size_t GPU::loadTexture(std::string type, std::string dir, std::string filename)
    {
		auto fileExploded = vel::helpers::functions::explode_string(filename, '.');

		scene::mesh::Texture texture;
        texture.type = "diffuse";
        texture.path = dir;
		texture.fullPath = dir + "/" + filename;
		texture.filename = fileExploded[0];
		texture.fileExt = "." + fileExploded[1];

		//std::cout << texture.path << "\n";
		//std::cout << texture.fullPath << "\n";
		//std::cout << texture.filename << "\n";
		//std::cout << texture.fileExt << "\n";
		//std::cout << "--------------\n";

        glGenTextures(1, &texture.id);


        int width, height, nrComponents;
        unsigned char*	data = stbi_load(texture.fullPath.c_str(), &width, &height, &nrComponents, 0);
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

			if (this->autoGenerateMipmaps)
				glGenerateMipmap(GL_TEXTURE_2D);

			// set texture parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			if (this->autoGenerateMipmaps)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			}
			else
			{
				auto mip_dir_path = texture.path + "/" + texture.filename + "_mipmaps";
				bool has_mip_dir = std::filesystem::is_directory(mip_dir_path);

				//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

				if (!has_mip_dir)
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				}
				else
				{
					auto dirIter = std::filesystem::directory_iterator(mip_dir_path);

					int mipcount = 0;

					for (auto& entry : dirIter)
					{
						if (entry.is_regular_file())
						{
							mipcount++;
						}
					}

					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipcount);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

					while (mipcount > 0)
					{
						auto full_mip_file_path = mip_dir_path + "/" + std::to_string(mipcount) + texture.fileExt;

						int mip_width, mip_height, mip_nrComponents;
						unsigned char*	mip_data = stbi_load(full_mip_file_path.c_str(), &mip_width, &mip_height, &mip_nrComponents, 0);

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
							std::cout << "Texture failed to load at path: " << full_mip_file_path << "\n";
							std::cin.get();
							exit(EXIT_FAILURE);
						}
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
            std::cout << "Texture failed to load at path: " << texture.fullPath << "\n";
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

    const std::vector<scene::mesh::Texture>& GPU::getTextures() const
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