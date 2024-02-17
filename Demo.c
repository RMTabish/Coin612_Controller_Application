#include <stdio.h>
#include <sys/types.h>
#include "guidescusb2.h"
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <fcntl.h>
#include "sys/time.h"
#include "time.h"
#include <stdlib.h>
#include <SDL/SDL.h>


/*
sudo apt-get install libsdl1.2-dev 
sudo apt-get install libsdl-image1.2-dev
sudo apt-get install libsdl-mixer1.2-dev
sudo apt-get install libsdl-ttf2.0-dev
sudo apt-get install libsdl-gfx1.2-dev

*/

#define WIDTH 640
#define HEIGHT 512

SDL_Overlay     *Overlay;
SDL_Surface     *Surface;
SDL_Rect        Rect;
SDL_Event       Event;
unsigned char* yuv422Data = NULL;
unsigned char* paramline = NULL;

int FPS1 = 0;
int FPS2 = 0;
double startTime1,startTime2;


int serailCallBack(int id,guide_usb_serial_data_t *pSerialData);
int connectStatusCallBack(int id,guide_usb_device_status_e deviceStatus);
int frameCallBack(int id,guide_usb_frame_data_t *pVideoData);

double tick(void)
{
    struct timeval t;
    gettimeofday(&t, 0);
    return t.tv_sec + 1E-6 * t.tv_usec;
}

int main(void)
{
    //Setting a Log Level
    guide_usb_setloglevel(LOG_LEVEL_INFO);

    //Set basic device information
    guide_usb_device_info_t* deviceInfo = (guide_usb_device_info_t*)malloc(sizeof (guide_usb_device_info_t));
    deviceInfo->width = WIDTH;
    deviceInfo->height = HEIGHT;
    deviceInfo->video_mode = YUV_PARAM;
    
   
   
    //Quantity of equipment acquired  Device ID No.: 1,2,3,4,...,count
    int ret = guide_usb_get_devcount();
    printf("devices counts:%d \n",ret);
    
    
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
        exit(1);
    }
    Surface = SDL_SetVideoMode(WIDTH, HEIGHT, 24,SDL_SWSURFACE);
    if(!Surface)
    {
         perror(" create vide omode error ! \n");
         return -1;
    }
    Overlay = SDL_CreateYUVOverlay(WIDTH, HEIGHT,SDL_UYVY_OVERLAY,Surface);
    Rect.x = 0;
    Rect.y = 0;
    Rect.w = WIDTH;
    Rect.h = HEIGHT;
    
    if(yuv422Data == NULL)
    {
        yuv422Data = (unsigned char*)malloc(WIDTH * HEIGHT * 2);
    }
    
    if(paramline == NULL)
    {
        paramline = (unsigned char*)malloc(WIDTH*2);
    }
    
    


    //Initialize device 1
    ret = guide_usb_initial(1);
    if(ret < 0)
    {
        printf("Initial device 1 fail:%d \n",ret);
        return ret;
    }
    else
    {
        //Endpoint communication is enabled on device 1
        ret = guide_usb_opencommandcontrol(1,(OnSerialDataReceivedCB)serailCallBack);
        printf("Initial device 1 success:%d\n",ret);
    }


    //Device 1 Starts the video streaming thread
    ret = guide_usb_openstream(1,deviceInfo,(OnFrameDataReceivedCB)frameCallBack,(OnDeviceConnectStatusCB)connectStatusCallBack);
    if(ret < 0)
    {
       printf("Open 1 fail:%d\n",ret);
       return ret;
    }
    else
    {
        printf("Open 1 return:%d\n",ret);
    }
    startTime1 = tick();

    /*************************************************************************************************************************************/
    //Initialize device 2
//    ret = guide_usb_initial(2);
//    if(ret < 0)
//    {
//        printf("Initial device 2 fail:%d \n",ret);
//        return ret;
//    }
//    else
//    {
//        //Endpoint communication is enabled on device 2
//        ret = guide_usb_opencommandcontrol(2,(OnSerialDataReceivedCB)serailCallBack);
//        printf("Initial device 2 success:%d\n",ret);
//    }


