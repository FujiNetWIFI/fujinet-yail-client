// Copyright (C) 2021 Brad Colbert
#include "fujinet-network.h"
#include "graphics.h"
#include "netimage.h"
#include "utility.h"
#include "settings.h"
#include "types.h"
#include "vbxe.h"

#include <atari.h>
#include <conio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//
extern byte buff[];
extern ImageData image;
extern Settings settings;
extern struct __vbxe* VBXE;

//byte palette[256 * 3];

void show_error_and_close_network(const char* message)
{
    show_error(message);
    network_close(settings.url);
}

char check_keypress(ushort delay)
{
    // Wait for keypress
    char input;
    ushort i = 0;
    while(i++ < delay)   // roughly 5 seconds
        if(kbhit())
        {
            input = cgetc();
            if(CH_ENTER == input)   // pause
            {
                cgetc();            // any key to resume
                return(0x0);
            }
            else  // a key was pressed so let's assume it's a command and process it by quitting and returning the key
                show_console();
                return input;
        }

    return 0x0;
}

byte load_front_buffer()
{
    //byte status = FN_ERR_UNKNOWN;
    uint16_t num_bytes;
    
    if(image.header.v2 < 4)
    {
        #define BUFFER_ONE_BLOCK_ONE_AND_TWO_SIZE 4080
        #define BUFFER_ONE_BLOCK_THREE_SIZE 640

        uint16_t num_bytes;

        num_bytes = network_read(settings.url, (uint8_t*)image.data, BUFFER_ONE_BLOCK_ONE_AND_TWO_SIZE);
        if(0 > num_bytes)
            goto quit;

        num_bytes = network_read(settings.url, (uint8_t*)image.data+0x1000, BUFFER_ONE_BLOCK_ONE_AND_TWO_SIZE);
        if(0 > num_bytes)
            goto quit;

        num_bytes = network_read(settings.url, (uint8_t*)image.data+0x2000, BUFFER_ONE_BLOCK_THREE_SIZE);
        if(0 > num_bytes)
            goto quit;
    }
    else // VBXE
    {
        // Yail stream format > 1.3 uses more generalized block types.  The number of blocks is sent as
        // part of the header.  This will be still left in the socket since our current header structure
        // doesn't include it.
        // We will load the palette into a temporary buffer and then iterate to load into the VBXE
        BlockHeaderV14 block_header;
        uint16_t j;
        uint8_t i, num_blocks;
        
        clrscr();

        // Read the number of blocks
        num_bytes = network_read(settings.url, &num_blocks, sizeof(num_blocks));
        if(0 > num_bytes)
        {
            show_error_and_close_network("Error reading num blocks\n\r");
            return num_bytes;
        }

        //cprintf("Number of blocks: %d\n\r", num_blocks);
        VBXE->VIDEO_CONTROL = 0x03;

        // Iterate, reading blocks
        for(i = 0; i < num_blocks; ++i)
        {
            // Read the block header
            num_bytes = network_read(settings.url, (uint8_t*)&block_header, sizeof(block_header));
            if(0 > num_bytes)
            {
                show_error_and_close_network("Error reading block header\n\r");
                return num_bytes;
            }

            //cprintf("Block type: %d, size: ", block_header.block_type);
            //cprintf("%02X%02X%02X%02X\n\r", ((uint8_t*)&block_header.size)[3], ((uint8_t*)&block_header.size)[2], ((uint8_t*)&block_header.size)[1], ((uint8_t*)&block_header.size)[0]);

            switch(block_header.block_type)
            {
                case IMAGE_BLOCK:  // Image data
                    // Clear image
                    clear_vbxe();

                    // Enable the VBXE XDL.  It's fun to watch the image load.
                    VBXE->VIDEO_CONTROL = 0x03;

                    // Load the image data.  This has to be done in 4K chunks.  Loading into the memory window
                    // defined by the XDL, which in this case is 0x8000.
                    for(j = 0; j < 19; ++j)
                    {
                        uint16_t byte_to_read = 4096;
                        if(j == 18)
                            byte_to_read = 3072;
                        VBXE->MEM_BANK_SEL = 128 + j;  // MOVE WINDOW STARTING $0000
                        num_bytes = network_read(settings.url, XDL, byte_to_read);
                        if(0 > num_bytes)
                            return num_bytes;
                    }
                    break;
                case PALETTE_BLOCK:  // Palette, reuse the buff since it's already allocated
                    // Disable VBXE render
                    //VBXE->VIDEO_CONTROL = 0x00;

                    VBXE->CSEL = 0x00;
                    VBXE->PSEL = 0x01;
                    {
                        uint8_t* palette = (uint8_t*)malloc((uint16_t)block_header.size);
                        if(NULL == palette)
                        {
                            show_error_and_close_network("Error allocating palette buffer\n");
                            break;
                        }
                        num_bytes = network_read(settings.url, palette, (uint16_t)block_header.size);
                        if(0 > num_bytes)
                        {
                            show_error_and_close_network("Error reading palette\n");
                            break;
                        }
                        for(j = 0; j < (uint16_t)block_header.size; j+=3)
                        {
                            VBXE->CR = palette[j+0];
                            VBXE->CG = palette[j+1];
                            VBXE->CB = palette[j+2];
                        }
                        free(palette);
                    }
                    break;
                case XDL_BLOCK:  // XDL
                default:
                    show_error_and_close_network("Unknown block type\n\r");
                    return FN_ERR_UNKNOWN;
            }
        }
    }

quit:
    return 0;
}

