#ifndef ZONESAVER_H
#define ZONESAVER_H
#include <vector>
#include <fstream>
#include <iostream>
#include <windows.h>
#include <algorithm>

CRITICAL_SECTION zone_saver_critical_section;
void InitZoneSaver(){
    InitializeCriticalSection(&zone_saver_critical_section);
}

namespace ZoneSaver{

class ZoneBlock{
public:
    unsigned int x, y;
    int z;
    char r, g, b, type;
    ZoneBlock(unsigned int _x, unsigned int _y, int _z, char _r, char _g, char _b, char _type){
        this->x = _x;
        this->y = _y;
        this->z = _z;
        this->r = _r;
        this->g = _g;
        this->b = _b;
        this->type = _type;
    }

};


class ZoneContainer{
public:
    std::vector<ZoneBlock*> blocks;
    unsigned int zone_x, zone_y;

    ZoneContainer(unsigned int _zonex, unsigned int _zoney){
        this->zone_x = _zonex;
        this->zone_y = _zoney;
    }

    //Block coordinates should be absolute, not relative to zone
    void SetZoneBlock(unsigned int x, unsigned int y, int z, char r, char g, char b, char type){
        EnterCriticalSection(&zone_saver_critical_section);
        //Update existing block if it exists.
        for (ZoneBlock* block : this->blocks){
            if (block->x == x && block->y == y && block->z == z){
                block->r = r;
                block->g = g;
                block->b = b;
                block->type = type;
                LeaveCriticalSection(&zone_saver_critical_section);
                return;
            }
        }

        //Add a new block if does not exist
        ZoneBlock* block = new ZoneBlock(x, y, z, r, g, b, type);
        this->blocks.push_back(block);
        LeaveCriticalSection(&zone_saver_critical_section);
    }

    void OutputFile(char worldName[]){
        EnterCriticalSection(&zone_saver_critical_section);
        char fileName[256] = {0};
        char folderName[256] = { 0 };

        //Folder for all saves
        sprintf(folderName, "%s", "build_saves");
        CreateDirectory(folderName, NULL);

        //Folder for specific world
        sprintf(folderName, "%s\\%s", "build_saves", worldName);
        CreateDirectory(folderName, NULL);

        //File for this Zone
        sprintf(fileName, "%s\\%u-%u.cwb", folderName, this->zone_x, this->zone_y);
        std::ofstream file;
        file.open(fileName, std::ios::out | std::ios::binary);

        //Write each block to file
        for (ZoneBlock* block : this->blocks){
            file.write((char*)block, sizeof(*block));
        }
        file.close();
        LeaveCriticalSection(&zone_saver_critical_section);
    }
};

class WorldContainer{
public:
    std::vector<ZoneContainer*> zones;
    ZoneContainer* SetBlock(unsigned int x, unsigned int y, int z, unsigned char r, unsigned char g, unsigned char b, unsigned char type){
        unsigned int zonex = x / 0x100;
        unsigned int zoney = y / 0x100;

        //check to see if there is already a zone for this block
        for (ZoneContainer* zc : zones){
            if ( (zc->zone_x == zonex) && (zc->zone_y == zoney) ){
                zc->SetZoneBlock(x, y, z, r, g, b, type);
                return zc;
            }
        }

        //No zone created yet
        EnterCriticalSection(&zone_saver_critical_section);
        ZoneContainer* zc = new ZoneContainer(zonex, zoney);
        zc->SetZoneBlock(x, y, z, r, g, b, type);
        this->zones.push_back(zc);
        return zc;
        LeaveCriticalSection(&zone_saver_critical_section);
    }

    void OutputFiles(char worldName[]){
        for (ZoneContainer* zc : zones){
            zc->OutputFile(worldName);
        }
    }
    std::vector<ZoneBlock*> LoadZoneBlocks(char worldName[], unsigned int zone_x, unsigned int zone_y){
        char * blocks;
        std::streampos fsize;
        std::vector<ZoneBlock*> emptyBlocks;

        //First, check to see if the zone is already in memory.
        for (ZoneContainer* zc : zones){
            if ( (zc->zone_x == zone_x) && (zc->zone_y == zone_y) ){
                return zc->blocks;
            }
        }

        //If not, look for the zone file, and try to make the zone from that.
        char fileName[256] = {0};
        char folderName[256] = { 0 };

        //Folder for all saves
        sprintf(folderName, "%s", "build_saves");
        CreateDirectory(folderName, NULL);

        //Folder for specific world
        sprintf(folderName, "%s\\%s", "build_saves", worldName);
        CreateDirectory(folderName, NULL);

        //File for this Zone
        ZoneContainer *zc = nullptr;
        sprintf(fileName, "%s\\%u-%u.cwb", folderName, zone_x, zone_y);
        std::ifstream file(fileName, std::ios::in | std::ios::binary | std::ios::ate);
        if (file.is_open()){
            //File exists, read it
            fsize = file.tellg();
            blocks = new char[fsize];
            file.seekg(0, std::ios::beg);
            file.read(blocks, fsize);
            file.close();

            //SetBlock for every block found in the save file.
            //This also causes a cube::Zone to be created if it is not.
            for (int i = 0; i < fsize; i+=sizeof(ZoneBlock)){
                unsigned int x, y, z;
                unsigned char r, g, b, type;

                memcpy(&x, blocks + i + 0, 4);
                memcpy(&y, blocks + i + 4, 4);
                memcpy(&z, blocks + i + 8, 4);
                r = blocks[i+12];
                g = blocks[i+13];
                b = blocks[i+14];
                type = blocks[i+15];

                zc = this->SetBlock(x, y, z, r, g, b, type);
            }

            delete[] blocks;

            if (zc != nullptr){
                return zc->blocks;
            }
            else {
                //This will only happen if a blocks file was somehow empty.
                return emptyBlocks;
            }



        }
        else {
            //The zone file does not exist. We don't need to make
            //one until the user places a block, so return an empty vector.
            return emptyBlocks;
        }
    }

    //Deletes the first ZoneContainer with the specified coordinate, returns whether it found one to delete.
    bool DeleteZoneContainer(unsigned int zone_x, unsigned int zone_y){
        EnterCriticalSection(&zone_saver_critical_section);
        for (ZoneContainer* zone : this->zones){
            if (zone->zone_x == zone_x && zone->zone_y == zone_y){
                std::vector<ZoneContainer*>::iterator position = std::find(this->zones.begin(), this->zones.end(), zone);
                if ( position != this->zones.end() ){
                    this->zones.erase(position);
                    delete zone;
                    LeaveCriticalSection(&zone_saver_critical_section);
                    return true;
                }

            }
        }
        LeaveCriticalSection(&zone_saver_critical_section);
        return false;
    }
};

}
#endif // ZONESAVER_H
