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

		std::map<std::string, ImFont*> imguiFonts;
		bool				imguiFrameOpen;

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


		ImFont* 			getImguiFont(std::string key) const;
		void				hideMouseCursor();
		void				showMouseCursor();
		bool				getImguiFrameOpen();

    };
    
    
};
