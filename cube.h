#ifndef CUBE_H
#define CUBE_H
#define no_shenanigans __attribute__((noinline)) __declspec(dllexport)

unsigned int imageBase = 0x400000;



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
    char r, g, b, type;
    BlockColor(char _r, char _g, char _b, char _t){
        this->r = _r;
        this->g = _g;
        this->b = _b;
        this->type = _t;
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
        void SetBlock(unsigned int x, unsigned int y, int z, char r, char g, char b, char type){
            BlockColor* color = new BlockColor(r, g, b, type);
            this->SetBlock(x, y, z, color, (cube::Zone*)nullptr);
            delete color;
        }

    };
    class GameController{
    public:
        char padding0[0x2DC];
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

    };

    class Creature{
    public:
        char padding0[0x10];
        long long unsigned int x, y, z;
        char padding1[0x138];
        Vector3_Float camera_offset;

    };

    class Zone{
    public:
        char padding0[0x60];
        unsigned int x;
        unsigned int y;
    };


}

#endif
