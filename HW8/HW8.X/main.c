#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h>
#include<string.h>
#include"i2c_master_noint.h"
#include"ssd1306.h"
#include "rtcc.h"

// DEVCFG0
#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = ON // disable secondary oscillator
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
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations

int main() {

    __builtin_disable_interrupts(); // disable interrupts while initializing things

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
    
    i2c_master_setup();
    ssd1306_setup();
    rtcc_setup(0, 0x20053006);
    
   __builtin_enable_interrupts();    

    rtccTime time; 
    char d[32];
    char m[32];
    int i=0;
    
    while (1) {
        sprintf(m, "Hi!"); 
        drawString(0, 0, m);    
        sprintf(m, "%d",i); 
        drawString(25, 0, m);
        time=readRTCC();
        sprintf(m, "%d%d:%d%d:%d%d", time.hr10, time.hr01, time.min10, time.min01, time.sec10, time.sec01); 
        drawString(5, 16, m);
        dayOfTheWeek(time.wk, d);
        sprintf(m,"%s %d%d/%d%d/%d%d", d, time.mn10, time.mn01, time.dy10, time.dy01, time.yr10, time.yr01);
        drawString(5, 24, m);
        ssd1306_update();
        
        if (_CP0_GET_COUNT() > 24000000 / 2)
        {
            _CP0_SET_COUNT(0);
            i = i+1;
        }
    }
}