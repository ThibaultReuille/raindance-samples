### ----- Project Configuration -----

cmake_minimum_required(VERSION 2.8)

### ----- Packing Resources

add_executable(agents Agents.cc)
add_executable(charts Charts.cc)
add_executable(cube Cube.cc)
add_executable(fonts Fonts.cc)
add_executable(interface Interface.cc)
add_executable(stream Stream.cc)
add_executable(terminal Terminal.cc)
add_executable(window Window.cc)
add_executable(stereo Stereo.cc)

### ----- Compiler Configuration -----

include(CheckCXXCompilerFlag)

CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wno-missing-field-initializers -O3 -Wno-deprecated")

### -----  Includes / Libraries / Packages -----

include_directories(${PROJECT_SOURCE_DIR}/../)

list(APPEND CMAKE_PREFIX_PATH ${PROJECT_SOURCE_DIR}/../raindance/Lib/glm-0.9.5.4)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(PythonLibs)

include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${GLM_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR}/../OculusSDK/LibOVR/Include)
include_directories(${PROJECT_SOURCE_DIR}/../OculusSDK/LibOVRKernel/Src/)

add_library(libovr STATIC IMPORTED)
set_target_properties(libovr PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/../OculusSDK/output/libovr.a)
 
### ----- Linking -----

target_link_libraries(agents ${OPENGL_LIBRARIES})
target_link_libraries(agents ${GLFW_STATIC_LIBRARIES})
target_link_libraries(agents ${GLEW_LIBRARIES})

target_link_libraries(charts ${OPENGL_LIBRARIES})
target_link_libraries(charts ${GLFW_STATIC_LIBRARIES})
target_link_libraries(charts ${GLEW_LIBRARIES})

target_link_libraries(cube ${OPENGL_LIBRARIES})
target_link_libraries(cube ${GLFW_STATIC_LIBRARIES})
target_link_libraries(cube ${GLEW_LIBRARIES})

target_link_libraries(fonts ${OPENGL_LIBRARIES})
target_link_libraries(fonts ${GLFW_STATIC_LIBRARIES})
target_link_libraries(fonts ${GLEW_LIBRARIES})

target_link_libraries(interface ${OPENGL_LIBRARIES})
target_link_libraries(interface ${GLFW_STATIC_LIBRARIES})
target_link_libraries(interface ${GLEW_LIBRARIES})

target_link_libraries(stereo ${OPENGL_LIBRARIES})
target_link_libraries(stereo ${GLFW_STATIC_LIBRARIES})
target_link_libraries(stereo ${GLEW_LIBRARIES})
target_link_libraries(stereo libovr)

target_link_libraries(stream ${OPENGL_LIBRARIES})
target_link_libraries(stream ${GLFW_STATIC_LIBRARIES})
target_link_libraries(stream ${GLEW_LIBRARIES})

target_link_libraries(terminal ${OPENGL_LIBRARIES})
target_link_libraries(terminal ${GLFW_STATIC_LIBRARIES})
target_link_libraries(terminal ${GLEW_LIBRARIES})

target_link_libraries(window ${OPENGL_LIBRARIES})
target_link_libraries(window ${GLFW_STATIC_LIBRARIES})
target_link_libraries(window ${GLEW_LIBRARIES})