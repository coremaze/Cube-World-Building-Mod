#ifndef CUBE_H
#define CUBE_H
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

namespace cube{
    class World{
    public:

    };
    class GameController{
    public:
        char padding0[0x2DC];
        unsigned int chunk_array_dimensions;
        unsigned int chunk_array_ptr;
        World world;
    };
    class Chunk{
    public:
       char padding0[0x18];
       unsigned int x;
       unsigned int y;
       char padding2[0x54];
       bool update;
       char padding1[0x1F3];
    };
    class Creature{
    public:
        char padding0[0x10];
        long long unsigned int x, y, z;
        char padding1[0x138];
        Vector3_Float camera_offset;

    };


}

#endif
