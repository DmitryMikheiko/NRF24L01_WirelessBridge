#ifndef _CONFIG_
#define _CONFIG_

#define UART_BAUD_RATE_115200bps  
//#define UART_BAUD_RATE_500kbps    
//#define UART_BAUD_RATE_1Mbps      

#define TestBoardRemap
//#define DBG_INFO
#define CRCEN
#define CharWaitDelay       100     // *100us 200*100us=20ms
#define RX_BUFFER_SIZE      40      // uart input buffer size 32..x bytes
#define NRF_RX_BUFFER       97      // 33..x bytes (97 bytes is preferable)

#ifdef TestBoardRemap
        
    #define UART_U2X 1
    
    #ifdef UART_BAUD_RATE_1Mbps 
        flash const char *UART_BAUD_RATE_STR = "1 Mbps"; 
        #define UART_UBRRL 1    // 16 - 115200 bps | 1 - 1Mbps | 3 - 0.5Mbps 
    #elif defined(UART_BAUD_RATE_500kbps)
        flash const char *UART_BAUD_RATE_STR = "500 kbps"; 
        #define UART_UBRRL 3
    #else
        flash const char *UART_BAUD_RATE_STR = "115200 bps";
        #define UART_UBRRL 16   // 115200bps
    #endif
#else
    #define UART_U2X 0 
    
    #ifdef UART_BAUD_RATE_1Mbps
        flash const char *UART_BAUD_RATE_STR = "500 kbps";
        #define UART_UBRRL 0    // 3 - 115200 bps | 1 - 1Mbps | 0 - 0.5Mbps 
    #elif defined(UART_BAUD_RATE_500kbps)
        flash const char *UART_BAUD_RATE_STR = "500 kbps"; 
        #define UART_UBRRL 0
    #else 
        flash const char *UART_BAUD_RATE_STR = "115200 bps";
        #define UART_UBRRL 3   // 115200bps
    #endif
#endif

#define LED5 PORTC.5
#define LED4 PORTC.4
#define LED3 PORTC.5
#define LED2 PORTC.2
#define LED1 PORTC.1
#define LED0 PORTC.0

#ifdef DBG_INFO
#define dbg_puts puts
#define dbg_putc putchar
#define dbg_printf printf
#else
#define dbg_puts(s);
#define dbg_putc(c); 
#define dbg_printf(f,...); 
#endif

#endif