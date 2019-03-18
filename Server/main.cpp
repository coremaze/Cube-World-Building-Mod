#undef __STRICT_ANSI__
#define no_shenanigans __attribute__((noinline)) __declspec(dllexport)
#include <windows.h>
#include <iostream>
#include "zlib.h"
#include "callbacks.h"
#include "packets.h"
#include "zonesaver.h"

class BlockColor{
public:
    unsigned char r, g, b, type;
    BlockColor(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _t){
        this->r = _r;
        this->g = _g;
        this->b = _b;
        this->type = _t;
    }
};

unsigned int base;
ZoneSaver::WorldContainer worldContainer;
unsigned int world_ptr;
char serverWorldName[256];

typedef void(__thiscall* cube_World_SetBlockInZone_t)(unsigned int, unsigned int, unsigned int, int, BlockColor*, unsigned int);
cube_World_SetBlockInZone_t cube_World_SetBlockInZone;



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

void SendBlockCompressPacket(SOCKET socket, std::vector<ZoneSaver::ZoneBlock*>* blocks){
    unsigned int num_blocks = blocks->size();
    unsigned int packet_ID = BUILDING_MOD_PACKET;
    unsigned int sub_id = BLOCK_COMPRESS_PACKET;
    unsigned int BLOCK_SIZE = 4 + 4 + 4 + 1 + 1 + 1 + 1;


    //make block data
    unsigned int block_data_size = BLOCK_SIZE * num_blocks;
    char* block_buf = new char[block_data_size];

    unsigned int i = 0;
    for (ZoneSaver::ZoneBlock* block : *blocks){
        memcpy(block_buf + i + 0,  &block->x, 4);
        memcpy(block_buf + i + 4,  &block->y, 4);
        memcpy(block_buf + i + 8,  &block->z, 4);
        memcpy(block_buf + i + 12, &block->r, 1);
        memcpy(block_buf + i + 13, &block->g, 1);
        memcpy(block_buf + i + 14, &block->b, 1);
        memcpy(block_buf + i + 15, &block->type, 1);
        i += BLOCK_SIZE;
    }

    //compress data
    unsigned int compressed_data_size = ((float)block_data_size * 1.1) + 12.0;
    char* compressed_data = new char[compressed_data_size];
    if (int err = compress(compressed_data, &compressed_data_size, block_buf, block_data_size)){
        printf("Failed to compress data: error %d\n", err);
    }


    //build packet
    unsigned int pkt_size = 4 + 4 + 4 + 4 + compressed_data_size;
    char* pkt_buf = new char[pkt_size];
    memcpy(pkt_buf    , &packet_ID, 4);
    memcpy(pkt_buf + 4, &sub_id   , 4);
    memcpy(pkt_buf + 8, &block_data_size, 4);
    memcpy(pkt_buf + 12, &compressed_data_size, 4);
    memcpy(pkt_buf + 16, compressed_data, compressed_data_size);
    AddPacket(socket, pkt_buf, pkt_size);

    delete[] compressed_data;
    delete[] block_buf;
    delete[] pkt_buf;
}

