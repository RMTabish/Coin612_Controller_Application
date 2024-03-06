#include <iostream>
#include "guidescusb2.h"
#include<iostream>
#include<opencv2/opencv.hpp>
#include "include/guidescusb2.h" 
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include "sys/time.h"
#include "time.h"
#include <stdlib.h>
#include <SDL/SDL.h>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

int globalVideoMode = -1;

SDL_Overlay *Overlay;
SDL_Surface *Surface;
SDL_Rect Rect;
unsigned char *yuv422Data = NULL;
#define WIDTH 640
#define HEIGHT 512
int FPS1 = 0;
double startTime1;
// Callback functions declarations (to be defined based on your SDK's requirements)
int serialCallBack(int id, guide_usb_serial_data_t *pSerialData);
int connectStatusCallBack(int id, guide_usb_device_status_e deviceStatus);
int frameCallBack(int id, guide_usb_frame_data_t *pVideoData);



// Function to send a command to the device
int sendCommand(int id, unsigned char *command, int length)
{
    int ret = guide_usb_sendcommand(id, command, length);
    if (ret == 0)
    {
        std::cout << "Command sent successfully." << std::endl;
    }
    else
    {
        std::cerr << "Failed to send command. Error code: " << ret << std::endl;
    }
    return ret;
}

// UI function to toggle shutter control
void toggleShutter(int id, bool &shutterOpen)
{
    shutterOpen = !shutterOpen;
    std::cout << "Shutter is now: " << (shutterOpen ? "Open" : "Closed") << std::endl;

    // Construct and send the specified command
    unsigned char shutterCommand[12] = {0x55, 0xAA, 0x07, 0xA0, 0x02, 0x08, 0x00, 0x00, 0x00, shutterOpen ? 0x01 : 0x00, 0xAC, 0xF0};
    sendCommand(id, shutterCommand, 12);
}

void setY16Mode(int id)
{
    // Construct and send the specified command for setting Y16 mode
    unsigned char y16Command[12] = {0x55, 0xAA, 0x07, 0x02, 0x01, 0x03, 0x00, 0x00, 0x00, 0x04, 0x03, 0xF0};
    sendCommand(id, y16Command, 12);
}
void setYUVMode(int id)
{
    // Construct and send the specified command for setting YUV mode
    unsigned char yuvCommand[12] = {0x55, 0xAA, 0x07, 0x02, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x07, 0xF0};
    sendCommand(id, yuvCommand, 12);
}

// Serial callback function
int serialCallBack(int id, guide_usb_serial_data_t *pSerialData)
{
    if (pSerialData->serial_recv_data_length == 12)
    {
        // Extract the video mode information from the received data
        globalVideoMode = pSerialData->serial_recv_data[7];
        std::cout << "Video mode received: " << globalVideoMode << std::endl;
    }
    return 0;
}


// Connect status callback function
int connectStatusCallBack(int id, guide_usb_device_status_e deviceStatus)
{
    switch (id)
    {
    case 1:
        switch (deviceStatus)
        {
        case DEVICE_CONNECT_OK:
            std::cout << "ID:" << id << " VideoStream Capture start..." << std::endl;
            break;
        case DEVICE_DISCONNECT_OK:
            std::cout << "ID:" << id << " VideoStream Capture end..." << std::endl;
            break;
        }
        break;
    default:
        break;
    }
    return 0;
}

// Frame callback function
int frameCallBack(int id, guide_usb_frame_data_t *pVideoData)
{
    // Handle frame data as needed
    return 0;
}
void setSharpness(int id, int sharpnessValue)
{
    // Construct and send the specified command for setting sharpness
    unsigned char sharpnessCommand[12] = {0x55, 0xAA, 0x07, 0x02, 0x02, 0x06, 0x00, 0x00, 0x00, static_cast<unsigned char>(sharpnessValue), 0x01, 0xF0};
    sendCommand(id, sharpnessCommand, 12);
}

