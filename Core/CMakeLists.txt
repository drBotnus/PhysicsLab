add_subdirectory(Vendor/glfw)
add_subdirectory(Vendor/vk-bootstrap)

add_library(Core STATIC
	src/Core/Application.cpp
	src/Core/Application.h

	src/Core/EntryPoint.h
	src/Core/Layer.h
	src/Core/LayerStack.h
	src/Core/Log.h
	src/Core/Time.h

	src/Renderer/Renderer.cpp
	src/Renderer/Renderer.h

	src/Renderer/Buffer.h
	src/Renderer/Buffer.cpp
	src/Renderer/VulkanContext.cpp
	src/Renderer/VulkanContext.h

	src/Platform/Window.cpp
	src/Platform/Window.h
  )

find_package(Vulkan REQUIRED)

target_link_libraries(Core PUBLIC glfw Vulkan::Vulkan vk-bootstrap::vk-bootstrap)

target_include_directories(Core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/Vendor)