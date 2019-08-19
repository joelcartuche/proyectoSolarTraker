/*
 * ProyectoSolarTracker.c
 *
 * Created: 12/08/2019 9:20:28
 * Author : Joel Cartuche
 */ 

#define F_CPU 16000000UL
#include <inttypes.h>
#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
 #include <avr/sleep.h>
#include <avr/eeprom.h>
#include "uart.h"
#include <stdlib.h>
#include <stdbool.h> //libreria para valores booleanos

#define t1 250
#define eeprom_true 0
#define eeprom_data 1


#define Vref 5.00

void datosSensores(void);
void iniciarConversion(char data);
void initialize(void);
void controlServoInferior(char tiempo);

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

volatile unsigned int time1, time2,contador,sensorSuperiorDerecha,sensorSuperiorIzquierda,sensorInferiorDerecha,sensorInferiorIzquierda;
bool verificar; // variable booleana para almacenar o verificar si el usario a pulsado el boton cuando el led se ha ensendido
unsigned int pulsador,timeSeg,timeMin; // variable que almacena 1 o 0 para cuando se quiera un tiempo de 1 o 2 segundos
unsigned char contadorServoSuperior,contadorServoInferior,datoServoSuperior,datoServoInferior;

volatile int Ain, AinLow ; 		//raw data Analog to Digital number
float voltage ;		//scaled input voltage
char v_string[10]; // scaled input voltage string to print


ISR (ADC_vect)
{
	AinLow = (int)ADCL;
	Ain = (int)ADCH*256;
	Ain = Ain + AinLow;
	
}

void Wait()
{
	uint8_t i;
	for(i=0;i<5;i++)//desplazado como 50
	{
		_delay_loop_2(0);
	}

}

void initialize(void)
{	
	
	
	   TCCR1A|=(1<<COM1A1)|(1<<COM1B1)|(1<<WGM11);        //NON Inverted PWM
	   TCCR1B|=(1<<WGM13)|(1<<WGM12)|(1<<CS11)|(1<<CS10); //PRESCALER=64 MODE 14(FAST PWM)

	   ICR1=4999;  //fPWM=50Hz (Period = 20ms Standard).

	   DDRB|=(1<<PB5)|(1<<PB6);   //PWM Pins as Out
	   OCR1A=97;//posicion 0 grados servo inferior
	   OCR1B=97;//posicion 0 grados servo inferior
		   
	//controlServoInferior(datoServoInferior);
	

	time1 = t1; // iniciamos el tieme1 en 250
	contador=0;


	uart_init(); // iniciamos el puerto serial
	stdout = stdin = stderr = &uart_str;
	fprintf(stdout,"%s","El sistema esta preparado para funcionar\n\r"); // enviamos el mensaje de que el sistema esta listo en el puerto serial
	eeprom_write_word((uint16_t*)eeprom_data,0); // borramos los datos almacenados en la eeprom

	sei() ;
	
}


