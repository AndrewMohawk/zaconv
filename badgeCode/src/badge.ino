#include <VirtualWire.h>
#include <MemoryFree.h>
#include <LCD5110_Graph.h>
#include <EEPROM.h>

//*THIS* badgenumber
int BadgeNumber = 1338;

//Send Rand Interval things
long previousMillis = 0;
long randInterval = 300;

//All badges seen
//int BadgesIveSeen[100];
//LOL @ trying to use RAM for that.. 2K >_< *facepalm*.. all badges now stored in EEPROM, 0 = this badge, 1 = badge seen 1.

/*
	EEPROM is valid for 100,000 writes, we shouldnt go over that hopefully..

	Structure as of 2013/09/21 is:

	0-1: <currentBadgeNumber>
	2-3: <number badges seen>
	
	4-5: Seen Badge 1
	6-7: Seen Badge 2
	8-9: Seen Badge 3
	10-11: Seen Badge 4
	12-13: Seen Badge 5

	ie, badge = (number * 2) +2
	badge4 = (4*2)+2 = 10
	badge5 = (5*2)+2 = 12
	etc etc


*/

//Total badges this badge has seen
int numBadgesSeen = 0;


//Last 5 badges seen
int LastFiveBadges[5];
//Number of entries in last 5 queue
int numLastFiveBadges = 0;

/*
Last Five relationships I have seen that I am sending out that are not this badges
ie, ive seen 2 other badges talk, and caught that they had a relationship

LastFiveRelationships1 -> LastFiveRelationships2
eg. Andrew -> Roelof

*/

int LastFiveRelationships1[5];
int LastFiveRelationships2[5];

//Number of entries in last five relationships 'table'
int numLastFiveRelationships = 0;

//LCD Object
LCD5110 myGLCD(8,9,10,11,12);

/*
External logos: stored in zaconlogo.c
*/

//Main ZaCon Logo
extern uint8_t zaconlogo[];

//Text that reads 'lineup'
extern uint8_t zacon5lineup[];

//Top logo - 'zacon'
extern uint8_t topLogo[];

//Top logo - smaller 'zacon'
extern uint8_t smallTopLogo[];

//Top logo - 'stats'
extern uint8_t statsTopHeader[];

//Fonts
extern unsigned char SmallFont[];
extern unsigned char TinyFont[];


/*
MENU ITEMS
*/

//About menu items
const char PROGMEM about1[] = {"Badges designed for ZaCon V"};
const char PROGMEM about2[] = {"Special thanks to everyone who submitted and made it great"};
const char PROGMEM about3[] = {"Keep these badges as tokens of our appreciation"};
const char PROGMEM about4[] = {"From AndrewMohawk and people@zacon.org.za"};

const char* PROGMEM AboutArray[] = 	   
{
    about1,
    about2,
    about3,
    about4
};

int numAboutItems = 4;

//Main Menu items

const char PROGMEM menu0[] PROGMEM = {"Schedule"};
const char PROGMEM menu1[] PROGMEM = {"Live Speaker Update"};
const char PROGMEM menu2[] PROGMEM = {"Stats"};
const char PROGMEM menu3[] PROGMEM = {"About"};
const char PROGMEM menu4[] PROGMEM = {"Intro"};


const char* PROGMEM MenuArray[] = 	   
{
	menu0,
	menu1,
	menu2,
	menu3,
	menu4,
};

//number of menu items we have
int numMenuItems = 5;

//Speaker information
const char PROGMEM string_0[] = "08h00-0900: Coffee and Registration";   // "String 0" etc are strings to store - change to suit.
const char PROGMEM string_1[] = "09h00-09h15: Marco Slaviero - Welcome and housekeeping";
const char PROGMEM string_2[] = "09h15-10h00: Tyrone Erasmus - Pentesting with Mercury";
const char PROGMEM string_3[] = "10h00-10h45: Glenn Wilkinson - Terrorism,tracking,privacy and human interactions";
const char PROGMEM string_4[] = "10h45-11h00: Tea Break";
const char PROGMEM string_5[] = "11h00-11h30: Simeon Miteff - Alternatives to stateful packet filtering in R&E networking";
const char PROGMEM string_6[] = "11h00-11h30: Manuel Corregedor - Anti-malware Technique Evaluator (ATE) - Pwning AVs";
const char PROGMEM string_7[] = "11h30-12h00: Ross Simpson - Hacking Games (or 'why client side logic is bad')";

