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


uint16_t  tick0Cnt;
uint16_t ticks0Needed;

uint8_t tick1Cnt;
uint8_t ledsRunning;


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

int isEngineRunning()
{
	int res = 0;
	
	return res;
	
}

void startLEDs()
{
	ledsRunning = 1;
}

void stopLEDs()
{
	ledsRunning = 0;
}

void switchLEDs()
{
	if (! ledsRunning)  {
		startLEDs();
	}

}

void startBuzzer()
{
	
}

void stopBuzzer()
{
	
}

void startTimer0()
{
	
}

void stopTimer0()
{
	
}



void beepTime(uint8_t oc0a)
{
	OCR0A = oc0a;
	
	startBuzzer();
	startTimer0();
}

void beepLong()
{
	beepTime(2);
}

void stopBeep()
{
	beepTime(1);
}


ISR(TIMER0_COMPA_vect)
{
	cli();
	
	
	stopTimer0();
	sei();
}

ISR(TIMER1_COMPA_vect)
{
	cli();
	tick1Cnt += 1;
	if (tick1Cnt >= 4) { tick1Cnt = 0;}
	
	if ( (isKsbPulled()) || ( isEngineRunning() &&  ((! isPassingBeamOn()) || isHandbreakPulled()  )) )	{
			switchLEDs();
		}  else {
			stopLEDs();
		}
	
	sei();
}

void setHW()
{
	//  set GPIO
	
	
		ledsRunning = 0;
	
	// set clock prescaler to 2
	cli();
	 //asm volatile (
	 //"sbi 0x1f,0x02" "\r\n"
	 //"sbi 0x1f,0x01" "\r\n"
	 //);
		CLKPR = (1<<CLKPCE);
		CLKPR = (1<<CLKPS0);
		
	// set timer 0
		
		tick0Cnt = 0;
		ticks0Needed = 20;   // 10 sec
		//TCCR1 = (1 << CTC1) | (1 << CS13) | (1 << CS12)| (1 << CS11)| (1 << CS10) ;   // set prescaler to 16384
		//OCR1A = 244;  // counter top value means approx   1 interrupt per sec
		//GTCCR = 0x00;
		//TIMSK  = 1 << OCIE1A;  //  interrupt needed
		//TCNT1 = 0x00 ;
		
	
	// set Timer 1    
	
	tick1Cnt = 0;
			
	TCCR1 = (1 << CTC1) | (1 << CS13) | (1 << CS12)| (1 << CS11)| (1 << CS10) ;   // set prescaler to 16384
	OCR1A = 244;  // counter top value means approx   1 interrupt per sec
	GTCCR = 0x00;
	TIMSK  = 1 << OCIE1A;  //  interrupt needed 
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