//    //Device 2 Starts the video streaming thread
//    ret = guide_usb_openstream(2,deviceInfo,(OnFrameDataReceivedCB)frameCallBack,(OnDeviceConnectStatusCB)connectStatusCallBack);
//    if(ret < 0)
//    {
//       printf("Open 2 fail:%d\n",ret);
//       return ret;
//    }
//    else
//    {
//        printf("Open 2 return:%d\n",ret);
//    }
//    startTime2 = tick();
    /*************************************************************************************************************************************/



    while (1)
    {
        usleep(10);
    }

    ret = guide_usb_closestream(1);
    printf("close 1 return:%d\n",ret);

//    ret = guide_usb_closestream(2);
//    printf("close 2 return:%d\n",ret);

    ret = guide_usb_closecommandcontrol(1);
    printf("closecommandcontrol 1 return:%d\n",ret);

//    ret = guide_usb_closecommandcontrol(2);
//    printf("closecommandcontrol 2 return:%d\n",ret);

    ret = guide_usb_exit(1);
    printf("exit 1 return:%d\n",ret);

//    ret = guide_usb_exit(2);
//    printf("exit 2 return:%d\n",ret);


    if(yuv422Data !=NULL)
    {
        free(yuv422Data);
        yuv422Data = NULL;
    }

    printf("guide_usb_destory_instance return:%d\n",ret);
    free(deviceInfo);
    
    SDL_Quit();

    return ret;
}

int serailCallBack(int id,guide_usb_serial_data_t *pSerialData)
{

    switch (id)
    {
      case 1:
        printf("ID:%d---->data length:%d \n",id,pSerialData->serial_recv_data_length);
        break;
      case 2:
        printf("ID:%d---->data length:%d \n",id,pSerialData->serial_recv_data_length);

        break;
      case 3:
       // printf("ID:%d---->data length:%d \n",id,pSerialData->serial_recv_data_length);
      break;
    }
}

int connectStatusCallBack(int id,guide_usb_device_status_e deviceStatus)
{
    switch (id)
    {
      case 1:
        switch (deviceStatus)
        {
            case DEVICE_CONNECT_OK:
                printf("ID:%d VideoStream Capture start...\n",id);
            break;
            case DEVICE_DISCONNECT_OK:
                printf("ID:%d VideoStream Capture end...\n",id);
            break;
        }
        break;
      case 2:
        switch (deviceStatus)
        {
            case DEVICE_CONNECT_OK:
                printf("ID:%d VideoStream Capture start...\n",id);
            break;
            case DEVICE_DISCONNECT_OK:
                printf("ID:%d VideoStream Capture end...\n",id);
            break;
        }
        break;
    }
}

int frameCallBack(int id,guide_usb_frame_data_t *pVideoData)
{
    //YUV数据 [pVideoData->frame_yuv_data:机芯送出来的YUV数据] yuv422 uyvy
    switch (id)
    {
      case 1: //设备1
        FPS1++;
        if((tick()-startTime1)>1)
        {
            startTime1 = tick();
            printf("FPS1-------------------------%d\n",FPS1);
            FPS1 = 0;
        }
        
        
        memcpy(yuv422Data,pVideoData->frame_yuv_data,WIDTH * HEIGHT * 2);
        SDL_LockYUVOverlay(Overlay);
        memcpy(Overlay->pixels[0],yuv422Data,WIDTH * HEIGHT * 2);
        SDL_UnlockYUVOverlay(Overlay);
        SDL_DisplayYUVOverlay(Overlay, &Rect);
        
       
        break;
      case 2:
        FPS2++;
        if((tick()-startTime2)>1)
        {
            startTime2 = tick();
            printf("FPS2-------------------------%d\n",FPS2);
            FPS2 = 0;
        }
        break;
      case 3:

      break;

     default:
        break;
    }
}
