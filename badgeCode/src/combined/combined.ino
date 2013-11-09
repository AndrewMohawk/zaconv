#include <Bounce.h>
#include <VirtualWire.h>
#include <MemoryFree.h>
#include <LCD5110_Graph.h>
#include <EEPROM.h>
#include "EepromUtil.h"

/* Buttons */

Bounce b1 = Bounce(A4, 10);
Bounce b2 = Bounce(A3, 10);
Bounce b3 = Bounce(A2, 10);
Bounce b4 = Bounce(A1, 10);

//*THIS* badgenumber
int BadgeNumber = 3030;

//Send Rand Interval things
long previousMillis = 0;
long randInterval = 1500;

// = "RC1140|Dev|runawaycoder"
char badgeNick1[15];// = "RC1140";
char badgeNick2[15];// = "Dev";
char badgeNick3[15];// = "runawaycoder";
//All badges seen
//int BadgesIveSeen[100];
//LOL @ trying to use RAM for that.. 2K >_< *facepalm*.. all badges now stored in EEPROM, 0 = this badge, 1 = badge seen 1.

/*
	EEPROM is valid for 100,000 writes, we shouldnt go over that hopefully..

	Structure as of 2013/09/21 is:

	0-1: 28 if seen before, else random.
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
LCD5110 myGLCD(8,9,10,12,11);

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
prog_char about1[] PROGMEM = "Badges designed for ZaCon V";
prog_char about2[] PROGMEM = "Special thanks to everyone who submitted and made it great";
prog_char about3[] PROGMEM = "Keep these badges as tokens of our appreciation";
prog_char about4[] PROGMEM = "From AndrewMohawk and people@zacon.org.za";

PROGMEM const char *AboutArray[] =
{
    about1,
    about2,
    about3,
    about4
};

int numAboutItems = 4;

//Main Menu items
prog_char menu0[] PROGMEM = "Schedule";
prog_char menu1[] PROGMEM = "Live Speaker Update";
prog_char menu2[] PROGMEM = "Stats";
prog_char menu3[] PROGMEM = "About";
prog_char menu4[] PROGMEM = "Intro";
prog_char menu5[] PROGMEM = "WHOAMI";


PROGMEM const char *MenuArray[] =
{
	menu0,
	menu1,
	menu2,
	menu3,
	menu4,
	menu5,
};
//number of menu items we have
int numMenuItems = 6;

//Speaker information
prog_char string_0[] PROGMEM = "08h00-0900: Coffee and Registration";   // "String 0" etc are strings to store - change to suit.
prog_char string_1[] PROGMEM = "09h00-09h15: Dominic White - Welcome to ZaCon V";
prog_char string_2[] PROGMEM = "09h15-09h45: Mark Cosijn - Vehicle CAN-fu";
prog_char string_3[] PROGMEM = "09h45-10h10: Jason 's0nic2k' Mitchell - Mains Signalling";
prog_char string_4[] PROGMEM = "10h10-10h25: TBA";
prog_char string_5[] PROGMEM = "10h25-10h40: Tea Break";
prog_char string_6[] PROGMEM = "10h40-11h25: Jeremy du Bruyn - RAT-a-tat-tat: Taking the fight to RAT controllers";
prog_char string_7[] PROGMEM = "11h25-12h05: Marcos Alvares - Automating Detection of Obfuscated Obfuscation...";
prog_char string_8[] PROGMEM = "12h05-12h50: Adam Schoeman - Amber: A Zero-Interaction Honeypot and Network...";
prog_char string_9[] PROGMEM = "12h50-14h00: Lunch Tiemz";
prog_char string_10[] PROGMEM = "14h00-14h30: Dimitri Foesekis - Markov Chains and Other Statistical Password Attacks...";
prog_char string_11[] PROGMEM = "14h30-15h30: Dave Hartley - Native Bridges over Troubled Water";
prog_char string_12[] PROGMEM = "15h30-16h00: Robert Gabriel - GSOC Data Diode";
prog_char string_13[] PROGMEM = "16h00-16h15: Tea Break";
prog_char string_14[] PROGMEM = "16h15-17h05: Roelof Temmingh - To Infini-Tea and Beyond";
prog_char string_15[] PROGMEM = "17h05-17h30: Schalk Heunis - Long tail of wifi antenna design";
prog_char string_16[] PROGMEM = "17h30-17h45: Dominic White - Close/Awards";

int currentLiveSpeaker = 0;

PROGMEM const char *Schedule[] =
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
  string_15,
  string_16,
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
    //Always init to 0 so that we can check later if a sync has been completed
	badgeNick1[0] = 0;
	badgeNick2[0] = 0;
	badgeNick3[0] = 0;

	//Setup RBG LED
	pinMode(redPin, OUTPUT);
	pinMode(greenPin, OUTPUT);
	pinMode(bluePin, OUTPUT);


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
	vw_set_tx_pin(7);
	vw_set_rx_pin(6);
	vw_set_ptt_inverted(true); // Required for DR3100
	vw_setup(2000);	 // Bits per sec
	vw_rx_start();       // Start the receiver PLL running




	/* EEPROM Config */
	char lowByte = (char)EEPROM.read(200);
	//If we have a nick Char set then try and load anything at address 201
	if ((char)lowByte == 'N')
	{
		EepromUtil::eeprom_read_string(201, badgeNick1, 15);
		EepromUtil::eeprom_read_string(216, badgeNick2, 15);
		EepromUtil::eeprom_read_string(231, badgeNick3, 15);
	}
	//28 if the badge has been through a power cycle, else its first boot so we set it to 0
	int usedBefore = EEPROMReadInt(0);
	if(usedBefore == 28)
	{
		numBadgesSeen = EEPROMReadInt(2);
	}
	else
	{
        EEPROMWriteInt(0,28); // write badge
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


	//Play Intro
	if(usedBefore == 28)
	{
		//another intro?
		drawZClogo();
		myGLCD.invert(false);
		myGLCD.clrScr();
	}
	else
	{
		badgeIntro();
	}


}


