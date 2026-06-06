#pragma once
#include <GLFW/glfw3.h>

namespace Core {

class Window
{
public:
	bool Init();
	void Shutdown();

	void OnUpdate();

	bool ShouldClose() const;

	::GLFWwindow* GetGLFWWindow() const
	{
		return static_cast<::GLFWwindow*>(m_NativeWindow);
	}

	void* GetNativeWindow() const
	{
		return m_NativeWindow;
	}
private:
	void* m_NativeWindow = nullptr;
};

}