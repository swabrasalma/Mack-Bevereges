
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#define E PB0
#define Rs PB2
#define Rw PB1

#define bit(p) (1<<p)
#define F_CPU 8000000UL //FCPU
#define TX_NEWLINE		{transmitByte(0x0d); transmitByte(0x0a);}
#define FOSC 8000000 // Clock Speed
#define BAUD 9600
#define UBRR FOSC/16/BAUD-1
// macro for easier usage
#define read_eeprom_word(address) eeprom_read_word ((const uint16_t*)address)
#define write_eeprom_word(address,value) eeprom_write_word ((uint16_t*)address,(uint16_t)value)
#define update_eeprom_word(address,value) eeprom_update_word ((uint16_t*)address,(uint16_t)value)


unsigned char str;
unsigned char option;
unsigned char NUM_OF_ORANGES[4];
unsigned char NUM_OF_MANGOES[4];
unsigned char NUM_OF_APPLES[4];
unsigned char NUM_OF_PINEAPPLES[4];
unsigned char NUM_OF_GUAVAS[4];
unsigned char PERCENT_OF_ORANGES[2];
unsigned char PERCENT_OF_MANGOES[2];
unsigned char PERCENT_OF_APPLES[2];
unsigned char PERCENT_OF_PINEAPPLES[2];
unsigned char PERCENT_OF_GUAVAS[2];
unsigned char nem[10];
unsigned char nemesis[10];
unsigned int oranges,mangoes,apples,pineapples,guavas;
unsigned int total_oranges, total_apples,total_pineapples, total_mangoes,total_guavas,total_fruits;
unsigned int used_oranges, used_apples,used_pineapples, used_mangoes,used_guavas,used_fruits;
unsigned int left_oranges, left_apples,left_pineapples, left_mangoes,left_guavas,left_fruits;
unsigned int percent_mangoes, percent_oranges, percent_apple,percent_pineapples,percent_guavas;
unsigned int fruit_count;
unsigned int count=0;
char display[]="WELCOME TO MACK BEVERAGES";
char display1[]="FILLING STAGE";
char display2[]="SEALING STAGE";
char display3[]="STORAGE STAGE";
char message[]= "MOVING BOTTLE TO THE NEXT STAGE";
char configer[]="PLEASE DO SYSTEM CONFIGRATION ";
char pausing[] ="OPERATION PAUSED";
char comment[]="SYSTEM RESTING";
char complete[] ="BOTTLING COMPLETE";
float available_litres,reduce_available_litres;
int bottle_number=0;
int bottles_required;
float size = 0,i;
int bottle=0;
int timing, t;
int working_time, rest_time, filled_bottles;
char alt;
void enable(){
	PORTB |= (1<<E);
	_delay_ms(20);
	PORTB &= ~(1<<E);
	
}
void clearscreen(){
	PORTB &= ~(1<<Rs);
	PORTA= 0x01;
	enable();
}
void bottle_filling(){
	for (int i=0;i<sizeof(display1);i++){
		PORTB |=(1<<Rs);
		PORTA=display1[i];
		enable();
		
	}

	for (int i=2;i>=0;i--)
	{
		PORTD=i;
		timer();
		timer();
		
		
	}
	bottle++;
	eeprom_write_byte(4,bottle);
	available_litres=(reduce_available_litres-(bottle*size));
	eeprom_write_byte(0,available_litres);
	
		 bottles_required--;
		 eeprom_write_byte(1,bottles_required);
	filled_bottles--;
	     
	off();
	
}
int timer(){
		_delay_ms(3000);
		timing++;
		return timing;
}
void off(){
	PORTD=0xFF;
 }
void bottle_sealing(){
	for (int i=0;i<sizeof(display2);i++){
		PORTB |=(1<<Rs);
		PORTA=display2[i];
		enable();
		
	}

	for (int i=1;i>=0;i--)
	{
		PORTD=i;
		timer();
		timer();
		
	}
	
	off();
}
void transmitString_F(char* string)
{
	while (pgm_read_byte(&(*string)))
	transmitByte(pgm_read_byte(&(*string++)));
}
void bottle_storage(){
	for (int i=0;i<sizeof(display3);i++){
		PORTB |=(1<<Rs);
		PORTA=display3[i];
		enable();
		
	}
	
}

