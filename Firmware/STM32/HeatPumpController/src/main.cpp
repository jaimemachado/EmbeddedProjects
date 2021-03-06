#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

#define MY_DEBUG

#include <MySensors.h>

#define CHILD_TEMP_HEAT_PUMP 1


#define PIN_CONTRAST_LCD PB7


U8G2_ST7920_128X64_F_SW_SPI  u8g2(U8G2_R0, /* clock=*/ PA0, /* data=*/ PA1, /* CS=*/ PA2, /* reset=*/ PA3);




void presentation()
{
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo("Ar Con Controller Central", "1.0");

	// Register all sensors to gateway (they will be created as child devices)
	present(CHILD_TEMP_HEAT_PUMP, S_TEMP);
}


uint8_t contrast = 0;

// the setup function runs once when you press reset or power the board
void setup() {
    // initialize digital pin PB1 as an output.
    pinMode(PC13 , OUTPUT);
    pinMode(PB12 , OUTPUT);
    pinMode(PIN_CONTRAST_LCD, OUTPUT);
    u8g2.begin();
  }
  
  // the loop function runs over and over again forever
  void loop() {
    digitalWrite(PC13 , HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(100);              // wait for a second
    digitalWrite(PC13 , LOW);    // turn the LED off by making the voltage LOW
    delay(100);              // wait for a second

    digitalWrite(PB12 , HIGH);   // turn the LED on (HIGH is the voltage level)


    analogWrite(PIN_CONTRAST_LCD, contrast);
    //digitalWrite(PIN_CONTRAST_LCD , HIGH);   // turn the LED on (HIGH is the voltage level)
    //pwmWrite(PIN_CONTRAST_LCD, 65535);

    contrast +=10;

    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_ncenB14_tr);
      u8g2.drawStr(0,24,"Hello World!");
    } while ( u8g2.nextPage() );
  }