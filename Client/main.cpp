#undef __STRICT_ANSI__
#include <windows.h>
#include <iostream>
#include "cube.h"
#include "zonesaver.h"
#include "callbacks.h"
#include <vector>
#include "packets.h"

#define no_shenanigans __attribute__((noinline)) __declspec(dllexport)

UINT_PTR base;
cube::GameController* GameController;
ZoneSaver::WorldContainer worldContainer;

BlockColor current_block_color = BlockColor(255, 255, 255, 1);
bool currently_building = false;

//packets
const unsigned int BUILDING_MOD_PACKET = 1263488066;
const unsigned int ZONE_LOAD_PACKET = 1;
const unsigned int ZONE_UNLOAD_PACKET = 2;
const unsigned int BLOCK_PLACE_PACKET = 3;


void SendZoneLoadPacket(SOCKET socket, unsigned int zone_x, unsigned int zone_y){
    unsigned int packet_id = BUILDING_MOD_PACKET;
    unsigned int sub_id = ZONE_LOAD_PACKET;
    unsigned int pkt_size = 4 + 4 + 4 + 4;

    char buf[pkt_size] = { 0 };
    memcpy(buf     , &packet_id, 4);
    memcpy(buf +  4, &sub_id   , 4);
    memcpy(buf +  8, &zone_x   , 4);
    memcpy(buf + 12, &zone_y   , 4);
    AddPacket(socket, buf, pkt_size);
}
void SendZoneUnloadPacket(SOCKET socket, unsigned int zone_x, unsigned int zone_y){
    unsigned int packet_id = BUILDING_MOD_PACKET;
    unsigned int sub_id = ZONE_UNLOAD_PACKET;
    unsigned int pkt_size = 4 + 4 + 4 + 4;

    char buf[pkt_size] = { 0 };
    memcpy(buf     , &packet_id, 4);
    memcpy(buf +  4, &sub_id   , 4);
    memcpy(buf +  8, &zone_x   , 4);
    memcpy(buf + 12, &zone_y   , 4);
    AddPacket(socket, buf, pkt_size);
}
void SendBlockPlacePacket(SOCKET socket, unsigned int x, unsigned int y, int z, unsigned char r, unsigned char g, unsigned char b, unsigned char type){
    unsigned int packet_id = BUILDING_MOD_PACKET;
    unsigned int sub_id = BLOCK_PLACE_PACKET;
    unsigned int pkt_size = 4 + 4 + 4 + 4 + 4 + 1 + 1 + 1 + 1;

    char buf[pkt_size] = {0};
    memcpy(buf     , &packet_id, 4);
    memcpy(buf +  4, &sub_id   , 4);
    memcpy(buf +  8, &x        , 4);
    memcpy(buf + 12, &y        , 4);
    memcpy(buf + 16, &z        , 4);
    memcpy(buf + 20, &r        , 1);
    memcpy(buf + 21, &g        , 1);
    memcpy(buf + 22, &b        , 1);
    memcpy(buf + 23, &type     , 1);
    AddPacket(socket, buf, pkt_size);
}


unsigned int __stdcall no_shenanigans HandlePacket(unsigned int packet_id, SOCKET socket){
    if (packet_id == BUILDING_MOD_PACKET){
        GameController->PrintMessage(L"Received BLOK.\n");
        return 1;
    }
    return 0;
}

void __stdcall no_shenanigans HandleReadyToSend(SOCKET socket){
    SendQueuedPackets(socket);
}



void PrintSelectBlockMessage(unsigned char r, unsigned char g, unsigned char b, unsigned char type){
    wchar_t response[256];
    if ((type & 0b00111111) == 2){ //block is water
        GameController->PrintMessage(L"Selected water block.\n", 135, 206, 250);
    }
    else if ((type & 0b00111111) == 3){ //block is a solid wet block
        GameController->PrintMessage(L"Selected ", (unsigned int)r, (unsigned int)g, (unsigned int)b);
        GameController->PrintMessage(L"wet", 135, 206, 250);
        swprintf(response, L" block color %u %u %u.\n", (unsigned int)r, (unsigned int)g, (unsigned int)b, type);
        GameController->PrintMessage(response, (unsigned int)r, (unsigned int)g, (unsigned int)b);
    }
    else { //other, solid block
    swprintf(response, L"Selected block color %u %u %u.\n", (unsigned int)r, (unsigned int)g, (unsigned int)b);
    GameController->PrintMessage(response, (unsigned int)r, (unsigned int)g, (unsigned int)b);
    }
}

