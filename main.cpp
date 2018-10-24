#undef __STRICT_ANSI__
#include <windows.h>
#include <iostream>
#include "cube.h"
#include "zonesaver.h"
#include <vector>

#define no_shenanigans __attribute__((noinline)) __declspec(dllexport)

UINT_PTR base;

cube::GameController* GameController;

ZoneSaver::WorldContainer worldContainer;

__stdcall bool no_shenanigans HandleMessage(wchar_t msg[], unsigned int msg_size){
    DWORD entityaddr = (DWORD)GameController;
    entityaddr += 0x39C;
    entityaddr = *(DWORD*)entityaddr;
    cube::Creature* player = (cube::Creature*)entityaddr;

    int r, g, b, type;

    if ( swscanf(msg, L"/block %d %d %d %d", &r, &g, &b, &type) == 4){
        typedef void(__thiscall* vector_int64_from_vector3_float_t)(Vector3_Int64*, Vector3_Float*);
        auto vector_int64_from_vector3_float = (vector_int64_from_vector3_float_t)(base + 0x02C460);

        Vector3_Int64 looking_at_offset = Vector3_Int64();
        vector_int64_from_vector3_float(&looking_at_offset, &player->camera_offset);

        Vector3_Int64 pos;
        pos.x = player->x;
        pos.y = player->y;
        pos.z = player->z;

        Vector3_Int64 block_pos;
        block_pos.x = (looking_at_offset.x + pos.x);
        block_pos.y = (looking_at_offset.y + pos.y);
        block_pos.z = (looking_at_offset.z + pos.z);

        unsigned int blockx = block_pos.x / 0x10000;
        unsigned int blocky = block_pos.y / 0x10000;
        unsigned int blockz = block_pos.z / 0x10000;

        unsigned int chunkx = block_pos.x / 0x200000;
        unsigned int chunky = block_pos.y / 0x200000;

        GameController->world.SetBlock(blockx, blocky, blockz, r, g, b, type);
        GameController->UpdateChunk(chunkx, chunky);

        //save everything
        worldContainer.SetBlock(blockx, blocky, blockz, (char)r, (char)g, (char)b, (char)type);
        worldContainer.OutputFiles(GameController->world.worldName);

        return true;
    }
    return false;
}

__stdcall void no_shenanigans HandleZoneLoaded(cube::Zone* zone){
    //Get a block vector. It will either be from an existing Zone, a Zone newly created from a file, or an empty vector
    std::vector<ZoneSaver::ZoneBlock*> blocks = worldContainer.LoadZoneBlocks(GameController->world.worldName, zone->x, zone->y);

    //Place blocks in the world and update them
    for (ZoneSaver::ZoneBlock* block : blocks){
        GameController->world.SetBlock(block->x, block->y, block->z, block->r, block->g, block->b, block->type);
        unsigned int chunkx = block->x / 0x200000;
        unsigned int chunky = block->y / 0x200000;
        GameController->UpdateChunk(chunkx, chunky);
    }
}

__stdcall void no_shenanigans HandleZoneDelete(cube::Zone* zone){
    worldContainer.DeleteZoneContainer(zone->x, zone->y);
}


DWORD WINAPI no_shenanigans RegisterCallbacks(){
        HMODULE modManagerDLL = LoadLibraryA("CallbackManager.dll");

        typedef bool (__stdcall *ChatEventCallback)(wchar_t buf[], unsigned int msg_size);
        typedef void (*RegisterChatEventCallback_t)(ChatEventCallback cb);
        auto RegisterChatEventCallback = (RegisterChatEventCallback_t)GetProcAddress(modManagerDLL, "RegisterChatEventCallback");
        RegisterChatEventCallback((ChatEventCallback)HandleMessage);

        typedef bool (__stdcall *ZoneLoadedCallback)(unsigned int zone_ptr);
        typedef void (*RegisterZoneLoadedCallback_t)(ZoneLoadedCallback cb);
        auto RegisterZoneLoadedCallback = (RegisterZoneLoadedCallback_t)GetProcAddress(modManagerDLL, "RegisterZoneLoadedCallback");
        RegisterZoneLoadedCallback((ZoneLoadedCallback)HandleZoneLoaded);

        typedef bool (__stdcall *ZoneDeleteCallback)(unsigned int zone_ptr);
        typedef void (*RegisterZoneDeleteCallback_t)(ZoneDeleteCallback cb);
        auto RegisterZoneDeleteCallback = (RegisterZoneDeleteCallback_t)GetProcAddress(modManagerDLL, "RegisterZoneDeleteCallback");
        RegisterZoneDeleteCallback((ZoneDeleteCallback)HandleZoneDelete);

        return 0;
}

extern "C" no_shenanigans BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    base = (UINT_PTR)GetModuleHandle(NULL);
    cube::SetBase(base);

    unsigned int GameController_ptr = *(DWORD*)(base + 0x36B1C8);
    GameController = (cube::GameController*)GameController_ptr;

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RegisterCallbacks, 0, 0, NULL);
            break;
    }
    return TRUE;
}
