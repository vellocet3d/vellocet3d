#pragma once

#include <vector>
#include <string>
#include <memory>
#include <optional>

#include "glm/glm.hpp"


#include "vel/Shader.h"
#include "vel/Mesh.h"
#include "vel/Texture.h"
#include "vel/Material.h"
#include "vel/CollisionDebugDrawer.h"
#include "vel/HDR.h"



namespace vel
{
	class Window;

	class GPU
	{
	private:
		Window*								window;

        const std::string                   defaultShaderPath;
		Shader*								activeShader;
        HDR*                                activeHdr;
		Mesh*								activeMesh;
		Material*							activeMaterial;
        
        Shader*                             equirectangularToCubemapShader;
        Shader*                             irradianceShader;
        Shader*                             prefilterShader;
        Shader*                             brdfShader;
        Shader*                             backgroundShader;
        
        unsigned int                        quadVAO;
        unsigned int                        quadVBO;
        void                                initQuad();
        void                                drawQuad();
        
        unsigned int                        cubeVAO;
        unsigned int                        cubeVBO;
        void                                initCube();
        void                                drawCube();
        
        
        unsigned int                        pbrCaptureFBO;
        unsigned int                        pbrCaptureRBO;
        

	public:
		GPU(Window* w);
		~GPU();
		GPU(GPU&&) = default;
        void                                initPbrShaders(Shader* equi, Shader* irr, Shader* pre, Shader* brdf, Shader* back);
        void								enableDepthTest();
		void								clearBuffers(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 0.0f);
		void								drawLinesOnly();

		const Shader* const					getActiveShader() const;
        const HDR* const					getActiveHdr() const;
		const Mesh*	const					getActiveMesh() const;
		const Material*	const				getActiveMaterial() const;
		void								resetActives();

		void								loadShader(Shader* s);
		void								loadMesh(Mesh* m);
		void								loadTexture(Texture* t);
        void                                loadHdr(HDR* h);


		void								useShader(Shader* s);
        void                                useHdr(HDR* h);
		void								useMaterial(Material* m);
		void								useMesh(Mesh* m);

		void								setShaderBool(const std::string &name, bool value) const;
		void								setShaderInt(const std::string &name, int value) const;
		void								setShaderFloat(const std::string &name, float value) const;
		void								setShaderMat4(const std::string &name, glm::mat4 value) const;
		void								setShaderVec3(const std::string &name, glm::vec3 value) const;
		void								setShaderVec4(const std::string &name, glm::vec4 value) const;

		void								drawGpuMesh();
		void								clearDepthBuffer();

		void								finish();
		void								enableBlend();
        void                                disableBlend();
        void                                enableCubeMapTextures();

		void								debugDrawCollisionWorld(CollisionDebugDrawer* cdd);

		void								clearShader(Shader* s);
        void                                clearHdr(HDR* h);
		void								clearMesh(Mesh* m);
		void								clearTexture(Texture* t);

		void                                drawSkybox(glm::mat4 projectionMatrix, glm::mat4 viewMatrix, unsigned int cm);

	};
}