void datosSensores(void){


	if (contador == 0)
	{
		char data0 =(0<<1)|(0<<0);
		iniciarConversion(data0);
		sleep_cpu();
		sensorSuperiorIzquierda = Ain;
		
		while (!(UCSR0A & (1<<UDRE0))) ;
		_delay_ms(1);
		contador ++;
		
/*
		fprintf(stdout,"%d%s\r\n",sensorSuperiorIzquierda," Servo Superior Izquierda");
		fprintf(stdout,"%s\r\n","**********************************************************************");
		_delay_ms(1000);*/
	
		
		
	}
	if (contador == 1)
	{
		char data1 = (0<<1)|(1<<0);
		iniciarConversion(data1);
		sleep_cpu();
		sensorSuperiorDerecha = Ain;
		while (!(UCSR0A & (1<<UDRE0))) ;
		_delay_ms(1);
		contador ++;
/*
		fprintf(stdout,"%d%s\r\n",sensorSuperiorDerecha," Servo Superior Derecha");
		fprintf(stdout,"%s\r\n","**********************************************************************");
		_delay_ms(1000);*/

	}
	if (contador== 2)
	{
		char data2 =(1<<1)|(0<<0);
		iniciarConversion(data2);
		sleep_cpu();
		sensorInferiorIzquierda = Ain;
		while (!(UCSR0A & (1<<UDRE0))) ;
		_delay_ms(1);
		contador ++;
/*
		fprintf(stdout,"%d%s\r\n",sensorInferiorIzquierda,"  Servo Inferior Izquierda");
		fprintf(stdout,"%s\r\n","**********************************************************************");
		_delay_ms(1000);*/
		
		
	}
	if(contador == 3)
	{
		char data3 = (1<<0)|(1<<1);
		iniciarConversion(data3);	
		sleep_cpu();
		sensorInferiorDerecha = Ain;
		while (!(UCSR0A & (1<<UDRE0))) ;
		_delay_ms(1);
		contador ++;
/*
		
		fprintf(stdout,"%d%s\r\n",sensorInferiorDerecha,"  Servo Inferior derecha");
		fprintf(stdout,"%s\r\n","**********************************************************************");
		_delay_ms(1000);*/
		
	}
	if (contador==4){
		
		/*int comprobarErroresSensores = comprobarSensores();
		
		fprintf(stdout,"%d%s\r\n",comprobarErroresSensores," ERROR");
		fprintf(stdout,"%s\r\n","**********************************************************************");
		_delay_ms(1000);*/
		


					contador = 0;
					char mediaIzquierda =0;
					char mediaDerecha = 0;
					char mediaSuperior = 0;
					char mediaInferior = 0;
					
					mediaIzquierda =  (sensorSuperiorIzquierda + sensorInferiorIzquierda)/2;
					mediaDerecha = (sensorSuperiorDerecha+sensorInferiorDerecha)/2;
					mediaSuperior = (sensorSuperiorDerecha+sensorSuperiorIzquierda)/2;
					mediaInferior =(sensorInferiorIzquierda+sensorInferiorDerecha)/2;
					
					
					fprintf(stdout,"%d%s\r\n",mediaIzquierda,"  mediaIzquierda");
					fprintf(stdout,"%d%s\r\n",mediaDerecha,"  mediaDerecha");
					fprintf(stdout,"%s\r\n","**********************************************************************");
					/*
					fprintf(stdout,"%d%s\r\n",mediaSuperior,"  mediaSuperior");
					fprintf(stdout,"%d%s\r\n",mediaInferior,"  mediaInferior");*/
					if (OCR1A<97)
					{
					OCR1A=97;
					
					}
					if (OCR1A>600)
					{
					OCR1A=600;
					
					}
					
					
					
					if (OCR1B<350)
					{
					OCR1B=350;
					
					}
					if (OCR1B>700)
					{
					OCR1B=700;
					
					}
										
					/***********************Servo inferior control *************************************************/
					
					
					
					if(mediaDerecha >mediaIzquierda && OCR1A<600){
						
						
						OCR1A=OCR1A+10;
						/* Wait();*/
						
						fprintf(stdout,"%d%s\r\n",OCR1A,"  Servo");
						fprintf(stdout,"%s\r\n","**********************************************************************");
						/*_delay_ms(1000);*/
					}
					if (mediaIzquierda > mediaDerecha && OCR1A>97)
					{
						OCR1A=OCR1A-10;
						/*Wait();*/
						
						fprintf(stdout,"%d%s\r\n",OCR1A,"  Servo");
						fprintf(stdout,"%s\r\n","**********************************************************************");
						/*_delay_ms(1000);*/
					}
					
					
					
										/***********************Servo superior control *************************************************/
					if(mediaSuperior>mediaInferior && OCR1B<700){
						
						
						OCR1B=OCR1B+10;
						/* Wait();*/
						
						fprintf(stdout,"%d%s\r\n",OCR1B,"  ServoINFERIOR");
						fprintf(stdout,"%s\r\n","**********************************************************************");
						/*_delay_ms(1000);*/
					}
					if (mediaInferior > mediaSuperior && OCR1B>350)
					{
						OCR1B=OCR1B-10;
						/*Wait();*/
						
						fprintf(stdout,"%d%s\r\n",OCR1B,"  ServoINFERIOR");
						fprintf(stdout,"%s\r\n","**********************************************************************");
						/*	_delay_ms(1000);*/
					}
					
					
					
				
	}

	
		
}




 void iniciarConversion(char data)
 {
	

//init the A to D converter
//channel zero/ right adj /internal Aref=Vcc
ADMUX = (1<<REFS0)|data ;
//enable ADC and set prescaler to 1/127*16MHz=125,000
//and set int enable
ADCSRA =( (1<<ADEN) | (1<<ADIE)) + 7 ;

SMCR = (1<<SM0) ; // sleep -- choose ADC mode
// the SLEEP instruction makes the MCU enter ADC Noise Reduction mode, page 51 on datasheet
// init the UART -- uart_init() is in uart.c


// Need the next two statments so that the USART finishes
// BEFORE the cpu goes to sleep.
while (!(UCSR0A & (1<<UDRE0))) ;
_delay_ms(1);

sleep_enable();
	 

		
	 
 }
 
 
int comprobarSensores(){
	
	int error =0;
	if (sensorInferiorIzquierda == 0)
	{
		fprintf(stdout,"%s\r\n"," ERROR en Servo Inferior Izquierda");
		fprintf(stdout,"%s\r\n","**********************************************************************");
		_delay_ms(1000);
		error++;
		
	}
	if (sensorInferiorDerecha == 0)
	{
		fprintf(stdout,"%s\r\n"," ERROR en Servo Inferior Derecha");
		fprintf(stdout,"%s\r\n","**********************************************************************");
		_delay_ms(1000);
		error++;
		
	}
	
	if (sensorSuperiorDerecha == 0)
	{
		fprintf(stdout,"%s\r\n"," ERROR en Servo Superior Derecha");
		fprintf(stdout,"%s\r\n","**********************************************************************");
		_delay_ms(1000);
		error ++;
		
	}
	if (sensorSuperiorIzquierda == 0)
	{
		fprintf(stdout,"%s\r\n"," ERROR en Servo Superior Izquierda");
		fprintf(stdout,"%s\r\n","**********************************************************************");
		_delay_ms(1000);
		error++;
		
	}
	
	return error;
 }
 

int main(void)
{
	initialize();	
	// bucle que se va a repetir en el arduino
	while(1)
	{   

	datosSensores();	
		
	}
}