void notice_message(){
	if (bottle != 0)
	{
		for (int i=0;i<sizeof(complete);i++){
			PORTB |=(1<<Rs);
			PORTA=complete[i];
			enable();
			
		}
		
	}else{
		for (int i=0;i<sizeof(configer);i++){
			PORTB |=(1<<Rs);
			PORTA=configer[i];
			enable();
	}	
	}
}
void notice(){
	for (int i=0;i<sizeof(complete);i++){
		PORTB |=(1<<Rs);
		PORTA=complete[i];
		enable();
		
	}
}
void message_comment(){
	for (int i=0;i<sizeof(comment);i++){
		PORTB |=(1<<Rs);
		PORTA=comment[i];
		enable();
		
	}
}
void welcome(){
	for (int i=0;i<sizeof(display);i++){
		PORTB |=(1<<Rs);
		PORTA=display[i];
		enable();
		
	}
}
void status(){
	for (int i=0;i<sizeof(message);i++){
		PORTB |=(1<<Rs);
		PORTA=message[i];
		enable();
		
	}
}
void motor(){
	PORTF = 0x0C;
	timer();
	PORTF = 0x06;
	timer();
	PORTF = 0x03;
	timer();
	PORTF = 0x09;
	timer();
}
void lcd_init(){
	PORTB &= ~(1<<Rs) | ~(1<<Rw) ;
	PORTA=0x38;//FORMAT
	enable();
	PORTA=0x0F;//DISPLAY CURSOR
	enable();
	PORTA=0x06;//INCREMENT CURSOR
	enable();
	
	//PORTA =0xC0;//ADDRESS
	//enable();
	
}
void USART_Init( unsigned int ubrr )
  {
	/* Set baud rate */
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN)|(1<<TXEN) | (1 << RXCIE);
	sei();
	/* Set frame format: 8data, 2stop bit */
	UCSR0C = (1<<USBS)|(3<<UCSZ0);
}

//**************************************************
//Function to receive a single byte
//*************************************************
unsigned char receiveByte( void )
{
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC)) );
	/* Get and return received data from buffer */
	return UDR0;
}

//***************************************************
//Function to transmit a single byte
//***************************************************
void transmitByte( unsigned char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE)) );
	
	/* Put data into buffer, sends the data */
	UDR0 = data;}
//***************************************************
//Function to transmit a string in RAM
//***************************************************
void transmitString(char* string)
{
    while (*string)
		transmitByte(*string++);
}
void transmitInt(int in){
	char buffer[100];
	itoa(in,buffer,10);
	transmitString(buffer);
	
}
 /*function that converts a character array to integer value*/
 int xter_to_int(char array_name[],int n){
	 memmove(nem,array_name,n);
	 int result = atoi(nem);
	 return result;
 }  
   int xter_to_integer(char array_name[]){
	   memmove(nemesis,array_name,2);
	   //transmitString_F(PSTR("The string is memory ");
	   //transmitString_F(PSTR(nemesis);
	   int result = atoi(nemesis);
	   return result;
   }
   int bottle_xter_to_int(char bottle[]){
	   char required[10];
	   memmove(required,bottle,3);
	   int result = atoi(required);
	   return result;
   }
  void err_msg(){
	  TX_NEWLINE;
	  menu('2');
  } 
 int checkpercent(int percent, int available_fruit) 
  {
	  
	 if(percent>=100){
		 transmitString_F(PSTR("The percentage should be less 100,please try again! "));
		 //transmitInt(percent);
		 err_msg();
	 }
	 else{
		 int fruit_num = (percent*30); //gives the number of fruit in 3000
		 
		 if(fruit_num<=available_fruit){
			 if(percent != 0){
				 count++;
			 }
			 
		 }
		 else{
			 transmitString_F(PSTR("The available number of the fruit is not enough,please try again..."));
			 transmitInt(fruit_num);
			 TX_NEWLINE;
			 transmitInt(percent);
			 TX_NEWLINE;
			 transmitInt(available_fruit);
			 TX_NEWLINE;
			 err_msg();
			 count=0;
		 }		 
	 }
	 return count;
 }
void validate(int counter1){
	if(counter1==4){
		
		int percent_sum =percent_guavas+percent_mangoes+percent_oranges+percent_pineapples+percent_apple;
		if(percent_sum==100){
			reduce_available_litres = 150.0;
			transmitString_F(PSTR("Operation successful, Press ENTER to continue..."));
			TX_NEWLINE;
			receiveByte();
			menu('3');
		}
		else{
			transmitString_F(PSTR("The entered percentage is "));
			transmitInt(percent_sum);
			transmitString_F(PSTR(" ,but the total percentage should be 100!, please try again"));
			count=0;
			err_msg();
		}
	}
	else{
		transmitString_F(PSTR("/*..You have entered "));
		transmitInt(counter1);
		transmitString_F(PSTR(" fruits but Exactly 4 fruits are needed for the mixture!,please try again..*/"));
		count=0;
		err_msg();
	}
	
} 
 long int multiply(long int y, long int x){
	 long int mult = 0;
	 for(int i=0;i<x;i++){
		 mult=mult+y;
		 
	 }
	 return mult;
 }
