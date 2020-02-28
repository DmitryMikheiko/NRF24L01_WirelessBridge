/*******************************************************
This program was created by the
CodeWizardAVR V3.12 Advanced
Automatic Program Generator
© Copyright 1998-2014 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : Wireless Bridge
Version : 
Date    : 19.08.2015    // v2.0 17.08.2016
Author  : MIH
Company : MIH Electro
Comments: 
Nrf24L01
Atmega8
16.0MHz
PD2 - Irq
PC0 - CSN
PC1 - CE
PB3 - Mosi
PB4 - Miso
PB5 - SCK 

Chip type               : ATmega8A
Program type            : Application
AVR Core Clock frequency: 7,372800 MHz
Memory model            : Small
External RAM size       : 0
Data Stack size         : 256
*******************************************************/

#include <mega8.h>
#include <delay.h>
#include <stdio.h>
#include <spi.h>
#include <nrf24l01.c>
#include <stdbool.h>
#include <string.h>
#include "CRC.C"
#include "WB_Protocol.c"
#include "Config.h"
// Declare your global variables here


volatile bool HackMode          =   false;
volatile bool SafeMode          =   false;
volatile bool WaitData          =   false;
volatile bool FullPack          =   false;
volatile bool ID_wait           =   false;
volatile bool RR_WaitData       =   false;
volatile bool WR_WaitData       =   false;
volatile bool ReturnToReadUart  =   false; 
volatile bool Tx_Completed      =   false;
volatile bool Tx_Failed         =   false;
extern volatile bool Tx_Run;
extern volatile bool NRF_IRQ_State;

char data[33];
char out[NRF_RX_BUFFER];
char ID[5];
char pos;
char data_count = 0;

void NRF_CheckTx(bool RxNeed);

// External Interrupt 0 service routine
interrupt [EXT_INT0] void ext_int0_isr(void)
{    
    char length,a,stat;      
    if(!NRF_IRQ_State) {         
        dbg_puts(" <i ret> ");
        return;    
    } 
    
    stat=read_irq();  
    dbg_printf(" <i %x ", stat);    
    if(stat & 0x40){ 
        read_rx(out,&length); 
        for(a=0;a<length;a++)putchar(out[a]);   
         /*if(strcmp(out,">C-NODE-10\r\n") == 0){ 
          LED5 = 1;
          LED4 = 1;
          LED3 = 1;
          LED2 = 1;
          LED1 = 1;
          LED0 = 1;
          delay_ms(100);
          LED5 = 0;     
          LED4 = 0;
          LED3 = 0;
          LED2 = 0;
          LED1 = 0;
          LED0 = 0;
         }  */
        if(HackModeState) putchar('>');
    }
    else if(stat & 0x20){ dbg_puts(" + "); Tx_Completed = true; }
    else if(stat & 0x10){ dbg_puts(" - "); Tx_Failed = true; }
     
    clr_irq();                                
    Tx_Run = false; 
    dbg_printf(" ret> ");   
}

void NRF_CheckTx(bool RxNeed){

    if(!(Tx_Completed || Tx_Failed)) return;
    if(Tx_Failed) putsf(tx_error);
    Tx_Completed = false; 
    Tx_Failed = false;
    if(RxNeed) rx_mode();
    Tx_Run = false;
}
#define DATA_REGISTER_EMPTY (1<<UDRE)
#define RX_COMPLETE (1<<RXC)
#define FRAMING_ERROR (1<<FE)
#define PARITY_ERROR (1<<UPE)
#define DATA_OVERRUN (1<<DOR)

// USART Receiver buffer

char rx_buffer[RX_BUFFER_SIZE];

#if RX_BUFFER_SIZE <= 256
char rx_wr_index = 0,rx_rd_index = 0;
#else
unsigned int rx_wr_index = 0,rx_rd_index = 0;
#endif

#if RX_BUFFER_SIZE < 256
volatile unsigned char rx_counter = 0;
#else
unsigned int rx_counter=0;
#endif

// This flag is set on USART Receiver buffer overflow
bit rx_buffer_overflow;

// USART Receiver interrupt service routine
interrupt [USART_RXC] void usart_rx_isr(void)
{
    char status  =   UCSRA;
    char data    =   UDR; 
    
    if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0){
        
        rx_buffer[rx_wr_index++]=data;
    #if RX_BUFFER_SIZE == 256
   // special case for receiver buffer size=256
        if (++rx_counter == 0) rx_buffer_overflow=1;
    #else
        if (rx_wr_index == RX_BUFFER_SIZE) rx_wr_index=0;
        if (++rx_counter == RX_BUFFER_SIZE)
        {
            rx_counter=0;
            rx_buffer_overflow=1;
        }
    #endif
   }  
   else putsf(uart_error);
}

