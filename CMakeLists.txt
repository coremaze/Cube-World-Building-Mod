cmake_minimum_required (VERSION 3.8)
project(project_BuildingMod)
add_subdirectory(cwmods)
add_subdirectory(imgui-1.73)
add_library (BuildingMod SHARED 
"main.cpp" 
"BuildingMod.cpp"
"BuildWindow.cpp"
"ChunkUpdate.cpp"
"DButton.cpp"
"BuildNetwork/BuildNetwork.cpp"
"BuildNetwork/BuildNetworkPacket.cpp"
"BuildNetwork/SingleBlockUpdatePacket.cpp"
"BuildNetwork/ZoneUpdatePacket.cpp"
"BuildNetwork/ZoneLoadPacket.cpp"
"BuildNetwork/PingPacket.cpp"
"ZoneSaver/ZoneBlock.cpp"
"ZoneSaver/ZoneContainer.cpp"
"ZoneSaver/WorldContainer.cpp"
"ZoneSaver/ZoneSaver.cpp"
)
target_link_libraries (BuildingMod LINK_PUBLIC CWSDK)
target_link_libraries (BuildingMod LINK_PUBLIC IMGUI)