void EEPROM_write(unsigned int uiAddress, unsigned char ucData)
{
	/* Wait for completion of previous write */
	while(EECR & (1<<EEWE));
	/* Set up address and data registers */
	EEAR = uiAddress;
	
	/* Write logical one to EEMWE */
	EECR |= (1<<EEMWE);
	/* Start eeprom write by setting EEWE */
	EECR |= (1<<EEWE);
}
void EEPROM_write_string(unsigned char address,unsigned char *number){
	while(*number){
		EEPROM_write(address,*number++);
	}
}
unsigned char EEPROM_read(unsigned int uiAddress)
{
	/* Wait for completion of previous write */
	while(EECR & (1<<EEWE));
	/* Set up address register */
	EEAR = uiAddress;
	/* Start eeprom read by writing EERE */
	EECR |= (1<<EERE);
	/* Return data from data register */
	return EEDR;
}   
void required_bottles(int num_bottles){
	if(size != 0){
		if(num_bottles<=bottle_number){
			float volume_used = (num_bottles*size);
			
			float volume_left = (reduce_available_litres)-volume_used;
			transmitString_F(PSTR("The volume of juice left after filling "));
			transmitInt(num_bottles);
			transmitString_F(PSTR(" bottles is "));
			transmitInt(volume_left);
			transmitString_F(PSTR(" liters"));
			TX_NEWLINE;
			TX_NEWLINE;
			TX_NEWLINE;
			transmitString_F(PSTR("*****SYSTEM CONFIGURATION COMPLETE*****"));
			TX_NEWLINE;
			TX_NEWLINE;
			transmitString_F(PSTR("During plant operation, you can view the report by pausing the sys                                                                                                                                    jb v vth ctem "));
			TX_NEWLINE;
			//report_menu();
		}
		else{
			transmitString_F(PSTR("The number of bottles that can be filled from the available juice is "));
			transmitInt(bottle_number);
			transmitString_F(PSTR(" bottles"));
			menu('4');
		}
		
	}
	else{
		transmitString_F(PSTR("*****Please set the bottle size to continue*****"));
		menu('3');
	}
}
int number_of_bottles(float size){
	float number_of_bottles = (reduce_available_litres)/size;
	if(number_of_bottles>=1)
	{
		if(ceilf(number_of_bottles)==number_of_bottles){
			return number_of_bottles;
		}
		else{
			float left_juice = number_of_bottles-floorf(number_of_bottles);
			int left_over = ceilf(left_juice*size);
			return floorf(number_of_bottles);
		}
	} 
	else{
		float less_juice = number_of_bottles*size;
		transmitString_F(PSTR("The remaining juice "));
		transmitInt(less_juice);
		transmitString_F(PSTR("liters can't fill at least one bottle!"));
		
		return 0;
	}
}
void report(unsigned char choice){
	switch(choice){
		
		case '1':
		TX_NEWLINE;
		transmitString_F(PSTR("|******************************************************|"));
		TX_NEWLINE;
		transmitString_F(PSTR("|    AVAILABLE DRINK IN THE STORAGE CONTAINER          |"));
		TX_NEWLINE;
		TX_NEWLINE;
		transmitString_F(PSTR("|******************************************************|"));
		TX_NEWLINE;
		TX_NEWLINE;
		TX_NEWLINE;
		
		
		transmitString_F(PSTR("The remaining juice in the storage is "));
		transmitInt(available_litres);
		TX_NEWLINE;
		
		break;
		
		case '2':
		TX_NEWLINE;
		transmitString_F(PSTR("|******************************************************|"));
		TX_NEWLINE;
		transmitString_F(PSTR("|    NUMBER OF BOTTLES FILLED WITH THE DRINK           |"));
		TX_NEWLINE;
		TX_NEWLINE;
		transmitString_F(PSTR("|******************************************************|"));
		TX_NEWLINE;
		TX_NEWLINE;
		transmitString_F(PSTR("The number of bottles filled with the drink is "));
		transmitInt(bottle);
		TX_NEWLINE;
		
		break;
	
		
		case '3':
		TX_NEWLINE;
		transmitString_F(PSTR("|******************************************************|"));
		TX_NEWLINE;
		transmitString_F(PSTR("|    AMOUNT OF FRUITS LEFT AFTER MIXING THE DRINK      |"));
		TX_NEWLINE;
		TX_NEWLINE;
		transmitString_F(PSTR("|******************************************************|"));
		TX_NEWLINE;
		TX_NEWLINE;
		transmitString_F(PSTR("Total fruits left after mixing: "));
		transmitInt(left_fruits);
		TX_NEWLINE;
		
		break;
		
		case '4':
		TX_NEWLINE;
		transmitString_F(PSTR("|******************************************************|"));
		TX_NEWLINE;
		transmitString_F(PSTR("|    HOW MUCH TIME THE PLANT HAS WORKED                |"));
		TX_NEWLINE;
		TX_NEWLINE;
		transmitString_F(PSTR("|******************************************************|"));
		TX_NEWLINE;
		TX_NEWLINE;
		transmitString_F(PSTR(" THE PLANT HAS WORKED FOR : "));
		transmitInt(working_time);
		transmitString_F(PSTR(" SECONDS."));
		TX_NEWLINE;
		
		break;
		
		case '5':
		TX_NEWLINE;
		transmitString_F(PSTR("|******************************************************|"));
		TX_NEWLINE;
		transmitString_F(PSTR("|    HOW MUCH TIME THE PLANT HAS RESTED                |"));
		TX_NEWLINE;
		TX_NEWLINE;
		transmitString_F(PSTR("|******************************************************|"));
		TX_NEWLINE;
		TX_NEWLINE;
		transmitString_F(PSTR(" THE PLANT HAS RESTED FOR : "));
		transmitInt(rest_time);
		transmitString_F(PSTR(" SECONDS."));
		TX_NEWLINE;
		
		break;
		
		case '6':
		TX_NEWLINE;
		transmitString_F(PSTR("|******************************************************|"));
		TX_NEWLINE;
		transmitString_F(PSTR("|    CHANGE BOTTLE VOLUME TO BE DISPENSED              |"));
		TX_NEWLINE;
		TX_NEWLINE;
		transmitString_F(PSTR("|******************************************************|"));
		TX_NEWLINE;
		TX_NEWLINE;
		reduce_available_litres=available_litres;
		menu('3');
		break;
		
		default:
		TX_NEWLINE;
		transmitString_F(PSTR("Invalid Option, Try Again (1 - 6)"));
		TX_NEWLINE;
		transmitString_F(PSTR("---------------------------------------"));
		TX_NEWLINE;
		transmitString_F(PSTR("*****Press Enter to try again*****"));
		TX_NEWLINE;
		transmitByte('>');
		
		break;
	}
}   
void report_menu(){
	 alt = UDR0;		
		TX_NEWLINE;
		TX_NEWLINE;
		transmitString_F(PSTR(">> 1 : VIEW AVAILABLE DRINK IN THE STORAGE CONTAINER"));
		TX_NEWLINE;
		transmitString_F(PSTR(">> 2 : VIEW NUMBER OF BOTTLES FILLED WITH THE DRINK "));
		TX_NEWLINE;
		transmitString_F(PSTR(">> 3 : VIEW AMOUNT OF FRUITS LEFT AFTER MIXING THE DRINK"));
		TX_NEWLINE;
		transmitString_F(PSTR(">> 4 : VIEW HOW MUCH TIME THE PLANT HAS WORKED"));
		TX_NEWLINE;
		transmitString_F(PSTR(">> 5 : VIEW HOW MUCH TIME THE PLANT HAS RESTED"));
		TX_NEWLINE;
		transmitString_F(PSTR(">> 6 : CHANGE BOTTLE VOLUME TO BE DISPENSED"));
		TX_NEWLINE;
		
		transmitByte('>');
		alt = receiveByte();
		TX_NEWLINE;

		report(alt);
}
void count_bottles(int counter){
	_delay_ms(500);
	PORTF=0x01;
	PORTC=0;
	_delay_ms(500);
	PORTF=0x02;
	PORTC=counter;
	_delay_ms(500);
}
void menu(unsigned char opt){
	
		
   switch (opt)
   {
	   case '1': TX_NEWLINE;
			transmitString_F(PSTR("|******************************************************|"));
			TX_NEWLINE;
			transmitString_F(PSTR("|    ENTER NUMBER OF FRUITS AVAILABLE IN STORE         |"));
			TX_NEWLINE;
			transmitString_F(PSTR("|   acceptable values ( from 0000 up to 5000 )         |"));
			TX_NEWLINE;
			TX_NEWLINE;
			transmitString_F(PSTR("|******************************************************|"));
			TX_NEWLINE;
			TX_NEWLINE;
			
			transmitString_F(PSTR("Enter number of oranges:  <FORMAT **** i.e 0024,2341,etc>"));
			TX_NEWLINE;
			transmitByte('>');
			
			/*store each received byte in an array*/
			oranges = receiveByte(); transmitByte(oranges);
			NUM_OF_ORANGES[0] = (oranges);
			oranges = receiveByte(); transmitByte(oranges);
			NUM_OF_ORANGES[1] = (oranges);
			oranges = receiveByte(); transmitByte(oranges);
			NUM_OF_ORANGES[2] = (oranges);
			oranges = receiveByte(); transmitByte(oranges);
			NUM_OF_ORANGES[3] = (oranges);
			//receiveByte();
			total_oranges = xter_to_int(NUM_OF_ORANGES,4);
			//EEPROM_write(0,total_oranges);
			transmitString_F(PSTR("                                                  "));
			TX_NEWLINE;
			TX_NEWLINE;
			
			transmitString_F(PSTR("Enter number of mangoes:  <FORMAT **** i.e 0024,2341,etc>"));
			TX_NEWLINE;
			transmitByte('>');
			
			/*store each received byte in an array*/
			mangoes = receiveByte(); transmitByte(mangoes);
			NUM_OF_MANGOES[0] = (mangoes);
			mangoes = receiveByte(); transmitByte(mangoes);
			NUM_OF_MANGOES[1] = (mangoes);
			mangoes = receiveByte(); transmitByte(mangoes);
			NUM_OF_MANGOES[2] = (mangoes);
			mangoes = receiveByte(); transmitByte(mangoes);
			NUM_OF_MANGOES[3] = (mangoes);
			total_mangoes = xter_to_int(NUM_OF_MANGOES,4); // available number of mangoes in integer form
			TX_NEWLINE;
			transmitString_F(PSTR("                                                  "));
			TX_NEWLINE;
			TX_NEWLINE;
			
			transmitString_F(PSTR("Enter number of Guavas:  <FORMAT **** i.e 0024,2341,etc>"));
			TX_NEWLINE;
			transmitByte('>');
			
			/*store each received byte in an array*/
			guavas = receiveByte(); transmitByte(guavas);
			NUM_OF_GUAVAS[0] = (guavas);
			guavas = receiveByte(); transmitByte(guavas);
			NUM_OF_GUAVAS[1] = (guavas);
			guavas = receiveByte(); transmitByte(guavas);
			NUM_OF_GUAVAS[2] = (guavas);
			guavas = receiveByte(); transmitByte(guavas);
			NUM_OF_GUAVAS[3] = (guavas);
			//receiveByte();
			total_guavas = xter_to_int(NUM_OF_GUAVAS,4); // available guavas in integer form
			//EEPROM_write(2,total_guavas);
			transmitString_F(PSTR("                                                  "));
			TX_NEWLINE;
			TX_NEWLINE;
			
			transmitString_F(PSTR("Enter number of Apples: <FORMAT **** i.e 0024,2341,etc>"));
			TX_NEWLINE;
			transmitByte('>');
			
			apples = receiveByte(); transmitByte(apples);
			NUM_OF_APPLES[0] = (apples);
			apples = receiveByte(); transmitByte(apples);
			NUM_OF_APPLES[1] = (apples);
			apples = receiveByte(); transmitByte(apples);
			NUM_OF_APPLES[2] = (apples);
			apples = receiveByte(); transmitByte(apples);
			NUM_OF_APPLES[3] = (apples);
			//receiveByte();
			total_apples = xter_to_int(NUM_OF_APPLES,4); // available apples in integer form
			//EEPROM_write(3,total_apples);
			transmitString_F(PSTR("                                                  "));
			TX_NEWLINE;
			TX_NEWLINE;
			
			transmitString_F(PSTR("Enter number of Pineapples:  <FORMAT **** i.e 0024,2341,etc>"));
			TX_NEWLINE;
			transmitByte('>');
			
			/*store each received byte in an array*/
			pineapples = receiveByte(); 
			transmitByte(pineapples);
			NUM_OF_PINEAPPLES[0] = (pineapples);
			pineapples = receiveByte(); 
			transmitByte(pineapples);
			NUM_OF_PINEAPPLES[1] = (pineapples);
			pineapples = receiveByte(); 
			transmitByte(pineapples);
			NUM_OF_PINEAPPLES[2] = (pineapples);
			pineapples = receiveByte(); 
			transmitByte(pineapples);
			NUM_OF_PINEAPPLES[3] = (pineapples);
			//receiveByte();
			total_pineapples = xter_to_int(NUM_OF_PINEAPPLES,4); //available pineapples in integer form
			//EEPROM_write(4,total_pineapples);
			transmitString_F(PSTR("                                                  "));
			TX_NEWLINE;
			TX_NEWLINE;
			total_fruits = total_apples + total_guavas + total_mangoes + total_oranges + total_pineapples;
			transmitString_F(PSTR("The total number of available fruits is: "));
			transmitInt(total_fruits);
			TX_NEWLINE;
			TX_NEWLINE;
			TX_NEWLINE;
			transmitString_F(PSTR("Operation successful,Press ENTER to continue......"));
			receiveByte();
			TX_NEWLINE;
			TX_NEWLINE;
			menu('2');  
			  
	   break;
	   case '2': check_missed();
	          TX_NEWLINE;
			  transmitString_F(PSTR("|******************************************************|"));
			  TX_NEWLINE;
			  transmitString_F(PSTR("|    ENTER PERCENTAGE OF ANY 4 FRUITS TO BE MIXED      |"));
			  TX_NEWLINE;
			  TX_NEWLINE;
			  transmitString_F(PSTR("|******************************************************|"));
			  TX_NEWLINE;
			  TX_NEWLINE;
			  transmitString_F(PSTR("Enter percentage of mangoes: <FORMAT ** i.e 09,13,00(if fruit not considered)>"));
			  TX_NEWLINE;
			  transmitByte('>');
			  
			  mangoes = receiveByte();
			  transmitByte(mangoes);
			  PERCENT_OF_MANGOES[0] = (mangoes);
			  mangoes = receiveByte();
			  transmitByte(mangoes);
			  PERCENT_OF_MANGOES[1] = (mangoes);
			  percent_mangoes = xter_to_integer(PERCENT_OF_MANGOES);
			  
			  TX_NEWLINE;
			  fruit_count = checkpercent(percent_mangoes,total_mangoes);
			  TX_NEWLINE;
			  
			  
			  transmitString_F(PSTR("Enter percentage of apples: <FORMAT ** i.e 09,13,00(if fruit not considered)>"));
			  TX_NEWLINE;
			  transmitByte('>');
			  
			  apples = receiveByte();
			  transmitByte(apples);
			  PERCENT_OF_APPLES[0] = (apples);
			  apples = receiveByte();
			  transmitByte(apples);
			  PERCENT_OF_APPLES[1] = (apples);
			  percent_apple = xter_to_integer(PERCENT_OF_APPLES);
			  TX_NEWLINE;
			  fruit_count = checkpercent(percent_apple,total_apples);
			  
			  TX_NEWLINE;
			  transmitString_F(PSTR("Enter percentage of oranges: <FORMAT ** i.e 09,13,00(if fruit not considered)>"));
			  TX_NEWLINE;
			  transmitByte('>');
			  
			  oranges = receiveByte();
			  transmitByte(oranges);
			  PERCENT_OF_ORANGES[0] = (oranges);
			  oranges = receiveByte();
			  transmitByte(oranges);
			  PERCENT_OF_ORANGES[1] = (oranges);
			  percent_oranges = xter_to_integer(PERCENT_OF_ORANGES);
			  TX_NEWLINE;
			  fruit_count = checkpercent(percent_oranges,total_oranges);
			  
			  TX_NEWLINE;
			  transmitString_F(PSTR("Enter percentage of pineapples: <FORMAT ** i.e 09,13,00(if fruit not considered)>"));
			  TX_NEWLINE;
			  transmitByte('>');
			  
			  pineapples = receiveByte();
			  transmitByte(pineapples);
			  PERCENT_OF_PINEAPPLES[0] = (pineapples);
			  pineapples = receiveByte();
			  transmitByte(pineapples);
			  PERCENT_OF_PINEAPPLES[1] = (pineapples);
			  percent_pineapples = xter_to_integer(PERCENT_OF_PINEAPPLES);
			  TX_NEWLINE;
			  fruit_count = checkpercent(percent_pineapples,total_pineapples);
			    
				
				TX_NEWLINE;
				transmitString_F(PSTR("Enter percentage of guavas: <FORMAT ** i.e 09,13,00(if fruit not considered)>"));
				TX_NEWLINE;
				transmitByte('>');
				
				guavas = receiveByte();
				transmitByte(guavas);
				PERCENT_OF_GUAVAS[0] = (guavas);
				guavas = receiveByte();
				transmitByte(guavas);
				PERCENT_OF_GUAVAS[1] = (guavas);
				percent_guavas = xter_to_integer(PERCENT_OF_GUAVAS);
				TX_NEWLINE;
				fruit_count = checkpercent(percent_guavas,total_guavas);
				transmitString_F(PSTR("The fruit count is "));
				transmitInt(fruit_count);
				TX_NEWLINE;
				TX_NEWLINE;
				// checks if 4 fruits have been selected, and the percentage is 100
		   //menu('3');
		   /*used fruits*/
		   used_mangoes = ((percent_mangoes)*(total_mangoes/100));// used mangoes
		  	used_oranges = ((percent_oranges)*(total_oranges/100));// used oranges
		    
		   used_guavas = ((percent_guavas)*(total_guavas/100));// used guavas
		   used_apples = ((percent_apple)*(total_apples/100));// used apples
		   used_pineapples = ((percent_pineapples)*(total_pineapples/100));// used pineapples
		   
		   /*left fruits*/
		   left_mangoes = total_mangoes - used_mangoes;
		   left_oranges = total_oranges - used_oranges;
		   left_guavas = total_guavas - used_guavas;
		   left_apples = total_apples - used_apples;
		   left_pineapples = total_pineapples-used_pineapples;
		  
		   left_fruits = ((left_mangoes) + (left_guavas)+ (left_apples)+((left_pineapples)+ (left_oranges)) );
		   eeprom_write_byte(3,left_fruits);
		   validate(fruit_count);
		   
		   break;
		   
	   case '3': check_missed();
	        TX_NEWLINE;
			transmitString_F(PSTR("|******************************************************|"));
			TX_NEWLINE;
			transmitString_F(PSTR("|    SELECT BOTTLE SIZE TO BE CONSIDERED               |"));
			TX_NEWLINE;
			transmitString_F(PSTR("|******************************************************|"));
			TX_NEWLINE;
			
			transmitString_F(PSTR("1. 500ml"));
			TX_NEWLINE;
			transmitString_F(PSTR("2. 300ml"));
			TX_NEWLINE;
			transmitByte('>');
			char bottle_size = receiveByte();
			transmitByte(bottle_size);
			TX_NEWLINE;
					switch(bottle_size){
						
						case '1':
						size = 0.5;
						
						bottle_number=number_of_bottles(size);
						transmitString_F(PSTR("The available juice is "));
						transmitInt(reduce_available_litres);
						transmitString_F(PSTR(" liters."));
						
						TX_NEWLINE;
						TX_NEWLINE;
						
						transmitString_F(PSTR("The possible number of bottles is "));
						transmitInt(bottle_number);
						TX_NEWLINE;
						TX_NEWLINE;
						transmitString_F(PSTR("Operation successful, Press ENTER to continue...."));
						receiveByte();
						TX_NEWLINE;
						TX_NEWLINE;
						menu('4');
						break;
						case '2':
						size = 0.3;
						
						bottle_number=number_of_bottles(size);
						TX_NEWLINE;
						TX_NEWLINE;
						transmitString_F(PSTR("The available juice is "));
						transmitInt(reduce_available_litres);
						transmitString_F(PSTR(" liters."));
						TX_NEWLINE;
						TX_NEWLINE;
						transmitString_F(PSTR("The possible number of bottles is "));
						transmitInt(bottle_number);
						TX_NEWLINE;
						TX_NEWLINE;
						transmitString_F(PSTR("Operation successful, Press ENTER to continue...."));
						receiveByte();
						TX_NEWLINE;
						TX_NEWLINE;
						menu('4');
						
						break;
						default:
						transmitString_F(PSTR("Invalid choice, please try again!"));
						menu('3');
						break;
						
					}
			
		   break;
	   case '4': check_missed();
	        TX_NEWLINE;
			transmitString_F(PSTR("|******************************************************|"));
			TX_NEWLINE;
			transmitString_F(PSTR("|    SET NUMBER OF REQUIRED BOTTLES                    |"));
			TX_NEWLINE;
			transmitString_F(PSTR("|******************************************************|"));
			TX_NEWLINE;
			  
			 char REQ_NUM[4]; 
			 char bottles;
			  transmitString_F(PSTR("Enter the number of bottles to be filled: "));
			  TX_NEWLINE;
			  transmitByte('>');
			  bottles = receiveByte();
			  transmitByte(bottles);
			  REQ_NUM[0] = (bottles);
			  bottles = receiveByte();
			  transmitByte(bottles);
			  REQ_NUM[1] = (bottles);
			  bottles = receiveByte();
			  transmitByte(bottles);
			  REQ_NUM[2] = (bottles);
			  
			  bottles_required = bottle_xter_to_int(REQ_NUM);
			  
			  TX_NEWLINE;
			  required_bottles(bottles_required);
			  filled_bottles=bottles_required;
		   break;
		
		default:
		TX_NEWLINE;
			transmitString_F(PSTR("Invalid Option, Try Again (1 - 4)"));
            
			  TX_NEWLINE;
			  transmitString_F(PSTR("---------------------------------------"));
			  TX_NEWLINE;
			  transmitString_F(PSTR("*****Press Enter to try again*****"));
			  TX_NEWLINE;
			  transmitByte('>');
			break;
   }
	 

}
//USART ISR
ISR(USART0_RX_vect){
		//char dataIn;
		if (working_time==0)
		{
			
		option = UDR0;
				
		TX_NEWLINE;
		transmitString_F(PSTR(">> Choose one of the following options to continue: "));
		TX_NEWLINE;
		transmitString_F(PSTR(">> 1 : Enter number of fruits available"));
		TX_NEWLINE;
		transmitString_F(PSTR(">> 2 : Enter percentage of any 4 fruits to be mixed: "));
		TX_NEWLINE;
		transmitString_F(PSTR(">> 3 : Select bottle size to use."));
		TX_NEWLINE;
		transmitString_F(PSTR(">> 4 : Set required number of bottles."));
		TX_NEWLINE;
		
		transmitByte('>');
		option = receiveByte();
		TX_NEWLINE;
		/*transmitByte(option);
		TX_NEWLINE;*/
		menu(option);
		}else{
			report_menu();
		}
}
int main(void)
{
	DDRA=0xFF;
	DDRB=0xFF;
	DDRD=0X0F;
	DDRC=0XFF;
	DDRF=0XFF;
	DDRE=0XFE;
	main_menu();
	//timing=0;
	working_time =eeprom_read_byte(2);
	available_litres=eeprom_read_byte(0); 
	bottles_required= eeprom_read_byte(1); 
	filled_bottles=bottles_required;
	left_fruits= eeprom_read_byte(3);
	bottle = eeprom_read_byte(4);
	rest_time=eeprom_read_byte(5);
	
	TX_NEWLINE;
	lcd_init();
   
	while (1){		
		if ((PIND&(1<<4))==0)
	{
		       if(bottles_required != NULL){
				   if (filled_bottles != 0)
				   {
				   
				   t=0;
				   PORTE=(1<<PE4);
				   PORTE &= ~(1<<PE3);
				   working_time = timer();
				   eeprom_write_byte(2,working_time);
					   clearscreen();
					   status();
					  motor();
					  clearscreen();
					   bottle_filling();
					  clearscreen();
					  status();
					  motor();
					  clearscreen();
					  bottle_sealing();
					  clearscreen();
					  status();
					  for (int l=1; l<=2; l++)
					   {
						motor();
					  }
					  clearscreen();
					  bottle_storage();
					  motor();
					  clearscreen();
					 if (available_litres==0)
					 {
						 working_time=0;
						available_litres=0; 
						bottles_required= 0; 
						filled_bottles=bottles_required;
						left_fruits= 0;
						bottle = 0;
						timing =0;
					 }
					  }else{
						   notice();
						   working_time = timer();
						   clearscreen();
					  }
					  
		       }else
			  {   
			   notice_message();
			 rest_time = timer();
		      clearscreen();
			 
			     }
		  }else
{         
	
    message_comment();
	PORTE=(1<<PE3);
	PORTE &= ~(1<<PE4);
	rest_time = timer();
	
	int stop = increment();
	
	if (stop==1)
	{
		report_info();
	}
	 rest_time = timer();
	  eeprom_write_byte(5,rest_time);
	 clearscreen();
	
	}
	}
}
void main_menu(){
	USART_Init(UBRR);
	//init_timer0();
	TX_NEWLINE;
	transmitString_F(PSTR("Session started.........."));
	TX_NEWLINE;
	TX_NEWLINE;
	transmitString_F(PSTR("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
	TX_NEWLINE;
	transmitString_F(PSTR("|            WELCOME TO MACK BEVERAGES LTD              |"));
	TX_NEWLINE;
	transmitString_F(PSTR("|                   -OUR MEMBERS-                       |"));
	TX_NEWLINE;
	transmitString_F(PSTR("|                                                       |"));
	TX_NEWLINE;
	transmitString_F(PSTR("|     NAMAYANJA SWABRA            15/U/10195/PS         |"));
	TX_NEWLINE;
	transmitString_F(PSTR("|     KAWUMA DANIEL               15/U/417              |"));
	TX_NEWLINE;
	transmitString_F(PSTR("|     WASAANYI JAMES              14/U/15864/EVE        |"));
	TX_NEWLINE;
	transmitString_F(PSTR("|     SSENSALO EDGAR              15/U/12932/EVE        |"));
	TX_NEWLINE;
	transmitString_F(PSTR("|     NAMUTEBI JANET              15/U/21941/PS         |"));
	TX_NEWLINE;
	transmitString_F(PSTR("|     MUKWAYA NICHOLAS            15/U/8201/EVE         |"));
	TX_NEWLINE;
	transmitString_F(PSTR("|     BATAIRE JONATHAN            15/U/4434/EVE         |"));
	TX_NEWLINE;
	transmitString_F(PSTR("|     BABIRYE SHARON VICTORIA     15/U/4256/EVE         |"));
	TX_NEWLINE;
	transmitString_F(PSTR("|     MULINDWA HENRY              15/U/8235/EVE         |"));
	TX_NEWLINE;
	transmitString_F(PSTR("|     KANABAHITA ISAAC            15/U/6159/PS          |"));
	TX_NEWLINE;
	transmitString_F(PSTR("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
	TX_NEWLINE;
	transmitString_F(PSTR("                                           "));
	TX_NEWLINE;
	transmitString_F(PSTR("PRESS ANY KEY to proceed..."));
}
void report_info(){
	USART_Init(UBRR);
	//init_timer0();
	TX_NEWLINE;
	transmitString_F(PSTR("System rested.........."));
	TX_NEWLINE;
	TX_NEWLINE;
	transmitString_F(PSTR("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
	TX_NEWLINE;
	transmitString_F(PSTR("|The system is ready to process the report|"));
	TX_NEWLINE;
	transmitString_F(PSTR("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
	TX_NEWLINE;
	transmitString_F(PSTR("                                           "));
	TX_NEWLINE;
	transmitString_F(PSTR("PRESS ANY KEY to proceed..."));
}
 void increment(){
	t=t+1;
       } 
void check_missed(){
	if(total_fruits==NULL){
		      TX_NEWLINE;
			  transmitString_F(PSTR("*****Press Enter Available Fruits First *****"));
			  TX_NEWLINE;
		menu('1');
	}
}
