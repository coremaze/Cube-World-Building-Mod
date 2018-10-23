#ifndef CUBE_H
#define CUBE_H
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

//Cube world classes
namespace cube{
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
