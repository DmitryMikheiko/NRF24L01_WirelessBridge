#include <stdbool.h> 
#include "Config.h"

#ifdef TestBoardRemap
#define ce  PORTB.2             //PORTB.2 - Atmega8 test board
#define csn PORTB.1
#else
#define ce  PORTC.1            
#define csn PORTC.0
#endif

#define ce_0    ce=0;
#define ce_1    ce=1;
#define csn_0   csn=0;
#define csn_1   csn=1;

#define R_REGISTER       0X00        //Read registers directly address bitwise
#define W_REGISTER       0X20        //write registers      Bitwise OR with address
#define R_RX_PAYLOAD     0X61        //read data 1-32 byte  From the beginning of 0 bytes
#define W_TX_PAYLOAD     0XA0        //write data 1-32 byte  From the beginning of 0 bytes
#define FLUSH_TX         0xE1        //clear TX FIFO regsiters
#define FLUSH_RX         0XE2        //clear RX FIFO regsiters  This command should not be used when the transfer acknowledge signal
#define RESUSE_TX_PL     0XE3        //Re-use on a packet of valid data when CE is high, the data packets continually re-launch
#define R_RX_PL_WID      0x60 
#define NOP              0XFF        //Empty command is used to retrieve data

char read_irq           (void);
void clr_irq            (void);
char read_fifo_status   (void);
void read_rx            (char *data, char *length);
void tx_mode            (void);
void rx_mode            (void); 
void send_data          (char *data, char N);
void SetID              (char *data);
void NRF24L01_hack_mode (bool state);

bool HackModeState          = false;
volatile bool Tx_Run        = false;
bool TxMode                 = false;
bool RxMode                 = false;
volatile bool NRF_IRQ_State = true;

/*
    1.Сделать макросы для чтения и записи команд, а также массивов
    2.Написать дефаны для всех команд и битов
    3.Сделать рефакторинг функций
*/

static void NRF_IRQ_Disable(void){
 NRF_IRQ_State = false;
}

static void NRF_IRQ_Enable(void){
 NRF_IRQ_State = true;
}

void tx_mode(void){

    NRF_IRQ_Disable();
    ce=0;
    csn=0;
    spi(0x20);
    spi(0xE); 
    csn=1;     
    TxMode=true;
    RxMode=false;
    delay_us(150);
    NRF_IRQ_Enable();   
}

void rx_mode(void){

    NRF_IRQ_Disable();  
    csn=0;
    spi(0x20);
    spi(0xF);
    csn=1;
    ce=1;     
    RxMode=true;
    TxMode=false;
    NRF_IRQ_Enable();
    delay_us(150); 
}

void send_data(char *data, char N){ 
    
    char a;  
    
    NRF_IRQ_Disable();    
    if(!TxMode) tx_mode();  
    while(read_irq() & 0x1); 
    csn=0;
    spi(0xE1);
    csn=1;
    //delay_us(10);
    csn=0;
    spi(0xA0);
    for (a=0;a<N;a++){spi(data[a]); }
    csn=1;
    NRF_IRQ_Enable();  
    ce=1;
    delay_us(15);
    ce=0;               
    Tx_Run = true;     
}

void read_rx(char *data, char *length){

    char a, pos = 0, p_length, n = 0;
    
    *length = 0;       
    dbg_puts("<r ");
   
    while(!(read_fifo_status() & 1) && n != 3) {
        n++;
        csn=0; 
        spi(R_RX_PL_WID);
        p_length = spi(0xFF); 
        csn=1;
        dbg_printf("[%u]:%u B",n,p_length);
        *length += p_length;   
        csn=0;
        spi(0x61);
        for (a = 0; a < p_length; a++) data[pos++] = spi(0xFF);
        csn=1; 
        data[pos]='\0';  
    }   
   dbg_puts(" /r> ");      
}

void SetID(char *data){
 char i;
 bit I=SREG.7; 
 #asm("cli")
 csn=0;
 spi(0x2A);   //Enable dyn. payload length for all data pipes
 for(i=0;i<5;i++) spi(data[i]);
 csn=1;
 csn=0;
 spi(0x30);   //Enable dyn. payload length for all data pipes
 for(i=0;i<5;i++) spi(data[i]);
 csn=1;
 SREG.7=I;
}

char read_fifo_status(){

    char status;
    
    csn=0;
    spi(0x17);
    status = spi(0xFF);
    csn=1;
          
 return status;
}

unsigned char read_irq(){ 

    char status;

    csn=0;
    spi(0x07);
    status = spi(0xFF);
    csn=1;      

 return status;
}

void clr_irq(){ 
 
    char status;
     
    dbg_puts(" (c ");
    csn=0;
    spi(0x07);
    status = spi(0xFF);
    csn=1;
    dbg_printf("%x",status);
    csn=0;
    spi(0x27);
    spi(status); 
    csn=1;  
    dbg_puts(" /c)");    
}
                              
void NRF24L01_init(void){

    bit I=SREG.7;
     
    #asm("cli")
    csn=1;
    ce=0;

    csn=0;
    spi(0x50);
    spi(0x73);  //Activate  feuatre register (only for NRF24L01)
    csn=1;

    csn=0;
    spi(0x3D);
    spi(0x06);  // 0x04-Enables Dynamic Payload Length  | 0x2  - Enables Payload with ACK
    csn=1; 
               
    csn=0;
    spi(0x3C);  //Enable dyn. payload length for all data pipes
    spi(0x3F);
    csn=1;
    
    delay_us(10);
    
    csn=0;
    spi(0x26);  // Disable LNA Gain  
    spi(0x0E);
    csn=1;
    
    csn=0;
    spi(0x20);
    spi(0xE);   //Config(0x0E) - CRC-2bytes | EN_CRC | PWR_UP
    csn=1;
    
    csn=0;
    spi(0x24);
    spi(0x13);  //Setup_Retr   500us 3-Re-Transmit
    csn=1;
    
    csn=0;            
    spi(0xE2);  // Flush RX FIFO
    csn=1;            
    
    csn=0;            
    spi(0xE1);  // Flush TX FIFO
    csn=1;

    delay_ms(1);
    clr_irq();
    SREG.7=I;
}

void NRF24L01_hack_mode(bool state){

    bit I=SREG.7; 
    #asm("cli")
    HackModeState = state;
    csn=0;
    spi(0x21);
    if(state)spi(0x00);
    else spi(0x3F);
    csn=1;
    SREG.7=I;
}

char NRF24L01_ReadRigester(char addr){

    char value;
    bit I=SREG.7;
     
    #asm("cli")
    csn=0;
    spi(addr);
    value=spi(0xFF);
    csn=1;
    SREG.7=I; 
    
return value;
}

void NRF24L01_WriteRigester(char addr, char value)
{
    bit I=SREG.7; 
    #asm("cli")
    csn=0;
    spi(addr+0x20);
    spi(value);
    csn=1;   
    delay_ms(1);
    SREG.7=I;
}