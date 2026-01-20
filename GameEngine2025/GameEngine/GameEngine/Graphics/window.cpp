#include "window.h"

Window::Window(char* name, int width, int height)
{
	this->name = name;
	this->width = width;
	this->height = height;
	
	// Initialize mouse tracking
	this->lastMouseX = width / 2.0;
	this->lastMouseY = height / 2.0;
	this->mouseDeltaX = 0.0;
	this->mouseDeltaY = 0.0;
	this->firstMouse = true;
	this->cursorLocked = false;
	
	init();

	for (int i = 0; i < MAX_KEYBOARD; i++)
	{
		this->keys[i] = false;
	}

	for (int i = 0; i < MAX_MOUSE; i++)
	{
		this->mouseButtons[i] = false;
	}
}

Window::~Window()
{
	glfwTerminate();
}

void Window::init()
{
	if (!glfwInit())
	{
		std::cout << "Error initializing glfw!" << std::endl;
	}
	else
	{
		std::cout << "Successfully initializing glfw!" << std::endl;
	}

	window = glfwCreateWindow(width, height, name, NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to create a GLFW window" << std::endl;
		glfwTerminate();
		return;
	}

	glfwMakeContextCurrent(window);

	//callbacks for user input
	glfwSetWindowUserPointer(window, this);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);

	if (glewInit() != GLEW_OK)
	{
		std::cout << "Error initializing glew!" << std::endl;
	}
	else
	{
		std::cout << "Successfully initializing glew!" << std::endl;
	}

	std::cout << "Open GL " << glGetString(GL_VERSION) << std::endl;
}

void Window::update()
{
	glfwPollEvents();
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	glfwSwapBuffers(window);
}

void Window::clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

GLFWwindow* Window::getWindow()
{
	return window;
}

int Window::getWidth()
{
	return width;
}

int Window::getHeight()
{
	return height;
}

void Window::setKey(int key, bool ok)
{
	this->keys[key] = ok;
}

void Window::setMouseButton(int button, bool ok)
{
	this->mouseButtons[button] = ok;
}

void Window::setMousePos(double xpos, double ypos)
{
	if (firstMouse)
	{
		lastMouseX = xpos;
		lastMouseY = ypos;
		firstMouse = false;
	}
	
	// Calculate delta
	mouseDeltaX = xpos - lastMouseX;
	mouseDeltaY = lastMouseY - ypos;  // Reversed: y-coordinates go from bottom to top
	
	lastMouseX = xpos;
	lastMouseY = ypos;
	
	this->xpos = xpos;
	this->ypos = ypos;
}

void Window::getMousePos(double &xpos, double &ypos)
{
	xpos = this->xpos;
	ypos = this->ypos;
}

void Window::lockCursor(bool lock)
{
	cursorLocked = lock;
	if (lock)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		firstMouse = true;  // Reset first mouse to avoid jump
	}
	else
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void Window::getMouseDelta(double &deltaX, double &deltaY)
{
	deltaX = mouseDeltaX;
	deltaY = mouseDeltaY;
}

void Window::resetMouseDelta()
{
	mouseDeltaX = 0.0;
	mouseDeltaY = 0.0;
}

//Handling key pressed
bool Window::isPressed(int key)
{
	return keys[key];
}

//Handling mouse buttons pressed
bool Window::isMousePressed(int button)
{
	return mouseButtons[button];
}

//Handling keyboard actions
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Window* wind = (Window*) glfwGetWindowUserPointer(window);

	if (action != GLFW_RELEASE)
		wind->setKey(key, true);
	else
		wind->setKey(key, false);
}

//Handling mouse actions
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	Window* wind = (Window*)glfwGetWindowUserPointer(window);

	if (action != GLFW_RELEASE)
		wind->setMouseButton(button, true);
	else
		wind->setMouseButton(button, false);
}

//Handling cursor position
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	Window* wind = (Window*)glfwGetWindowUserPointer(window);
	wind->setMousePos(xpos, ypos);
}
