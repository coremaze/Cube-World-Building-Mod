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

float degrees_to_radians(float degrees){
    return (degrees * 3.1415926535) / 180.0;
}
float radians_to_degrees(float radians){
    return (radians * 180.0) / 3.1415926535;
}

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
    else if ( !wcscmp(msg, L"/block")){
//        unsigned int blockx = player->x / 0x10000;
//        unsigned int blocky = player->y / 0x10000;
//        int blockz = player->z / 0x10000 - 2;
//        BlockColor* color = GameController->world.GetBlock(blockx, blocky, blockz, (cube::Zone*)nullptr);
        wchar_t response[256];

        swprintf(response, L"%f %f\n",  GameController->cameraYaw, GameController->cameraPitch);

        float yaw = degrees_to_radians(GameController->cameraYaw + 90.0);
        float pitch = degrees_to_radians(GameController->cameraPitch);

        float camera_x_direction = cos(yaw) * sin(pitch);
        float camera_y_direction = sin(yaw) * sin(pitch);
        float camera_z_direction = -cos(pitch);

        float blocks_away_from_player_x = camera_x_direction * GameController->cameraZoom;
        float blocks_away_from_player_y = camera_y_direction * GameController->cameraZoom;
        float blocks_away_from_player_z = camera_z_direction * GameController->cameraZoom;

        //get the location of the camera in world units.
        auto camera_x = player->x + (long long int)(blocks_away_from_player_x * 65536.0);
        auto camera_y = player->y + (long long int)(blocks_away_from_player_y * 65536.0);
        auto camera_z = player->z + (long long int)((blocks_away_from_player_z + 1.6) * 65536.0); //it seems like this calculation was always ~1 block too low.

        unsigned int blockx = camera_x / 0x10000;
        unsigned int blocky = camera_y / 0x10000;
        unsigned int blockz = camera_z / 0x10000;
        unsigned int lastblockx = blockx;
        unsigned int lastblocky = blocky;
        unsigned int lastblockz = blockz;


        //raycast
        float reach_limit = 65536.0 * 30.0; //30 blocks
        float raycast_precision = 1000.0;
        bool withinReach = false;
        bool wantFaceBlock = true;
        for (float world_units_traveled = 0.0; world_units_traveled <= reach_limit; world_units_traveled += raycast_precision){
            lastblockx = blockx;
            lastblocky = blocky;
            lastblockz = blockz;
            blockx = camera_x / 0x10000;
            blocky = camera_y / 0x10000;
            blockz = camera_z / 0x10000;
            BlockColor* color = GameController->world.GetBlock(blockx, blocky, blockz, (cube::Zone*)nullptr);
            if (color->type != 0){
                withinReach = true;
                if (wantFaceBlock){
                    blockx = lastblockx;
                    blocky = lastblocky;
                    blockz = lastblockz;
                }
                break;
            }
            camera_x -= (long long int)(camera_x_direction * raycast_precision);
            camera_y -= (long long int)(camera_y_direction * raycast_precision);
            camera_z -= (long long int)(camera_z_direction * raycast_precision);
        }

        if (withinReach){
            unsigned int chunkx = blockx / 32;
            unsigned int chunky = blocky / 32;

            GameController->world.SetBlock(blockx, blocky, blockz, 255, 255, 255, 1);
            GameController->UpdateChunk(chunkx, chunky);

            //save everything
            worldContainer.SetBlock(blockx, blocky, blockz, 255, 255, 255, 1);
            worldContainer.OutputFiles(GameController->world.worldName);

            GameController->PrintMessage(response);
        }
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