byte load_back_buffer()
{
    #define BUFFER_TWO_BLOCK_ONE_SIZE 3440
    #define BUFFER_TWO_BLOCK_TWO_SIZE 4080
    #define BUFFER_TWO_BLOCK_THREE_SIZE 1280

    uint16_t num_bytes;

    num_bytes = network_read(settings.url, (uint8_t*)0x8280, BUFFER_TWO_BLOCK_ONE_SIZE);
    if(0 > num_bytes)
        goto quit;

    num_bytes = network_read(settings.url, (uint8_t*)0x9000, BUFFER_TWO_BLOCK_TWO_SIZE);
    if(0 > num_bytes)
        goto quit;

    num_bytes = network_read(settings.url, (uint8_t*)0xA000, BUFFER_TWO_BLOCK_THREE_SIZE);
    if(0 > num_bytes)
        goto quit;

quit:
    return 0;
}

char stream_image(char* args[], const byte video)
{
    ushort i = 0;
    char input;

    OS.soundr = 0; // Turn off SIO beeping sound

    #if DEBUG
    cprintf("reading from %s\n\r", url);
    for(i = 0; args[i] != 0x0; i++)
        cprintf("%s ", args[i]);
    cputs("\n\r");
    #endif

    hide_console();

    if(FN_ERR_OK != network_init())
    {
        show_error_and_close_network("Failed to initialize network\n\r");
        return 0x0;
    }

    if(FN_ERR_OK != network_open(settings.url, 12, 0))
    {
        show_error_and_close_network("Failed to open URL\n\r");
        return 0x0;
    }

    // Send which graphics mode we are in
    memset(buff, 0, 256);
    sprintf((char*)buff, "gfx %d ", settings.gfx_mode &= ~GRAPHICS_CONSOLE_EN);
    if(FN_ERR_OK != network_write(settings.url, buff, 6))
    {
        show_error_and_close_network("Unable to write graphics mode\n\r");
        return 0x0;
    }

    memset(buff, 0, 256);
    if(0 == strncmp(args[0], "http", 4))
    {
        // Build up the search string
        memcpy(buff, "showurl ", 8);
        for(i = 0; i < 8; ++i)
        {
            if(0x0 == args[i])
                break;

            if(i > 0)
                strcat(buff, " ");
            strcat(buff, args[i]);
        }
    }
    else if(args[0])
    {
        // Build up the search string
        memcpy(buff, "search", 6);
        for(i = 0; i < 8; ++i)
        {
            if(0x0 == args[i])
                break;

            if(i > 0)
                strcat((char*)buff, " ");
            else
                strcat((char*)buff, " \"");
            strcat((char*)buff, args[i]);
        }
        if(i > 0)
            strcat((char*)buff, "\"");
    }
    else if(video)
        memcpy(buff, "video", 5);
    else  // no search terms provided for the stream command so just stream files from the server (path must be set on server startup)
        memcpy(buff, "files", 5);

    i = strlen((char*)buff);

    if(FN_ERR_OK != network_write(settings.url, buff, i))
    {
        show_error_and_close_network("Unable to write request\n\r");
        return 0x0;
    }

    // We are starting streaming so remove the attract mode disable VBI because we will
    // disable attract mode ourselves.
    remove_attract_disable_vbi();

    while(true)
    {
        uint16_t num_bytes;

        // Read the header
        num_bytes = network_read(settings.url, (unsigned char*)&image.header, sizeof(image.header));
        if(0 > num_bytes)
        {
            show_error_and_close_network("Error reading header\n\r");
            break;
        }
        // cprintf("v=%d,%d,%d gfx=%02X\n\r", 
        //         image.header.v1, image.header.v2, image.header.v3, image.header.gfx);

        // Read the block information, unused for now
        if(image.header.v2 < 4)
        {
            num_bytes = network_read(settings.url, buff, sizeof(BlockHeaderV13));
            if(0 > num_bytes)
            {
                show_error_and_close_network("Error reading block header\n\r");
                break;
            }
        }

        // Read the image data
        if(video)
        {
            // Load data into the buffer that isn't being shown
            #define SWAP_DISPLAY_LISTS
            #ifdef SWAP_DISPLAY_LISTS
            if(settings.gfx_mode & GRAPHICS_BUFFER_TWO)
            {
                if(FN_ERR_OK != load_front_buffer())
                {
                    show_error_and_close_network("Error reading front buffer\n\r");
                    break;
                }
            }
            else
            #endif
            {
                #ifdef SWAP_DISPLAY_LISTS
                if(FN_ERR_OK != load_back_buffer())
                {
                    show_error_and_close_network("Error reading back buffer\n\r");
                    break;
                }
                #else
                num_bytes = network_read(settings.url, (uint8_t*)0x8280, 8800);
                if(0 > num_bytes)
                {
                    show_error_and_close_network("Error reading\n\r");
                    break;
                }
                #endif
            }

            #ifdef SWAP_DISPLAY_LISTS
            setGraphicsMode((settings.gfx_mode & 0xEF) ^ GRAPHICS_BUFFER_TWO);

            // Delay for a bit to make sure the DLs have swapped.  Waiting for the VBI to finish
            wait_vbi();

            #else
            #define BUFFER_ONE_BLOCK_ONE_AND_TWO_SIZE 4080
            #define BUFFER_ONE_BLOCK_THREE_SIZE 640
            memcpy((void*)image.data, (void*)0x8280, BUFFER_ONE_BLOCK_ONE_AND_TWO_SIZE);
            memcpy((void*)(image.data+0x1000), (void*)(0x8280+BUFFER_ONE_BLOCK_ONE_AND_TWO_SIZE), BUFFER_ONE_BLOCK_ONE_AND_TWO_SIZE);
            memcpy((void*)(image.data+0x2000), (void*)(0x8280+BUFFER_ONE_BLOCK_ONE_AND_TWO_SIZE+BUFFER_ONE_BLOCK_ONE_AND_TWO_SIZE), BUFFER_ONE_BLOCK_THREE_SIZE);
            #endif

            input = check_keypress(2);
            if(input)
            {
                setGraphicsMode((settings.gfx_mode & 0xDF) | GRAPHICS_CONSOLE_EN); // Switch back to the front buffer
                goto quit;
            }

        } // video
        else
        {
            if(FN_ERR_OK != load_front_buffer())
            {
                show_error_and_close_network("Error reading front buffer\n\r");
                break;
            }

            input = check_keypress(30000);
            if(input)
                goto quit;
        }

        if(FN_ERR_OK != network_write(settings.url, (uint8_t*)"next", 4))
        {
            show_error("Unable to write request\n\r");
            break;
        }

        OS.atract = 0x00;   // disable attract mode
    }

quit:
    if(FN_ERR_OK != network_write(settings.url, (uint8_t*)"quit", 4))
        show_error("Unable to write quit request\n\r");

    network_close(settings.url);

    OS.soundr = 3; // Restore SIO beeping sound

    // We are no longer streaming so disable attract mode with a VBI
    add_attract_disable_vbi();

    return input;
}
