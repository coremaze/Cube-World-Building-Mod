#ifndef CUBE_H
#define CUBE_H
#define no_shenanigans __attribute__((noinline)) __declspec(dllexport)
#include <math.h>

unsigned int imageBase = 0x400000;

float degrees_to_radians(float degrees){
    return (degrees * 3.1415926535) / 180.0;
}
float radians_to_degrees(float radians){
    return (radians * 180.0) / 3.1415926535;
}

//Some classes used by cube
class Vector3_Int64{
    public:
        long long signed int x, y, z;
        Vector3_Int64(){
        x = 0;
        y = 0;
        z = 0;
        }
        Vector3_Int64(long long signed int _x, long long signed int _y, long long signed int _z){
        x = _x;
        y = _y;
        z = _z;
    }
};
class Vector3_Float{
    public:
        float x, y, z;
        Vector3_Float(){
        x = 0.0;
        y = 0.0;
        z = 0.0;
        }
        Vector3_Float(float _x, float _y, float _z){
        x = _x;
        y = _y;
        z = _z;
    }
};
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
class Block{
public:
    unsigned int x, y;
    int z;
    BlockColor color = BlockColor(0,0,0,0);
    Block(unsigned int _x, unsigned int _y, int _z, char _r, char _g, char _b, char _t){
        this->x = _x;
        this->y = _y;
        this->z = _z;
        this->color.r = _r;
        this->color.g = _g;
        this->color.b = _b;
        this->color.type = _t;
    }
};


Color ASMPrintMessage_defaultColor = Color(1.0, 1.0, 1.0, 1.0);
DWORD ASMPrintMessage_defaultColorPtr = (DWORD)&ASMPrintMessage_defaultColor;

wchar_t ASMPrintMessage_defaultMessage[1024];
DWORD ASMPrintMessage_defaultMessagePtr = (DWORD)&ASMPrintMessage_defaultMessage;

char ASMPrintMessage_msgObject[255];
DWORD ASMPrintMessage_msgObjectPtr = (DWORD)&ASMPrintMessage_msgObject;

//Cube world classes
namespace cube{
    void SetBase(unsigned int base){
        imageBase = base;
    }
    class Zone;

    class Chunk{
    public:
       char padding0[0x18];
       unsigned int x;
       unsigned int y;
       char padding2[0x54];
       uint8_t needs_update;
       char padding1[0x1F3];
    };
    class World{
    public:
        char padding0[0x94];
        char worldName[0x10];

        void SetBlock(unsigned int x, unsigned int y, int z, BlockColor* color, Zone* zone){
            typedef void(__thiscall* cube_World_SetBlockInZone_t)(cube::World*, unsigned int, unsigned int, int, BlockColor*, cube::Zone*);
            auto cube_World_SetBlockInZone = (cube_World_SetBlockInZone_t)(imageBase + 0x4E7A0);
            cube_World_SetBlockInZone(this, x, y, z, color, zone);
        }
        void SetBlock(unsigned int x, unsigned int y, int z, unsigned char r, unsigned char g, unsigned char b, unsigned char type){
            BlockColor* color = new BlockColor(r, g, b, type);
            this->SetBlock(x, y, z, color, (cube::Zone*)nullptr);
            delete color;
        }

        BlockColor* GetBlock(unsigned int x, unsigned int y, int z, Zone* zone){
            typedef BlockColor*(__thiscall* cube_World_GetBlock_t)(cube::World*, unsigned int, unsigned int, int, cube::Zone*);
            auto cube_World_GetBlock = (cube_World_GetBlock_t)(imageBase + 0x2F7E0);
            BlockColor* color = cube_World_GetBlock(this, x, y, z, zone);
            return color;
        }



    };

    class Creature{
    public:
        char padding0[0x10];
        long long unsigned int x, y, z; //0x10 ~ 0x27
        char padding2[0x60];
        float physical_size; //0x88
        char padding1[0xD4];
        Vector3_Float camera_offset;

    };