unsigned int __stdcall no_shenanigans HandlePacket(unsigned int packet_id, SOCKET socket)
{
    //Only want to deal with building mod packets.
    if (packet_id != BUILDING_MOD_PACKET)
    {
        return 0;
    }

    //Let's only keep track of all the sockets which are using the mod.
    //This will let non-modded clients connect without breaking them.
    AddSocket(socket);

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
        std::vector<ZoneSaver::ZoneBlock*> blocks = worldContainer.LoadZoneBlocks(serverWorldName, zone_x, zone_y);

        //Limit number of blocks in each packet
        std::vector<ZoneSaver::ZoneBlock*> blocks_segment;

        for (ZoneSaver::ZoneBlock* block : blocks){
            blocks_segment.push_back(block);
            if (blocks_segment.size() == 100){
                SendBlockCompressPacket(socket, &blocks_segment);
                blocks_segment.clear();
            }
        }
        if (blocks_segment.size()){
            SendBlockCompressPacket(socket, &blocks_segment);
            blocks_segment.clear();
        }



        for (ZoneSaver::ZoneBlock* block : blocks){
            BlockColor* color = new BlockColor(block->r, block->g, block->b, block->type);
            cube_World_SetBlockInZone(world_ptr, block->x, block->y, block->z, color, NULL);
            delete color;
        }

        //For some reason I have to do this twice to get visuals to stop glitching
        for (ZoneSaver::ZoneBlock* block : blocks){
            BlockColor* color = new BlockColor(block->r, block->g, block->b, block->type);
            cube_World_SetBlockInZone(world_ptr, block->x, block->y, block->z, color, NULL);
            delete color;
        }



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


        /*
        Unlike in the client, it's possible that a player could send a BLOCK_PLACE_PACKET
        without first sending a ZONE_LOAD_PACKET. If this happens without handling this,
        it could overwrite the CWB file for that entire zone.
        */
        unsigned int zone_x = x/256;
        unsigned int zone_y = y/256;
        if (!worldContainer.HasZoneContainer(zone_x, zone_y)){
            std::vector<ZoneSaver::ZoneBlock*> blocks = worldContainer.LoadZoneBlocks(serverWorldName, zone_x, zone_y);

            for (ZoneSaver::ZoneBlock* block : blocks){
                BlockColor* color = new BlockColor(block->r, block->g, block->b, block->type);
                cube_World_SetBlockInZone(world_ptr, block->x, block->y, block->z, color, NULL);
                delete color;
            }
        }


        //save everything
        worldContainer.SetBlock(x, y, z, r, g, b, type);
        worldContainer.OutputFiles(serverWorldName);

        BlockColor* color = new BlockColor(r, g, b, type);
        cube_World_SetBlockInZone(world_ptr, x, y, z, color, NULL);
        delete color;

    }

//    printf("I know about %d sockets.\n", knownSockets.size());
//    for (SOCKET s : knownSockets){
//        printf("%u, ", s);
//    }
//    printf("\n");

    return 1;
}

void __stdcall no_shenanigans HandleReadyToSend(SOCKET socket){
    SendQueuedPackets(socket);
}

void __stdcall no_shenanigans HandleWorldCreated(unsigned int w_ptr){
    world_ptr = w_ptr;
    //printf("World created: %X", world_ptr);
}

void __stdcall no_shenanigans HandlePlayerDisconnect(SOCKET socket){
    PurgeSocket(socket);
}

void __stdcall no_shenanigans HandlePlayerConnect(SOCKET socket){
    //printf("Socket %d logged in.\n", socket);
    //This is here in case a player disconnects in a strange way,
    //or if there is an exception that caused an old socket to not get purged.
    //This socket will be re-added once they send a building mod packet.
    PurgeSocket(socket);
}

DWORD WINAPI no_shenanigans RegisterCallbacks(){

        RegisterCallback("RegisterPacketCallback", HandlePacket);
        RegisterCallback("RegisterReadyToSendCallback", HandleReadyToSend);
        RegisterCallback("RegisterWorldCreatedCallback", HandleWorldCreated);
        RegisterCallback("RegisterPlayerDisconnectCallback", HandlePlayerDisconnect);
        RegisterCallback("RegisterPlayerConnectCallback", HandlePlayerConnect);

        return 0;
}

extern "C" no_shenanigans bool APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    base = (unsigned int)GetModuleHandle(NULL);
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            zlib_init();
            PacketsInit();
            InitZoneSaver();

            cube_World_SetBlockInZone = (cube_World_SetBlockInZone_t)(base + 0x1FF00);

            ZoneSaver::GetServerSaveName(serverWorldName);
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RegisterCallbacks, 0, 0, NULL);
            break;
    }
    return true;
}
