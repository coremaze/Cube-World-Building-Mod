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

BlockColor current_block_color = BlockColor(255, 255, 255, 1);




__stdcall bool no_shenanigans HandleMessage(wchar_t msg[], unsigned int msg_size){
    cube::Creature* player = GameController->GetLocalPlayer();

    int r, g, b, type;

    if ( !wcscmp(msg, L"/place")){
        Block* block = GameController->GetBlockAtCrosshair(40.0, true);
        if (block != (Block*)nullptr){
            r = current_block_color.r;
            g = current_block_color.g;
            b = current_block_color.b;
            type = current_block_color.type;
            //Put block in the world
            GameController->world.SetBlock(block->x, block->y, block->z, r, g, b, type);
            //Update visuals
            unsigned int chunkx = block->x / 32;
            unsigned int chunky = block->y / 32;
            GameController->UpdateChunk(chunkx, chunky);

            //save everything
            worldContainer.SetBlock(block->x, block->y, block->z, r, g, b, type);
            worldContainer.OutputFiles(GameController->world.worldName);

            delete block;
        }

        return true;
    }
    else if ( !wcscmp(msg, L"/break")){
        Block* block = GameController->GetBlockAtCrosshair(40.0, false);
        if (block != (Block*)nullptr){
            unsigned int blockx = block->x;
            unsigned int blocky = block->y;
            int blockz = block->z;
            r = 255;
            g = 255;
            b = 255;
            type = 0;
            GameController->world.SetBlock(block->x, block->y, block->z, r, g, b, type);

            unsigned int chunkx = blockx / 32;
            unsigned int chunky = blocky / 32;
            GameController->UpdateChunk(chunkx, chunky);

            //save everything
            worldContainer.SetBlock(block->x, block->y, block->z, r, g, b, type);
            worldContainer.OutputFiles(GameController->world.worldName);

            delete block;
        }
        return true;

    }
    else if ( swscanf(msg, L"/setcolor %d %d %d", &r, &g, &b) == 3){
        current_block_color.r = r;
        current_block_color.g = g;
        current_block_color.b = b;
        current_block_color.type = 1;
        wchar_t response[256];
        swprintf(response, L"Selected block color %u %u %u.\n", (unsigned int)r, (unsigned int)g, (unsigned int)b);
        GameController->PrintMessage(response, (unsigned int)r, (unsigned int)g, (unsigned int)b);
        return true;
    }
    else if ( !wcscmp(msg, L"/get")){
        Block* block = GameController->GetBlockAtCrosshair(40.0, false);
        if (block != (Block*)nullptr){
            current_block_color.r = block->color.r;
            current_block_color.g = block->color.g;
            current_block_color.b = block->color.b;
            current_block_color.type = block->color.type;
            wchar_t response[256];
            swprintf(response, L"Selected block color %u %u %u.\n", (unsigned int)block->color.r, (unsigned int)block->color.g, (unsigned int)block->color.b);
            GameController->PrintMessage(response, (unsigned int)block->color.r, (unsigned int)block->color.g, (unsigned int)block->color.b);
            delete block;
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

void no_shenanigans ControlsChecker(){
    //quick and dirty just so I could demonstrate how it could work
    Sleep(10000);
    unsigned char r = 255;
    unsigned char g = 255;
    unsigned char b = 255;
    unsigned char type = 0;
    while (true){
        Sleep(100);
        if (GameController->M1 > (uint8_t)0){
            Block* block = GameController->GetBlockAtCrosshair(40.0, false);
            if (block != (Block*)nullptr){
                unsigned int blockx = block->x;
                unsigned int blocky = block->y;
                int blockz = block->z;
                r = 255;
                g = 255;
                b = 255;
                type = 0;
                GameController->world.SetBlock(block->x, block->y, block->z, r, g, b, type);

                unsigned int chunkx = blockx / 32;
                unsigned int chunky = blocky / 32;
                GameController->UpdateChunk(chunkx, chunky);

                //save everything
                worldContainer.SetBlock(block->x, block->y, block->z, r, g, b, type);
                worldContainer.OutputFiles(GameController->world.worldName);

                delete block;
            }
        }
        else if (GameController->M2 > (uint8_t)0){
            Block* block = GameController->GetBlockAtCrosshair(40.0, true);
            if (block != (Block*)nullptr){
                r = current_block_color.r;
                g = current_block_color.g;
                b = current_block_color.b;
                type = current_block_color.type;
                //Put block in the world
                GameController->world.SetBlock(block->x, block->y, block->z, r, g, b, type);
                //Update visuals
                unsigned int chunkx = block->x / 32;
                unsigned int chunky = block->y / 32;
                GameController->UpdateChunk(chunkx, chunky);

                //save everything
                worldContainer.SetBlock(block->x, block->y, block->z, r, g, b, type);
                worldContainer.OutputFiles(GameController->world.worldName);

                delete block;
            }

        }
        else if (GameController->M3 > (uint8_t)0){
            Block* block = GameController->GetBlockAtCrosshair(40.0, false);
            if (block != (Block*)nullptr){
                current_block_color.r = block->color.r;
                current_block_color.g = block->color.g;
                current_block_color.b = block->color.b;
                current_block_color.type = block->color.type;
                wchar_t response[256];
                swprintf(response, L"Selected block color %u %u %u.\n", (unsigned int)block->color.r, (unsigned int)block->color.g, (unsigned int)block->color.b);
                GameController->PrintMessage(response, (unsigned int)block->color.r, (unsigned int)block->color.g, (unsigned int)block->color.b);
                delete block;

            }

        }
    }
}

extern "C" no_shenanigans BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    base = (UINT_PTR)GetModuleHandle(NULL);
    cube::SetBase(base);
    GameController = cube::GetGameController();

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RegisterCallbacks, 0, 0, NULL);
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ControlsChecker, 0, 0, NULL);
            break;
    }
    return TRUE;
}
