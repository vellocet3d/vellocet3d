#pragma once

#include <string>
#include <chrono>
#include <map>

#include "glm/glm.hpp"


#include "vel/InputState.h"
#include "vel/Config.h"


struct GLFWwindow;
struct GLFWusercontext;
struct ImFont;

namespace vel
{

    class Window
    {
       
    private:
        glm::ivec2			screenSize;
        bool				fullScreen;
		bool				cursorHidden;
		bool				useImGui;
        InputState			inputState;
        GLFWwindow*			glfwWindow;
        double				scrollX = 0.0;
        double				scrollY = 0.0;
        void				setKeys();
        void				setMouse();
        void				setScroll();
        void				setCallbacks();

		GLFWusercontext*	openGLContext1;
		GLFWusercontext*	openGLContext2;

		size_t				nextFreeContext;

		std::map<std::string, ImFont*> imguiFonts;

    public:
							Window(Window&&) = default;
							Window(Config c);
							~Window();
        void				setTitle(std::string title);
        bool				shouldClose();
        void				setToClose();
        void				update();
        const InputState&	getInputState() const;
        const glm::ivec2&	getScreenSize() const;
        void				swapBuffers();
		void				renderGui();
        //void				vsync();
		GLFWusercontext*	getNextFreeOpenGLContext();
		void				setOpenGLContext(GLFWusercontext* c);
		ImFont*				getImguiFont(std::string key);

    };
    
    
};