void promptSharpness(int id)
{
    std::cout << "Select sharpness option:\n";
    std::cout << "1. Soft mode sharpening\n";
    std::cout << "2. Standard mode\n";
    std::cout << "3. Enhanced mode\n";
    std::cout << "Enter your choice: ";

    int sharpnessOption;
    std::cin >> sharpnessOption;

    int sharpnessValue;

    switch (sharpnessOption)
    {
    case 1:
        // Set sharpness value for soft mode
        sharpnessValue = 0;
        break;

    case 2:
        // Set sharpness value for standard mode
        sharpnessValue = 1;
        break;

    case 3:
        // Set sharpness value for enhanced mode
        sharpnessValue = 2;
        break;

    default:
        std::cerr << "Invalid choice. Setting sharpness to default (soft mode).\n";
        sharpnessValue = 0;
    }

    setSharpness(id, sharpnessValue);
}

// Function to query and set the video mode
void queryAndSetVideoMode(int id, guide_usb_device_info_t *deviceInfo)
{
    // Open command control with the serial callback
    int ret = guide_usb_opencommandcontrol(id, serialCallBack);
    if (ret < 0)
    {
        std::cerr << "Command control for device " << id << " failed: " << ret << std::endl;
        return;
    }

    // Send the query command
    unsigned char queryCommand[12] = {0x55, 0xAA, 0x07, 0x02, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x84, 0xF0};
    guide_usb_sendcommand(id, queryCommand, 12);

    // Wait for the response (you may need to adjust the sleep duration)
    usleep(1000000); // 1 second

    // Map the video mode to the appropriate enum value and set it in deviceInfo
    switch (globalVideoMode)
    {
    case 0:
        deviceInfo->video_mode = YUV;
        break;

    case 1:
        deviceInfo->video_mode = YUV_PARAM;
        break;

    case 2:
        deviceInfo->video_mode = Y16;
        break;

    case 3:
        deviceInfo->video_mode = Y16_PARAM;
        break;

    case 4:
        deviceInfo->video_mode = Y16_YUV;
        break;



    default:
        std::cerr << "Unknown video mode received: " << globalVideoMode << std::endl;
        break;
    }

    std::cout << "Video mode set to: " << deviceInfo->video_mode << std::endl;

    // Close command control after receiving the response
    guide_usb_closecommandcontrol(id);
}


int main()
{

    guide_usb_setloglevel(LOG_LEVEL_INFO);

    int ret = guide_usb_get_devcount();
    std::cout << "Device count: " << ret << std::endl;

    if (ret < 1)
    {
        std::cerr << "No devices found." << std::endl;
        return -1;
    }

    // Initialize device
    ret = guide_usb_initial(1);
    if (ret < 0)
    {
        std::cerr << "Initial device 1 fail: " << ret << std::endl;
        return ret;
    }
    // Set basic device information
    guide_usb_device_info_t *deviceInfo = new guide_usb_device_info_t;
    deviceInfo->width = WIDTH;
    deviceInfo->height = HEIGHT;
    deviceInfo->video_mode = Y16_YUV;
    // Endpoint communication is enabled on device 1
    ret = guide_usb_opencommandcontrol(1, serialCallBack);
    if (ret < 0)
    {
        std::cerr << "Command control for device 1 failed: " << ret << std::endl;
        return ret;
    }





    // Device 1 starts the video streaming thread
    ret = guide_usb_openstream(1, deviceInfo, frameCallBack, connectStatusCallBack);
    if (ret < 0)
    {
        std::cerr << "Open stream for device 1 failed: " << ret << std::endl;
        return ret;
    }

    bool shutterOpen = false;

    while (true)
    {
        std::cout << "Select an action:\n";
        std::cout << "1. Toggle Shutter\n";
        std::cout << "2. Set Y16 Mode\n";
        std::cout << "3. Set YUV Mode\n";
        std::cout << "4. Set Sharpness\n";
        std::cout << "5. Exit\n";
        std::cout << "Enter your choice: ";

        int choice;
        std::cin >> choice;

        switch (choice)
        {
        case 1:
            toggleShutter(1, shutterOpen);
            break;

        case 2:
            setY16Mode(1);
            break;

        case 3:
            setYUVMode(1);
            break;

        case 4:
            promptSharpness(1);
            break;

        case 5:
            std::cout << "Exiting...\n";
            break;

        default:
            std::cerr << "Invalid choice. Try again.\n";
        }

        if (choice == 5)
            break; 
    }

    // Cleanup
    guide_usb_closestream(1);
    guide_usb_closecommandcontrol(1);
    guide_usb_exit(1);

    delete deviceInfo;

    return 0;
}