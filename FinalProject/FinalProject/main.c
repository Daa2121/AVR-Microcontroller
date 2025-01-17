/*
 * FinalProject.c
 *
 * Created: 11/6/2023 6:18:22 PM
 * Author : daapkx
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include "util/delay.h"
#include <stdlib.h>

#define BAUD 9600UL
volatile int ubrr = ( F_CPU/(16ul*BAUD)) -1;
//individual variables for each timer:
volatile int hour = 0;
volatile int minutes = 0;
volatile int seconds = 0;
volatile int stopwatch_hour = 0;
volatile int stopwatch_minutes = 59;
volatile int stopwatch_seconds = 55;
volatile int timer_hour = 0;
volatile int timer_minutes = 0;
volatile int timer_seconds = 0;
char hour_display[4]; //array of char used to convert int to string for display
char min_display[4];
char sec_display[4];
char stopwatch_hour_display[4]; //array of char used to convert int to string for display
char stopwatch_min_display[4];
char stopwatch_sec_display[4];
char timer_hour_display[4]; //array of char used to convert int to string for display
char timer_min_display[4];
char timer_sec_display[4];
int clk_value = 0; //used to hold current values of the clock to convert into strings
int stopwatch_value = 0; //used to hold current values of stopwatch to convert into strings
int timer_value = 0; //used to hold current values of stopwatch to convert into strings
int mode = 0; //used to change menu
void usart_init(){
	UCSR1A = 0x00;
	UCSR1B = 0x00 | (1<<RXEN) | (1<<TXEN); //enable the transmitter and receiver
	UCSR1C = 0x00 | (1<<UCSZ0) | (1<<UCSZ1); //8 bits data
	UBRR1 = ubrr;
}

void usart_txt(char data){
	UDR1 = data;
	while(~UCSR1A & (1<<TXC)); //wait until the data is transmitted
	UCSR1A |= (1<<TXC);
}

void usart_txtstr(char *Str){	
	for(int i = 0; Str[i] != '\0'; i++){
		usart_txt(Str[i]);
	}
}

void intro(){
	usart_txtstr("Please input current time using Simon board switches (HH:MM:SS)");
	usart_txtstr("\n");
	clk_value = hour;
	itoa(clk_value, hour_display, 10);
	clk_value = minutes;
	itoa(clk_value, min_display, 10);
	clk_value = seconds;
	itoa(clk_value, sec_display, 10);
	display_time();
}

void menu(){
	usart_txtstr("MENU: SW7 to edit clock | SW8 for timer | SW9 for stopwatch | SW6 to go back to menu | SW4 to display current time | SW5 to display timer");
	usart_txtstr("\n");
	
}

ISR(TIMER1_COMPA_vect)
{
	TCNT1 = 0x00; 
	//increment the seconds every second
	seconds++;
	//once the seconds have reached a full minute increment minutes
	if(seconds>59)
	{
		seconds = 0;
		minutes++;
		
	}
	//once the minutes have reached a full hour increment the hours
	if(minutes>59)
	{
		hour++;
		minutes = 0;
	}
	//reset hours to 0 once it is 25
	if(hour>24)
	{
		hour=0;
	}
}
ISR(TIMER3_COMPA_vect){
	TCNT3 = 0x00;
	//increment the seconds every second
	stopwatch_seconds++;
	//once the seconds have reached a full minute increment minutes
	if(stopwatch_seconds>59)
	{
		stopwatch_seconds = 0;
		stopwatch_minutes++;
		
	}
	//once the minutes have reached a full hour increment the hours
	if(stopwatch_minutes>59)
	{
		stopwatch_hour++;
		stopwatch_minutes = 0;
	}
	//reset hours to 0 once it is 25
	if(stopwatch_hour>24)
	{
		stopwatch_hour=0;
	}
	
	//default threshold for stopwatch
	if(stopwatch_hour == 1){
		timer3_stop();
		usart_txtstr("STOPWATCH: ONE HOUR HAS BEEN REACHED ");
		usart_txtstr("\n");
		//Flash lights on and off 5 times
		PORTD = 0x00;
		_delay_ms(1000);
		PORTD = 0xFF;
		_delay_ms(1000);
		
		PORTD = 0x00;
		_delay_ms(1000);
		PORTD = 0xFF;
		_delay_ms(1000);
		
		PORTD = 0x00;
		_delay_ms(1000);
		PORTD = 0xFF;
		_delay_ms(1000);
		
		//reset the stopwatch
		stopwatch_hour = 0;
		stopwatch_minutes = 0;
		stopwatch_seconds = 0;
		usart_txtstr("STOPWATCH HAS BEEN RESET!");
		usart_txtstr("\n");
		timer3_start(); //restart the stopwatch
		mode = 3; //go to the stopwatch section
	}
}
ISR(TIMER4_COMPA_vect){
	TCNT4 = 0x00;
	//decrement the timer every second
	timer_seconds--;
	// if the seconds have reached 0 decrement the minutes
	if(timer_seconds < 0){
		timer_minutes--;
		timer_seconds = 59;
	}
	//if the minutes have reached 0 decrement the hours
	if(timer_minutes < 0){
		timer_hour--;
		timer_minutes = 59;
	}
	//the hours should never be negative so once we reach hour 0 we need to keep it 0 regardless of how many minutes and seconds are left
	if(timer_hour < 0){
		timer_hour = 0;
	}
	// end of countdown 
	if(timer_hour == 0 && timer_minutes == 0 && timer_seconds == 0){
		timer4_stop();
		usart_txtstr("Timer: 0:0:0");
		usart_txtstr("\n");
		usart_txtstr("COUNTDOWN COMPLETE");
		usart_txtstr("\n");
		//Flash lights on and off 3 times
		PORTD = 0x00;
		_delay_ms(1000);
		PORTD = 0xFF;
		_delay_ms(1000);
		
		PORTD = 0x00;
		_delay_ms(1000);
		PORTD = 0xFF;
		_delay_ms(1000);
		
		PORTD = 0x00;
		_delay_ms(1000);
		PORTD = 0xFF;
		_delay_ms(1000);
		
		mode = 2; //go to timer section
	}
	
}


void display_time(){
	clk_value = hour; //grab current value 
	itoa(clk_value, hour_display, 10); //translate to string
	usart_txtstr(hour_display);  //display new string
	usart_txtstr(":");
	clk_value = minutes; //grab current value 
	itoa(clk_value, min_display, 10); //translate to string
	usart_txtstr(min_display); //display new string
	usart_txtstr(":");
	clk_value = seconds; //grab current value 
	itoa(clk_value, sec_display, 10); //translate to string
	usart_txtstr(sec_display); //display new string
	usart_txtstr("\n");
}
void display_stopwatch(){
	stopwatch_value = stopwatch_hour; //grab current value 
	itoa(stopwatch_value, stopwatch_hour_display, 10); //translate to string
	usart_txtstr(stopwatch_hour_display); //display new string
	usart_txtstr(":");
	stopwatch_value = stopwatch_minutes; //grab current value 
	itoa(stopwatch_value, stopwatch_min_display, 10); //translate to string
	usart_txtstr(stopwatch_min_display); //display new string
	usart_txtstr(":");
	stopwatch_value = stopwatch_seconds; //grab current value 
	itoa(stopwatch_value, stopwatch_sec_display, 10); //translate to string
	usart_txtstr(stopwatch_sec_display); //display new string
	usart_txtstr("\n");
}
void display_timer(){
	timer_value = timer_hour; //grab current value 
	itoa(timer_value, timer_hour_display, 10); //translate to string
	usart_txtstr(timer_hour_display); //display new string
	usart_txtstr(":");
	timer_value = timer_minutes; //grab current value 
	itoa(timer_value, timer_min_display, 10); //translate to string
	usart_txtstr(timer_min_display);
	usart_txtstr(":");
	timer_value = timer_seconds; //grab current value 
	itoa(timer_value, timer_sec_display, 10); //translate to string
	usart_txtstr(timer_sec_display); //display new string
	usart_txtstr("\n"); 
}

void timer_init_ctc()
{
	TCCR1A = 0x00;
	TCCR1B |=0x00 | (1<<WGM13) | (1<<WGM12); // ctc mode
	TCNT1 = 0x00;
	TIMSK1 |= (1<<OCIE1A); // enable the interrupt for the timer  
	OCR1A = 0xF424;
	
}
void timer3_init_ctc(){
	TCCR3A = 0x00;
	TCCR3B |=0x00 | (1<<WGM13) | (1<<WGM12); // ctc mode
	TCNT3 = 0x00;
	TIMSK3 |= (1<<OCIE3A); // enable the interrupt for the timer
	OCR3A = 0xF424;
}
void timer4_init_ctc(){
	TCCR4A = 0x00;
	TCCR4B |=0x00 | (1<<WGM13) | (1<<WGM12); // ctc mode
	TCNT4 = 0x00;
	TIMSK4 |= (1<<OCIE3A); // enable the interrupt for the timer
	OCR4A = 0xF424;
}

void timer1_start()
{
	TCCR1B |= (1<<CS12); //pre-scalar 256
}
void timer3_start()
{
	TCCR3B |= (1<<CS12); //pre-scalar 256
}
void timer4_start()
{
	TCCR4B |= (1<<CS12); //pre-scalar 256
}

void timer1_stop()
{
	TCCR1B &=~( (1<<CS12) | (1<<CS11) | (1<<CS10) );
	
}
void timer3_stop()
{
	TCCR3B &=~( (1<<CS12) | (1<<CS11) | (1<<CS10) );
	
}
void timer4_stop()
{
	TCCR4B &=~( (1<<CS12) | (1<<CS11) | (1<<CS10) );
	
}

int main(void)
{
	//Port mode configuration for the switches and lights
	DDRA = 0x00;	
	PORTA = 0xFF;
	DDRE = 0x00;
	PORTE = 0xFF;
	DDRD = 0xFF;
	PORTD = 0xFF;
		
	usart_init(); //init USART
	menu();  // display the menu first
	intro(); // display the intro for the project
	timer_init_ctc(); // initalize all three timers for the project
	timer3_init_ctc();
	timer4_init_ctc();
	sei(); // enable interrupts at the micro controller level
	mode = 1; //start with time setting
    while (1) 
    {
		if(mode == 5){ //display time
			timer1_start(); //start the timer when we want to display the clck
			while(mode == 5){ //use while loop to stay in the section corresponding to this specific mode and only switch modes when the value is altered
				usart_txtstr("Clock: ");
				display_time();
				_delay_ms(1000); //wait one sec to display new value
				
				//use different switches to change modes
				if(~PINA & (1<<PINA5)){
					while(~PINA & (1<<PINA5));
					mode = 1; //set time
				}
				if(~PINA & (1<<PINA6)){
					while(~PINA & (1<<PINA6));
					mode = 2; //timer
				}
				if(~PINA & (1<<PINA7)){
					while(~PINA & (1<<PINA7));
					mode = 3; //Stopwatch
				}
				if(~PINA & (1<<PINA4)){
					while(~PINA & (1<<PINA4));
					mode = 4; //menu
				}
				if(~PINA & (1<<PINA3)){
					while(~PINA & (1<<PINA3));
					mode = 5; //display time
				}
				if(~PINE & (1<<PINE6)){ 
					while(~PINE & (1<<PINE6)); //while the switch is pushed wait
					mode = 6; //display timer
				}
			}
		}
	
		if(mode == 1){ //set time
			usart_txtstr("Use SW1 for HH | Use SW2 for MM | Use SW3 for SS");
			usart_txtstr("\n");
			timer1_stop(); //stop the timer so that we can change the values
			while(mode == 1){
				
				if(~PINA & (1<<PINA0)){ // use switch 1 to the change hours
					while(~PINA & (1<<PINA0)); //while the switch is pushed wait
					hour++;
					if (hour > 24){ //increment through the hours and once it reaches 24 go to 0 if we increment again
						hour = 0;
					}
					clk_value = hour;
					itoa(clk_value, hour_display, 10); //translate the int value to strings so that it can be displayed in the terminal
					display_time();
					
				}
				
				if(~PINA & (1<<PINA1)){ // use switch 2 to change minutes
					while(~PINA & (1<<PINA1)); //while the switch is pushed wait
					minutes++;
					if (minutes > 59){ //increment through the minutes and once it reaches 59 go to 0 if it is incremented again
						minutes = 0;
					}
					clk_value = minutes;
					itoa(clk_value, min_display, 10); //translate the int value to strings so that it can be displayed in the terminal
					display_time();
					
				}
				
				if(~PINA & (1<<PINA2)){ // change seconds
					while(~PINA & (1<<PINA2)); //while the switch is pushed wait
					seconds++;
					if (seconds > 59){ //increment through the seconds and once it reaches 59 go to 0 if it is incremented again
						seconds = 0;
					}
					clk_value = seconds;
					itoa(clk_value, sec_display, 10);//translate the int value to strings so that it can be displayed in the terminal
					display_time();
					
				}
				
				//use different switches to change modes
				if(~PINA & (1<<PINA5)){
					while(~PINA & (1<<PINA5));
					mode = 1; //set time
				}
				if(~PINA & (1<<PINA6)){
					while(~PINA & (1<<PINA6));
					mode = 2; //timer
				}
				if(~PINA & (1<<PINA7)){
					while(~PINA & (1<<PINA7));
					mode = 3; //Stopwatch
				}
				if(~PINA & (1<<PINA4)){
					while(~PINA & (1<<PINA4));
					mode = 4; //menu
				}
				if(~PINA & (1<<PINA3)){
					while(~PINA & (1<<PINA3));
					mode = 5; //display time
				}
				if(~PINE & (1<<PINE6)){
					while(~PINE & (1<<PINE6)); //while the switch is pushed wait
					mode = 6; //display timer
				}
			}
		}
		
		if(mode == 6){ //display timer
			usart_txtstr("SW1 to pause | SW2 to unpause | SW8 to change timer | SW6 for menu ");
			usart_txtstr("\n");
			timer4_start();
			while(mode == 6){
				usart_txtstr("Timer: ");
				display_timer();
				_delay_ms(1000);
				
				if(~PINA & (1<<PINA0)){ // use switch 1 to pause timer
					while(~PINA & (1<<PINA0)); //while the switch is pushed wait
					timer4_stop();
					
				}
				
				if(~PINA & (1<<PINA1)){ // use switch 2 to unpause timer
					while(~PINA & (1<<PINA1)); //while the switch is pushed wait
					timer4_start();
					
				}
				
				
				
				//use different switches to change modes
				if(~PINA & (1<<PINA5)){
					while(~PINA & (1<<PINA5));
					mode = 1; //set time
				}
				if(~PINA & (1<<PINA6)){
					while(~PINA & (1<<PINA6));
					mode = 2; //timer
				}
				if(~PINA & (1<<PINA7)){
					while(~PINA & (1<<PINA7));
					mode = 3; //Stopwatch
				}
				if(~PINA & (1<<PINA4)){
					while(~PINA & (1<<PINA4));
					mode = 4; //menu
				}
				if(~PINA & (1<<PINA3)){
					while(~PINA & (1<<PINA3));
					mode = 5; //display time
				}
				if(~PINE & (1<<PINE6)){ 
					while(~PINE & (1<<PINE6)); //while the switch is pushed wait
					mode = 6; //display timer
				}
				
			}
		}
		
		if(mode == 2)//change timer
		{ 
			timer4_stop(); //stop the timer to change its value
			//display text to explain how the timer works and which buttons are used
			usart_txtstr("This is the timer");
			usart_txtstr("\n");
			usart_txtstr("Please input starting time to countdown from");
			usart_txtstr("\n");
			usart_txtstr("SW1 to change hours | SW2 to change minutes | SW3 to change seconds | SW5 to start timer | SW6 to go to menu");
			usart_txtstr("\n");
			while(mode == 2){
				
				if(~PINA & (1<<PINA0)){ // use switch 1 to change hours
					while(~PINA & (1<<PINA0)); //while the switch is pushed wait
					timer_hour++;
					if (timer_hour > 24){ //increment through the hours and once it reaches 24 go to 0 if we increment again
						timer_hour = 0;
					}
					timer_value = timer_hour;
					itoa(timer_value, timer_hour_display, 10); //translate the int value to strings so that it can be displayed in the terminal
					display_timer();
					
				}
				
				if(~PINA & (1<<PINA1)){ // use switch 2 to change minutes
					while(~PINA & (1<<PINA1)); //while the switch is pushed wait
					timer_minutes++;
					if (timer_minutes > 59){ //increment through the minutes and once it reaches 59 go to 0 if we increment again
						timer_minutes = 0;
					}
					timer_value = timer_minutes;
					itoa(timer_value, timer_min_display, 10); //translate the int value to strings so that it can be displayed in the terminal
					display_timer();
					
				}
				
				if(~PINA & (1<<PINA2)){ // use switch 3 to change seconds
					while(~PINA & (1<<PINA2)); //while the switch is pushed wait
					timer_seconds++;
					if (timer_seconds > 59){ //increment through the seconds and once it reaches 59 go to 0 if we increment again
						timer_seconds = 0;
					}
					timer_value = timer_seconds;
					itoa(timer_value, timer_sec_display, 10); //translate the int value to strings so that it can be displayed in the terminal
					display_timer();
					
				}
			
				//use different switches to change modes
				if(~PINA & (1<<PINA5)){
					while(~PINA & (1<<PINA5));
					mode = 1; //set time
				}
				if(~PINA & (1<<PINA6)){
					while(~PINA & (1<<PINA6));
					mode = 2; //timer
				}
				if(~PINA & (1<<PINA7)){
					while(~PINA & (1<<PINA7));
					mode = 3; //Stopwatch
				}
				if(~PINA & (1<<PINA4)){
					while(~PINA & (1<<PINA4));
					mode = 4; //menu
				}
				if(~PINA & (1<<PINA3)){
					while(~PINA & (1<<PINA3));
					mode = 5; //display time
				}
				if(~PINE & (1<<PINE6)){ 
					while(~PINE & (1<<PINE6)); //while the switch is pushed wait
					mode = 6; //display timer
				}
			}
		}
		if(mode == 3){ //stopwatch
			usart_txtstr("This is the stopwatch");
			usart_txtstr("\n");
			usart_txtstr("SW1 to pause | SW2 to unpause | SW3 to reset | SW6 to go to menu");
			usart_txtstr("\n");
			timer3_start();
			while(mode == 3){
				usart_txtstr("Stopwatch: ");
				display_stopwatch();
				_delay_ms(1000);
				
				if(~PINA & (1<<PINA0)){ // use switch 1 to stop stopwatch
					while(~PINA & (1<<PINA0)); //while the switch is pushed wait
					timer3_stop();
					
				}
				
				if(~PINA & (1<<PINA1)){ // use switch 2 to resume stopwatch
					while(~PINA & (1<<PINA1)); //while the switch is pushed wait
					timer3_start();
				}
				
				if(~PINA & (1<<PINA2)){ // use switch 3 to reset stopwatch
					while(~PINA & (1<<PINA2)); //while the switch is pushed wait
					timer3_stop();
					stopwatch_hour = 0;
					stopwatch_minutes = 0;
					stopwatch_seconds = 0;
					timer3_start();
				}
				
				
				//use different switches to change modes
				if(~PINA & (1<<PINA5)){
					while(~PINA & (1<<PINA5));
					mode = 1; //set time
				}
				if(~PINA & (1<<PINA6)){
					while(~PINA & (1<<PINA6));
					mode = 2; //timer
				}
				if(~PINA & (1<<PINA7)){
					while(~PINA & (1<<PINA7));
					mode = 3; //Stopwatch
				}
				if(~PINA & (1<<PINA4)){
					while(~PINA & (1<<PINA4));
					mode = 4; //menu
				}
				if(~PINA & (1<<PINA3)){
					while(~PINA & (1<<PINA3));
					mode = 5; //display time
				}
				if(~PINE & (1<<PINE6)){ 
					while(~PINE & (1<<PINE6)); //while the switch is pushed wait
					mode = 6; //display timer
				}
			}
		}
		
		if(mode == 4){ //menu
			menu();
			usart_txtstr("\n");
			while(mode == 4){
				// use different switches to change modes
				if(~PINA & (1<<PINA5)){
					while(~PINA & (1<<PINA5));
					mode = 1; //set time
				}
				if(~PINA & (1<<PINA6)){
					while(~PINA & (1<<PINA6));
					mode = 2; //timer
				}
				if(~PINA & (1<<PINA7)){
					while(~PINA & (1<<PINA7));
					mode = 3; //Stopwatch
				}
				if(~PINA & (1<<PINA4)){
					while(~PINA & (1<<PINA4));
					mode = 4; //menu
				}
				if(~PINA & (1<<PINA3)){
					while(~PINA & (1<<PINA3));
					mode = 5; //display time
				}
				if(~PINE & (1<<PINE6)){ 
					while(~PINE & (1<<PINE6)); //while the switch is pushed wait
					mode = 6; //display timer
				}
			}
		}
    }
}

