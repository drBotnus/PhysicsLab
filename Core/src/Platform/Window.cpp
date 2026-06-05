#include "Platform/Window.h"

#include <GLFW/glfw3.h>

namespace Core
{

bool Window::Init()
{
	if (!::glfwInit()) {
		return false;
	}

	::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_NativeWindow = ::glfwCreateWindow(1280, 720, "Physics Lab", nullptr, nullptr);

	return m_NativeWindow != nullptr;
}

void Window::Shutdown()
{
	::glfwDestroyWindow(static_cast<::GLFWwindow*>(m_NativeWindow));

	::glfwTerminate();
}

void Window::OnUpdate()
{
	::glfwPollEvents();
}

bool Window::ShouldClose() const
{
	return ::glfwWindowShouldClose(static_cast<::GLFWwindow*>(m_NativeWindow));
}

}