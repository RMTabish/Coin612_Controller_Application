gcc -I./include Demo.c -L./lib -lGuideSCUSB2 -lpthread -lm -ludev -lSDL -o Demo
g++ -I./include -o demo code.cpp `pkg-config --cflags --libs opencv4` -L./lib -lGuideSCUSB2 -lpthread -lm -ludev -lSDL


Note: If you are prompted not to find sdl.h, please install the following library

/*
sudo apt-get install libsdl1.2-dev 
sudo apt-get install libsdl-image1.2-dev
sudo apt-get install libsdl-mixer1.2-dev
sudo apt-get install libsdl-ttf2.0-dev
sudo apt-get install libsdl-gfx1.2-dev

*/

