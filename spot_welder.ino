// Spot welder software written for arduino nano with an Nokia 5110 display and a rotary encoder
// Krzysztof Mazur 2018
// Some of the code is inspired by a project from Nick Koumaris @  http://www.educ8s.tv   

#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <EEPROM.h>
#define BUTTONPIN 2		//Trigger button pin number
#define OUTPUTPIN 7		//SSR pin number

int selected = 0;				//number of the selected menu item
int firstOption = 0;			//number of the menu item that is currently at the top of the display
int level = 1;					//menu level which the program is currently in 
unsigned long lastTrigger = 0;  // the last time the button was toggled

String menu[7] = {"Pulse1: ", "Delay: ", "Pulse2: ", "Mode: ", "Save config", "Load config", "Contrast"}; //menu item labels

int pulse1;				//variable to hold the length of the first pulse
int delayTime;			//variable to hold the length of the delay between pulses
int pulse2;				//variable to hold the length of the second pulse
int contrast;			//variable to hold the contrast setting
String mode[3] = { "2Pulse", "1Pulse", "Direct" }; //modes labels
int selectedMode = 0;	//number of the currently selected mode

String save[3] = { "Back", "Save1", "Save2" }; 		//save menu options labels
int selectedSave = 0;                      					//number of the selected option in save menu

String load[3] = { "Back", "Load1", "Load2" };		//load menu options labels
int selectedLoad = 0;                      					//number of the selected option in load menu                     

//Flags indicating encoder actions
boolean up = false;
boolean down = false;
boolean middle = false;
//necessary definitions for the encoder
ClickEncoder *encoder; 
int16_t last, value;

//Display configuration:

// Hardware SPI (faster, but must use certain hardware pins):
// SCK is LCD serial clock (SCLK) - this is pin 13 on Arduino Nano
// MOSI is LCD DIN - this is pin 11 on an Arduino Nano
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544( 5, 4, 3); 


  void saveEEPROM(char n)		//function for saving weld times in the EEPROM
  {
    int eeAddress = n*100;			//changing the address depending on the save number
    EEPROM.put(eeAddress, pulse1);
    eeAddress += sizeof(int);
    EEPROM.put(eeAddress, delayTime);
    eeAddress += sizeof(int);
    EEPROM.put(eeAddress, pulse2);
    eeAddress += sizeof(int);
    EEPROM.put(eeAddress, selectedMode);
  }
  
  void loadEEPROM(char n)			//function for reading weld times from the EEPROM
  {
    int eeAddress = n*100;			//changing the address depending on the save number
    EEPROM.get(eeAddress, pulse1);
    eeAddress += sizeof(int);
    EEPROM.get(eeAddress, delayTime);
    eeAddress += sizeof(int);
    EEPROM.get(eeAddress, pulse2);
    eeAddress += sizeof(int);
    EEPROM.get(eeAddress, selectedMode);
	//Changing the string shown in main level of the menu:
	menu[0]="Pulse1: ";
	menu[0] += pulse1;
	menu[1]="Delay: ";
	menu[1] += delayTime;
	menu[2]="Pulse2: ";
	menu[2] += pulse2;
	menu[3]="Mode: ";
	menu[3] += mode[selectedMode];
  }

  void setContrast()		//function for changing the contrast and refreshing the display with the new setting
  {
    display.setContrast(contrast);
    display.display();
  }

  void timerIsr() 			//Encoder routine called every second
  {
  encoder->service();
  }

void displayIntMenuPage(String menuItem, int itemValue) //function that draws a menu showing current value of an integer variable
{
    display.setTextSize(1);
    display.clearDisplay();
    display.setTextColor(BLACK, WHITE);
    display.setCursor(5, 0);
    display.print(menuItem);
    display.drawFastHLine(0,10,83,BLACK);
    display.setCursor(5, 15);
    display.print("Value");
    display.setTextSize(2);
    display.setCursor(5, 25);
    display.print(itemValue);
    display.setTextSize(2);
    display.display();
}

