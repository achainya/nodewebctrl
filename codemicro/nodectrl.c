#include "16F877A.h"
//#device PASS_STRINGS=IN_RAM 
#device ADC = 8
#include <stdlib.h>


#fuses HS, NOWDT, NOPROTECT, NOLVP   // Definimos los FUSES para el uC PIC16F877

#use delay(clock=20000000)   // Frecuencia de reloj (20MHz)
#org 0x1F00, 0x1FFF void loader() { }   // Definimos el segmento para el Bootloader
#use standard_io(d)
#use standard_io(e)
#use rs232(baud=9600, xmit=PIN_C6, rcv=PIN_C7)

#define  LR     PIN_E0
#define  LY     PIN_E1
#define  LB     PIN_E2

#include "LCD416.c"

// Constantes y definiciones ////////////////////////////////////////////////////////////

int const MAXLENBUFF=5;                         // Maxima longitud del buffer de recepción RS232

// Variables Globales ///////////////////////////////////////////////////////////////////

char buffrec[MAXLENBUFF];                        // Buffer de Recepcion
int  xbuffrec=0x00;                              // índice del Buffer de Recepcion

int1 new_command=0;                              // Flag para indicar comando disponible
int1 pdte_send_prompt=0;                         // Flags para enviar echos fuera de la int_rda
int1 pdte_send_tmp_prompt=0;

char statusPortE = 0x00;

#bit statusLed1 = statusPortE.0
#bit statusLed2 = statusPortE.1
#bit statusLed3 = statusPortE.2

// Declaracion de Funciones /////////////////////////////////////////////////////////////

void On_reset(void);                             // Tras RESET del Micro
void Ini_buffrec(void);                          // Borra el buffer de recepción
void Add_buffrec(char c);                        // Añade un caracter al buffer de recepción
void controlLeds(char *command);

//================================================================
//Definiendo Variables Globales
//================================================================

unsigned int sensorValue = 0;
int8 i;

// INTERRUPCIONES ///////////////////////////////////////////////////////////////////////

// INTERRUPCION RDA - Recepción USART -

#int_rda
void serial_isr() {
   if(kbhit()){                                  // Si hay algo pendiente de recibir ...
      Add_buffrec(getc());                       // lo recibo y lo añado al Buffer de Recepcion
   }
}

// Control del Buffer de recepcion ---------------------------------------

void Ini_buff_rec(void){                         // Inicia a "\0" el Buffer de Recepcion

   int i;
   for(i=0;i<MAXLENBUFF;i++){                    // Bucle que pone a 0 todos los
      buffrec[i]=0x00;                           // caracteres en el Buffer de Recepcion
   }
   xbuffrec=0x00;                                // Inicializo el indice de siguiente caracter recibido
}

void Add_buffrec(char c){                         // Añade caracter al Buffer de Recepcion
      switch(c){
         case 0x0D:                              // [Enter] -> Habilita Flag para procesar comando en Main
            buffrec[xbuffrec++]=' ';             // Añade un espacio en blanco al final y ...
            new_command=1;                       // Habilita Flag para procesar comando en Main
            break;
         default:
            buffrec[xbuffrec++]=c;               // Añade caracter recibido al Buffer
            //putc(c);                             // y lo monitorizo
      }
}

// Rutinas y Funciones de RROS ///////////////////////////////////////////

void On_reset(void){                             // Inicializacion del Micro tras RESET

   disable_interrupts(GLOBAL);                   // todas las interrupciones desactivadas

   delay_ms(100);

   Ini_buff_rec();                               // Inicializo Buffer de recepcion
   new_command=0;                                // Desactivo flag de comando pendiente.
   pdte_send_prompt=0;                           // Desactivo Flags para enviar echo fuera de la int_rda
   pdte_send_tmp_prompt=0;

   enable_interrupts(int_rda);                   // interrupcion RDA habilitada
   enable_interrupts(global);                    // todas las interrupciones activadas
}

void controlLeds(char *command) {
  char Led, Estado;
  Led = command[0];
  Estado = command[1];
  if(Estado == 'h') {
    output_high(Led + 23);
    switch (Led) {
      case '1':   statusLed1 = 1; break;
      case '2':   statusLed2 = 1; break;
      case '3':   statusLed3 = 1; break;
    }
  } else {
    output_low(Led + 23);
    switch (Led) {
      case '1':   statusLed1 = 0; break;
      case '2':   statusLed2 = 0; break;
      case '3':   statusLed3 = 0; break;
    }
  }
  lcd_gotoxy(1, 4); printf(lcd_putc, ">%u", statusPortE);
}

void main() {
   unsigned int X, Y;
   lcd_init();
   lcd_putc('\f');
   set_tris_a(0b00000001);
   set_tris_e(0b00000000);
   setup_adc(ADC_CLOCK_INTERNAL);
   setup_adc_ports(AN0);
   set_adc_channel(0);
   sensorValue = read_adc();
   delay_us(20);
   On_reset();
   lcd_gotoxy(1, 1); printf(lcd_putc, "NodeWebCtrl v0.1");
   lcd_gotoxy(3, 2); printf(lcd_putc, "UNAP - EPIE");
   lcd_gotoxy(4, 3); printf(lcd_putc, "@achainya");
                     
   output_e(0x00);
   srand(256);
   do {
      
      // Deteccion de la necesidad de procesar comandos pendientes ///////
      if(new_command==1){                        // Si hay un nuevo comando ...
         new_command=0;                          // deshabilito el aviso y ...
         controlLeds(buffrec);
         Ini_buff_rec();                         // inicializo todo para nuevo comando y ...
      }

      X = (rand()%255) + 1;
      Y = (rand()%255) + 1;
      set_adc_channel(0);
      sensorValue = read_adc();
      delay_us(20);
	   printf("{\"x\":\"%03u\",\"y\":\"%03u\",\"z\":\"%03u\"}\r", X, Y, sensorValue);
      lcd_gotoxy(1, 4); printf(lcd_putc, "x:%03uy:%03uz:%03u", X, Y, sensorValue);
      delay_ms(200);

   } while(TRUE);
}
