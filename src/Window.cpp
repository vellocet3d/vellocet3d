#include <iostream>


#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "vel/Window.h"
#include "vel/nvapi.hpp"

// stutter caused when not in fullscreen mode: https://stackoverflow.com/a/21663076/1609485
// https://www.reddit.com/r/opengl/comments/8754el/stuttering_with_learnopengl_tutorials/dwbp7ta?utm_source=share&utm_medium=web2x
// could be because of multiple monitors all running different refresh rates

//TODO: Need to refactor this to use Log.h

namespace vel
{
	void APIENTRY glDebugOutput(GLenum source,
		GLenum type,
		unsigned int id,
		GLenum severity,
		GLsizei length,
		const char *message,
		const void *userParam)
	{
		// ignore non-significant error/warning codes
		//if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

		std::cout << "---------------" << std::endl;
		std::cout << "Debug message (" << id << "): " << message << std::endl;

		switch (source)
		{
		case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
		case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
		case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
		} std::cout << std::endl;

		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
		case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
		case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
		case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
		case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
		} std::cout << std::endl;

		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
		case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
		case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
		} std::cout << std::endl;
		std::cout << std::endl;
	}


    Window::Window(Config c) :
        screenSize(glm::ivec2(c.SCREEN_WIDTH, c.SCREEN_HEIGHT)),
        fullScreen(c.FULLSCREEN),
		cursorHidden(c.CURSOR_HIDDEN),
		useImGui(c.USE_IMGUI),
		vsync(c.VSYNC),
		imguiFrameOpen(false)
    {
		// should we include nvidia api so we can set application profile?
#ifdef WINDOWS_BUILD
		initNvidiaApplicationProfile(c.APP_EXE_NAME, c.APP_NAME);
#endif


        // Initialize GLFW. This is the library that creates our cross platform (kinda since
        // apple decided to ditch opengl support for metal only) window object
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        
		if (c.OPENGL_DEBUG_CONTEXT)
		{
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
		}

        
        

        glfwSetErrorCallback([](int error, const char* description) {
            std::cout << description << "\n";
            std::cin.get();
        });

        if (!this->fullScreen) {
            this->glfwWindow = glfwCreateWindow(this->screenSize.x, this->screenSize.y, c.APP_NAME.c_str(), NULL, NULL);
        }
        else {
            this->glfwWindow = glfwCreateWindow(this->screenSize.x, this->screenSize.y, c.APP_NAME.c_str(), glfwGetPrimaryMonitor(), NULL);
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

			if(this->vsync)
				glfwSwapInterval(1); // 0 = no vsync 1 = vsync
			else
				glfwSwapInterval(0);

			

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

				glfwSetInputMode(this->glfwWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

                // Set window input mode
				if (this->cursorHidden)
				{
					glfwSetInputMode(this->glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
					//glfwSetInputMode(this->glfwWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
				}



				if (c.OPENGL_DEBUG_CONTEXT)
				{
					int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
					if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
					{
						glEnable(GL_DEBUG_OUTPUT);
						glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
						glDebugMessageCallback(glDebugOutput, nullptr);
						glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
						std::cout << "OpenGL debug context should be loaded" << std::endl;
					}
					else
					{
						std::cout << "OpenGL debug context unable to load" << std::endl;
						std::cin.get();
					}
				}



                // Set opengl viewport size
                glViewport(0, 0, this->screenSize.x, this->screenSize.y);

				//glfwFocusWindow(this->glfwWindow);

				

				if (this->useImGui)
				{
					// Setup Dear ImGui context
					IMGUI_CHECKVERSION();
					ImGui::CreateContext();
					ImGuiIO& io = ImGui::GetIO(); (void)io;
					//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
					//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

					// get pointer to default font
					this->imguiFonts["default"] = io.Fonts->AddFontDefault();

					// create fonts
					for (auto& f : c.imguiFonts)
						this->imguiFonts[f.key] = io.Fonts->AddFontFromFileTTF(f.path.c_str(), f.pixels);

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



	void Window::showMouseCursor()
	{
		glfwSetInputMode(this->glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	void Window::hideMouseCursor()
	{
		glfwSetInputMode(this->glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		//glfwFocusWindow(this->glfwWindow);
		//glfwSetInputMode(this->glfwWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}

	ImFont* Window::getImguiFont(std::string key) const
	{
		return this->imguiFonts.at(key);
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

		glfwSetWindowFocusCallback(this->glfwWindow, [](GLFWwindow* window, int focused) {
		
			if (focused)
				std::cout << "window focused\n";
			else
				std::cout << "window NOT focused\n";
		
		});

    }

	bool Window::getImguiFrameOpen()
	{
		return this->imguiFrameOpen;
	}

	void Window::renderGui()
	{
		if (this->useImGui)
		{
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			this->imguiFrameOpen = false;
		}
	}

	void Window::updateInputState()
	{
		glfwPollEvents();
		setKeys();
		setMouse();
		setScroll();
	}

    void Window::update() 
    {
        

		if (this->useImGui)
		{
			// Start the Dear ImGui frame
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			this->imguiFrameOpen = true;
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
		this->inputState.mouseLeftButton = glfwGetMouseButton(this->glfwWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
		this->inputState.mouseRightButton = glfwGetMouseButton(this->glfwWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
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
		///////////////////////////////////////////
		this->inputState.mouseLeftButton_Released = glfwGetMouseButton(this->glfwWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE;
		this->inputState.mouseRightButton_Released = glfwGetMouseButton(this->glfwWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE;
		this->inputState.keySpace_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_SPACE) == GLFW_RELEASE;
		this->inputState.keyApostrophe_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_APOSTROPHE) == GLFW_RELEASE;
		this->inputState.keyComma_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_COMMA) == GLFW_RELEASE;
		this->inputState.keyMinus_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_MINUS) == GLFW_RELEASE;
		this->inputState.keyPeriod_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_PERIOD) == GLFW_RELEASE;
		this->inputState.keySlash_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_SLASH) == GLFW_RELEASE;
		this->inputState.key0_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_0) == GLFW_RELEASE;
		this->inputState.key1_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_1) == GLFW_RELEASE;
		this->inputState.key2_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_2) == GLFW_RELEASE;
		this->inputState.key3_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_3) == GLFW_RELEASE;
		this->inputState.key4_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_4) == GLFW_RELEASE;
		this->inputState.key5_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_5) == GLFW_RELEASE;
		this->inputState.key6_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_6) == GLFW_RELEASE;
		this->inputState.key7_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_7) == GLFW_RELEASE;
		this->inputState.key8_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_8) == GLFW_RELEASE;
		this->inputState.key9_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_9) == GLFW_RELEASE;
		this->inputState.keySemicolon_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_SEMICOLON) == GLFW_RELEASE;
		this->inputState.keyEqual_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_EQUAL) == GLFW_RELEASE;
		this->inputState.keyA_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_A) == GLFW_RELEASE;
		this->inputState.keyB_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_B) == GLFW_RELEASE;
		this->inputState.keyC_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_C) == GLFW_RELEASE;
		this->inputState.keyD_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_D) == GLFW_RELEASE;
		this->inputState.keyE_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_E) == GLFW_RELEASE;
		this->inputState.keyF_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_F) == GLFW_RELEASE;
		this->inputState.keyG_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_G) == GLFW_RELEASE;
		this->inputState.keyH_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_H) == GLFW_RELEASE;
		this->inputState.keyI_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_I) == GLFW_RELEASE;
		this->inputState.keyJ_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_J) == GLFW_RELEASE;
		this->inputState.keyK_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_K) == GLFW_RELEASE;
		this->inputState.keyL_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_L) == GLFW_RELEASE;
		this->inputState.keyM_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_M) == GLFW_RELEASE;
		this->inputState.keyN_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_N) == GLFW_RELEASE;
		this->inputState.keyO_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_O) == GLFW_RELEASE;
		this->inputState.keyP_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_P) == GLFW_RELEASE;
		this->inputState.keyQ_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_Q) == GLFW_RELEASE;
		this->inputState.keyR_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_R) == GLFW_RELEASE;
		this->inputState.keyS_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_S) == GLFW_RELEASE;
		this->inputState.keyT_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_T) == GLFW_RELEASE;
		this->inputState.keyU_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_U) == GLFW_RELEASE;
		this->inputState.keyV_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_V) == GLFW_RELEASE;
		this->inputState.keyW_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_W) == GLFW_RELEASE;
		this->inputState.keyX_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_X) == GLFW_RELEASE;
		this->inputState.keyY_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_Y) == GLFW_RELEASE;
		this->inputState.keyZ_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_Z) == GLFW_RELEASE;
		this->inputState.keyLeftBracket_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_LEFT_BRACKET) == GLFW_RELEASE;
		this->inputState.keyRightBracket_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_RIGHT_BRACKET) == GLFW_RELEASE;
		this->inputState.keyBackslash_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_BACKSLASH) == GLFW_RELEASE;
		this->inputState.keyGraveAccent_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_GRAVE_ACCENT) == GLFW_RELEASE;
		this->inputState.keyEscape_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_ESCAPE) == GLFW_RELEASE;
		this->inputState.keyEnter_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_ENTER) == GLFW_RELEASE;
		this->inputState.keyTab_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_TAB) == GLFW_RELEASE;
		this->inputState.keyBackspace_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_BACKSPACE) == GLFW_RELEASE;
		this->inputState.keyInsert_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_INSERT) == GLFW_RELEASE;
		this->inputState.keyDelete_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_DELETE) == GLFW_RELEASE;
		this->inputState.keyRight_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_RIGHT) == GLFW_RELEASE;
		this->inputState.keyLeft_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_LEFT) == GLFW_RELEASE;
		this->inputState.keyDown_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_DOWN) == GLFW_RELEASE;
		this->inputState.keyUp_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_UP) == GLFW_RELEASE;
		this->inputState.keyPageUp_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_PAGE_UP) == GLFW_RELEASE;
		this->inputState.keyPageDown_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_PAGE_DOWN) == GLFW_RELEASE;
		this->inputState.keyHome_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_HOME) == GLFW_RELEASE;
		this->inputState.keyEnd_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_END) == GLFW_RELEASE;
		this->inputState.keyCapsLock_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_CAPS_LOCK) == GLFW_RELEASE;
		this->inputState.keyScrollLock_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_SCROLL_LOCK) == GLFW_RELEASE;
		this->inputState.keyNumLock_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_NUM_LOCK) == GLFW_RELEASE;
		this->inputState.keyPrintScreen_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_PRINT_SCREEN) == GLFW_RELEASE;
		this->inputState.keyPause_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_PAUSE) == GLFW_RELEASE;
		this->inputState.keyF1_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_F1) == GLFW_RELEASE;
		this->inputState.keyF2_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_F2) == GLFW_RELEASE;
		this->inputState.keyF3_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_F3) == GLFW_RELEASE;
		this->inputState.keyF4_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_F4) == GLFW_RELEASE;
		this->inputState.keyF5_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_F5) == GLFW_RELEASE;
		this->inputState.keyF6_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_F6) == GLFW_RELEASE;
		this->inputState.keyF7_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_F7) == GLFW_RELEASE;
		this->inputState.keyF8_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_F8) == GLFW_RELEASE;
		this->inputState.keyF9_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_F9) == GLFW_RELEASE;
		this->inputState.keyF10_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_F10) == GLFW_RELEASE;
		this->inputState.keyF11_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_F11) == GLFW_RELEASE;
		this->inputState.keyF12_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_F12) == GLFW_RELEASE;
		this->inputState.keypad0_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_0) == GLFW_RELEASE;
		this->inputState.keypad1_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_1) == GLFW_RELEASE;
		this->inputState.keypad2_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_2) == GLFW_RELEASE;
		this->inputState.keypad3_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_3) == GLFW_RELEASE;
		this->inputState.keypad4_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_4) == GLFW_RELEASE;
		this->inputState.keypad5_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_5) == GLFW_RELEASE;
		this->inputState.keypad6_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_6) == GLFW_RELEASE;
		this->inputState.keypad7_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_7) == GLFW_RELEASE;
		this->inputState.keypad8_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_8) == GLFW_RELEASE;
		this->inputState.keypad9_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_9) == GLFW_RELEASE;
		this->inputState.keypadDecimal_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_DECIMAL) == GLFW_RELEASE;
		this->inputState.keypadDivide_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_DIVIDE) == GLFW_RELEASE;
		this->inputState.keypadMultiply_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_MULTIPLY) == GLFW_RELEASE;
		this->inputState.keypadSubtract_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_SUBTRACT) == GLFW_RELEASE;
		this->inputState.keypadAdd_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_ADD) == GLFW_RELEASE;
		this->inputState.keypadEnter_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_ENTER) == GLFW_RELEASE;
		this->inputState.keypadEqual_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_KP_EQUAL) == GLFW_RELEASE;
		this->inputState.keyLeftShift_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE;
		this->inputState.keyLeftControl_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE;
		this->inputState.keyLeftAlt_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_LEFT_ALT) == GLFW_RELEASE;
		this->inputState.keyLeftSuper_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_LEFT_SUPER) == GLFW_RELEASE;
		this->inputState.keyRightShift_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_RELEASE;
		this->inputState.keyRightControl_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_RIGHT_CONTROL) == GLFW_RELEASE;
		this->inputState.keyRightAlt_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_RIGHT_ALT) == GLFW_RELEASE;
		this->inputState.keyRightSuper_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_RIGHT_SUPER) == GLFW_RELEASE;
		this->inputState.keyMenu_Released = glfwGetKey(this->glfwWindow, GLFW_KEY_MENU) == GLFW_RELEASE;
    }

    void Window::setMouse() 
    {
		double mXPos;
		double mYPos;

        glfwGetCursorPos(this->glfwWindow, &mXPos, &mYPos);

		//std::cout << mXPos << "\n";

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