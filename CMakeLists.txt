cmake_minimum_required (VERSION 3.8)
project(project_BuildingMod)
add_subdirectory(cwmods)
add_subdirectory(imgui-1.73)

add_library (BuildingMod SHARED 
main.cpp
BuildWindow.cpp
BuildingMod.cpp
DButton.cpp
)

target_link_libraries (BuildingMod LINK_PUBLIC CWSDK)
target_link_libraries (BuildingMod LINK_PUBLIC IMGUI)
target_include_directories (BuildingMod PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/cwmods)
target_include_directories (BuildingMod PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/cwmods/common)
target_include_directories (BuildingMod PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/cwmods/cube)
target_include_directories (BuildingMod PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/cwmods/gfx)
target_include_directories (BuildingMod PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/cwmods/IDA)
target_include_directories (BuildingMod PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/cwmods/msvc)
target_include_directories (BuildingMod PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/cwmods/plasma)