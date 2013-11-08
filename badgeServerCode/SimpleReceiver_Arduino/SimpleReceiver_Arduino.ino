#include <VirtualWire.h>
void setup()
{
    Serial.begin(19200);

vw_set_tx_pin(7);
	vw_set_rx_pin(6);
	vw_set_ptt_inverted(true); // Required for DR3100
	vw_setup(2000);	 // Bits per sec
	vw_rx_start();       // Start the receiver PLL runningg
}

void loop()
{
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
   if (vw_get_message(buf, &buflen)) // Non-blocking
  {
  char* entireMessage = (char*)buf;
  char message_mode = entireMessage[0];
  Serial.println(entireMessage);
  }
}