    class GameController{
    public:
        unsigned int field_0; //0x0
        uint8_t M1; //0x4
        uint8_t M2; //0x5
        uint8_t key_1; //0x6
        uint8_t key_2; //0x7
        uint8_t key_3; //0x8
        uint8_t key_4; //0x9
        uint8_t M3; //0xA
        uint8_t key_W; //0xB
        uint8_t key_S; //0xC
        uint8_t key_A; //0xD
        uint8_t key_D; //0xE
        uint8_t key_R; //0xF
        uint8_t key_T; //0x10
        uint8_t key_E; //0x11
        uint8_t key_Space; //0x12
        uint8_t key_Ctrl; //0x13
        uint8_t key_LeftShift; //0x14
        uint8_t key_LeftShift2; //0x15
        uint8_t field_16; //0x16
        uint8_t key_Q; //0x17
        uint8_t field_18;
        uint8_t field_19;
        char padding0[0x156];
        unsigned int isFullscreen; //0x170
        unsigned int width; //0x174
        unsigned int height; //0x178
        unsigned int antialiasingLevel; //0x17C
        unsigned int renderDistance; //0x180
        unsigned int soundFXVolume; //0x184
        unsigned int musicVolume; //0x188
        float cameraSpeed; //0x18C unsure about these types, didn't check
        float cameraSmoothness; //0x190
        unsigned int invertYAxis; //0x193
        unsigned int language; //0x198
        unsigned int miliseconds_per_frame; //0x19C
        unsigned int shutdown; //0x1A0
        float cameraPitch; //0x1A4
        float cameraRoll; //0x1A8
        float cameraYaw; //0x1AC
        float destinationCameraPitch; //0x1B0
        float destinationCameraRoll; //0x1B4
        float destinationCameraYaw; //0x1B8
        float cameraZoom; //0x1BC looks like distance in blocks
        float destinationCameraZoom; //0x1C0
        char padding1[0x114];
        unsigned int entitylistptr; //0x2D8
        unsigned int chunk_array_dimensions; //0x2DC
        Chunk* chunks; //0x2E0 Pointer to an array of chunks
        World world;

        void UpdateChunk(unsigned int chunk_x, unsigned int chunk_y){
            //the dimensions are stored as a side length. Must square to get total number of chunks.
            int chunk_count = this->chunk_array_dimensions;
            chunk_count = chunk_count * chunk_count;

            //Find the right chunk in the chunk array
            for(int i = 0; i < chunk_count ;i++){
                cube::Chunk* chunk = &this->chunks[i];
                if (chunk->x == chunk_x && chunk->y == chunk_y){
                    //Set a flag to tell the game to update the chunk.
                    chunk->needs_update = true;
                    return;
                }
            }
        }

        //Don't know enough about GameController to properly write this, but printing is important
        void no_shenanigans ASMPrintMessage(){
            asm("push [_ASMPrintMessage_defaultMessagePtr]");
            asm("mov ecx, [_ASMPrintMessage_msgObjectPtr]");

            asm("mov eax, [_imageBase]");
            asm("add eax, 0x0EB60");
            asm("call eax"); //call some message constructing function

            asm("mov ecx, [_imageBase]");
            asm("add ecx, 0x36B1C8");
            asm("mov ecx, [ecx]"); //ecx points to gamecontroller
            asm("mov ecx, [ecx + 0x800A14]"); //ecx points to chatwidget

            asm("push [_ASMPrintMessage_defaultColorPtr]");
            asm("push [_ASMPrintMessage_msgObjectPtr]");
            asm("mov edx, [_imageBase]");
            asm("add edx, 0x3AB30");
            asm("call edx"); //prints message


            asm("mov ecx, [_ASMPrintMessage_msgObjectPtr]");

            asm("mov eax, [_imageBase]");
            asm("add eax, 0x193E50");
            asm("call eax"); //destructor for that message object
            asm("ret");
        }
        void no_shenanigans PrintMessage(wchar_t message[]){
            wcsncpy(ASMPrintMessage_defaultMessage, message, 255);
            ASMPrintMessage_defaultColor.red = 1.0;
            ASMPrintMessage_defaultColor.blue = 1.0;
            ASMPrintMessage_defaultColor.green = 1.0;
            ASMPrintMessage_defaultColor.alpha = 1.0;
            this->ASMPrintMessage();
        }
        void no_shenanigans PrintMessage(wchar_t message[], int r, int g, int b){
            wcsncpy(ASMPrintMessage_defaultMessage, message, 255);
            ASMPrintMessage_defaultColor.red = r / 255.0;
            ASMPrintMessage_defaultColor.green = g / 255.0;
            ASMPrintMessage_defaultColor.blue = b / 255.0;
            this->ASMPrintMessage();
        }

