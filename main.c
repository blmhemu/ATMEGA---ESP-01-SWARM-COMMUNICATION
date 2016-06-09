/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^  ESP - UART   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>

/*------------MACROS--------------*/

#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
#define buff_size 128

int density[5];                                               // DENSITY OF COLOURED PATCHES
int X = 0,Y = 0 ;                                           // COORDINATES OF HIGHEST DENSITY

int flag=1;
/*-----------FUNCTIONS------------*/


void initUART(void) {                                          // INITIALIZES UART
    
    UBRRL = BAUD_PRESCALE;                                     // SETS THE PRESCALER TO TRANSMIT @ 9600 BAUD
    UBRRH = (BAUD_PRESCALE >> 8);
    UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1);       // SETS 8 BIT DATA FRAME
    UCSRB |= (1 << RXEN)  | (1 << TXEN);                       // RX AND TX ENABLED
    //UCSRB |= (1 << RXCIE) | (1 << TXCIE);                    // RECEIVE COMPLETE AND TRANSMIT COMPLETE INTERRUPT ENABLED

}

void transmitByte(char c){                                     // TRASMITS A BYTE DATA
    
    while(!(UCSRA & (1<<UDRE)));                               // WAIT UNTIL DATA REGISTER BECOME EMPTY AND THEN TRANSMIT
    UDR=c;
    
}

char receiveByte(void){                                        // RECEIVES A BYTE DATA
    
    while(!(UCSRA & (1<<RXC)));                                // WAIT UNTIL RECEIVING IS COMPLETE AND RETURN THE DATA
    return (char)UDR;
    
}

void transmitString(char c[]){                                 // TRANSMITING A STRING OF DATA
    
    for(int i=0;i<strlen(c);i++)
        transmitByte(c[i]);

}

int ifIPD(char *p){                                            // AUXILIARY CODE TO RECEIVE SOME USEFUL DATA
    if((p[1]=='I')&&(p[2]=='P')&&(p[3]=='D'))
    {
        static int j=0;
        density[j] = 0;
        int i=7;
        int length = 0;
        while( p[i] != ':')
        {
            length = 10 * length + (p[i] - '0');
            i++;
        }
        length = length-2;                                      // LENGTH - 2 AS \r AND \n ARE ALSO CONSIDERED IN DATA
        if(j<5){
            for(i=i;i<length;i++){
                density[j] = 10 * density[j] + (p[i]- '0');
            }
            j++;
            return 2;
        }
        if(j==5){
            int cc=0;
            int k=0;
            for(k=0;k< length;k++){
                if(p[i]==','){
                    cc=1;
                    continue;
                }
                if(cc==0)X = 10*X + (p[i]-'0');
                if(cc==1)Y = 10*Y + (p[i]-'0');
            }
        }
    }
    return 0;
}

int if_OK( char *p)
{
    if(p[1]== 'K')return 1;
    else return 0;
}

int if_Error(char *p)
{
    if((p[1]=='R')&&(p[2]=='R')&&(p[3]=='O')&&(p[4]=='R'))return -1;
    else return 0;
}


void check(void){
    char c[50];
    int i=0;
    for(i=0;i<50;i++){
        c[i]=receiveByte();
    }
    int check = 0;
    for(i=0;i<50;i++)
    {
        if(c[i] == 'O')check = if_OK(&c[i]);
        if(c[i] == 'E')check = if_Error(&c[i]);
        if(c[i] == '+')check = ifIPD(&c[i]);
        
    }
}

void clientMode(void){
    
    transmitString("AT+RST\r\n");                             // RESET THE ESP
    _delay_ms(10000);                                         // TIME FOR ESP TO RESET
    transmitString("AT+CWMODE=1\r\n");                        // SET ESP IN BOTH STATION MODE
    _delay_ms(100);
    transmitString("AT+CIPMUX=0\r\n");                        // ENABLE SINGLE CONNECTION MODE
    _delay_ms(100);
    transmitString("AT+CIPSTART=0,\"TCP\",\"IP\",80\r\n");    // JOIN TO THE SERVER **__** ENTER THE IP OF SERVER **__**
    _delay_ms(100);

}

/*---------MAIN FUNCTION----------*/

int x=0;
int main(void) {
    initUART();                                                // UART INITIALIZATION
    clientMode();                                              // SETUP AS CLIENT MODE
    while (1) {
        check();
    }
    
	return 0;                                                  // never reached
}
