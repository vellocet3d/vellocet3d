#include <iostream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "vel/Window.h"

// stutter caused when not in fullscreen mode: https://stackoverflow.com/a/21663076/1609485
// https://www.reddit.com/r/opengl/comments/8754el/stuttering_with_learnopengl_tutorials/dwbp7ta?utm_source=share&utm_medium=web2x
// could be because of multiple monitors all running different refresh rates

namespace vel
{
    Window::Window(int screenWidth, int screenHeight, bool fullScreen, bool cursorHidden, bool useImGui) :
        screenSize(glm::ivec2(screenWidth, screenHeight)),
        fullScreen(fullScreen),
		cursorHidden(cursorHidden),
		useImGui(useImGui)
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
            this->glfwWindow = glfwCreateWindow(this->screenSize.x, this->screenSize.y, "Vellocet3D", NULL, NULL);
        }
        else {
            this->glfwWindow = glfwCreateWindow(this->screenSize.x, this->screenSize.y, "Vellocet3D", glfwGetPrimaryMonitor(), NULL);
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

				if (this->useImGui)
				{
					// Setup Dear ImGui context
					IMGUI_CHECKVERSION();
					ImGui::CreateContext();
					ImGuiIO& io = ImGui::GetIO(); (void)io;
					//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
					//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

					// Setup Dear ImGui style
					ImGui::StyleColorsDark();
					//ImGui::StyleColorsClassic();

					// Setup Platform/Renderer bindings
					ImGui_ImplGlfw_InitForOpenGL(this->glfwWindow, true);
					ImGui_ImplOpenGL3_Init("#version 450 core");
				}

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

	void Window::renderGui()
	{
		if (this->useImGui)
		{
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}
	}

    void Window::update() 
    {
        glfwPollEvents();
        setKeys();
        setMouse();
        setScroll();

		if (this->useImGui)
		{
			// Start the Dear ImGui frame
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
		}

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
		this->inputState.keySpace = glfwGetKey(this->glfwWindow, GLFW_KEY_SPACE) == GLFW_PRESS;
		this->inputState.keyApostrophe = glfwGetKey(this->glfwWindow, GLFW_KEY_APOSTROPHE) == GLFW_PRESS;
		this->inputState.keyComma = glfwGetKey(this->glfwWindow, GLFW_KEY_COMMA) == GLFW_PRESS;
		this->inputState.keyMinus = glfwGetKey(this->glfwWindow, GLFW_KEY_MINUS) == GLFW_PRESS;
		this->inputState.keyPeriod = glfwGetKey(this->glfwWindow, GLFW_KEY_PERIOD) == GLFW_PRESS;
		this->inputState.keySlash = glfwGetKey(this->glfwWindow, GLFW_KEY_SLASH) == GLFW_PRESS;
		this->inputState.key0 = glfwGetKey(this->glfwWindow, GLFW_KEY_0) == GLFW_PRESS;
		this->inputState.key1 = glfwGetKey(this->glfwWindow, GLFW_KEY_1) == GLFW_PRESS;
		this->inputState.key2 = glfwGetKey(this->glfwWindow, GLFW_KEY_2) == GLFW_PRESS;
		this->inputState.key3 = glfwGetKey(this->glfwWindow, GLFW_KEY_3) == GLFW_PRESS;
		this->inputState.key4 = glfwGetKey(this->glfwWindow, GLFW_KEY_4) == GLFW_PRESS;
		this->inputState.key5 = glfwGetKey(this->glfwWindow, GLFW_KEY_5) == GLFW_PRESS;
		this->inputState.key6 = glfwGetKey(this->glfwWindow, GLFW_KEY_6) == GLFW_PRESS;
		this->inputState.key7 = glfwGetKey(this->glfwWindow, GLFW_KEY_7) == GLFW_PRESS;
		this->inputState.key8 = glfwGetKey(this->glfwWindow, GLFW_KEY_8) == GLFW_PRESS;
		this->inputState.key9 = glfwGetKey(this->glfwWindow, GLFW_KEY_9) == GLFW_PRESS;
		this->inputState.keySemicolon = glfwGetKey(this->glfwWindow, GLFW_KEY_SEMICOLON) == GLFW_PRESS;
		this->inputState.keyEqual = glfwGetKey(this->glfwWindow, GLFW_KEY_EQUAL) == GLFW_PRESS;
		this->inputState.keyA = glfwGetKey(this->glfwWindow, GLFW_KEY_A) == GLFW_PRESS;
		this->inputState.keyB = glfwGetKey(this->glfwWindow, GLFW_KEY_B) == GLFW_PRESS;
		this->inputState.keyC = glfwGetKey(this->glfwWindow, GLFW_KEY_C) == GLFW_PRESS;
		this->inputState.keyD = glfwGetKey(this->glfwWindow, GLFW_KEY_D) == GLFW_PRESS;
		this->inputState.keyE = glfwGetKey(this->glfwWindow, GLFW_KEY_E) == GLFW_PRESS;
		this->inputState.keyF = glfwGetKey(this->glfwWindow, GLFW_KEY_F) == GLFW_PRESS;
		this->inputState.keyG = glfwGetKey(this->glfwWindow, GLFW_KEY_G) == GLFW_PRESS;
		this->inputState.keyH = glfwGetKey(this->glfwWindow, GLFW_KEY_H) == GLFW_PRESS;
		this->inputState.keyI = glfwGetKey(this->glfwWindow, GLFW_KEY_I) == GLFW_PRESS;
		this->inputState.keyJ = glfwGetKey(this->glfwWindow, GLFW_KEY_J) == GLFW_PRESS;
		this->inputState.keyK = glfwGetKey(this->glfwWindow, GLFW_KEY_K) == GLFW_PRESS;
		this->inputState.keyL = glfwGetKey(this->glfwWindow, GLFW_KEY_L) == GLFW_PRESS;
		this->inputState.keyM = glfwGetKey(this->glfwWindow, GLFW_KEY_M) == GLFW_PRESS;
		this->inputState.keyN = glfwGetKey(this->glfwWindow, GLFW_KEY_N) == GLFW_PRESS;
		this->inputState.keyO = glfwGetKey(this->glfwWindow, GLFW_KEY_O) == GLFW_PRESS;
		this->inputState.keyP = glfwGetKey(this->glfwWindow, GLFW_KEY_P) == GLFW_PRESS;
		this->inputState.keyQ = glfwGetKey(this->glfwWindow, GLFW_KEY_Q) == GLFW_PRESS;
		this->inputState.keyR = glfwGetKey(this->glfwWindow, GLFW_KEY_R) == GLFW_PRESS;
		this->inputState.keyS = glfwGetKey(this->glfwWindow, GLFW_KEY_S) == GLFW_PRESS;
		this->inputState.keyT = glfwGetKey(this->glfwWindow, GLFW_KEY_T) == GLFW_PRESS;
		this->inputState.keyU = glfwGetKey(this->glfwWindow, GLFW_KEY_U) == GLFW_PRESS;
		this->inputState.keyV = glfwGetKey(this->glfwWindow, GLFW_KEY_V) == GLFW_PRESS;
		this->inputState.keyW = glfwGetKey(this->glfwWindow, GLFW_KEY_W) == GLFW_PRESS;
		this->inputState.keyX = glfwGetKey(this->glfwWindow, GLFW_KEY_X) == GLFW_PRESS;
		this->inputState.keyY = glfwGetKey(this->glfwWindow, GLFW_KEY_Y) == GLFW_PRESS;
		this->inputState.keyZ = glfwGetKey(this->glfwWindow, GLFW_KEY_Z) == GLFW_PRESS;
		this->inputState.keyLeftBracket = glfwGetKey(this->glfwWindow, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS;
		this->inputState.keyRightBracket = glfwGetKey(this->glfwWindow, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS;
		this->inputState.keyBackslash = glfwGetKey(this->glfwWindow, GLFW_KEY_BACKSLASH) == GLFW_PRESS;
		this->inputState.keyGraveAccent = glfwGetKey(this->glfwWindow, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS;
		this->inputState.keyEscape = glfwGetKey(this->glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS;
		this->inputState.keyEnter = glfwGetKey(this->glfwWindow, GLFW_KEY_ENTER) == GLFW_PRESS;
		this->inputState.keyTab = glfwGetKey(this->glfwWindow, GLFW_KEY_TAB) == GLFW_PRESS;
		this->inputState.keyBackspace = glfwGetKey(this->glfwWindow, GLFW_KEY_BACKSPACE) == GLFW_PRESS;
		this->inputState.keyInsert = glfwGetKey(this->glfwWindow, GLFW_KEY_INSERT) == GLFW_PRESS;
		this->inputState.keyDelete = glfwGetKey(this->glfwWindow, GLFW_KEY_DELETE) == GLFW_PRESS;
		this->inputState.keyRight = glfwGetKey(this->glfwWindow, GLFW_KEY_RIGHT) == GLFW_PRESS;
		this->inputState.keyLeft = glfwGetKey(this->glfwWindow, GLFW_KEY_LEFT) == GLFW_PRESS;
		this->inputState.keyDown = glfwGetKey(this->glfwWindow, GLFW_KEY_DOWN) == GLFW_PRESS;
		this->inputState.keyUp = glfwGetKey(this->glfwWindow, GLFW_KEY_UP) == GLFW_PRESS;
		this->inputState.keyPageUp = glfwGetKey(this->glfwWindow, GLFW_KEY_PAGE_UP) == GLFW_PRESS;
		this->inputState.keyPageDown = glfwGetKey(this->glfwWindow, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS;
		this->inputState.keyHome = glfwGetKey(this->glfwWindow, GLFW_KEY_HOME) == GLFW_PRESS;
		this->inputState.keyEnd = glfwGetKey(this->glfwWindow, GLFW_KEY_END) == GLFW_PRESS;
		this->inputState.keyCapsLock = glfwGetKey(this->glfwWindow, GLFW_KEY_CAPS_LOCK) == GLFW_PRESS;
		this->inputState.keyScrollLock = glfwGetKey(this->glfwWindow, GLFW_KEY_SCROLL_LOCK) == GLFW_PRESS;
		this->inputState.keyNumLock = glfwGetKey(this->glfwWindow, GLFW_KEY_NUM_LOCK) == GLFW_PRESS;
		this->inputState.keyPrintScreen = glfwGetKey(this->glfwWindow, GLFW_KEY_PRINT_SCREEN) == GLFW_PRESS;
		this->inputState.keyPause = glfwGetKey(this->glfwWindow, GLFW_KEY_PAUSE) == GLFW_PRESS;
		this->inputState.keyF1 = glfwGetKey(this->glfwWindow, GLFW_KEY_F1) == GLFW_PRESS;
		this->inputState.keyF2 = glfwGetKey(this->glfwWindow, GLFW_KEY_F2) == GLFW_PRESS;
		this->inputState.keyF3 = glfwGetKey(this->glfwWindow, GLFW_KEY_F3) == GLFW_PRESS;
		this->inputState.keyF4 = glfwGetKey(this->glfwWindow, GLFW_KEY_F4) == GLFW_PRESS;
		this->inputState.keyF5 = glfwGetKey(this->glfwWindow, GLFW_KEY_F5) == GLFW_PRESS;
		this->inputState.keyF6 = glfwGetKey(this->glfwWindow, GLFW_KEY_F6) == GLFW_PRESS;
		this->inputState.keyF7 = glfwGetKey(this->glfwWindow, GLFW_KEY_F7) == GLFW_PRESS;
		this->inputState.keyF8 = glfwGetKey(this->glfwWindow, GLFW_KEY_F8) == GLFW_PRESS;
		this->inputState.keyF9 = glfwGetKey(this->glfwWindow, GLFW_KEY_F9) == GLFW_PRESS;
		this->inputState.keyF10 = glfwGetKey(this->glfwWindow, GLFW_KEY_F10) == GLFW_PRESS;
		this->inputState.keyF11 = glfwGetKey(this->glfwWindow, GLFW_KEY_F11) == GLFW_PRESS;
		this->inputState.keyF12 = glfwGetKey(this->glfwWindow, GLFW_KEY_F12) == GLFW_PRESS;
		this->inputState.keypad0 = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_0) == GLFW_PRESS;
		this->inputState.keypad1 = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_1) == GLFW_PRESS;
		this->inputState.keypad2 = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_2) == GLFW_PRESS;
		this->inputState.keypad3 = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_3) == GLFW_PRESS;
		this->inputState.keypad4 = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_4) == GLFW_PRESS;
		this->inputState.keypad5 = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_5) == GLFW_PRESS;
		this->inputState.keypad6 = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_6) == GLFW_PRESS;
		this->inputState.keypad7 = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_7) == GLFW_PRESS;
		this->inputState.keypad8 = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_8) == GLFW_PRESS;
		this->inputState.keypad9 = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_9) == GLFW_PRESS;
		this->inputState.keypadDecimal = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_DECIMAL) == GLFW_PRESS;
		this->inputState.keypadDivide = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_DIVIDE) == GLFW_PRESS;
		this->inputState.keypadMultiply = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_MULTIPLY) == GLFW_PRESS;
		this->inputState.keypadSubtract = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS;
		this->inputState.keypadAdd = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_ADD) == GLFW_PRESS;
		this->inputState.keypadEnter = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_ENTER) == GLFW_PRESS;
		this->inputState.keypadEqual = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_EQUAL) == GLFW_PRESS;
		this->inputState.keyLeftShift = glfwGetKey(this->glfwWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
		this->inputState.keyLeftControl = glfwGetKey(this->glfwWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
		this->inputState.keyLeftAlt = glfwGetKey(this->glfwWindow, GLFW_KEY_LEFT_ALT) == GLFW_PRESS;
		this->inputState.keyLeftSuper = glfwGetKey(this->glfwWindow, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS;
		this->inputState.keyRightShift = glfwGetKey(this->glfwWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
		this->inputState.keyRightControl = glfwGetKey(this->glfwWindow, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
		this->inputState.keyRightAlt = glfwGetKey(this->glfwWindow, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS;
		this->inputState.keyRightSuper = glfwGetKey(this->glfwWindow, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS;
		this->inputState.keyMenu = glfwGetKey(this->glfwWindow, GLFW_KEY_MENU) == GLFW_PRESS;
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
        this->inputState.scrollX = (float)scrollX;
        this->inputState.scrollY = (float)scrollY;
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