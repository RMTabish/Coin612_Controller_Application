#include<iostream>
#include<opencv2/opencv.hpp>
#include "include/guidescusb2.h" 
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <fcntl.h>
#include "sys/time.h"
#include "time.h"
#include <stdlib.h>
#include <SDL/SDL.h>
// Define your camera's width and height
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

// Callback functions declarations (to be defined based on your SDK's requirements)
int serialCallBack(int id, guide_usb_serial_data_t *pSerialData);
int connectStatusCallBack(int id, guide_usb_device_status_e deviceStatus);
int frameCallBack(int id, guide_usb_frame_data_t *pVideoData);
double tick(void)
{
    struct timeval t;
    gettimeofday(&t, 0);
    return t.tv_sec + 1E-6 * t.tv_usec;
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

int serialCallBack(int id,guide_usb_serial_data_t *pSerialData)
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
int main() {
    guide_usb_setloglevel(LOG_LEVEL_INFO);

    // Set basic device information
    auto *deviceInfo = new guide_usb_device_info_t;
    deviceInfo->width = WIDTH;
    deviceInfo->height = HEIGHT;
    deviceInfo->video_mode = YUV_PARAM; // Make sure this is correctly defined as per your camera's documentation

    int ret = guide_usb_get_devcount();
    std::cout << "Device count: " << ret << std::endl;

    if (ret < 1) {
        std::cerr << "No devices found." << std::endl;
        return -1;
    }

    // Initialize OpenCV window
    cv::namedWindow("Frame", cv::WINDOW_AUTOSIZE);

    // Initialize device
    ret = guide_usb_initial(1);
    if (ret < 0) {
        std::cerr << "Initial device 1 fail: " << ret << std::endl;
        return ret;
    }

    // Endpoint communication is enabled on device 1
    ret = guide_usb_opencommandcontrol(1, serialCallBack);
    if (ret < 0) {
        std::cerr << "Command control for device 1 failed: " << ret << std::endl;
        return ret;
    }

    // Device 1 starts the video streaming thread
    ret = guide_usb_openstream(1, deviceInfo, frameCallBack, connectStatusCallBack);
    if (ret < 0) {
        std::cerr << "Open stream for device 1 failed: " << ret << std::endl;
        return ret;
    }

    std::cout << "Press any key to exit..." << std::endl;
    cv::waitKey(0); // Wait for a key press to exit

    // Cleanup
    guide_usb_closestream(1);
    guide_usb_closecommandcontrol(1);
    guide_usb_exit(1);

    delete deviceInfo;
    cv::destroyAllWindows();

    return 0;
}

int frameCallBack(int id, guide_usb_frame_data_t *pVideoData) {
    // Assuming YUV422 format for demonstration
    cv::Mat yuvImg(HEIGHT, WIDTH, CV_8UC2, pVideoData->frame_yuv_data); // Create an OpenCV Mat from the YUV data
    cv::Mat bgrImg;
    cv::cvtColor(yuvImg, bgrImg, cv::COLOR_YUV2BGR_UYVY); // Convert YUV to BGR for OpenCV compatibility

    cv::imshow("Frame", bgrImg); // Display the frame
    cv::waitKey(1); // Refresh window

    return 0;
}

