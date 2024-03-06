#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "guidescusb2.h"
#include <SDL/SDL.h>

#define WIDTH 640
#define HEIGHT 512

SDL_Overlay *Overlay;
SDL_Surface *Surface;
SDL_Rect Rect;
SDL_Event Event;
unsigned char *yuv422Data = NULL;

// Function prototypes
int getSharpness(int deviceId);
int setSharpness(int deviceId, unsigned int sharpnessValue);
int serialCallBack(int id, guide_usb_serial_data_t *pSerialData);
int connectStatusCallBack(int id, guide_usb_device_status_e deviceStatus);
int frameCallBack(int id, guide_usb_frame_data_t *pVideoData);

int main(void) {
    guide_usb_setloglevel(LOG_LEVEL_INFO);

if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
    fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
    exit(1);
}

    Surface = SDL_SetVideoMode(WIDTH, HEIGHT, 24, SDL_SWSURFACE);
    if(!Surface) {
        fprintf(stderr, "Create video mode error! %s\n", SDL_GetError());
        exit(1);
    }

    Overlay = SDL_CreateYUVOverlay(WIDTH, HEIGHT, SDL_UYVY_OVERLAY, Surface);
    Rect.x = 0;
    Rect.y = 0;
    Rect.w = WIDTH;
    Rect.h = HEIGHT;

    yuv422Data = (unsigned char*)malloc(WIDTH * HEIGHT * 2);
int cc = guide_usb_get_devcount();
printf("Device count: %d\n", cc);
       int deviceId = 1; 
    int ret = guide_usb_initial(deviceId);
    if(ret < 0) {
        printf("Initial device 1 fail:%d \n", ret);
        return ret;
    }

    ret = guide_usb_opencommandcontrol(1, (OnSerialDataReceivedCB)serialCallBack);
    if(ret < 0) {
        printf("Open command control for device 1 failed: %d\n", ret);
        return ret;
    }

    
  //  getSharpness(1);
    setSharpness(1, 128); // Example: Set sharpness to 128

    SDL_Quit();
    free(yuv422Data);

    return 0;
}

int getSharpness(int deviceId) {
    unsigned char cmd[] = {0x06}; 
    int ret = guide_usb_sendcommand(deviceId, cmd, sizeof(cmd));
    if(ret < 0) {
        printf("Failed to get sharpness for device %d\n", deviceId);
    }
    return ret;
}

int setSharpness(int deviceId, unsigned int sharpnessValue) {
    unsigned char cmd[] = {0x06, sharpnessValue}; 
    int ret = guide_usb_sendcommand(deviceId, cmd, sizeof(cmd));
    if(ret < 0) {
        printf("Failed to set sharpness for device %d\n", deviceId);
    } else {
        printf("Sharpness set to %u for device %d\n", sharpnessValue, deviceId);
    }
    return ret;
}

int serialCallBack(int id, guide_usb_serial_data_t *pSerialData) {
    printf("Serial Data Callback - Device ID: %d, Data Length: %d\n", id, pSerialData->serial_recv_data_length);
    // Example: Print the received data
    printf("Received Data: %.*s\n", pSerialData->serial_recv_data_length, pSerialData->serial_recv_data);
    return 0; }


int connectStatusCallBack(int id, guide_usb_device_status_e deviceStatus) {
    switch (deviceStatus) {
        case DEVICE_CONNECT_OK:
            printf("Device %d connected successfully.\n", id);
            break;
        case DEVICE_DISCONNECT_OK:
            printf("Device %d disconnected successfully.\n", id);
            break;
        default:
            printf("Device %d status changed: %d\n", id, deviceStatus);
            break;
    }
    return 0;
}
int frameCallBack(int id, guide_usb_frame_data_t *pVideoData) {
    static int frameCounter = 0;
    frameCounter++;
    printf("Frame Callback - Device ID: %d, Frame Count: %d\n", id, frameCounter);

 
    SDL_LockYUVOverlay(Overlay);
    memcpy(Overlay->pixels[0], pVideoData->frame_yuv_data, WIDTH * HEIGHT * 2); // Assuming YUV422
    SDL_UnlockYUVOverlay(Overlay);
    SDL_DisplayYUVOverlay(Overlay, &Rect);

    return 0; 
}