const char PROGMEM string_8[] = "12h30-13h00: Joshua - HTML5 exploits or 'Cutting the edge on web development'";
const char PROGMEM string_9[] = "13h00-14h00: Lunch Break";
const char PROGMEM string_10[] = "14h00-14h55: Andrew MacPherson - 88MPH: Digital tricks to bypass Physical security";
const char PROGMEM string_11[] = "14h55-15h55: Jacques Louw - Offensive Software Defined Radio";
const char PROGMEM string_12[] = "15h55-16h15: Tea Break";
const char PROGMEM string_13[] = "16h15-16h55: Schalk Heunis - Information from DSC Keybus";
const char PROGMEM string_14[] = "16h55-17h40: Tom Van den Bon - USB Reverse Engineering, Just as much fun as RS-232 RE";


const char* PROGMEM Schedule[] =
{   
  string_0,
  string_1,
  string_2,
  string_3,
  string_4,
  string_5,
  string_6,
  string_7,
  string_8,
  string_9,
  string_10,
  string_11,
  string_12,
  string_13,
  string_14,
};
//number of speakers
int numScheduleItems = 15;


//string used for carrying vars
char currentStr[100];
char currentRFStr[12];
char currentSpeaker[20];
/*
MISC Vars
*/

//scroller variables
int SpeakerScrollSize = 60;
int SpeakerScrollCurrent = 84;
//default menu
int defaultMenu = 0;
//current mode (menu item selected)
int currentMode = 0;
//screen load vars
int LoadedScreen = 0;
//current selected schedule item
int currentScheduleItem = 0;
int currentAboutItem = 0;



/*
	BUTTON STUFF
*/

//last button state (HIGH or LOW)
long debounceDelay = 100;    // the debounce time; increase if the output flickers
long lastButtonCheck = 0;


int redPin = 5;
int greenPin = 4;
int bluePin = 3;

void EEPROMWriteInt(int p_address, int p_value)
{
byte lowByte = ((p_value >> 0) & 0xFF);
byte highByte = ((p_value >> 8) & 0xFF);

EEPROM.write(p_address, lowByte);
EEPROM.write(p_address + 1, highByte);
}

unsigned int EEPROMReadInt(int p_address)
{
byte lowByte = EEPROM.read(p_address);
byte highByte = EEPROM.read(p_address + 1);

return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
}

void setup()
{
	//Start Serial Comms
	Serial.begin(9600);
	
	//Random seed
	randomSeed(analogRead(0));

	//Random Send Interval
	randInterval = random(350,2000);
	
	//Number of menu items (so i can be lazy)
	numMenuItems = sizeof(MenuArray)/sizeof(char *);
	numScheduleItems = sizeof(Schedule)/sizeof(char *);
	
	
	//Current speaker scroll size
	SpeakerScrollSize = strlen(currentSpeaker);

	/*
		LCD INIT
	*/
	myGLCD.InitLCD(70);
	myGLCD.setFont(SmallFont);
	
	//pinMode(A5, INPUT_PULLUP);	
	//delay(1000);

	
	/*
		RF Init
	*/
	vw_set_tx_pin(6);
	vw_set_rx_pin(7);
	vw_set_ptt_inverted(true); // Required for DR3100
	vw_setup(2000);	 // Bits per sec
	vw_rx_start();       // Start the receiver PLL running
	


	/* EEPROM Config */
	//EEPROMWriteInt(0,12345);
	int eBadge = EEPROMReadInt(0);
	if(eBadge == BadgeNumber)
	{
			numBadgesSeen = EEPROMReadInt(2);
	}
	else
	{
			/* No writes for now -- saving */
			EEPROMWriteInt(0,BadgeNumber); // write badge
			EEPROMWriteInt(2,0); // write num seen
	}

	
	//Clear variables
	for (int i=0;i<5;i++)
	{
	  
	  LastFiveBadges[i] = random(1111,9999);
	  LastFiveRelationships1[i] = random(1111,9999);
	  LastFiveRelationships2[i] = random(1111,9999);
	}

	
	
	//Setup buttons, analog pins (used as digital) 1-4 for buttons 1-4
	
	pinMode(A1,INPUT);
	digitalWrite(A1, HIGH);
	pinMode(A2,INPUT);
	digitalWrite(A2, HIGH);
	pinMode(A3,INPUT);
	digitalWrite(A3, HIGH);
	pinMode(A4,INPUT);
	digitalWrite(A4, HIGH);
	
	
	//Setup RBG LED
	pinMode(redPin, OUTPUT);
	pinMode(greenPin, OUTPUT);
	pinMode(bluePin, OUTPUT);
	
	
	
	//Play Intro
	badgeIntro();


	Serial.println("startup finished");
	showFreeMem();
	delay(1000);
}


