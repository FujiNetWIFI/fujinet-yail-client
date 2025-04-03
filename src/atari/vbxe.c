#include "vbxe.h"
#include "xdlc.h"
#include "utility.h"
#include "settings.h"
#include "graphics.h"

#include <conio.h>

#include <atari.h>
#include <string.h>
#include <stdbool.h>

extern Settings settings;
extern byte framebuffer[];

struct __vbxe* VBXE = NULL; // Empty if there is no VBXE present

void setup_VBXE(void)
{
    // The memory window to the VBXE
    if(VBXE_D640.CORE_VERSION != 0x10 && (VBXE_D640.MINOR_BERSION & 0x70) != 0x20) {
        // Try the second address
        if(VBXE_D740.CORE_VERSION != 0x10 && (VBXE_D740.MINOR_BERSION & 0x70) != 0x20) {
            show_error("VBXE not detected\n\r");
            return;
        }
        else
            VBXE = &VBXE_D740;
    }
    else
        VBXE = &VBXE_D640;

    //cprintf("VBXE detected, %04X\n\r", VBXE);

    VBXE->VIDEO_CONTROL = 0x82;    // No-trans and 

    // Set memory window location to the same location as our "Frame Buffer".
    VBXE->MEM_BANK_SEL = (uint8_t)((uint16_t)&framebuffer[0] >> 1); //0x60;
    VBXE->MEMAC_CTRL = 0x88; // let only the CPU have access

    // XDL ADDRESS, this is placed after the image on a 4K boundary ($1000)
    // The image will take up 320x240 (76800, $12C00) bytes.  This is 19 banks (@ $4096 per bank),
    // so the XDL will be placed at $15000.
    VBXE->XDL_ADR0 = 0x00;
    VBXE->XDL_ADR1 = 0x50;
    VBXE->XDL_ADR2 = 0x01;

    VBXE->MEM_BANK_SEL = 128 + 21;

    // Setup XDL
    XDL[0] = XDLC_GMON | XDLC_RPTL | XDLC_OVADR; // OVERLAY, REPEAT, SET OVADR
    //if(GRAPHICS_20 == settings.gfx_mode)
        XDL[1] = XDLC_ATT | XDLC_END; // ATTRIB, END XDL
    //else if(GRAPHICS_21 == settings.gfx_mode)
    //    XDL[1] = XDLC_ATT | XDLC_HR | XDLC_END; // ATTRIB, HR, END XDL
    XDL[2] = 239; // Repeat 239 lines
    // Overlay location (0x0000)
    XDL[3] = 0x00;
    XDL[4] = 0x00;
    XDL[5] = 0x00;
    // Step
    XDL[6] = 0x40; // 320 bytes
    XDL[7] = 0x01;
    // Width, palette, priority
    XDL[8] = XDLC_ATT_OV_WIDTH_NORMAL | XDLC_ATT_OV_PALETTE_01; // NORMAL WIDTH, PALETTE 1 FOR OVERLAY
    XDL[9] = XDLC_ATT_MAIN_PRIORITY_OVERLAY_ALL ^ XDLC_ATT_MAIN_PRIORITY_OVERLAY_PF1; // OVERLAY HAS PRIORITY OVER GTIA EXCEPT PF1

    // XDL disable
    VBXE->VIDEO_CONTROL = 0x00;

    // Clear the screen
    clear_vbxe();

    // Disable ANTIC
    //OS.sdmctl &= ~0x20;
}

void clear_vbxe() {
    int i;

    // Clear the screen
    for(i = 0; i < 19; ++i) {
        VBXE->MEM_BANK_SEL = 128 + i;  // MOVE WINDOW STARTING $0000
        memset(XDL, 0, 4096);
    }    
}