#ifndef _DEBUG_TERMINAL_IO_
// Get a character from the USART Receiver buffer
#define _ALTERNATE_GETCHAR_
#pragma used+
char getchar(void)
{
    char data;
    while(rx_counter == 0);
    data = rx_buffer[rx_rd_index++];
    #if RX_BUFFER_SIZE != 256
    if (rx_rd_index == RX_BUFFER_SIZE) rx_rd_index = 0;
    #endif
    #asm("cli")
    --rx_counter;
    #asm("sei")
    return data;
}
#pragma used-
#endif

// Standard Input/Output functions

void main(void)
{
// Declare your local variables here
    char length     = 0;    
    char wd;
    bool time_out   = false;    
    char INum       = 255;  
    unsigned short int CRC;
// Input/Output Ports initialization
// Port B initialization
// Function: Bit7=In Bit6=In Bit5=Out Bit4=In Bit3=Out Bit2=Out Bit1=In Bit0=In 
DDRB=(0<<DDB7) | (0<<DDB6) | (1<<DDB5) | (0<<DDB4) | (1<<DDB3) | (1<<DDB2) | (1<<DDB1) | (1<<DDB0);
// State: Bit7=T Bit6=T Bit5=0 Bit4=P Bit3=0 Bit2=0 Bit1=T Bit0=T 
PORTB=(0<<PORTB7) | (0<<PORTB6) | (0<<PORTB5) | (1<<PORTB4) | (0<<PORTB3) | (0<<PORTB2) | (0<<PORTB1) | (0<<PORTB0);

// Port C initialization
// Function: Bit6=In Bit5=In Bit4=In Bit3=In Bit2=In Bit1=Out Bit0=Out 
DDRC=(1<<DDC6) | (1<<DDC5) | (1<<DDC4) | (1<<DDC3) | (1<<DDC2) | (1<<DDC1) | (1<<DDC0);
// State: Bit6=T Bit5=T Bit4=T Bit3=T Bit2=T Bit1=0 Bit0=0 
PORTC=(0<<PORTC6) | (0<<PORTC5) | (0<<PORTC4) | (0<<PORTC3) | (0<<PORTC2) | (0<<PORTC1) | (0<<PORTC0);

// Port D initialization
// Function: Bit7=In Bit6=In Bit5=In Bit4=In Bit3=In Bit2=In Bit1=In Bit0=In 
DDRD=(0<<DDD7) | (0<<DDD6) | (0<<DDD5) | (0<<DDD4) | (0<<DDD3) | (0<<DDD2) | (0<<DDD1) | (0<<DDD0);
// State: Bit7=T Bit6=T Bit5=T Bit4=T Bit3=T Bit2=P Bit1=T Bit0=T 
PORTD=(0<<PORTD7) | (0<<PORTD6) | (0<<PORTD5) | (0<<PORTD4) | (0<<PORTD3) | (1<<PORTD2) | (0<<PORTD1) | (0<<PORTD0);

// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: Timer 0 Stopped
TCCR0=(0<<CS02) | (0<<CS01) | (0<<CS00);
TCNT0=0x00;

// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: Timer1 Stopped
// Mode: Normal top=0xFFFF
// OC1A output: Disconnected
// OC1B output: Disconnected
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer1 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: Off
// Compare B Match Interrupt: Off
TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);
TCCR1B=(0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (0<<WGM12) | (0<<CS12) | (0<<CS11) | (0<<CS10);
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=0x00;
OCR1AL=0x00;
OCR1BH=0x00;
OCR1BL=0x00;

// Timer/Counter 2 initialization
// Clock source: System Clock
// Clock value: Timer2 Stopped
// Mode: Normal top=0xFF
// OC2 output: Disconnected
ASSR=0<<AS2;
TCCR2=(0<<PWM2) | (0<<COM21) | (0<<COM20) | (0<<CTC2) | (0<<CS22) | (0<<CS21) | (0<<CS20);
TCNT2=0x00;
OCR2=0x00;

// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=(0<<OCIE2) | (0<<TOIE2) | (0<<TICIE1) | (0<<OCIE1A) | (0<<OCIE1B) | (0<<TOIE1) | (0<<TOIE0);

// External Interrupt(s) initialization
// INT0: On
// INT0 Mode: Falling Edge
// INT1: Off
GICR|=(0<<INT1) | (1<<INT0);
MCUCR=(0<<ISC11) | (0<<ISC10) | (1<<ISC01) | (0<<ISC00);
GIFR=(0<<INTF1) | (1<<INTF0);

// USART initialization
// Communication Parameters: 8 Data, 1 Stop, No Parity
// USART Receiver: On
// USART Transmitter: On
// USART Mode: Asynchronous
// USART Baud Rate: 19200
UCSRA=(0<<RXC) | (0<<TXC) | (0<<UDRE) | (0<<FE) | (0<<DOR) | (0<<UPE) | (UART_U2X<<U2X) | (0<<MPCM);
UCSRB=(1<<RXCIE) | (0<<TXCIE) | (0<<UDRIE) | (1<<RXEN) | (1<<TXEN) | (0<<UCSZ2) | (0<<RXB8) | (0<<TXB8);
UCSRC=(1<<URSEL) | (0<<UMSEL) | (0<<UPM1) | (0<<UPM0) | (0<<USBS) | (1<<UCSZ1) | (1<<UCSZ0) | (0<<UCPOL);
UBRRH=0x00;
UBRRL=UART_UBRRL;    //23-19200 3-115200

// Analog Comparator initialization
// Analog Comparator: Off
// The Analog Comparator's positive input is
// connected to the AIN0 pin
// The Analog Comparator's negative input is
// connected to the AIN1 pin
ACSR=(1<<ACD) | (0<<ACBG) | (0<<ACO) | (0<<ACI) | (0<<ACIE) | (0<<ACIC) | (0<<ACIS1) | (0<<ACIS0);
SFIOR=(0<<ACME);

// ADC initialization
// ADC disabled
ADCSRA=(0<<ADEN) | (0<<ADSC) | (0<<ADFR) | (0<<ADIF) | (0<<ADIE) | (0<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);

// SPI initialization
// SPI Type: Master
// SPI Clock Rate: 1843,200 kHz
// SPI Clock Phase: Cycle Start
// SPI Clock Polarity: Low
// SPI Data Order: MSB First
SPCR=(0<<SPIE) | (1<<SPE) | (0<<DORD) | (1<<MSTR) | (0<<CPOL) | (0<<CPHA) | (0<<SPR1) | (0<<SPR0);
SPSR=(0<<SPI2X);

// TWI initialization
// TWI disabled
TWCR=(0<<TWEA) | (0<<TWSTA) | (0<<TWSTO) | (0<<TWEN) | (0<<TWIE);

// Global enable interrupts

delay_ms(1000);

NRF24L01_init();
rx_mode();  
clr_irq();   

#ifdef TestBoardRemap
printf("\r\nNRF24L01+ demo board adapter\r\nBaudrate: %p\r\n", UART_BAUD_RATE_STR);
#else
printf("\r\nNRF24L01+ adapter\r\nBaudrate: %p\r\n", UART_BAUD_RATE_STR);
#endif
#asm("sei")
    while (1){      
        time_out = false;
        while(rx_counter == 0) NRF_CheckTx(true); 
        for(length = 0; length < 32;){             
            if(SafeMode){  
                data[length++] = getchar();   
                 if(!WaitData){
                    if(data[0] != '$')length--; 
                    else{ 
                        data[length]='\0';  
                        INum=FindInstruction(data,length);
                        if(INum != 255){          
                            IProcc(INum, data);
                            length=0;
                        }
                     }
                 } 
             else{                    
                if(data_count==0) data_count = data[0];
                else{
                    if(length==(data_count+1)){ 
                        data_count  = 0;
                        WaitData    = false; 
                        #ifdef CRCEN
                        CRC = CRC16(data,length-2);
                        if((data[length-2] != (char)CRC) || (data[length-1] != (char)(CRC>>8))){
                            ReturnToReadUart=true;
                            putsf(crc_error_message);  
                        }
                        #endif
                    if(RR_WaitData){ IProcc(5, data);  ReturnToReadUart = true;}
                    if(WR_WaitData){ IProcc(6, data);  ReturnToReadUart = true;} 
                    break;    
                  }
                }
                        
             }
                while(rx_counter == 0);
            }
            else {
                if(!FullPack){
                    wd = CharWaitDelay;
                    while(rx_counter==0){
                        delay_us(100);
                        if(--wd==0){
                            time_out = true;
                            break;
                        }
                    }          
                    if(!time_out) 
                        data[length++] = getchar(); 
                    else break;  
                }   
                else {
                    data[length++]=getchar();   
                }
            } 
            NRF_CheckTx(false);
        }       
        if(ReturnToReadUart){
            ReturnToReadUart = false;
            continue;
        }  
       
        data[length] = '\0'; 
          
        if(ID_wait){      
            ID[0]=data[1];
            ID[1]=data[2];
            ID[2]=data[3];
            ID[3]=data[4];
            ID[4]=data[5];
            SetID(ID);
            ID_wait = false;
            printf("%s",true_message); 
            continue;
        } 
              
        if(!SafeMode){   
            INum = FindInstruction(data,length);
            if(INum != 255){ 
                IProcc(INum,data);
                continue;  
            }
        }   
    
        dbg_puts("\r\n{");
        while(Tx_Run) NRF_CheckTx(false); 
        dbg_printf("TX[%u]",length);
        send_data(data,length); 
        dbg_putc('}');              
      }      
}