void showAbout()
{
   
	if(LoadedScreen == 0)
	{
		loadTopHeader("ABOUT BADGES");
	}
   
   
   //return;
   strcpy_P(currentStr, (char*)pgm_read_word(&(AboutArray[currentAboutItem]))); // Necessary casts and dereferencing, just copy. 
   //char* currentStr = Schedule[currentScheduleItem];
   
   
   int numChars = strlen(currentStr);
   int charsPerRow = 20;
   int numRows = (numChars / charsPerRow) + 1;

  
   
   for(int x=0;x<numRows;x++)
   {
     int rowStart = (x*charsPerRow);
     int rowEnd = rowStart + charsPerRow;
     String thisLine;
     for(int y=rowStart;y<rowEnd;y++)
     {
         int strPos = 0;
         if(y < numChars)
         {
           thisLine += currentStr[y];
           //Serial.println(currentStr[y]);
         }
     }
     
     myGLCD.print(thisLine,0,25+(x*6));
     myGLCD.update();
   }
   
 
   Serial.println(currentStr);
   showFreeMem();
   //myGLCD.update();
   LoadedScreen = 1;
   
   
   int b = readButtons();
   //Serial.println(b);
   if(b == 2)
   {
       LoadedScreen = 0;
       myGLCD.clrScr();
       myGLCD.update();
       if(currentAboutItem == numAboutItems-1)
       {
           currentAboutItem = 0;
       }
       else
       {
         currentAboutItem++;
       }
   }
   if(b == 1)
   {
       LoadedScreen = 0;
       myGLCD.clrScr();
       myGLCD.update();
       if(currentAboutItem == 0)
       {
           currentAboutItem = numAboutItems - 1;
       }
       else
       {
           currentAboutItem--;
       }
   }
   if(b == 4)
   {
       LoadedScreen = 0;
       currentMode = 0;
   }
   //delay(800);

   
   
}

void showSchedule()
{
   
	if(LoadedScreen == 0)
	{
		myGLCD.clrScr();
		loadTopHeader("");
	}
   /*myGLCD.setFont(SmallFont);
   myGLCD.print("SCHEDULE",CENTER,2);
   myGLCD.setFont(TinyFont);
   myGLCD.update();
   */
   //return;
   strcpy_P(currentStr, (char*)pgm_read_word(&(Schedule[currentScheduleItem]))); // Necessary casts and dereferencing, just copy. 
   //char* currentStr = Schedule[currentScheduleItem];
   
   
   int numChars = strlen(currentStr);
   int charsPerRow = 20;
   int numRows = (numChars / charsPerRow) + 1;

  
   
   for(int x=0;x<numRows;x++)
   {
     int rowStart = (x*charsPerRow);
     int rowEnd = rowStart + charsPerRow;
     String thisLine;
     for(int y=rowStart;y<rowEnd;y++)
     {
         int strPos = 0;
         if(y < numChars)
         {
           thisLine += currentStr[y];
           //Serial.println(currentStr[y]);
         }
     }
	 //Serial.println(x);
	 //Serial.println(thisLine);
	 //showFreeMem();

     myGLCD.print(thisLine,0,18+(x*6));
     myGLCD.update();
   }
   
 
   //Serial.println(currentStr);
   
   //myGLCD.update();
   LoadedScreen = 1;
   
   
   int b = readButtons();
   //Serial.println(b);
   if(b == 2)
   {
       LoadedScreen = 0;
       myGLCD.clrScr();
       myGLCD.update();
       if(currentScheduleItem == numScheduleItems-1)
       {
           currentScheduleItem = 0;
       }
       else
       {
         currentScheduleItem++;
       }
   }
   if(b == 1)
   {
       LoadedScreen = 0;
       myGLCD.clrScr();
       myGLCD.update();
       if(currentScheduleItem == 0)
       {
           currentScheduleItem = numScheduleItems - 1;
       }
       else
       {
           currentScheduleItem--;
       }
   }
   if(b == 4)
   {
       LoadedScreen = 0;
       currentMode = 0;
   }
   //delay(800);

   
   
}


