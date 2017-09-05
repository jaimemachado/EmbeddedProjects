#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "heatpumpUI.h"

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

#define MY_DEBUG

#include <MySensors.h>


// Workaround for STM32 support
#define ADC_CR2_TSVREFE  (1 << 23) // from libopencm3
#define digitalPinToInterrupt(x) (x)
#define snprintf_P snprintf
#define vsnprintf_P vsnprintf

// Enable debug prints to serial monitor





U8G2_ST7920_128X64_F_SW_SPI  u8g2(U8G2_R0, /* clock=*/ 4, /* data=*/ 5, /* CS=*/ 7, /* reset=*/ 8);
heatpumpUI UI;
HeatPumpConfig curConfig; 

void setup() {
  u8g2.begin();
// initialize pins PC13, as an output.
  pinMode(LED_BUILTIN, OUTPUT);		//This set PC13 port, written in the board, in Output
  Serial.begin(115200);		//This initialize serial at 9600 baud
  Serial.println("Hello world this is Serial");
  UI.begin(&u8g2);
}


void presentation()
{
	// Send the sketch version information to the gateway and Controller
	//sendSketchInfo("Air Quality Sensor", "1.0");

	// Register all sensors to gateway (they will be created as child devices)
	//present(CHILD_ID_MQ, S_AIR_QUALITY);
}


// the loop function runs over and over again forever
void loop() {
 
  UI.run();

  HeatPumpConfig config = UI.getConfig();

  if( !(curConfig == config))
  {
    curConfig = config;
    Serial.println("New Config");

  }
  
  //display->DrawMainScreen();

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.drawStr(0,24,"Hello World!");
  } while ( u8g2.nextPage() );
}