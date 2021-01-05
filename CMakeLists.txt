# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("OpenCLPlayGround")

set(OpenCL_INCLUDE_DIRS "C:\\Program Files (x86)\\IntelSWTools\\system_studio_2020\\OpenCL\\sdk\\include")
set(OpenCL_LIBRARY "C:\\Program Files (x86)\\IntelSWTools\\system_studio_2020\\OpenCL\\sdk\\lib\\x64\\OpenCL.lib")
include_directories(${OpenCL_INCLUDE_DIRS})
link_directories(${OpenCL_LIBRARY})

# Include sub-projects.
#add_subdirectory ("openCL")


# Add source to this project's executable.
add_executable (OpenCLTester "tester.cpp")

target_link_libraries(OpenCLTester ${OpenCL_LIBRARY})