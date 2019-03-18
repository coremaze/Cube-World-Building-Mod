#ifndef ZLIB_CW_H
#define ZLIB_CW_H
#include <windows.h>
#include <iostream>
typedef int __cdecl (*zlibf_t)( ... );
zlibf_t compress;
zlibf_t uncompress;
void zlib_init(){

    HINSTANCE zlib = LoadLibraryA("zlib1.dll");
    if (!zlib){
        printf("Unable to load zlib1.dll.\n");
        ExitProcess(1);
    }
    compress = (zlibf_t)GetProcAddress(zlib, "compress");
    uncompress = (zlibf_t)GetProcAddress(zlib, "uncompress");
}

#endif
