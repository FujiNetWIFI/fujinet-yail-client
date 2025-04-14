#ifndef SETTINGS_H
#define SETTINGS_H

#include "types.h"
#include "fujinet-fuji.h"

//
#define SERVER_URL_SIZE MAX_APPKEY_LEN
#define AI_MODEL_NAME_SIZE 20

#define SETTINGS_NONE 0
#define SETTINGS_URL  1
#define SETTINGS_GFX  2
#define SETTINGS_AI_MODEL  3

// A structure for holding the application runtime settings
typedef struct {
    byte gfx_mode;
    char url[SERVER_URL_SIZE];
    char ai_model_name[AI_MODEL_NAME_SIZE];
} Settings;

//unsigned char sio_openkey(AppKeyDataBlock* data, unsigned char open_mode, unsigned char key);

uint8_t get_settings(void);

uint8_t put_settings(byte select);

void print_settings(uint8_t mode, char* url, char* ai_model);

#endif