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


//U8G2_ST7920_128X64_F_SW_SPI  u8g2(U8G2_R0, /* clock=*/ PA0, /* data=*/ PA1, /* CS=*/ PA2, /* reset=*/ PA3);


void presentation()
{
    fsafsd
ddaasdas    sfdasdfsaa = 2;
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo("Ar Con Controller Central", "1.0");

	// Register all sensors to gateway (they will be created as child devices)
	present(CHILD_TEMP_HEAT_PUMP, S_TEMP);
}



// the setup function runs once when you press reset or power the board
void setup() {
    // initialize digital pin PB1 as an output.
  //  pinMode(PC13 , OUTPUT);
//    pinMode(PB12 , OUTPUT);
  //  u8g2.begin();
  }
  
  // the loop function runs over and over again forever
  void loop() {
    //digitalWrite(PC13 , HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(100);              // wait for a second
    //digitalWrite(PC13 , LOW);    // turn the LED off by making the voltage LOW
    delay(100);              // wait for a second

    //digitalWrite(PB12 , HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(10);              // wait for a second
  //  digitalWrite(PB12 , LOW);    // turn the LED off by making the voltage LOW
    delay(10);     


    //u8g2.firstPage();
  //  do {
   //   u8g2.setFont(u8g2_font_ncenB14_tr);
//u8g2.drawStr(0,24,"Hello World!");
  //  } while ( u8g2.nextPage() );
  }
