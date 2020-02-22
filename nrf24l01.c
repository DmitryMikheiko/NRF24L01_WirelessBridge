#include <stdbool.h> 

#define TestBoardRemap

#ifdef TestBoardRemap
#define ce  PORTB.2             //PORTB.2 - Atmega8 test board
#define csn PORTB.1
#else
#define ce  PORTC.1            
#define csn PORTC.0
#endif

#define R_REGISTER = 0X00 ;        //Read registers directly address bitwise
#define W_REGISTER = 0X20;        //write registers      Bitwise OR with address
#define R_RX_PAYLOAD = 0X61;        //read data 1-32 byte  From the beginning of 0 bytes
#define W_TX_PAYLOAD = 0XA0;        //write data 1-32 byte  From the beginning of 0 bytes
#define FLUSH_TX = 0xE1;        //clear TX FIFO regsiters
#define FLUSH_RX = 0XE2;        //clear RX FIFO regsiters  This command should not be used when the transfer acknowledge signal
#define RESUSE_TX_PL = 0XE3;        //Re-use on a packet of valid data when CE is high, the data packets continually re-launch
#define NOP = 0XFF ;        //Empty command is used to retrieve data
const unsigned char R_RX_PL_WID = 0x60; 
unsigned char read_irq();
void clr_irq();
void read_rx(unsigned char *data,unsigned char *length);
void tx_mode(void);
void rx_mode(void); 
void send_data(unsigned char *data,unsigned char N);
void SetID(unsigned char *data);
void NRF24L01_hack_mode(bool state);
bool HackModeState=false;
volatile bool Tx_Run=false;
bool TxMode=false;
bool RxMode=false;

unsigned char read_fifo_status()
{unsigned char stat;
 bit I=SREG.7; 
 #asm("cli")
 csn=0;
 spi(0x17);
 stat=spi(0xFF);
 csn=1;      
 SREG.7=I;
 return stat;
}
void  tx_mode(void)
{
 bit I=SREG.7; 
 #asm("cli") 
 ce=0;
 csn=0;
 spi(0x20);
 spi(0xE); 
 csn=1;
 ce=1;     
 TxMode=true;
 RxMode=false;
 delay_ms(2);   
 SREG.7=I;
}
void rx_mode(void)
{
 bit I=SREG.7; 
 #asm("cli")  
 ce=1;
 csn=0;
 spi(0x20);
 spi(0xF);
 csn=1;     
 RxMode=true;
 TxMode=false;
 delay_ms(2);
 SREG.7=I; 
}
void send_data(unsigned char *data,unsigned char N)
{ unsigned char a;
 bit I=SREG.7; 
 #asm("cli")    
 if(!TxMode) tx_mode();  
  while(read_irq() & 0x1); 
 csn=0;
 spi(0xE1);
 csn=1;
 delay_us(10);
 csn=0;
 spi(0xA0);
 for (a=0;a<N;a++){spi(data[a]); }
 csn=1;  
 clr_irq();               
 Tx_Run=true; 
 SREG.7=I;    
}
void read_rx(unsigned char *data,unsigned char *length)
{unsigned char a,pos=0;
 
 bit I=SREG.7; 
 #asm("cli")    
 while(!(read_fifo_status()&1))
 {
 csn=0;
 spi(R_RX_PL_WID);
 *length=spi(0xFF);
 csn=1;   
 csn=0;
 spi(0x61);
 for (a=0;a<(*length);a++) data[pos++]=spi(0xFF);
 csn=1;
 csn=0;
 spi(0xE2);
 csn=1; 
 data[pos]='\0';
 clr_irq();  
 }
 SREG.7=I; 
}
unsigned char read_irq()
{unsigned char stat;
 bit I=SREG.7; 
 #asm("cli")
 csn=0;
 spi(0x07);
 stat=spi(0xFF);
 csn=1;      
 SREG.7=I;
 return stat;
}
void SetID(unsigned char *data)
{
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
void clr_irq()
{ unsigned char stat;
 bit I=SREG.7; 
 #asm("cli")
 stat=read_irq();
 csn=0;
 spi(0x27);
 spi(stat); 
 csn=1;    
 SREG.7=I;
}                              
void NRF24L01_init(void)
{
bit I=SREG.7; 
#asm("cli")
csn=1;
ce=0;

csn=0;
spi(0x50);
spi(0x73);   //Activate
csn=1;

csn=0;
spi(0x3D);
spi(0x06);   // 0x04-Enables Dynamic Payload Length  | 0x2  - Enables Payload with ACK
csn=1;            
csn=0;
spi(0x3C);   //Enable dyn. payload length for all data pipes
spi(0x3F);
csn=1;
delay_us(10);
csn=0;
spi(0x26);    // Disable LNA Gain  
spi(0x0E);
csn=1;
csn=0;
spi(0x20);
spi(0xE);   //Config(0x0E) - CRC-2bytes | EN_CRC | PWR_UP
csn=1;
csn=0;
spi(0x24);
spi(0x13);   //Setup_Retr   500us 3-Re-Transmit
csn=1;
delay_ms(1);
clr_irq();
SREG.7=I;
}
void NRF24L01_hack_mode(bool state)
{
bit I=SREG.7; 
#asm("cli")
HackModeState=state;
csn=0;
spi(0x21);
if(state)spi(0x00);
else spi(0x3F);
csn=1;
SREG.7=I;
}
unsigned char NRF24L01_ReadRigester(unsigned char addr)
{
unsigned char value;
bit I=SREG.7; 
#asm("cli")
csn=0;
 spi(addr);
 value=spi(0xFF);
 csn=1;
SREG.7=I;
return value;
}
void NRF24L01_WriteRigester(unsigned char addr,unsigned char value)
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