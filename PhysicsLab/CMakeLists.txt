# CMakeList.txt : CMake project for PhysicsLab, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (PhysicsLab "PhysicsLab.cpp" "PhysicsLab.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET PhysicsLab PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
