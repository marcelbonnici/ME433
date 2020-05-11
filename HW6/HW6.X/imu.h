#ifndef IMU_H__
#define IMU_H__

#include<xc.h> // processor SFR definitions

#define IMU_WHOAMI 0x0F
#define IMU_ADDR 0b11010110
#define IMU_CTRL1_XL 0x10
#define IMU_CTRL2_G 0x11
#define IMU_CTRL3_C 0x12
#define IMU_OUT_TEMP_L 0x20

void imu_setup();
void imu_read(unsigned char, signed short *, int);
void setPin(unsigned char address, unsigned char reg, unsigned char value);
unsigned char readPin(unsigned char address, unsigned char reg);
void bar_x(signed short x, int one);
void bar_y(signed short y, int one);
void read_multiple(unsigned char address, unsigned char reg, unsigned char *d, int len);
#endif