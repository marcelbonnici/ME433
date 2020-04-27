#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include "i2c_master_noint.h"
// DEVCFG0
#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // disable secondary oscillator
#pragma config IESO = OFF // disable switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // disable clock output
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // disable clock switch and FSCM
#pragma config WDTPS = PS1048576 // use largest wdt
#pragma config WINDIS = OFF // use non-window mode wdt
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz

// DEVCFG3
#pragma config USERID = 00000000 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations

void setPin(unsigned char reg, unsigned char value){//then write read function: start send, stop
    i2c_master_start();
    i2c_master_send(0b01000000);
    i2c_master_send(reg);
    i2c_master_send(value);
    i2c_master_stop();
}
unsigned char readPin(){ //unsigned char address, unsigned char reg
    i2c_master_start();
    i2c_master_send(0b01000000);    // W add
    i2c_master_send(0x19);      // GPIO B
    i2c_master_restart();
    i2c_master_send(0b01000001);    //R add
    unsigned char recv = i2c_master_recv();
    i2c_master_ack(1);
    i2c_master_stop();
    return recv;
}
int main() {
    //polling method: all call master setup. 
    
    //start, send the address for writing, send register you wanna read from, restart, send the address with reading, receive, acknowledge, stop
    __builtin_disable_interrupts(); // disable interrupts while initializing things

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 0;
    
    i2c_master_setup();
    setPin(0x00,0b00000000);    // GPA all outputs
    setPin(0x0A,0b10000000);    // Start outputs on high
    setPin(0x10,0b11111111);    // GPB all inputs
    __builtin_enable_interrupts();

    while (1) {
        // use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
        // remember the core timer runs at half the sysclk
        while (_CP0_GET_COUNT() <= 12000000){
            
        }
        LATAINV=0x10;
        _CP0_SET_COUNT(0);/*
        setPin(0x0A,0b10000000);
        while (_CP0_GET_COUNT() <= 12000000){
            
        }
        LATAINV=0x10;
        _CP0_SET_COUNT(0);
        setPin(0x0A,0b00000000);*/
        if (readPin()>>7==1){
            setPin(0x0A,0b10000000);
        }
        else{
            setPin(0x0A,0b00000000);
        }
    }
}