void showAbout()
{

	if(LoadedScreen == 0)
	{
		loadTopHeader("ABOUT BADGES");
	}

   strcpy_P(currentStr, (char*)pgm_read_word(&(AboutArray[currentAboutItem]))); // Necessary casts and dereferencing, just copy.



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
         }
     }

     myGLCD.print(thisLine,0,25+(x*6));
     myGLCD.update();
   }

   LoadedScreen = 1;


   int b = readButtons();

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
   processAButton(b);
}
void showLiveSpeaker()
{
	if(LoadedScreen == 0)
	{
		myGLCD.clrScr();
		loadTopHeader("");

	strcpy_P(currentStr, (char*)pgm_read_word(&(Schedule[currentLiveSpeaker]))); // Necessary casts and dereferencing, just copy.

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
         }
     }

     myGLCD.print(thisLine,0,18+(x*6));
     myGLCD.update();
   }
}


   LoadedScreen = 1;
   int b = readButtons();
   if(b == 4)
   {
       LoadedScreen = 0;
       currentMode = 0;
   }
}
void showSchedule()
{

	if(LoadedScreen == 0)
	{
		myGLCD.clrScr();
		loadTopHeader("");
	}
   strcpy_P(currentStr, (char*)pgm_read_word(&(Schedule[currentScheduleItem]))); // Necessary casts and dereferencing, just copy.

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
         }
     }

     myGLCD.print(thisLine,0,18+(x*6));
     myGLCD.update();
   }


   LoadedScreen = 1;


   int b = readButtons();

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




}

void drawZClogo()
{
    myGLCD.clrScr();
    myGLCD.drawBitmap(0,0,zaconlogo,84,48);
    myGLCD.update();
    delay(2000);
}