void badgeIntro()
{
    myGLCD.clrScr();
    myGLCD.drawBitmap(0,0,zaconlogo,84,48);
    myGLCD.update();
    delay(2000);
    
    screenScroll();
    
    myGLCD.clrScr();
    myGLCD.drawBitmap(0,0,zaconlogo,84,48);
    myGLCD.update();
    myGLCD.invert(true);
    delay(2000);
    myGLCD.invert(false);
    
    myGLCD.clrScr();
    //showLineup();
}

void MenuScreen()
{
    if(LoadedScreen == 0)
    {
	  
      loadTopHeader("www.zacon.org.za");
      MainMenu();
      LoadedScreen = 1;
	  
    }
	
    
    int b = readButtons();
	
    if(b == 1) // Left
    {
       LoadedScreen = 0;
       if(defaultMenu == 0)
        {
          defaultMenu = numMenuItems - 1;
        }
        else
        {
          defaultMenu--;
        }
    }
    else if (b == 2) // Right
    {
        LoadedScreen = 0;
        if(defaultMenu == (numMenuItems - 1))
        {
          defaultMenu = 0;
        }
        else
        {
          defaultMenu++;
        }
    }
    else if (b == 3) // Select
    {

		
		/* 
			MENU ITEMS 
			----------
			0: Schedule
			1: Live Speaker
			2: Stats
			3: About
			4: Intro

			MODES
			----------
			0: Main Menu
			1: Current Schedule
			2: Live Speaker
			3: About
			4: Stats
		*/

        LoadedScreen = 0;
        
        if(defaultMenu == 0) // schedule
        {
			Serial.println("loading schedule");
            currentMode = 1; 
        }
		else if(defaultMenu == 1) //Live Speaker
		{
			//currentMode = 2;
		}
		else if(defaultMenu == 2) //Stats
		{
			currentMode = 4;
		}
		else if(defaultMenu == 3) // About
		{
			currentMode = 3;
		}
		else if(defaultMenu == 4) // intro
        {
          currentMode = 99; //so we dont get both menu and intro trying to run
		  badgeIntro();
          currentMode = 0; // back to menu
        }

        /*if(defaultMenu == 5)
        {
          currentMode = 3; 
          myGLCD.clrScr();
          myGLCD.update();
          defaultMenu = 0;
          showAbout();
        }*/
    }
    else if (b == 4)
    {
      currentMode = 0;
    }
    
}

/*void showAbout()
{
  myGLCD.clrScr();
  myGLCD.drawBitmap(14,0,smallTopLogo,56,16);
  myGLCD.update();
  myGLCD.setFont(TinyFont);
  myGLCD.print("www.zacon.org.za",CENTER,16);
  int b = readButtons();
  Serial.print(F("ABOUT"));
  strcpy_P(currentStr, (char*)pgm_read_word(&(AboutArray[defaultMenu])));
  if(b == 0)
  {
    if(defaultMenu > 0)
    {
    defaultMenu--;
    }
  }
  else if (b == 1)
  {
    if(defaultMenu < 5)
    {
      defaultMenu++;
    }
  }
  else if(b == 3)
   {
       LoadedScreen = 0;
       currentMode = 0;
       MenuScreen();
   }
   else
   {
     Serial.println(F("NOT 3!"));
   }
   myGLCD.print(currentStr,0,10);
   myGLCD.setFont(TinyFont);
   
}*/