void displayStringMenuPage(String menuItem, String itemValue)	//function that draws a menu showing current value of an string variable
{
    display.setTextSize(1);
    display.clearDisplay();
    display.setTextColor(BLACK, WHITE);
    display.setCursor(5, 0);
    display.print(menuItem);
    display.drawFastHLine(0,10,83,BLACK);
    display.setCursor(5, 15);
    display.print("Choice:");
    display.setTextSize(2);
    display.setCursor(5, 25);
    display.print(itemValue);
    display.setTextSize(2);
    display.display();
}

void displayMenuItem(String item, int position, boolean selected) //function drawing a single item in the main menu
{
    if(selected)	
    {
      display.setTextColor(WHITE, BLACK);				//write the menu item white on black if its the currently selected option
    }else
    {
      display.setTextColor(BLACK, WHITE);				//else write it black on white
    }
    display.setCursor(0, position);
    display.print(">"+item);
}

void readRotaryEncoder()		//function for setting the flags indicating encoder rotation
{
  value += encoder->getValue();	
  
  if (value/2 > last) {
    last = value/2;
    down = true;
    delay(150);				//debounce for 150 ms
  }else   if (value/2 < last) {
    last = value/2;
    up = true;
    delay(150);				//debounce for 150 ms
  }
}

 void drawMenu()	//function drawing the whole main menu
  {
    
  if (level==1) 
  {    
    display.setTextSize(1);
    display.clearDisplay();
    display.setTextColor(BLACK, WHITE);
    display.setCursor(10, 0);
    display.print("SPOT WELDER");
    display.drawFastHLine(0,10,83,BLACK);
	displayMenuItem(menu[firstOption], 15,(selected==firstOption));			//only three menu items starting from the firstOption are displayed
    displayMenuItem(menu[firstOption+1], 25,(selected==firstOption+1));
    displayMenuItem(menu[firstOption+2], 35,(selected==firstOption+2));
    display.display();
  }else{
    switch(selected)
    {
      case 0:
      displayIntMenuPage(menu[0], pulse1);
      break;
      case 1:
      displayIntMenuPage(menu[1], delayTime);
      break;
      case 2:
      displayIntMenuPage(menu[2], pulse2);
      break;
      case 3:
      displayStringMenuPage(menu[3], mode[selectedMode]);
      break;
      case 4:
      displayStringMenuPage(menu[4], save[selectedSave]);
      break;
      case 5:
      displayStringMenuPage(menu[5], load[selectedLoad]);
      break;
      case 6:
      displayIntMenuPage(menu[6], contrast);
      break;
      default:
      break;
    }
  }
  }

  void setup()
  {
	  
  //Setting up the encoder:
  encoder = new ClickEncoder(A1, A0, A2); 
  encoder->setAccelerationEnabled(true);
  //Setting up the display:
  display.begin();      
  display.clearDisplay(); 
  display.setRotation(2);
  //Reading initial values from EEPROM
  EEPROM.get(0, contrast);
  setContrast();  
  loadEEPROM(1);
  //Setting pins:
  pinMode(BUTTONPIN,INPUT_PULLUP);			//set button pin as input with pullup (external pullup is highly recomended)
  pinMode(OUTPUTPIN, OUTPUT);
  digitalWrite(OUTPUTPIN, LOW);
  //Setting the interrupt to check the encoder every second:
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
  
  
  last = encoder->getValue(); //setting the initial value of the encoder variable
}

