#include "NodeConfig.h"
// Enable debug prints
#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

#include <MySensors.h>

void presentation()
{
	// Send the sketch version information to the gateway and Controller
	//sendSketchInfo("Dust Sensor", "1.1");

	// Register all sensors to gateway (they will be created as child devices)
	//present(CHILD_ID_DUST, S_DUST);
}

void loop()
{
	sleep(100);
}