void loadTopHeader(char* text)
{
	myGLCD.clrScr();
  myGLCD.drawBitmap(14,0,smallTopLogo,56,16);
  myGLCD.update();
  myGLCD.setFont(TinyFont);
   myGLCD.print(text,CENTER,16);
  myGLCD.update();
}







void showStats()
{
	if(LoadedScreen == 0)
	{
		loadTopHeader("Stats");
		myGLCD.setFont(TinyFont);
	
	String statsString = "Badge Number:";
	statsString += BadgeNumber;
	myGLCD.print(statsString,LEFT,25);
	
	statsString = "Num Badges Seen:";
	statsString += numBadgesSeen;
	myGLCD.print(statsString,LEFT,32);
	myGLCD.update();
	LoadedScreen = 1;
	}
	

	int b = readButtons();
	if(b == 4)
	{
		LoadedScreen = 0;
		currentMode = 0;
		
	}
	
	//myGLCD.print(numBadgesSeen,LEFT,25);
	


}
void MainMenu()
{
  myGLCD.setFont(SmallFont);
  myGLCD.print("MENU",CENTER,25);  
  myGLCD.setFont(TinyFont);
  //myGLCD.print("                                                                                    ",0,35);
  strcpy_P(currentStr, (char*)pgm_read_word(&(MenuArray[defaultMenu]))); // Necessary casts and dereferencing, just copy. 
  myGLCD.print(currentStr,CENTER,35);  
  myGLCD.update();
  myGLCD.setFont(SmallFont);
}

void showCurrentSpeaker()
{
    //Serial.print(SpeakerScrollCurrent);
    if(SpeakerScrollCurrent < -(SpeakerScrollSize*6))
    {
        SpeakerScrollCurrent = 84;
        myGLCD.print("                                                                                    ",0,40);
    }
    else
    {
        SpeakerScrollCurrent = SpeakerScrollCurrent - 1;
    }
    myGLCD.setFont(SmallFont);
    myGLCD.print("                                                                                    ",0,40);
    myGLCD.print(currentSpeaker,SpeakerScrollCurrent,40);
    myGLCD.setFont(SmallFont);
    myGLCD.update();
}

/*
	Clears the screen with a 'scroll' effect
*/
void screenScroll()
{
  myGLCD.clrScr();
    for (int i=0; i<84; i++)
    {
      myGLCD.drawLine(i, 0, i, 48);
      myGLCD.update();
    }
  myGLCD.clrScr();
}
void showLineup()
{
  screenScroll();
  myGLCD.drawBitmap(0,0,zacon5lineup,84,48);
  myGLCD.update();
  delay(2000);
}