void PrintCommand(wchar_t* command, wchar_t* description){
    GameController->PrintMessage(command, 0, 255, 127);
    GameController->PrintMessage(L" - ");
    GameController->PrintMessage(description);
    GameController->PrintMessage(L"\n");
}

void PrintHelpMessage(){
    GameController->PrintMessage(L"[");
    GameController->PrintMessage(L"Building Mod", 135, 206, 250);
    GameController->PrintMessage(L"]\n");

    PrintCommand(L"/build", L"Toggles building mode.");
    PrintCommand(L"/build color # # #", L"Select block color.");
    PrintCommand(L"/build water", L"Select a water block.");
    PrintCommand(L"/build wet", L"Change current block to wet.");
    PrintCommand(L"/build dry", L"Change current block to normal.");

}

void UpdateChunkAndAdjacent(unsigned int blockx, unsigned int blocky){
    //First, update the actual chunk in question
    const unsigned int CHUNK_SIZE_IN_BLOCKS = 32;
    unsigned int chunkx = blockx / CHUNK_SIZE_IN_BLOCKS;
    unsigned int chunky = blocky / CHUNK_SIZE_IN_BLOCKS;
    GameController->UpdateChunk(chunkx, chunky);

    //If a block is on the border of a chunk, update the bordering chunk too
    //X
    if ( (blockx % CHUNK_SIZE_IN_BLOCKS) == 0){
        GameController->UpdateChunk(chunkx-1, chunky);
    }
    else if ( (blockx % CHUNK_SIZE_IN_BLOCKS) == CHUNK_SIZE_IN_BLOCKS-1){
        GameController->UpdateChunk(chunkx+1, chunky);
    }

    //Y
    if ( (blocky % CHUNK_SIZE_IN_BLOCKS) == 0){
        GameController->UpdateChunk(chunkx, chunky-1);
    }
    else if ( (blocky % CHUNK_SIZE_IN_BLOCKS) == CHUNK_SIZE_IN_BLOCKS-1){
        GameController->UpdateChunk(chunkx, chunky+1);
    }
}

void no_shenanigans ControlsChecker(){
    unsigned char r = 255;
    unsigned char g = 255;
    unsigned char b = 255;
    unsigned char type = 0;
    while (currently_building){
        //This is inside the loop just to make sure that it can never access GameController
        //prior to the GameController's creation.
        if (GameController->shutdown != 0){
            return;
        }

        Sleep(10);
        if (GameController->M1 > (uint8_t)0){
            //Break block
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

                //update visuals
                UpdateChunkAndAdjacent(blockx, blocky);

                //save everything
                worldContainer.SetBlock(block->x, block->y, block->z, r, g, b, type);
                worldContainer.OutputFiles(GameController->world.worldName);

                if (GameController->on_server){
                    //Send build packet (or in this case, break packet)
                    SendBlockPlacePacket(GameController->server_socket, block->x, block->y, block->z, r, g, b, type);
                }

                delete block;
                Sleep(100);
            }
        }
        else if (GameController->M2 > (uint8_t)0){
            //Place block
            Block* block = GameController->GetBlockAtCrosshair(40.0, true);
            if (block != (Block*)nullptr){
                r = current_block_color.r;
                g = current_block_color.g;
                b = current_block_color.b;
                type = current_block_color.type;

                //Put block in the world
                GameController->world.SetBlock(block->x, block->y, block->z, r, g, b, type);

                //Update visuals
                UpdateChunkAndAdjacent(block->x, block->y);

                //save everything
                worldContainer.SetBlock(block->x, block->y, block->z, r, g, b, type);
                worldContainer.OutputFiles(GameController->world.worldName);

                if (GameController->on_server){
                    //Send build packet
                    SendBlockPlacePacket(GameController->server_socket, block->x, block->y, block->z, r, g, b, type);
                }

                delete block;
                Sleep(100);
            }

        }
        else if (GameController->M3 > (uint8_t)0){
            Block* block = GameController->GetBlockAtCrosshair(40.0, false);
            if (block != (Block*)nullptr){
                current_block_color.r = block->color.r;
                current_block_color.g = block->color.g;
                current_block_color.b = block->color.b;
                current_block_color.type = block->color.type;
                PrintSelectBlockMessage(block->color.r, block->color.g, block->color.b, block->color.type);
                delete block;
                Sleep(250);
            }

        }
    }
}

