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

#define shortBeepCnt  2
#define longBeepCnt  8

#define morseDelay  60


#define ticks1Needed 4


uint8_t  tipCnt;
uint8_t tipsNeeded;

uint8_t  tick0Cnt;
uint8_t  tick0Needed;

uint8_t ticks1Cnt;
uint8_t ledsRunning;

#define morseAlarm ". . . . . . . ."
#define morseK "- . -"
#define morseB "- . . ."
#define morseL ". - . ."

#define breakCountDown   60
#define ksbCountDown    120
#define lightCountDown  120

char* currentMorseLetter;	
uint32_t  alarmSecondCount;


int isHandbreakPulled()   // yellow
{
	int res = 0;
	if ( ((PINB & (1<< PINB0)) == 0 )  ){
		res = 1;
	}
	return res;
}

int isKsbPulled()  //  if-lightsensor
{
	int res = 0;
	if ( ((PINB & (1<< PINB2)) == 0 )  ){
		res = 1;
	}
	return res;
}

int isPassingBeamOn()    //  white
{
	int res = 0;
	if ( ((PINA & (1<< PINA3)) == 0 )  ){
		res = 1;
	}	
	return res;
}

int isEngineRunning()  //  green
{
	int res = 0;
	if ( ((PINB & (1<< PINB1)) == 0 )  ){
		res = 1;
	}	
	return res;
	
}

void startLEDs()
{
	ledsRunning = 1;
	PORTA |= (1 << PORTA0);
}

void stopLEDs()
{
	ledsRunning = 0;
	
	// set led ports to 0
	PORTA &= ~((1 << PORTA0) | (1 << PORTA1));
}

void togglePortAPin(uint8_t pos)
{
	if ((PORTA & (1 << pos)) != 0 )  {
		PORTA &= ~(1 << pos);
	} else {
		PORTA |= (1 << pos);			
	}
}

void toggleLEDs()
{
	if (! ledsRunning)  {
		startLEDs();
	}
	// toggle led ports
	togglePortAPin(PORTA0);
	togglePortAPin(PORTA1);
}

void startTimer0()
{
	TCNT0 = 0x00;
	tick0Cnt = 0;
	TCCR0B = ((1 << CS02)|  (1 << CS00)  ) ;   // set prescaler to 1024, hence start timer

}

void stopTimer0()
{
	TCCR0B = 0X00;   // set prescaler to 0, hence stop timer
}


void startBuzzer()
{
	// set buzzer on
	PORTA |= (1<<PORTA2);
	startTimer0();
}

void stopBuzzer()
{
	// set buzzer off
	PORTA &= ~(1 << PORTA2);
	stopTimer0();
}




void stopBeep()
{
	stopBuzzer();
}

void morseNextTip()
{
	if (tipCnt < tipsNeeded) {
		char tip = currentMorseLetter[tipCnt];
		tipCnt += 1;
		if (tip == '.') {
			tick0Needed = shortBeepCnt;
			startBuzzer();
		}  else if (tip == '-') {
			tick0Needed = longBeepCnt;
			startBuzzer();
		} else if (tip == ' ') {
			tick0Needed = shortBeepCnt;
			startTimer0();
		}	
		tick0Cnt = 0;
		startTimer0();	
	} else {
		stopBuzzer();
		stopTimer0();
	}
}

void morseLetter(char* letter) 
{
	stopBuzzer(); // if it still accidentally should run
	currentMorseLetter = letter;
	tipCnt = 0; 
	tipsNeeded = strlen(letter);
	morseNextTip();
}

ISR(TIM0_COMPA_vect)
{
	cli();
	if (tick0Cnt < tick0Needed) {
		tick0Cnt += 1;
	} else {
		stopBeep();
		//stopTimer0();
		if (tipCnt < tipsNeeded)  {
			morseNextTip();
		}
	}
	sei();
}

ISR(TIM1_COMPA_vect)
{
	cli();
	if (ticks1Cnt >= ticks1Needed) {
		if   ( isEngineRunning() &&  ((isKsbPulled()) ||(! isPassingBeamOn()) || isHandbreakPulled()  )) 	{
				++ alarmSecondCount;
				toggleLEDs();
							
				if (isHandbreakPulled())  {  // yellow
					if (alarmSecondCount > breakCountDown )  morseLetter(morseB);
				} else {
					if (isKsbPulled())  {   //  if-lightsensor
						if (alarmSecondCount > ksbCountDown ) morseLetter(morseK);
					}  else { if (!isPassingBeamOn())  {
							if (alarmSecondCount > lightCountDown ) morseLetter(morseL  );    //   white
						} else {
							// nothing to do on buzzer	
						}
					}
				}	
			
			}  else {
				stopLEDs();
				stopBuzzer();
				alarmSecondCount = 0;
			}
		}
		++ ticks1Cnt;
	sei();
}

void setHW()
{
	cli();
	//  set GPIO
	
		DDRA = ((1 << PORTA0) | (1 << PORTA1) | (1 << PORTA2)    );  //  set as needed for port A and B
		
		DDRB  = 0x00;	//  all input except reset (managed by cpu or debugger)

	
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
	tick0Needed = 0;
	tipCnt = 0;
	tipsNeeded = 0;
	
	alarmSecondCount = 0;
			
		OCR0A = 195;  // counter  value means approx  0.025  sec interval 
		TCNT0 = 0x0000;
		TCCR0A =  (1 << WGM01);  //  CTC
//		TCCR0B = ((1 << CS02)|  (1 << CS00)  ) ;   // set prescaler to 1024 and start
		TCCR0B = 0x00;    // keep timer 0 stopped
			
		GTCCR = 0x00;
		TIMSK0  = 1 << OCIE0A;  //  interrupt needed

// set Timer 1    
	
			
			
	OCR1A = 7812;  // counter top value means approx   1 interrupt per sec	
	TCNT1 = 0x0000;	
	TCCR1A =0x00;
	TCCR1B = ((1 << CS12)|  (1 << CS10) | (1 << WGM12)  ) ;   // set prescaler to 1024 , CTC ClearTimerOnCompare
	TCCR1C = 0x00;
	
	GTCCR = 0x00;
	TIMSK1  = 1 << OCIE1A;  //  interrupt needed 	
	
	ticks1Cnt = 0;
		
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