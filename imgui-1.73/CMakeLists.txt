cmake_minimum_required (VERSION 3.8)
if(NOT CMAKE_CL_64)
   message(FATAL_ERROR "CWSDK REQUIRES clang-cl x64 to build. " )
endif()
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    message(FATAL_ERROR "CWSDK cannot be built debug mode due to MSVC binary compatibility issues." )
endif()
project("IMGUI" VERSION "1.0.0")
add_library (IMGUI 
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_widgets.cpp
    examples/imgui_impl_dx11.cpp
    examples/imgui_impl_win32.cpp)
target_include_directories (IMGUI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})