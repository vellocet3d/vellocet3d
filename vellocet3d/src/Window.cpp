#include <iostream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "vel/Window.h"

// stutter caused when not in fullscreen mode: https://stackoverflow.com/a/21663076/1609485
// https://www.reddit.com/r/opengl/comments/8754el/stuttering_with_learnopengl_tutorials/dwbp7ta?utm_source=share&utm_medium=web2x
// could be because of multiple monitors all running different refresh rates

namespace vel
{
    Window::Window(int screenWidth, int screenHeight, bool fullScreen, bool cursorHidden) :
        screenSize(glm::ivec2(screenWidth, screenHeight)),
        fullScreen(fullScreen),
		cursorHidden(cursorHidden)
    {
        // Initialize GLFW. This is the library that creates our cross platform (kinda since
        // apple decided to ditch opengl support for metal only) window object
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        glfwSetErrorCallback([](int error, const char* description) {
            std::cout << description << "\n";
            std::cin.get();
        });

        if (!this->fullScreen) {
            this->glfwWindow = glfwCreateWindow(this->screenSize.x, this->screenSize.y, "Useless3D", NULL, NULL);
        }
        else {
            this->glfwWindow = glfwCreateWindow(this->screenSize.x, this->screenSize.y, "Useless3D", glfwGetPrimaryMonitor(), NULL);
        }

        if (this->glfwWindow == NULL) 
        {
            glfwTerminate();
            std::cout << "Failed to create GLFW window\n";
            std::cin.get();
            exit(EXIT_FAILURE);
        }
        else 
        {

            glfwMakeContextCurrent(this->glfwWindow);

            // Initialize glad. Glad is a .c file which is included in our project.
            // GLAD manages function pointers for OpenGL so we want to initialize GLAD before we call any OpenGL function
            if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) 
            {
                std::cout << "Failed to initialize GLAD\n";
                std::cin.get();
                exit(EXIT_FAILURE);
            }
            else 
            {
                // Associate this object with the window
                glfwSetWindowUserPointer(this->glfwWindow, this);

                // Set callback functions used by glfw (for when polling is unavailable or it makes better sense
                // to use a callback)
                this->setCallbacks();

                // Set window input mode
				if (this->cursorHidden)
				{
					glfwSetInputMode(this->glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				}

                // Set opengl viewport size
                glViewport(0, 0, this->screenSize.x, this->screenSize.y);
            }

        }


    }
    Window::~Window() 
    {
        glfwDestroyWindow(this->glfwWindow);

        // Terminate GLFW application process
        glfwTerminate();
    }

    void Window::setTitle(std::string title)
    {
        glfwSetWindowTitle(this->glfwWindow, title.c_str());
    }

    void Window::setCallbacks() 
    {
        // Scroll
        glfwSetScrollCallback(this->glfwWindow, [](GLFWwindow* window, double xoffset, double yoffset) {

            // get this from window
            void* data = glfwGetWindowUserPointer(window);
            Window* w = static_cast<Window*>(data);
            w->scrollX = xoffset;
            w->scrollY = yoffset;

        });

        // Window size updated
        glfwSetFramebufferSizeCallback(this->glfwWindow, [](GLFWwindow* window, int width, int height) {

            // get this from window
            void* data = glfwGetWindowUserPointer(window);
            Window* w = static_cast<Window*>(data);
            w->screenSize.x = width;
            w->screenSize.y = height;

            glViewport(0, 0, width, height);

        });
    }

    void Window::update() 
    {
        glfwPollEvents();
        setKeys();
        setMouse();
        setScroll();
    }


    void Window::swapBuffers() 
    {
        // swap the color buffer (a large buffer that contains color values for each pixel in GLFW's window) 
        // that has been used to draw in during this iteration and show it as output to the screen
        glfwSwapBuffers(this->glfwWindow);
    }

    bool Window::shouldClose() 
    {
        return glfwWindowShouldClose(this->glfwWindow);
    }

    void Window::setKeys() 
    {
        this->inputState.keyEsc = glfwGetKey(this->glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS;
        this->inputState.keyW = glfwGetKey(this->glfwWindow, GLFW_KEY_W) == GLFW_PRESS;
        this->inputState.keyA = glfwGetKey(this->glfwWindow, GLFW_KEY_A) == GLFW_PRESS;
        this->inputState.keyS = glfwGetKey(this->glfwWindow, GLFW_KEY_S) == GLFW_PRESS;
        this->inputState.keyD = glfwGetKey(this->glfwWindow, GLFW_KEY_D) == GLFW_PRESS;
        this->inputState.keySpace = glfwGetKey(this->glfwWindow, GLFW_KEY_SPACE) == GLFW_PRESS;

		this->inputState.keyUp = glfwGetKey(this->glfwWindow, GLFW_KEY_UP) == GLFW_PRESS;
		this->inputState.keyDown = glfwGetKey(this->glfwWindow, GLFW_KEY_DOWN) == GLFW_PRESS;
		this->inputState.keyRight = glfwGetKey(this->glfwWindow, GLFW_KEY_RIGHT) == GLFW_PRESS;
		this->inputState.keyLeft = glfwGetKey(this->glfwWindow, GLFW_KEY_LEFT) == GLFW_PRESS;
    }

    void Window::setMouse() 
    {
		double mXPos;
		double mYPos;

        glfwGetCursorPos(this->glfwWindow, &mXPos, &mYPos);

		this->inputState.mouseXPos = (float)mXPos;
		this->inputState.mouseYPos = (float)mYPos;
    }

    void Window::setScroll() 
    {
        this->inputState.scrollX = scrollX;
        this->inputState.scrollY = scrollY;
        scrollX = 0;
        scrollY = 0;
    }

    void Window::setToClose() 
    {
        glfwSetWindowShouldClose(this->glfwWindow, true);
    }

    const InputState& Window::getInputState() const
    {
        return this->inputState;
    }

    const glm::ivec2& Window::getScreenSize() const
    {
        return this->screenSize;
    }

}