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
#define longBeepCnt  10

#define morseDelay  60

/*
enum morseStates
{
	morseBeep,
	morseBreak,
	morseEndletter
};

*/

#define ticks1Needed 2

uint16_t  morseDelayCnt;

uint16_t  tick0Cnt;
uint16_t ticks0Needed;


uint8_t morseCnt;
uint8_t ticks1Cnt;
uint8_t ledsRunning;
uint8_t morseState;



#define morseAlarm ". . . . . . . ."
#define morseK "- . -"
#define morseB "- . . ."


uint8_t  morseCnt;
uint8_t  morseTips;	
char* currentMorseLetter;	


int isHandbreakPulled()
{
	int res = 0;
	if ( ((PINB & (1<< PINB0)) == 0 )  ){
		res = 1;
	}
	return res;
}

int isKsbPulled()
{
	int res = 0;
	if ( ((PINB & (1<< PINB2)) == 0 )  ){
		res = 1;
	}
	return res;
}

int isPassingBeamOn()
{
	int res = 0;
	if ( ((PINA & (1<< PINA3)) == 0 )  ){
		res = 1;
	}	
	return res;
}

int isEngineRunning()
{
	int res = 0;
	if ( ((PINB & (1<< PINB1)) == 0 )  ){
		res = 1;
	}	
	return res;
	
}

void startLEDs()
{
	if (ledsRunning == 0) {
		ledsRunning = 1;
		PORTA |= (1 << PORTA0);
	}
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
	PORTA |= (1<<PORTA2);
	startTimer0();
}

void stopBuzzer()
{
	// set buzzer off
	PORTA &= ~(1 << PORTA2);
	stopTimer0();
}



void beepTime(uint16_t cnt)
{
	tick0Cnt = 0;
	ticks0Needed = cnt;
	
	
	startBuzzer();
}

void breakTime(uint16_t cnt)
{
	tick0Cnt = 0;
	ticks0Needed = cnt;
	
	startTimer0();
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

void morseLetter(morseLetterType* letter, uint8_t pos) 
{
	if (morseDelayCnt >= morseDelay)  {
		if ((pos == 0) ||(currentMorseLetter != letter)) {
			currentMorseLetter =  letter;
			pos = 0;
		}
		if ((*currentMorseLetter)[pos] == 1)  {
			beepShort();
		} else {
			if ((*currentMorseLetter)[pos] == 2) {
				beepLong();
			} else {
				// take a break  :-)
			}
		}
	} else {
		++ morseDelayCnt;
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
	if (ticks1Cnt >= ticks1Needed) {
		if   ( isEngineRunning() &&  ((isKsbPulled()) ||(! isPassingBeamOn()) || isHandbreakPulled()  )) 	{
				toggleLEDs();
				if (isHandbreakPulled())  {
					morseLetter(morseAlarm);
				} else {
					if (isKsbPulled())  {
						morseLetter(morseK);
					}  else { if (!isPassingBeamOn())  {
							morseLetter(morseB  );
						} else {
							// nothing to do on buzzer	
						}
					}
				}	
			
			}  else {
				stopLEDs();
				stopBuzzer();
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
		ticks0Needed = shortBeepCnt;   // somewhat more than 0.1 sec
			
		OCR0A = 195;  // counter  value means approx  0.025  sec interval 
		TCNT0 = 0x0000;
		TCCR0A =  (1 << WGM01);  //  CTC
//		TCCR0B = ((1 << CS02)|  (1 << CS00)  ) ;   // set prescaler to 1024 and start
		TCCR0B = 0x00;    // keep timer 0 stopped
			
		GTCCR = 0x00;
		TIMSK0  = 1 << OCIE0A;  //  interrupt needed

// set Timer 1    
	
	morseCnt = 0;
	morseDelayCnt = 0;
			
			
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