        //This is definitely not the proper implementation.
        //Fix this later once it's better understood.
        Creature* no_shenanigans GetLocalPlayer(){
            DWORD entityaddr = (DWORD)this;
            entityaddr += 0x39C;
            entityaddr = *(DWORD*)entityaddr;
            Creature* player = (Creature*)entityaddr;
            return player;
        }

        Block* no_shenanigans GetBlockAtCrosshair(float reach_in_blocks_from_camera, bool want_face_block){
            Creature* player = this->GetLocalPlayer();
            //Calculate the direction the camera is in
            float yaw = degrees_to_radians(this->cameraYaw + 90.0);
            float pitch = degrees_to_radians(this->cameraPitch);

            float camera_x_direction = cos(yaw) * sin(pitch);
            float camera_y_direction = sin(yaw) * sin(pitch);
            float camera_z_direction = -cos(pitch);

            //Calculate where the camera is, relative to the player
            float blocks_away_from_player_x = camera_x_direction * this->cameraZoom;
            float blocks_away_from_player_y = camera_y_direction * this->cameraZoom;
            float blocks_away_from_player_z = camera_z_direction * this->cameraZoom;

            //Get the absolute location of the camera, in world units (not blocks)
            float VERTICAL_OFFSET = player->physical_size/2 + 0.5;//Calculation is ~1 block too low otherwise.
            const float SIZE_OF_BLOCK_IN_WORLD_UNITS = 65536.0;
            auto camera_x = player->x + (long long int)(blocks_away_from_player_x * SIZE_OF_BLOCK_IN_WORLD_UNITS);
            auto camera_y = player->y + (long long int)(blocks_away_from_player_y * SIZE_OF_BLOCK_IN_WORLD_UNITS);
            auto camera_z = player->z + (long long int)((blocks_away_from_player_z + VERTICAL_OFFSET) * SIZE_OF_BLOCK_IN_WORLD_UNITS);

            //We're going to start the raycast at the current position of the camera.
            unsigned int blockx = camera_x / 0x10000;
            unsigned int blocky = camera_y / 0x10000;
            unsigned int blockz = camera_z / 0x10000;

            //These are for the case that you want the block on the face of whatever it hits.
            unsigned int lastblockx = blockx;
            unsigned int lastblocky = blocky;
            unsigned int lastblockz = blockz;

            float reach_limit = SIZE_OF_BLOCK_IN_WORLD_UNITS * reach_in_blocks_from_camera; //maximum reach in world units
            float raycast_precision = 1000.0;
            bool withinReach = false;

            //Move the position back in the opposite direction until it hits a block.
            for (float world_units_traveled = 0.0; world_units_traveled < reach_limit; world_units_traveled += raycast_precision){
                //Update previous block before changing the current block
                lastblockx = blockx;
                lastblocky = blocky;
                lastblockz = blockz;
                blockx = camera_x / 0x10000;
                blocky = camera_y / 0x10000;
                blockz = camera_z / 0x10000;
                //Figure out what's in the new location
                BlockColor* color = this->world.GetBlock(blockx, blocky, blockz, (cube::Zone*)nullptr);
                if ((color->type & 0b00111111) != 0){ //If the block is not an air block
                    withinReach = true;
                    if (want_face_block){
                        //If you want the face block, change the block coords to whatever was seen last time. ie, go back one.
                        blockx = lastblockx;
                        blocky = lastblocky;
                        blockz = lastblockz;
                    }
                    break; //Leave loop if successfully hit a block
                }
                //Move the position back a tiny bit
                camera_x -= (long long int)(camera_x_direction * raycast_precision);
                camera_y -= (long long int)(camera_y_direction * raycast_precision);
                camera_z -= (long long int)(camera_z_direction * raycast_precision);
            }

            //We've got a block coordinate now, whether it be the block we were looking at, the block on the face of another, or the block at the edge of our reach.
            if (withinReach){
                BlockColor* color = this->world.GetBlock(blockx, blocky, blockz, (cube::Zone*)nullptr);
                Block* block = new Block(blockx, blocky, blockz, color->r, color->g, color->b, color->type);
                return block;
            }
            else {
                Block* block = (Block*)nullptr;
                return block;
            }

        }

    };

    GameController* GetGameController(){
        return (GameController*) *(DWORD*)(imageBase + 0x36B1C8);
    }



    class Zone{
    public:
        char padding0[0x60];
        unsigned int x;
        unsigned int y;
    };


}

#endif
