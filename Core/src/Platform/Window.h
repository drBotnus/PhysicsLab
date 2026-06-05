#pragma once

namespace Core {

class Window
{
public:
	bool Init();
	void Shutdown();

	void OnUpdate();

	bool ShouldClose() const;

	void* GetNativeWindow() const
	{
		return m_NativeWindow;
	}
private:
	void* m_NativeWindow = nullptr;
};

}