#include <VirtualWire.h>
void setup (){
  Serial.begin(19200);
  Serial.flush();
  digitalWrite (13, HIGH);      //turn on debugging LED
 vw_set_ptt_inverted(true); // Required for DR3100
vw_setup(2000);	 // Bits per sec
vw_set_tx_pin(7);
vw_set_rx_pin(6);

	pinMode(5, OUTPUT);
	pinMode(4, OUTPUT);
	pinMode(3, OUTPUT);
/*
LED_ONE();
delay(200);
LED_TWO();
delay(200);
LED_THREE();
delay(200);
LED_OFF();
*/
vw_rx_start();
}

void LED_ONE()
{
  analogWrite(5,255);
  analogWrite(4,0);
    analogWrite(3,0);
}

void LED_TWO()
{
    analogWrite(4,255);
  analogWrite(5,0);
    analogWrite(3,0);
}
void LED_THREE()
{
    analogWrite(3,255);
  analogWrite(5,0);
    analogWrite(4,0);
}
void LED_OFF()
{
  
  analogWrite(5,0);
  analogWrite(4,0);
  analogWrite(3,0);
}


void loop ()
{
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  if (vw_get_message(buf, &buflen)) // Non-blocking
  {
	/* LED EATS POWER! STOP USING IT FOR ANY VALID SIGNAL! */
   char* entireMessage = (char*)buf;
  LED_ONE();
  Serial.print("message:");Serial.println(entireMessage);
  LED_OFF();
  }
  
   if(Serial.available()){
   }

  int i=0;
  char commandbuffer[100];

  if(Serial.available()){
     delay(100);
     while( Serial.available() && i< 99) {
        commandbuffer[i++] = Serial.read();
     }
     commandbuffer[i++]='\0';
  }
  

  if(i>0)
  {
    Serial.print("Sending:"); 
    Serial.print(strlen(commandbuffer));
    Serial.println((char*)commandbuffer);
     digitalWrite(13, true); // Flash a light to show transmitting
    LED_TWO();
    vw_send((uint8_t *)commandbuffer, strlen(commandbuffer));
    vw_wait_tx(); // Wait until the whole message is gone
    digitalWrite(13, false);
    LED_OFF();

    
  }
  

}