void badgeIntro()
{
    drawZClogo();
    screenScroll();


    drawZClogo();
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
    switch(b){
        case 1:
            LoadedScreen = 0;
            if(defaultMenu == 0)
            {
                defaultMenu = numMenuItems - 1;
            }
            else
            {
                defaultMenu--;
            }
            break;
        case 2:
            LoadedScreen = 0;
            if(defaultMenu == (numMenuItems - 1))
            {
                defaultMenu = 0;
            }
            else
            {
                defaultMenu++;
            }

            break;
        case 3:

            /*
               MENU ITEMS
               ----------
                0: Schedule
                1: Live Speaker
                2: Stats
                3: About
                4: Intro
				5: WHOAMI

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
                currentMode = 2;
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
            else if(defaultMenu == 5) // WHOAMI
			{
                currentMode = 5;
			}
            break;
        case 4:
            currentMode = 0;
			int x = -1;
			randomSeed(analogRead(0));
			if(BadgeNumber > 2000) // Attendees
			{
				x = 5;
			}
			if (BadgeNumber > 3000) // Speakers
			{
				x = random(2,5);
			}
			if (BadgeNumber > 4000) // Ubers
			{
				x= random(1,5);
			}

			if(x == 1)
			{
				vw_send((uint8_t*)"C1111", 5);
				vw_wait_tx();
			}
			if(x == 2)
			{
				vw_send((uint8_t*)"C2222", 5);
				vw_wait_tx();
			}
			if(x == 3)
			{
				vw_send((uint8_t*)"C3333", 5);
				vw_wait_tx();
			}
			if(x == 4)
			{
				vw_send((uint8_t*)"C4444", 5);
				vw_wait_tx();
			}
			if(x == 5)
			{
				vw_send((uint8_t*)"C5555", 5);
				vw_wait_tx();
			}
            break;
    }
}


char** str_split(char* a_str, const char a_delim)
{
	char** result = 0;
	size_t count = 0;
	char* tmp = a_str;
	char* last_comma = 0;

	/* Count how many elements will be extracted. */
	while (*tmp)
	{
		if (a_delim == *tmp)
		{
			count++;
			last_comma = tmp;
		}
		tmp++;
	}

	/* Add space for trailing token. */
	count += last_comma < (a_str + strlen(a_str) - 1);

	/* Add space for terminating null string so caller
	*        knows where the list of returned strings ends. */
	count++;

	result = (char**)malloc(sizeof(char*)* count);

	if (result)
	{
		size_t idx = 0;
		char* token = strtok(a_str, "|");

		while (token)
		{
			*(result + idx++) = strdup(token);
			token = strtok(0, "|");
		}
		*(result + idx) = 0;
	}

	return result;
}

//"RC1140|Dev|Test.co.za";
void showWHOAMI()
{
  if(LoadedScreen == 0 )
  {

     if(strlen(badgeNick1) > 0)
     {
       myGLCD.clrScr();
	   loadTopHeader("WHOAMI");
       myGLCD.setFont(SmallFont);

	   myGLCD.print(badgeNick1, CENTER, 24);
	   myGLCD.print(badgeNick2, CENTER, 32);
	   myGLCD.print(badgeNick3, CENTER, 40);
       myGLCD.update();
     }
     else
     {
       myGLCD.clrScr();
	   loadTopHeader("WHOAMI");
       myGLCD.setFont(TinyFont);
       myGLCD.print("No Handle Set",CENTER,25);
       myGLCD.update();
     }
     LoadedScreen = 1;
  }
  processButtons();
}

void loadTopHeader(char* text)
{
	myGLCD.clrScr();
    myGLCD.drawBitmap(14,0,smallTopLogo,56,16);
    myGLCD.update();
    myGLCD.setFont(TinyFont);
    myGLCD.print(text,CENTER,16);
    myGLCD.update();
}

void exitToMainMenu()
{
    LoadedScreen = 0;
    currentMode = 0;
}

void processButtons()
{
    processAButton(readButtons());

}

void processAButton(int buttonID)
{
    //Calculate the switchid by adding the button and current mode
    //then xoring the mode out , this allows mulitple screens
    //to share the same function code without insanely nested if/elses.
    /*
	LCD/Mode Loop
	-------------

	Modes:
	0: Main Menu
	1: Current Schedule
	2: Live Speaker
	3: About
	4: Stats
	5: WHOAMI
	INTRO EXCLUDED -- doesnt need to loop into it
    */
    switch(buttonID)
    {
        case 1://Save handle on the WHOAMI screen
			switch (currentMode)
			{
				case 5:
                    vw_send((uint8_t*)"C3333", 5);
                    vw_wait_tx();
					procesHandleSave();
					break;
			}
            break;
        case 2:
            switch (currentMode)
			{
				case 5:
                    vw_send((uint8_t*)"C4444", 5);
                    vw_wait_tx();
					break;
			}
            break;
        case 3:
            switch (currentMode)
			{
				case 5:
                    vw_send((uint8_t*)"C5555", 5);
                    vw_wait_tx();
			}
            break;
        case 4://General navigate to main menu function
            exitToMainMenu();
            break;
    }
}

void showSavedMessage()
{
	myGLCD.clrScr();
	myGLCD.setFont(TinyFont);
	myGLCD.print("Nick Saved To EEPROM", CENTER, 25);
	myGLCD.update();
}

void procesHandleSave()
{
	EepromUtil::eeprom_write_string(200, "N");
	EepromUtil::eeprom_write_string(201, badgeNick1);
	EepromUtil::eeprom_write_string(216, badgeNick2);
	EepromUtil::eeprom_write_string(231, badgeNick3);
	showSavedMessage();
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
    processButtons();
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



void handleLiveSpeaker(char * entireMessage)
{
	char LiveSpeaker[3];
	strncpy(LiveSpeaker,(entireMessage+1),2);
	LiveSpeaker[2] = '\0';
	int speakerNum = atoi(LiveSpeaker);
	currentLiveSpeaker = speakerNum;
	LED_GREEN();
	delay(200);
	LED_OFF();
	LoadedScreen = 0;


}
void handleBadgeResetMode(char * entireMessage)
{
	char verify[5];
	strncpy(verify,(entireMessage+1),4);
	verify[4] = '\0';
	int ageVerify = atoi(verify);
	if(ageVerify == 1985)
	{
		EEPROMWriteInt(0,26); // write badge
        EEPROMWriteInt(2,0); // write num seen
		numBadgesSeen = 0;
		LED_RED();
		delay(200);
		LED_OFF();
	}
}
void handleCoolBadgeMode(char * entireMessage)
{
	char badgeMode[5];
	strncpy(badgeMode,(entireMessage+1),4);
	badgeMode[4] = '\0';
	int coolmode = atoi(badgeMode);
	Serial.print("coolmode");Serial.println(coolmode);
	switch (coolmode)
	{
		case 1111:
			/* ITS THE 5-0! */
			for(int x=0;x<3;x++)
			{
			LED_RED();
			delay(200);
			LED_BLUE();
			delay(200);
			LED_WHITE();
			delay(200);
			LED_RED();
			delay(200);
			LED_BLUE();
			delay(200);
			LED_WHITE();
			delay(200);
			LED_RED();
			delay(200);
			LED_BLUE();
			delay(200);
			LED_WHITE();
			delay(200);
			LED_OFF();
			}
			break;
		case 2222:
			/* OUTOFIDASNOW */
			LED_BLUE();
			delay(200);
			LED_GREEN();
			delay(200);
			LED_PURPLE();
			delay(200);
			LED_RED();
			delay(200);
			LED_ORANGE();
			delay(200);
			LED_WHITE();
			delay(200);
			break;
		case 3333:
			/* WHAT ABOOUTBLUE */
			for(int i=0;i<255;i++)
			{
				setLED(0,0,i);
				delay(5);
			}
			for(int i=255;i>0;i--)
			{
				setLED(0,0,i);
				delay(5);
			}
			break;

		case 4444:
			/* WHAT ABOOUTBLUE */
			for(int i=0;i<255;i++)
			{
				setLED(i,0,0);
				delay(5);
			}
			for(int i=255;i>0;i--)
			{
				setLED(i,0,0);
				delay(5);
			}
			break;
		case 5555:
			/* WHAT ABOOUTBLUE */
			for(int i=0;i<255;i++)
			{
				setLED(0,i,0);
				delay(5);
			}
			for(int i=255;i>0;i--)
			{
				setLED(0,i,0);
				delay(5);
			}
			break;
	}
}

void handleSendMode(char * entireMessage)
{
	//Seen a Badge
       //S<NNNN> where NNNN = badge

       char SeenBadge[5];
       strncpy(SeenBadge,(entireMessage+1),4);
       SeenBadge[4] = '\0';
       int badgeNum = atoi(SeenBadge);

       boolean seenAlready = false;

       for(int i=0;i<=numBadgesSeen;i++)
       {
		 int EEPROMBadge = EEPROMReadInt( ((i*2)+2) );
         if(EEPROMBadge == badgeNum)
         {
           seenAlready = true;
         }

       }

       if(seenAlready == false)
       {
			numBadgesSeen = numBadgesSeen + 1;
			EEPROMWriteInt( ((numBadgesSeen*2)+2), badgeNum );
			EEPROMWriteInt(2,numBadgesSeen);
			LED_BLUE();
			delay(200);
			LED_OFF();

       }



       seenAlready = false;
       for(int i=0;i<numLastFiveBadges;i++)
       {
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
           LastFiveBadges[numLastFiveBadges] = badgeNum;
           numLastFiveBadges++;
       }



}

void handleReceiveMode(char * entireMessage)
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
       }
     }
}


