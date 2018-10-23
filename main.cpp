#undef __STRICT_ANSI__
#include <windows.h>
#include <iostream>
#include "cube.h"
#include "zonesaver.h"
#include <vector>

#define no_shenanigans __attribute__((noinline)) __declspec(dllexport)

UINT_PTR base;



class Color{
    public:
    float red, green, blue, alpha;
    Color(float r, float g, float b, float a){
        red = r;
        green = g;
        blue = b;
        alpha = a;
    }

};
Color defaultColor = Color(1.0, 1.0, 1.0, 1.0);
DWORD defaultColorPtr = (DWORD)&defaultColor;

wchar_t defaultMessage[1024];
DWORD defaultMessagePtr = (DWORD)&defaultMessage;

char msgObject[255];
DWORD msgObjectPtr = (DWORD)&msgObject;

void no_shenanigans ASMPrintMessage(){

    asm("push [_defaultMessagePtr]");
    asm("mov ecx, [_msgObjectPtr]");

    asm("mov eax, [_base]");
    asm("add eax, 0x0EB60");
    asm("call eax"); //call some message constructing function

    asm("mov ecx, [_base]");
    asm("add ecx, 0x36B1C8");
    asm("mov ecx, [ecx]"); //ecx points to gamecontroller
    asm("mov ecx, [ecx + 0x800A14]"); //ecx points to chatwidget

    asm("push [_defaultColorPtr]");
    asm("push [_msgObjectPtr]");
    asm("mov edx, [_base]");
    asm("add edx, 0x3AB30");
    asm("call edx"); //prints message


    asm("mov ecx, [_msgObjectPtr]");

    asm("mov eax, [_base]");
    asm("add eax, 0x193E50");
    asm("call eax"); //destructor for that message object

}

void no_shenanigans PrintMessage(wchar_t message[]){

    wcsncpy(defaultMessage, message, 255);
    defaultColor.red = 1.0;
    defaultColor.blue = 1.0;
    defaultColor.green = 1.0;
    defaultColor.alpha = 1.0;
    ASMPrintMessage();
}
void no_shenanigans PrintMessage(wchar_t message[], int r, int g, int b){
    wcsncpy(defaultMessage, message, 255);
    defaultColor.red = r / 255.0;
    defaultColor.green = g / 255.0;
    defaultColor.blue = b / 255.0;
    ASMPrintMessage();
}





cube::GameController* GameController;

unsigned int World_ptr;
unsigned int SetBlockInZone_ptr;

unsigned int SetBlockX;
unsigned int SetBlockY;
unsigned int SetBlockZ;


DWORD color_ptr = (DWORD)NULL;

ZoneSaver::WorldContainer worldContainer;


void __stdcall no_shenanigans ASMSetBlocks(){
    asm("mov ecx, [_World_ptr]");

    asm("xor eax, eax"); //pass NULL as the zone and let it figure it out
    asm("push eax");
    asm("push [_color_ptr]");
    asm("push [_SetBlockZ]");
    asm("push [_SetBlockY]");
    asm("push [_SetBlockX]");
    asm("call [_SetBlockInZone_ptr]");

    asm("ret");
}



void no_shenanigans SetBlock(unsigned int blockX, unsigned int blockY, unsigned int blockZ, char r, char g, char b, char type){
    BlockColor color(r, g, b, type);
    color_ptr = (DWORD)&color;

    SetBlockX = blockX;
    SetBlockY = blockY;
    SetBlockZ = blockZ;
    ASMSetBlocks();
}

void no_shenanigans WriteJMP(BYTE* location, BYTE* newFunction){
	DWORD dwOldProtection;
	VirtualProtect(location, 5, PAGE_EXECUTE_READWRITE, &dwOldProtection);
    location[0] = 0xE9; //jmp
    *((DWORD*)(location + 1)) = (DWORD)(( (unsigned INT32)newFunction - (unsigned INT32)location ) - 5);
	VirtualProtect(location, 5, dwOldProtection, &dwOldProtection);
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

        SetBlock(blockx, blocky, blockz, (char)r, (char)g, (char)b, (char)type);
        GameController->UpdateChunk(chunkx, chunky);

        //save everything
        worldContainer.SetBlock(blockx, blocky, blockz, (char)r, (char)g, (char)b, (char)type);
        worldContainer.OutputFiles(GameController->world.worldName);

        return true;
    }
    return false;
}

__stdcall void no_shenanigans HandleChunkLoaded(unsigned int zone_ptr){
    wchar_t msg[1024];
    cube::Zone* zone = (cube::Zone*)(zone_ptr);

    //Get a block vector. It will either be from an existing Zone, a Zone newly created from a file, or an empty vector
    std::vector<ZoneSaver::ZoneBlock*> blocks = worldContainer.LoadZoneBlocks(GameController->world.worldName, zone->x, zone->y);

//    if (blocks.size() > 0){
//        swprintf(msg, L"A cube::Zone has been loaded. X: %u, Y: %u\n", zone->x, zone->y);
//        PrintMessage(msg);
//
//        wchar_t msg2[1024];
//        swprintf(msg2, L"Vector size: %d\n", blocks.size());
//        PrintMessage(msg2);
//
//        for (ZoneSaver::ZoneBlock* block : blocks){
//            wchar_t msg3[1024];
//            swprintf(msg3, L"X: %u, Y: %u, Z: %u\n", block->x, block->y, block->z);
//            PrintMessage(msg3);
//
//        }
//    }

    //Place blocks in the world and update them
    for (ZoneSaver::ZoneBlock* block : blocks){
        SetBlock(block->x, block->y, block->z, block->r, block->g, block->b, block->type);
        unsigned int chunkx = block->x / 0x200000;
        unsigned int chunky = block->y / 0x200000;
        GameController->UpdateChunk(chunkx, chunky);
    }
}


DWORD WINAPI no_shenanigans RegisterCallbacks(){
        HMODULE modManagerDLL = LoadLibraryA("CallbackManager.dll");

        typedef bool (__stdcall *ChatEventCallback)(wchar_t buf[], unsigned int msg_size);
        typedef void (*RegisterChatEventCallback_t)(ChatEventCallback cb);
        auto RegisterChatEventCallback = (RegisterChatEventCallback_t)GetProcAddress(modManagerDLL, "RegisterChatEventCallback");
        RegisterChatEventCallback((ChatEventCallback)HandleMessage);

        typedef bool (__stdcall *ChunkLoadedCallback)(unsigned int zone_ptr);
        typedef void (*RegisterChunkLoadedCallback_t)(ChunkLoadedCallback cb);
        auto RegisterChunkLoadedCallback = (RegisterChunkLoadedCallback_t)GetProcAddress(modManagerDLL, "RegisterChunkLoadedCallback");
        RegisterChunkLoadedCallback((ChunkLoadedCallback)HandleChunkLoaded);

        return 0;
}

extern "C" no_shenanigans BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    base = (UINT_PTR)GetModuleHandle(NULL);
    unsigned int GameController_ptr = *(DWORD*)(base + 0x36B1C8);
    GameController = (cube::GameController*)GameController_ptr;
    World_ptr = (unsigned int)&(GameController->world);//GameController_ptr + 0x2E4;

    SetBlockInZone_ptr = base + 0x4E7A0;

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RegisterCallbacks, 0, 0, NULL);
            break;
    }
    return TRUE;
}
