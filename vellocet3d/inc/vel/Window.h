#pragma once

#include <string>
#include <chrono>

#include "glm/glm.hpp"

#include "vel/InputState.h"


struct GLFWwindow;

namespace vel
{

    class Window
    {
       
    private:
        glm::ivec2			screenSize;
        bool				fullScreen = false;
        InputState			inputState;
        GLFWwindow*			glfwWindow;
        double				scrollX = 0.0;
        double				scrollY = 0.0;
        void				setKeys();
        void				setMouse();
        void				setScroll();
        void				setCallbacks();


    public:
							Window(Window&&) = default;
							Window(int screenWidth, int screenHeight, bool fullScreen);
							~Window();
        void				setTitle(std::string title);
        bool				shouldClose();
        void				setToClose();
        void				update();
        const InputState&	getInputState() const;
        const glm::ivec2&	getScreenSize() const;
        void				swapBuffers();
        //void				vsync();

    };
    
    
};