/*
int readButtons()
{
    int buttonPressed;
    int c=analogRead(5);
    if(c > 100 && c < 200)
    {
        //Button 1
        buttonPressed = 0;
        Serial.println(F("prev"));
        
    }
    else if (c < 100 && c > 80)
    {
        buttonPressed = 1;
        Serial.println(F("next"));
        //Button 2
    }
    else if (c < 80 && c > 50)
    {
        buttonPressed = 2;
        Serial.println(F("select"));
        //button 3
    }
    else if (c < 50 && c > 0)
    {
        buttonPressed = 3;
        Serial.println(F("other"));
        //button 4
    }
    
   return buttonPressed;
}


*/
void parseCmds(uint8_t* buf,int buflen)
{
	
	//showFreeMem();
  char* entireMessage = (char*)buf;
  char message_mode = entireMessage[0];
  Serial.print("mm:");Serial.print(entireMessage);Serial.println("!");
  
  
  if(message_mode == 'S')
   {
       //Seen a Badge
       //S<NNNN> where NNNN = badge
       
       char SeenBadge[5];
       strncpy(SeenBadge,(entireMessage+1),4);
       SeenBadge[4] = '\0';
       int badgeNum = atoi(SeenBadge);
        
       boolean seenAlready = false;
       for(int i=0;i<numBadgesSeen;i++)
       {
         if(EEPROMReadInt( ((i*2)+2) ) == badgeNum)
         {
           seenAlready = true;
         }
		 
       }
       
       if(seenAlready == false)
       {
			numBadgesSeen = numBadgesSeen + 1;
		 EEPROMWriteInt( ((numBadgesSeen*2)+2), badgeNum );

       }
       
       
       
       seenAlready = false;
       for(int i=0;i<numLastFiveBadges;i++)
       {
           //Serial.print("LB:");Serial.print(i);Serial.print(":");Serial.println(LastFiveBadges[i]);
           if(LastFiveBadges[i] == badgeNum)
           {
             seenAlready = true;
           }
       }
      
       if(seenAlready == false)
       {
           if(numLastFiveBadges == 5)
           {
             numLastFiveBadges = 0;
           }
           Serial.print(F("adding "));Serial.print(badgeNum);Serial.println(F("-"));
           LastFiveBadges[numLastFiveBadges] = badgeNum;
           numLastFiveBadges++;
       }
       
       
       
   }
  else if(message_mode == 'R')
   {
     //Seen a relationship
     //R<NNNN>:<NNNN> - as above
     //Serial.println("This is a Relationship");
     char BadgeOneC[5];
     strncpy(BadgeOneC,(entireMessage+1),4);
     BadgeOneC[4] = '\0';
     int badgeOneI = atoi(BadgeOneC);
     
     char BadgeTwoC[5];
     strncpy(BadgeTwoC,(entireMessage+6),4);
     BadgeTwoC[4] = '\0';
     int badgeTwoI = atoi(BadgeTwoC);
     
     if(badgeOneI == BadgeNumber || badgeTwoI == BadgeNumber)
     {
       //Ignore
     }
     else
     {
       boolean seenAlready = false;
     
       for(int i=0;i<numLastFiveRelationships;i++)
       {
           if(LastFiveRelationships1[i] == badgeOneI && LastFiveRelationships2[i] == badgeTwoI)
           {
				seenAlready = true;
           }
           else if(LastFiveRelationships1[i] == badgeTwoI && LastFiveRelationships2[i] == badgeOneI)
           {
               seenAlready = true;
           }
           
       }
       
       if(seenAlready == false)
       {
           LastFiveRelationships1[numLastFiveRelationships] = badgeOneI;
           LastFiveRelationships2[numLastFiveRelationships] = badgeTwoI;
           numLastFiveRelationships++;
           if(numLastFiveRelationships == 5)
           {
               numLastFiveRelationships = 0;
           }

           //Serial.print("Welcome to Badge");Serial.println(BadgeNumber);
           Serial.print(F("seen a relationship between "));
           Serial.print(badgeTwoI);
           Serial.print(F(" and "));
           Serial.println(badgeOneI);

     
       }
     }
     
     
     
     
   }

}
void showFreeMem()
{
    Serial.print("freeMemory()=");
    Serial.println(freeMemory());
}
//new button reader, saves resistors and i have the pins anyway (yes i know resistors are nearly free anyway!)
int readButtons()
{
	int b1 = digitalRead(A1);
	int b2 = digitalRead(A2);
	int b3 = digitalRead(A3);
	int b4 = digitalRead(A4);
	
	int pressedState = LOW;
	

	long currentMillis = millis();
	//Serial.println(currentMillis - lastButtonCheck);
	if(currentMillis - lastButtonCheck > debounceDelay)
	{
		//Serial.println("SOT");
		if(b1 == pressedState)
		{
			
			Serial.println("B1 Pushed");
			return 1;
		}
		if(b2 == pressedState)
		{
			Serial.println("B2 Pushed");
			return 2;
			
		}
		if(b3 == pressedState)
		{
			Serial.println("B3 Pushed");
			return 3;
			
		}
		if(b4 == pressedState)
		{
			Serial.println("B4 Pushed");
			return 4;
			
		}
		//Serial.print("1:");Serial.print(b1);Serial.print("2:");Serial.println(b2);
		//Serial.println("EOT");
		lastButtonCheck = millis();
		
	}

	return -1;
	
	
	//delay(100);

}

/*
LED Colours
------------
RED:		255,0,0
GREEN:		0,255,0
BLUE:		0,0,255
YELLOW:		255,255,0
WHITE:		255,255,255
ORANGE:		237,120,6

*/
void setLED(int red,int green,int blue)
{
	analogWrite(redPin,red);
	analogWrite(bluePin,blue);
	analogWrite(greenPin,green);
}

