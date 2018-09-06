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

#define shortBeepCnt  5
#define longBeepCnt  20


uint16_t  tick0Cnt;
uint16_t ticks0Needed;

uint8_t tick1Cnt;
uint8_t ticks1Needed;
uint8_t ledsRunning;

typedef int8_t   morseLetterType [6] ;

morseLetterType morseAlarm = {1,1,1,1,1,1};
morseLetterType morseK = {2,1,2,0,0,0};
morseLetterType morseB = {2,1,1,1,0,0};	
	
morseLetterType * currentMorseLetter;	


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
	// set 1 led port
}

void stopLEDs()
{
	ledsRunning = 0;
	// set led ports to 0
}

void switchLEDs()
{
	if (! ledsRunning)  {
		startLEDs();
	}
	// toggle led ports
}

void startTimer0()
{
	TCNT0 = 0x00;
	tick0Cnt = 0x0000;
	TCCR0B = ((1 << CS02)|  (1 << CS00)  ) ;   // set prescaler to 1024, hence start timer

}

void stopTimer0()
{
	TCCR0B = 0X00;   // set prescaler to 0, hence stop timer
}


void startBuzzer()
{
	// set buzzer on
	startTimer0();
}

void stopBuzzer()
{
	// set buzzer off
	stopTimer0();
}



void beepTime(uint16_t cnt)
{
	tick0Cnt = 0;
	ticks0Needed = cnt;
	
	
	startBuzzer();
}

void beepLong()
{
	beepTime(longBeepCnt);
}

void beepShort()
{
	beepTime(shortBeepCnt);
}

void stopBeep()
{
	stopBuzzer();
}

void morseLetter(morseLetterType letter, int pos) 
{
	if (pos == 0) {
		currentMorseLetter = (morseLetterType *) &letter;
	}
	if (*currentMorseLetter[pos] == 1)  {
		beepShort();
	} else {
		if (*currentMorseLetter[pos] == 2) {
			beepLong();
		} else {
			// take a break  :-)
		}
	}
}

ISR(TIM0_COMPA_vect)
{
	cli();
	tick0Cnt += 1;
	
	if (tick0Cnt > ticks0Needed) {
		stopBeep();
	}
	sei();
}

ISR(TIM1_COMPA_vect)
{
	cli();
	tick1Cnt += 1;
	if (tick1Cnt > 6) { tick1Cnt = 0;}
	
	if   ( isEngineRunning() &&  ((isKsbPulled()) ||(! isPassingBeamOn()) || isHandbreakPulled()  )) 	{
			switchLEDs();
			if (isHandbreakPulled())  {
				morseLetter(morseAlarm,tick1Cnt -1);
			} else {
				if (isKsbPulled())  {
					morseLetter(morseK, tick1Cnt-1);
				}  else { if (!isPassingBeamOn())  {
						morseLetter(morseB  ,tick1Cnt -1);
					} else {
						// nothing to do on buzzer	
					}
				}
			}	
		}  else {
			stopLEDs();
		}
	
	sei();
}

void setHW()
{
	cli();
	//  set GPIO
	
		PORTA = 0x00;
		DDRA = 0x00;  // tobe set as needed for port A and B
		
		PORTB = 0x00;
		DDRB  = 0x00;	

	
	//  set pcintn interrupts so that the system can get halted when idle
	
		ledsRunning = 0;
	
	 // set clock prescaler to 2
	
	 //asm volatile (
	 //"sbi 0x1f,0x02" "\r\n"
	 //"sbi 0x1f,0x01" "\r\n"
	 //);
	 
//		CLKPR = (1<<CLKPCE);
//		CLKPR = (1<<CLKPS0);
		
	// set timer 0
		
		tick0Cnt = 0;
		ticks0Needed = shortBeepCnt;   // somewhat more than 0.1 sec
			
		OCR0A = 195;  // counter  value means approx  0.025  sec interval 
		TCNT0 = 0x0000;
		TCCR0A =  (1 << WGM01);  //  CTC
//		TCCR0B = ((1 << CS02)|  (1 << CS00)  ) ;   // set prescaler to 1024 and start
		TCCR0B = 0x00;    // keep timer 0 stopped
			
		GTCCR = 0x00;
		TIMSK0  = 1 << OCIE0A;  //  interrupt needed

// set Timer 1    
	
	tick1Cnt = 0;
			
	OCR1A = 7812;  // counter top value means approx   1 interrupt per sec	
	TCNT1 = 0x0000;	
	TCCR1A =0x00;
	TCCR1B = ((1 << CS12)|  (1 << CS10) | (1 << WGM12)  ) ;   // set prescaler to 1024 , CTC ClearTimerOnCompare
	TCCR1C = 0x00;
	
	GTCCR = 0x00;
	TIMSK1  = 1 << OCIE1A;  //  interrupt needed 	
		
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