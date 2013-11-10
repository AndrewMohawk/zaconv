#include <VirtualWire.h>
void setup (){
  Serial.begin(19200);
  Serial.flush();
  digitalWrite (13, HIGH);      //turn on debugging LED
   vw_set_ptt_inverted(true); // Required for DR3100
    vw_setup(2000);	 // Bits per sec
    vw_set_tx_pin(7);
}

void loop (){

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
    
    vw_send((uint8_t *)commandbuffer, strlen(commandbuffer));
    vw_wait_tx(); // Wait until the whole message is gone
    digitalWrite(13, false);
    delay(200);
  }

}