void handleNickUpdateMode(uint8_t* buf,int buflen)
{
		sprintf(badgeNick1, "%.12s", buf+1);
		sprintf(badgeNick2, "%.12s", buf+16);
		sprintf(badgeNick3, "%.12s", buf+32);
    	LED_GREEN();
		delay(200);
		LED_OFF();
		LoadedScreen = 0;
}

void parseCmds(uint8_t* buf,int buflen)
{
  char* entireMessage = (char*)buf;
  char message_mode = entireMessage[0];
  Serial.print("message:");Serial.println(entireMessage);
  switch (message_mode)
  {
	  case 'S':
		  handleSendMode(entireMessage);
		  break;
	  case 'L':
		  handleLiveSpeaker(entireMessage);
		  break;
	  case 'R':
	      handleReceiveMode(entireMessage);
		  break;
	  case 'U':
	      handleNickUpdateMode(buf,buflen);
		  break;
	  case 'A':
		  handleBadgeResetMode(entireMessage);
		  break;
	  case 'C':
		  handleCoolBadgeMode(entireMessage);
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
	if (b1.update() && b1.fallingEdge() )
	{
		return 1;
	}
	if (b2.update() && b2.fallingEdge() )
	{
		return 2;
	}
	if (b3.update() && b3.fallingEdge() )
	{
		return 3;
	}
	if (b4.update() && b4.fallingEdge() )
	{
		return 4;
	}
	return -1;
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

void LED_OFF()
{
	setLED(0,0,0);
}

void LED_RED()
{
	setLED(255,0,0);
}

void LED_GREEN()
{
	setLED(0,255,0);
}

void LED_BLUE()
{
	setLED(0,0,255);
}

void LED_PURPLE()
{
	setLED(255,255,0);
}

void LED_WHITE()
{
	setLED(255,255,255);
}

void LED_ORANGE()
{
	setLED(237,120,6);
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
	5: WHOAMI
	INTRO EXCLUDED -- doesnt need to loop into it

  */

  switch(currentMode) {
    case 0:
        MenuScreen();
        break;
    case 1:
        showSchedule();
        break;
    case 2:
        showLiveSpeaker();
        break;
    case 3:
        showAbout();
        break;
    case 4:
		showStats();
        break;
	case 5:
		showWHOAMI();
        break;
  }

  /* RF Loop */


  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  unsigned long currentMillis = millis();


  if(currentMillis - previousMillis > randInterval)
  {
	previousMillis = currentMillis;
    randInterval = random(1000,4000);

    sprintf(currentRFStr,"S%i",BadgeNumber);

    vw_send((uint8_t *)currentRFStr, 6);
    vw_wait_tx(); // Wait until the whole message is gone
    LED_OFF();

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
  }

  if (vw_get_message(buf, &buflen)) // Non-blocking
  {
	/* LED EATS POWER! STOP USING IT FOR ANY VALID SIGNAL! */
    parseCmds(buf,buflen);
  }
}

