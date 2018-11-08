#undef __STRICT_ANSI__
#define no_shenanigans __attribute__((noinline)) __declspec(dllexport)
#include <windows.h>
#include <iostream>
#include "callbacks.h"
#include "packets.h"
#include "zonesaver.h"

unsigned int base;
ZoneSaver::WorldContainer worldContainer;

const unsigned int BUILDING_MOD_PACKET = 1263488066; //hehe
const unsigned int ZONE_LOAD_PACKET = 1;
const unsigned int ZONE_UNLOAD_PACKET = 2;
const unsigned int BLOCK_PLACE_PACKET = 3;
const unsigned int BLOCK_COMPRESS_PACKET = 4;

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

unsigned int __stdcall no_shenanigans HandlePacket(unsigned int packet_id, SOCKET socket)
{
    AddSocket(socket);

    //Only want to deal with building mod packets.
    if (packet_id != BUILDING_MOD_PACKET)
    {
        return 0;
    }

    //All packets for this mod shall have the same initial packet ID,
    //however, immediately after they will have another ID to distinguish themselves.
    unsigned int sub_id;
    recv(socket, (char*)&sub_id, 4, 0);
    printf("Sub ID: %d - ", sub_id);

    if (sub_id == ZONE_LOAD_PACKET)
    {
        /*
        Spec:
        <4 bytes packet id>
        <4 bytes sub id>
        <4 bytes zone x>
        <4 bytes zone y>
        */
        unsigned int zone_x;
        unsigned int zone_y;
        recv(socket, (char*)&zone_x, 4, 0);
        recv(socket, (char*)&zone_y, 4, 0);
        printf("A player loaded Zone (%d, %d), socket %d\n", zone_x, zone_y, socket);

        //Get a block vector. It will either be from an existing Zone, a Zone newly created from a file, or an empty vector
//        std::vector<ZoneSaver::ZoneBlock*> blocks = worldContainer.LoadZoneBlocks("SERVER", zone_x, zone_y);
//
//        //Change this to a BLOCK_COMPRESS_PACKET
//        for (ZoneSaver::ZoneBlock* block : blocks){
//            SendBlockPlacePacket(socket, block->x, block->y, block->z, block->r, block->g, block->b, block->type);
//        }

    }

    else if (sub_id == ZONE_UNLOAD_PACKET)
    {
        /*
        Spec:
        <4 bytes packet id>
        <4 bytes sub id>
        <4 bytes zone x>
        <4 bytes zone y>
        */
        unsigned int zone_x;
        unsigned int zone_y;
        recv(socket, (char*)&zone_x, 4, 0);
        recv(socket, (char*)&zone_y, 4, 0);
        printf("A player unloaded Zone (%d, %d)\n", zone_x, zone_y);
    }

    else if (sub_id == BLOCK_PLACE_PACKET)
    {
        /*
        Spec:
        <4 bytes packet id>
        <4 bytes sub id>
        <4 bytes block x>
        <4 bytes block y>
        <4 bytes block z>
        <1 byte red>
        <1 byte green>
        <1 byte blue>
        <1 byte type>
        */
        unsigned int x;
        unsigned int y;
        int z;
        unsigned char r, g, b, type;
        recv(socket, (char*)&x, 4, 0);
        recv(socket, (char*)&y, 4, 0);
        recv(socket, (char*)&z, 4, 0);
        recv(socket, (char*)&r, 1, 0);
        recv(socket, (char*)&g, 1, 0);
        recv(socket, (char*)&b, 1, 0);
        recv(socket, (char*)&type, 1, 0);

        printf("A player placed a block at (%d, %d, %d) of type %u %u %u %u\n",
               x, y, z, (unsigned int)r, (unsigned int)g, (unsigned int)b, (unsigned int)type);

        for (SOCKET s : knownSockets){
            SendBlockPlacePacket(s, x, y, z, r, g, b, type);

        }

        //save everything
        worldContainer.SetBlock(x, y, z, r, g, b, type);
        worldContainer.OutputFiles("SERVER");

    }

    printf("I know about %d sockets.\n", knownSockets.size());
    for (SOCKET s : knownSockets){
        printf("%u, ", s);
    }
    printf("\n");

    return 1;
}

void __stdcall no_shenanigans HandleReadyToSend(SOCKET socket){
    SendQueuedPackets(socket);
}


DWORD WINAPI no_shenanigans RegisterCallbacks(){

        RegisterCallback("RegisterPacketCallback", HandlePacket);
        RegisterCallback("RegisterReadyToSendCallback", HandleReadyToSend);

        return 0;
}

extern "C" no_shenanigans bool APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    base = (unsigned int)GetModuleHandle(NULL);

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            PacketsInit();
            InitZoneSaver();
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RegisterCallbacks, 0, 0, NULL);
            break;
    }
    return true;
}
