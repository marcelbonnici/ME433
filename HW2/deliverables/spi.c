#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include <math.h>
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
#pragma config WDTPS = PS1 // use largest wdt
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








void initSPI();
unsigned char spi_io(unsigned char o);
unsigned short channel_voltage(unsigned char c, unsigned short v);
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

    // do your TRIS and LAT commands here
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 0;
    TRISBbits.TRISB4 = 1;
    
    initSPI();
    
    __builtin_enable_interrupts();
    unsigned char i=0;
    int count = 0;
    int countd=0;
    int dual=0;
    int b_pre=0;
    int c_pre=0;
    while (1) {
        //unsigned char c=0; //0 means A, 1 means B I think
        
        unsigned short p;
        if (dual==0){
            dual=1;
            if (count == 4095*2){
                count=0;
            }
            if (count % (4095*2) <4095){
                b_pre=b_pre+1;
            }
            if (count % (4095*2) >= 4095){
                b_pre=b_pre-1;
            }

            // use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
            // remember the core timer runs at half the sysclk
            LATAbits.LATA0=0; //bring cs low
            //spi_io(i); //write the byte
            p=channel_voltage(0, (2047*sin(((2*b_pre*3.14159)/4095)-(3.14159/2)))+2047);
            spi_io((unsigned char)(p>>8));
            spi_io((unsigned char)p);
            LATAbits.LATA0=1;//bring cs high
            count=count+1;
            i++;
            if(i==100){
                i=0;
            }
        }
        else{
            dual=0;
            if (countd == 4095*2){
                countd=0;
            }
            if (countd % (4095*2) < 4095){
                c_pre=c_pre-1;
            }
            if (countd % (4095*2) >= 4095){
                c_pre=c_pre+1;
            }

            // use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
            // remember the core timer runs at half the sysclk
            LATAbits.LATA0=0; //bring cs low
            //spi_io(i); //write the byte
            p=channel_voltage(1, c_pre);
            spi_io((unsigned char)(p>>8));
            spi_io((unsigned char)p);
            LATAbits.LATA0=1;//bring cs high
            countd=countd+1;
            i++;
            if(i==100){
                i=0;
            }  
        }
        _CP0_SET_COUNT(0);
        while (_CP0_GET_COUNT() < 2100/2){//Hz 48000000/2
    }
}
}
//unsigned short V=2048;
//unsigned char c;
// initialize SPI1
void initSPI() {
    // Pin B14 has to be SCK1
    // Turn of analog pins
    ANSELA=0;//...
    // Make an output pin for CS, so PIC A0 -> CS
    TRISAbits.TRISA0=0;//...
    LATAbits.LATA0=1;//...
    // Set SDO1 for A1, so PIC A1 -> MCP SDI?
    RPA1Rbits.RPA1R=0b0011;//...
    // Set SDI1 for B5, so PIC B5 -> MCP __?
    SDI1Rbits.SDI1R=0b0001;//...

    // setup SPI1
    SPI1CON = 0; // turn off the spi module and reset it
    SPI1BUF; // clear the rx buffer by reading from it
    SPI1BRG = 1; // 1000 for 24kHz, 1 for 12MHz; // baud rate to 10 MHz [SPI4BRG = (48000000/(2*desired))-1] // 100khz(Nsc limit)/12k=8.33 so SPI1BRG's 1000/8.33 is 120.04.. sound round up to 121
    SPI1STATbits.SPIROV = 0; // clear the overflow bit
    SPI1CONbits.CKE = 1; // data changes when clock goes from hi to lo (since CKP is 0)
    SPI1CONbits.MSTEN = 1; // master operation
    SPI1CONbits.ON = 1; // turn on spi 
}
//send a byte via SPI and return the response
unsigned char spi_io(unsigned char o){
    SPI1BUF=o;
    while(!SPI1STATbits.SPIRBF){;}//wait to receive the byte
    return SPI1BUF;
}

unsigned short channel_voltage(unsigned char c, unsigned short v){
    unsigned short p = (c<<15);
    p=p|(0b111<<12);
    p=p|v;
    return p;
}