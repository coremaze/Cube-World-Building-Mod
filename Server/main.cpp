#undef __STRICT_ANSI__
#define no_shenanigans __attribute__((noinline)) __declspec(dllexport)
#include <windows.h>
#include <iostream>
#include "callbacks.h"
#include "packets.h"
unsigned int base;


const unsigned int BUILDING_MOD_PACKET = 1263488066; //hehe
const unsigned int ZONE_LOAD_PACKET = 1;
const unsigned int ZONE_UNLOAD_PACKET = 2;
const unsigned int BLOCK_PLACE_PACKET = 3;

unsigned int __stdcall no_shenanigans HandlePacket(unsigned int packet_id, SOCKET socket){

//    if (packet_id == 10){
//        printf("Packet ID: %d\n", packet_id);
//        char buf[4] = { 0 };
//        memcpy(buf, &BUILDING_MOD_PACKET, 4);
//        AddPacket(socket, buf, 4);
//    }

    if (packet_id == BUILDING_MOD_PACKET){
        unsigned int sub_id;
        recv(socket, (char*)&sub_id, 4, 0);
        printf("Sub ID: %d\n", sub_id);

        if (sub_id == ZONE_LOAD_PACKET){
            unsigned int zone_x;
            unsigned int zone_y;
            recv(socket, (char*)&zone_x, 4, 0);
            recv(socket, (char*)&zone_y, 4, 0);
            printf("A player loaded Zone (%d, %d)\n", zone_x, zone_y);
        }

        else if (sub_id == ZONE_UNLOAD_PACKET){
            unsigned int zone_x;
            unsigned int zone_y;
            recv(socket, (char*)&zone_x, 4, 0);
            recv(socket, (char*)&zone_y, 4, 0);
            printf("A player unloaded Zone (%d, %d)\n", zone_x, zone_y);
        }

        else if (sub_id == BLOCK_PLACE_PACKET){
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

            printf("A player placed a block at (%d, %d, %d) of type ", x, y, z);
            //printf("of type %u %u %u %u\n", (unsigned int)r, (unsigned int)g, (unsigned int)b, (unsigned int)type);
            std::cout << (unsigned int)r << " " << (unsigned int)g << " " << (unsigned int)b << " " << (unsigned int)type << "\n";

        }

        return 1;


    }


    return 0;
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
            PacketQueueInit();
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RegisterCallbacks, 0, 0, NULL);
            break;
    }
    return true;
}
