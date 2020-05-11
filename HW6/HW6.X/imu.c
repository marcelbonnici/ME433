#include "imu.h"
#include "i2c_master_noint.h"
#include "ssd1306.h"

void imu_setup(){
    unsigned char who = 0;
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 0;
    
    // read from IMU_WHOAMI
    who = readPin(IMU_ADDR,0x0F);
    
    if (who != 0b01101001){
        while(1){}
    }
    
    // init IMU_CTRL1_XL
    setPin(IMU_ADDR,IMU_CTRL1_XL,0b10000010);
    // init IMU_CTRL2_G
    setPin(IMU_ADDR,IMU_CTRL2_G,0b10001000);
    // init IMU_CTRL3_C
    setPin(IMU_ADDR,IMU_CTRL3_C,0b00000100);
}

void imu_read(unsigned char reg, signed short *d, int len){
    int i=0;
    int j=len-1;
    int r2=len*2;
    unsigned char raw[r2];
    // read multiple from the imu, each data takes 2 reads so you need len*2 chars
    read_multiple(IMU_ADDR,reg,raw,r2);
    // turn the chars into the shorts
    for(i=2*len-1;i>0;i=i-2){
        d[j]=(raw[i]<<8)|(raw[i-1]);
        j=j-1;
    }
}

void bar_x(signed short x, int one){
    int c=0;
    int twosix=2*2*2*2*2*2*2*2*2*2*2*2*2*2-1;
    x=64*x/twosix;
    if(x<0){
        for(c=0;c>x;c--){
            int xaxis=64+c;
            ssd1306_drawPixel(xaxis,16,one);
        }
    }
    if(x>=0){
        for(c=0;c<x;c++){
            int xaxis=64+c;
            ssd1306_drawPixel(xaxis,16,one);
        }
    }
}

void bar_y(signed short y, int val){
    int c=0;
    int twosix=2*2*2*2*2*2*2*2*2*2*2*2*2*2-1;
    y = 16*y/twosix;
    if(y<0){
        for(c=0;c>y;c--){
            int yaxis=16+c;
            ssd1306_drawPixel(64,yaxis,val);
        }
    }
    if(y>=0){
        for(c=0;c<y;c++){
            int yaxis=16+c;
            ssd1306_drawPixel(64,yaxis,val);
        }
    }
}

void setPin(unsigned char address, unsigned char reg, unsigned char value){
    i2c_master_start();
    i2c_master_send(address);
    i2c_master_send(reg);
    i2c_master_send(value);
    i2c_master_stop();
}

unsigned char readPin(unsigned char address, unsigned char reg){
    unsigned char value;
    i2c_master_start();
    i2c_master_send(address);
    i2c_master_send(reg);
    i2c_master_restart();
    address = address | 0b00000001;
    i2c_master_send(address);
    value = i2c_master_recv();
    i2c_master_ack(1);
    i2c_master_stop();
    return value;
}

void read_multiple(unsigned char address, unsigned char reg, unsigned char *data, int len){
    
    i2c_master_start();
    i2c_master_send(address);
    i2c_master_send(reg);
    i2c_master_restart();
    address = address | 0b00000001;
    i2c_master_send(address);
    int c;
    for(c=0;c<len;c++){
        data[c] = i2c_master_recv();
        if(c == len-1){
            i2c_master_ack(1);
        }
        else{
            i2c_master_ack(0);
        }
    }
    i2c_master_stop();
}