__stdcall bool no_shenanigans HandleMessage(wchar_t msg[], unsigned int msg_size){
    unsigned char r = 255;
    unsigned char g = 255;
    unsigned char b = 255;

    if ( !wcscmp(msg, L"/build")){
        currently_building = !currently_building;
        wchar_t response[256];
        if (currently_building){
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ControlsChecker, 0, 0, NULL);
            GameController->PrintMessage(L"<Building Mode>\n");
        }
        else {
            GameController->PrintMessage(L"<Battle Mode>\n");
        }

        return true;
    }
    else if ( swscanf(msg, L"/build color %d %d %d", &r, &g, &b) == 3){
        current_block_color.r = r;
        current_block_color.g = g;
        current_block_color.b = b;
        current_block_color.type = 1; //solid
        PrintSelectBlockMessage(r, g, b, current_block_color.type);
        return true;
    }
    else if ( !wcscmp(msg, L"/build wet")){
        current_block_color.type = 3; //wet
        PrintSelectBlockMessage(current_block_color.r, current_block_color.g, current_block_color.b, current_block_color.type);
        return true;
    }
    else if ( !wcscmp(msg, L"/build water")){
        current_block_color.type = 2; //water
        current_block_color.r = 255;
        current_block_color.g = 255;
        current_block_color.b = 255;
        PrintSelectBlockMessage(current_block_color.r, current_block_color.g, current_block_color.b, current_block_color.type);
        return true;
    }
    else if ( !wcscmp(msg, L"/build dry")){
        current_block_color.type = 1; //solid
        PrintSelectBlockMessage(current_block_color.r, current_block_color.g, current_block_color.b, current_block_color.type);
        return true;
    }
    else if ( !wcscmp(msg, L"/build help") ){
        PrintHelpMessage();
    return true;
    }


    return false;
}

__stdcall void no_shenanigans HandleZoneLoaded(cube::Zone* zone){
    if (GameController->on_server){
        SendZoneLoadPacket(GameController->server_socket, zone->x, zone->y);
    }

    //Get a block vector. It will either be from an existing Zone, a Zone newly created from a file, or an empty vector
    std::vector<ZoneSaver::ZoneBlock*> blocks = worldContainer.LoadZoneBlocks(GameController->world.worldName, zone->x, zone->y);

    //Place blocks in the world and update them
    for (ZoneSaver::ZoneBlock* block : blocks){
        GameController->world.SetBlock(block->x, block->y, block->z, block->r, block->g, block->b, block->type);
        UpdateChunkAndAdjacent(block->x, block->y);
    }
}

__stdcall void no_shenanigans HandleZoneDelete(cube::Zone* zone){
    if (GameController->on_server){
        SendZoneUnloadPacket(GameController->server_socket, zone->x, zone->y);
    }
    worldContainer.DeleteZoneContainer(zone->x, zone->y);
}

__stdcall int no_shenanigans HandleDodgeAttemptCheck(int attempting_to_dodge){
    if (currently_building){
        return -1; //disable
    }
    return 0; //normal
}

__stdcall int no_shenanigans HandlePrimaryAttackAttemptCheck(int attempting_to_attack){
    if (currently_building){
        return -1; //disable
    }
    return 0; //normal
}

__stdcall int no_shenanigans HandleAbilityAttackAttemptCheck(int attempting_to_attack, unsigned int keyNumber){
    if (currently_building){
        return -1; //disable
    }
    return 0; //normal
}

DWORD WINAPI no_shenanigans RegisterCallbacks(){

        RegisterCallback("RegisterChatEventCallback", HandleMessage);
        RegisterCallback("RegisterZoneLoadedCallback", HandleZoneLoaded);
        RegisterCallback("RegisterZoneDeleteCallback", HandleZoneDelete);
        RegisterCallback("RegisterDodgeAttemptCheckCallback", HandleDodgeAttemptCheck);
        RegisterCallback("RegisterPrimaryAttackAttemptCheckCallback", HandlePrimaryAttackAttemptCheck);
        RegisterCallback("RegisterAbilityAttackAttemptCheckCallback", HandleAbilityAttackAttemptCheck);

        RegisterCallback("RegisterPacketCallback", HandlePacket);
        RegisterCallback("RegisterReadyToSendCallback", HandleReadyToSend);

        return 0;
}




extern "C" no_shenanigans BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    base = (UINT_PTR)GetModuleHandle(NULL);
    cube::SetBase(base);
    GameController = cube::GetGameController();

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            PacketQueueInit();
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RegisterCallbacks, 0, 0, NULL);
            break;
    }
    return TRUE;
}
