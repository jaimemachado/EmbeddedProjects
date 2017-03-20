




#include <Time.h>
#include "NodeConfig.h"
#include <MySensors.h>
#include "TimeScheduler.h"

#define CHILD_ID_DUST 0
#define DUST_SENSOR_ANALOG_PIN 1

unsigned long SLEEP_TIME = 1*1000; // Sleep time between reads (in milliseconds)
//VARIABLES
int val = 0;           // variable to store the value coming from the sensor
float valDUST =0.0;
float lastDUST =0.0;
int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;
float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;


MyMessage dustMsg(CHILD_ID_DUST, V_LEVEL);


//TimeSchedule Library
TimeScheduler timesch;
void receiveTime(unsigned long time)
{
	unsigned long tTime = time;
	Serial.print("Time Update Received: ");
	Serial.println(tTime);
	timesch.setTime_(tTime);
}
void timeUpdate()
{
	requestTime(receiveTime);
}

void setup()
{
	Serial.println("Setup");
	timeUpdate();
//	setSyncProvider(timeUpdate);  //set function to call when sync required

	timesch.init(timeUpdate);
}


void presentation()
{
	Serial.println("Presentation");
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo("Inteligent Switch", "1.1");

	// Register all sensors to gateway (they will be created as child devices)
	present(CHILD_ID_DUST, S_DUST);


}

bool executed = false;;
void loop()
{
	//	timesch.handleTimeUpdates();


	//if(timesch.timeInited())
	{
//		if(((hour() == 18) &&
			//(minute() == 28)) && !executed)
		//{
			//Serial.print("Time: ");
		//}
		//Execute code that depends on time
		uint16_t voMeasured = analogRead(DUST_SENSOR_ANALOG_PIN);// Get DUST value

																 // 0 - 5V mapped to 0 - 1023 integer values
																 // recover voltage
		calcVoltage = voMeasured * (1.85 / 1024.0);

		// linear eqaution taken from http://www.howmuchsnow.com/arduino/airquality/
		// Chris Nafis (c) 2012
		dustDensity = (0.17 * calcVoltage - 0.1) * 1000;
		dustDensity = calcVoltage * 100;

		Serial.print("Raw Signal Value (0-1023): ");
		Serial.print(voMeasured);

		Serial.print(" - Voltage: ");
		Serial.print(calcVoltage);

		Serial.print(" - Dust Density: ");
		Serial.println(dustDensity); // unit: ug/m3

		if (ceil(dustDensity) != lastDUST) {
			send(dustMsg.set((int16_t)ceil(dustDensity)));
			lastDUST = ceil(dustDensity);
		}
		timesch.printTime();

		sleep(SLEEP_TIME);
	}
	//sleep(500);
}

