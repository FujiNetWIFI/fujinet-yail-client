// Copyright (C) 2021 Brad Colbert

#ifndef FILES_H
#define FILES_H

#ifdef YAIL_BUILD_FILE_LOADER

#include "types.h"

#define FILETYPE_PBM 0x01
#define FILETYPE_PGM 0x02
#define FILETYPE_YAI 0x03

// Display list storage structure
struct dl_store
{
    unsigned size;
    void* mem;
};
#define MAX_N_DL 8
#define DL_TOKEN 0x01

// Display list interupt storage structure
struct dli_store
{
    unsigned size;
    void* mem;
};
#define MAX_N_DLI 8
#define DLI_TOKEN 0x02

// Screen memory storage structure
struct mem_store
{
    unsigned size;
    void* mem;
};
#define MAX_N_MEM 8
#define MEM_TOKEN 0x03

typedef struct file_header
{
    unsigned char v1;
    unsigned char v2;
    unsigned char v3;
    unsigned char gfx;
} FileHeader;


byte imageFileType(const char filename[]);
byte load_image_file(const char filename[]);
void saveFile(const char filename[]);

#endif // FILES_H

#endif // FILES_H