void loop() 
{

  drawMenu(); // call the drawMenu() function to draw the main menu if the program is in level 1

  readRotaryEncoder();	//check if the encoder was rotated

  //check if the button was shortly pressed:
   ClickEncoder::Button b = encoder->getButton();
   if (b != ClickEncoder::Open) {
   switch (b) {
      case ClickEncoder::Clicked:
         middle=true;
        break;
    }
  }    
  //If the encoder was rotated counter-clockwise:
  if (up && level == 1 ) { //if we are in the main menu
	  up=false;	//clear the flag
    selected--;		//change the selected menu item accordingly
    if(selected<0)selected=0;
    if(firstOption>selected)firstOption=selected;	//change the displayed menu items if we get above the first displayed item
    
     
    
  }else if (up && level == 2) { 		//if we are in an perticular menu option:
    up=false;		//clear the flag
    switch(selected)	//decrement the selected variable
    {
      case 0:
      pulse1 -= 2;
      break;
      case 1:
      delayTime -= 2;
      break;
      case 2:
      pulse2 -= 2;
      break;
      case 3:
      selectedMode--;
      if(selectedMode<0)selectedMode = 0;
      break;
      case 4:
      selectedSave--;
      if(selectedSave<0)selectedSave = 0;
      break;
      case 5:
      selectedLoad--;
      if(selectedLoad<0)selectedLoad = 0;
      break;
      case 6:
      contrast--;
      setContrast();  //instantly change the contrast to help show the difference
      break;
      default:
      break;
    }
   
  }
  //If the encoder was rotated clockwise:
  if (down && level == 1) //if we are in the main menu
  {
    down = false;			//clear the flag
    selected++;			//change the selected menu item accordingly
    if(selected>6)selected=6;
    if((selected-firstOption)==3)firstOption++;		//change the displayed menu items if we get below the last displayed item

  
  }else if (down && level == 2) {		//if we are in an perticular menu option:
    down=false;		//clear the flag
    switch(selected)		//increment the selected variable
    {
      case 0:
      pulse1 += 2;
      break;
      case 1:
      delayTime += 2;
      break;
      case 2:
      pulse2 += 2;
      break;
      case 3:
      selectedMode++;
      if(selectedMode>2)selectedMode = 2;
      break;
      case 4:
      selectedSave++;
      if(selectedSave>2)selectedSave = 2;
      break;
      case 5:
      selectedLoad++;
      if(selectedLoad>2)selectedLoad = 2;
      break;
      case 6:
      contrast++;
      setContrast();		//instantly change the contrast to help show the difference
      break;
      default:
      break;
    }
  }
  
  if (middle) //Middle Button is Pressed
  {
    middle = false; 	//clear the flag

    if (level == 1 ) {
      level=2;			//if we are in the main menu go into the selected option
     }
      else 
     {
     switch(selected)		//if we are in menu level 2
     {
       case 0:						
        menu[0]="Pulse1: ";			//change the main menu string when going back from menu level 2
        menu[0] += pulse1;
       break;
       case 1:
        menu[1]="Delay: ";			//change the main menu string when going back from menu level 2
        menu[1] += delayTime;
       break;
       case 2:
        menu[2]="Pulse2: ";			//change the main menu string when going back from menu level 2
        menu[2] += pulse2;
       break;
       case 3:
        menu[3]="Mode: ";			//change the main menu string when going back from menu level 2
        menu[3] += mode[selectedMode];
       break;
       case 4:
        if(selectedSave>0)saveEEPROM(selectedSave);	//save values to the selected slot 
        selectedSave=0;												//default to zero to avoid unwanted saving
       break;
       case 5:
        if(selectedLoad>0)loadEEPROM(selectedLoad);		//load values from the selected slot 
        selectedLoad=0;												//default to zero to avoid unwanted loading
       break;
       case 6:
        EEPROM.put(0, contrast);									//save the contrast setting to EEPROM
       break;
       default:
       break;
     }
     level=1; 
     }
   }   
   if(!digitalRead(BUTTONPIN))					//Trigger button was pressed
   {
	   if(level == 1 && millis()-lastTrigger>2000 && selectedMode != 2)		//if we are in the main menu and the last trigger was more than 2 seconds ago initiate pulse sequence
	   {
		   if(selectedMode == 0)		//keeping the conditions outside to achive more precise timing
		   {
			   digitalWrite(OUTPUTPIN, HIGH);
			   delay(pulse1);
			   digitalWrite(OUTPUTPIN, LOW);
			   delay(delayTime);
			   digitalWrite(OUTPUTPIN, HIGH);
			   delay(pulse2);
			   digitalWrite(OUTPUTPIN, LOW);
			   lastTrigger=millis();
		   }else
		   {
			   digitalWrite(OUTPUTPIN, HIGH);
			   delay(pulse1);
			   digitalWrite(OUTPUTPIN, LOW);
			   lastTrigger=millis();
		   }
	   }
	   if(level == 1 && selectedMode == 2)		// if we are in the direct mode activate the output for as long as the button is pressed (USE AT OWN RISK!)
	   {
		   while(!digitalRead(BUTTONPIN))
		   {
			   digitalWrite(OUTPUTPIN, HIGH);
		   }
		   digitalWrite(OUTPUTPIN, LOW);
	   }
   }
   
  }
  