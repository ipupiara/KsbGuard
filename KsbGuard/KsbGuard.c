/*
 * KsbGuard.c
 *
 * Created: 12.06.2018 20:09:44
 *  Author: mira
 */ 


#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>
#include <util/atomic.h>


uint16_t  tickCnt;
uint16_t ticksNeeded;


int isHandbreakPulled()
{
	int res = 0;
	
	return res;
}

int isKsbPulled()
{
	int res = 0;
	
	return res;
}

int isPassingBeamOn()
{
	int res = 0;
	
	return res;
}

void startLEDs()
{
	
}

void stopLEDs()
{
	
}

void switchLEDs()
{

}

void beepShort()
{
	
}

void beepLong()
{
	
}

void stopBeep()
{
	
}


ISR(TIMER0_COMPA_vect)
{
	
}

ISR(TIMER1_COMPA_vect)
{
	
}

void setHW()
{
	// set clock prescaler to 2
	cli();
	 //asm volatile (
	 //"sbi 0x1f,0x02" "\r\n"
	 //"sbi 0x1f,0x01" "\r\n"
	 //);
		CLKPR = (1<<CLKPCE);
		CLKPR = (1<<CLKPS0);
		
	// set timer 0
	
	// set timer 1
	
	// Timer 1    prepare  for ADC triggering
			
	tickCnt = 0;
	ticksNeeded = 20;   // 10 sec
	TCCR1 = (1 << CTC1) | (1 << CS13) | (1 << CS12)| (1 << CS11)| (1 << CS10) ;   // set prescaler to 16384
	OCR1A = 0xF4;  // counter top value means approx   2 ADC measures per sec
	GTCCR = 0x00;
	TIMSK  = 1 << OCIE1A;  //  interrupt needed for ADC trigger
	TCNT1 = 0x00 ;
	
		
		
	 sei();
	
}

int main(void)
{
	setHW();
    while(1)
    {
        //TODO:: Please write your application code 
    }
}