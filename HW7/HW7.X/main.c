#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include"ssd1306.h"
#include<stdio.h>
#include<string.h>
#include"ws2812b.h"

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
    ws2812b_setup();
    ssd1306_setup();
    adc_setup();
    ctmu_setup();
    
    __builtin_enable_interrupts();
    
    wsColor strobe[4];

    strobe[0].r = 0;    
    strobe[0].g = 0;
    strobe[0].b = 0;

    strobe[1].r = 0;    
    strobe[1].g = 0;
    strobe[1].b = 0;
    
    strobe[2].r = 0;
    strobe[2].g = 0;
    strobe[2].b = 0;

    strobe[3].r = 0;   
    strobe[3].g = 0;
    strobe[3].b = 0;

    unsigned char m[32];
    int leftbase=8950; //tentative
    int rightbase=8850; //tentative
    int deltal=0;
    int deltar=0;
    int deltas=0;
    int dim=0;
    float leftp=0;
    float rightp=0;
    float position=0;

    while (1) {
        ws2812b_setColor(strobe,4.0);
        int leftatm = 0;
        int rightatm = 0;
        int i = 0;
        while (i<10) {
            leftatm = leftatm+ctmu_read(4,15);
            rightatm = rightatm+ctmu_read(5,15);
            i=i+1;
        }
        
        //EQUATION 1
        deltal=leftbase-leftatm;
        deltar=rightbase-rightatm;
        deltas=deltal+deltar;
        
        //EQUATION 2
        leftp=(deltal*100)/(deltal+deltar);
        
        //EQUATION 3
        rightp=((1-deltar)*100)/(deltal+deltar);
        
        //EQUATION 4
        position=(leftp+rightp)/2;
        
        sprintf(m, "%d", leftatm); 
        drawString(0, 0, m);
        sprintf(m, "%d", rightatm); 
        drawString(0, 8, m);
        sprintf(m, "%f", position);
        drawString(0, 16, m);
        ssd1306_update();
        
        if (deltas<1000) {
           strobe[0].g=0;
           strobe[1].g=0;
           strobe[2].r=0;
        }
        if (deltas>=1000) {
            if (position>=0 & position<=30) {
                dim=2.55*position;
                strobe[2].r=dim;
            }
            if (position<0 & position>=-35) {
               dim=-2.55*position;
                strobe[2].r=dim;
            }
            if (position>30) {
                strobe[0].g=255;
                strobe[1].g=0;
                strobe[2].r=255;
            }
            if (position<-35) {
                strobe[0].g=0;
                strobe[1].g=255;
                strobe[2].r=0;
            }
        }
    }
}