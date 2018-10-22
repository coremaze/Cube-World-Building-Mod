#undef __STRICT_ANSI__
#include <windows.h>
#include <iostream>
#include "cube.h"
#include "zonesaver.h"

#define no_shenanigans __attribute__((noinline)) __declspec(dllexport)

UINT_PTR base;

cube::GameController* GameController;

unsigned int GameController_ptr;
unsigned int World_ptr;
unsigned int GetZone_ptr;
unsigned int SetBlockInZone_ptr;

unsigned int SetBlockX;
unsigned int SetBlockY;
unsigned int SetBlockZ;

char color[4];
DWORD color_ptr = (DWORD)&color;

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


void no_shenanigans UpdateChunk(unsigned int x, unsigned int y){
    unsigned int first_chunk_ptr = GameController->chunk_array_ptr;
    unsigned int array_size = GameController->chunk_array_dimensions;
    array_size = array_size * array_size;

    unsigned int chunk_ptr = first_chunk_ptr;
    cube::Chunk* chunk = (cube::Chunk*)(chunk_ptr);
    for(unsigned int chunk_num = 0; ;chunk_num++){
        if (chunk_num >= array_size){
            return;
        }
        chunk = (cube::Chunk*)(chunk_ptr);

        if (chunk->x == x && chunk->y == y){
            chunk->update = true;
            return;
        }

        chunk_ptr += 0x268;
    }


}
void no_shenanigans SetBlock(unsigned int blockX, unsigned int blockY, unsigned int blockZ, char r, char g, char b, char type){
    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = type;

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
        UpdateChunk(chunkx, chunky);

        //save everything
        worldContainer.SetBlock(blockx, blocky, blockz, (char)r, (char)g, (char)b, (char)type);
        worldContainer.OutputFiles();

        return true;
    }
    return false;
}

typedef bool (__stdcall *ChatEventCallback)(wchar_t buf[], unsigned int msg_size);
typedef void (*RegisterChatEventCallback_t)(ChatEventCallback cb);
DWORD WINAPI no_shenanigans RegisterCallbacks(){
        HMODULE modManagerDLL = LoadLibraryA("CallbackManager.dll");
        auto RegisterChatEventCallback = (RegisterChatEventCallback_t)GetProcAddress(modManagerDLL, "RegisterChatEventCallback");
        RegisterChatEventCallback((ChatEventCallback)HandleMessage);
        return 0;
}

extern "C" no_shenanigans BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    base = (UINT_PTR)GetModuleHandle(NULL);
    GameController_ptr = *(DWORD*)(base + 0x36B1C8);
    GameController = (cube::GameController*)GameController_ptr;
    World_ptr = (unsigned int)&(GameController->world);//GameController_ptr + 0x2E4;
    GetZone_ptr = base + 0x34D10;
    SetBlockInZone_ptr = base + 0x4E7A0;

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RegisterCallbacks, 0, 0, NULL);
            break;
    }
    return TRUE;
}