void LED0()
{
	setLED(0,0,0);
}

void LED1()
{
	setLED(255,0,0);
}

void LED2()
{
	setLED(0,255,0);
}

void LED3()
{
	setLED(0,0,255);
}

void LED4()
{
	setLED(255,255,0);
}

void LED5()
{
	setLED(255,255,255);
}



void loop()
{  
  
  
  /* 
	LCD/Mode Loop 
	-------------

	Modes:
	0: Main Menu
	1: Current Schedule
	2: Live Speaker
	3: About
	4: Stats
	INTRO EXCLUDED -- doesnt need to loop into it	
  
  */

  if(currentMode == 0)
  {
    MenuScreen();
  }
  else if(currentMode == 1)
  {
      showSchedule();
  }
  else if(currentMode == 2)
  {
	//showLiveSpeaker();
  }
  else if(currentMode == 3)
  {
	  showAbout();
  }
  else if(currentMode == 4)
  {
	  showStats();
  }
  

  /* RF Loop */

  
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  unsigned long currentMillis = millis();
 
 
if(currentMillis - previousMillis > randInterval) 
  {
    LED3();
	previousMillis = currentMillis;  
    randInterval = random(350,2000);
    
    sprintf(currentRFStr,"S%i",BadgeNumber);
	
    vw_send((uint8_t *)currentRFStr, 6);
    vw_wait_tx(); // Wait until the whole message is gone
    //Serial.print(F("Sent: ")); Serial.print(currentRFStr); Serial.println(F("!"));
    //LED0();
    
	//LED4();
    
	//sprintf(currentRFStr,"R%i:%i",BadgeNumber,LastFiveBadges[0]);
	//vw_send((uint8_t *)currentRFStr, 11);

	/*
	
	So ideally if it was a bit quicker id love to send all 5 last badges ive seen 
	as well as the last 5 relationships i've seen, but its not gonna happen unless
	you dont want the menu system to work. so we are gonna just random it.
	*/

	if(numLastFiveRelationships > 0)
	{
		int r_ID = random(0,numLastFiveRelationships);
		sprintf(currentRFStr,"R%i:%i",LastFiveRelationships1[r_ID],LastFiveRelationships2[r_ID]);
        vw_send((uint8_t *)currentRFStr, 11);
        vw_wait_tx(); // Wait until the whole message is gone
	}

	if(numLastFiveBadges > 0)
	{
		int r_ID = random(0,numLastFiveBadges);
		sprintf(currentRFStr,"R%i:%i",BadgeNumber,LastFiveBadges[r_ID]);
        vw_send((uint8_t *)currentRFStr, 11);
        vw_wait_tx(); // Wait until the whole message is gone
	}
/*
	for(int i=0;i<1;i++)
    {
		
      if(LastFiveBadges[i] > 0)
      {
	    sprintf(currentRFStr,"R%i:%i",BadgeNumber,LastFiveBadges[i]);
		vw_send((uint8_t *)currentRFStr, 11);
        //char msgC[100];
        //sprintf(currentRFStr,"R%i:%i",BadgeNumber,LastFiveBadges[i]);
		//sprintf(currentRFStr,"R%i",LastFiveBadges[i]);
        //vw_send((uint8_t *)currentRFStr, 11);
        vw_wait_tx(); // Wait until the whole message is gone
        
      }
	}
	//LED3();
	for(int i=0;i<5;i++)
    {
      if(LastFiveRelationships1[i] > 0)
      {
        //char msgC[100];
        //sprintf(currentRFStr,"R%i:%i",LastFiveRelationships1[i],LastFiveRelationships2[i]);
        //vw_send((uint8_t *)currentRFStr, 11);
        //vw_wait_tx(); // Wait until the whole message is gone
        
      }
    }

*/
	//Serial.println(millis());
	LED0();
    

  }
  
  
  
  
  if (vw_get_message(buf, &buflen)) // Non-blocking
  {
    
   
    LED2();
    parseCmds(buf,buflen);
    LED0();
    //digitalWrite(13, false);
  }
  
  
  

  
}

