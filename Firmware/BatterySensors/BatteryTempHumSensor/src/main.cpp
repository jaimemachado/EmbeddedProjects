
// Enable debug prints to serial monitor
//#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

#include <DHT.h>
#include <MySensors.h>

#define CHILD_ID_TEMP 0
#define CHILD_ID_HUMI 0

#define DHTPIN 8 // what digital pin we're connected to
// Uncomment whatever type you're using!
#define DHTTYPE DHT11 // DHT 11

int BATTERY_SENSE_PIN = A6; // select the input pin for the battery sense point

#ifdef MY_DEBUG
unsigned long SLEEP_TIME = 3000; // sleep time between reads (seconds * 1000 milliseconds)
#else
unsigned long SLEEP_TIME = 900000; // sleep time between reads (seconds * 1000 milliseconds)
#endif
int oldBatteryPcnt = 0;

DHT dht(DHTPIN, DHTTYPE);

MyMessage tempMsg(CHILD_ID_TEMP, V_TEMP);
MyMessage humiMsg(CHILD_ID_HUMI, V_HUM);

void setup()
{
// use the 1.1 V internal reference
#if defined(__AVR_ATmega2560__)
    analogReference(INTERNAL1V1);
#else
    analogReference(INTERNAL);
#endif
}

void presentation()
{
    // Send the sketch version information to the gateway and Controller
    sendSketchInfo("Temperature Batt Meter", "1.0");
    // Register all sensors to gateway (they will be created as child devices)
    present(CHILD_ID_TEMP, S_TEMP);
    present(CHILD_ID_HUMI, S_HUM);
}

void updateBatteryValue()
{
    // get the battery Voltage
    int sensorValue = analogRead(BATTERY_SENSE_PIN);
#ifdef MY_DEBUG
    Serial.println(sensorValue);
#endif

    // 1M, 470K divider across battery and using internal ADC ref of 1.1V
    // Sense point is bypassed with 0.1 uF cap to reduce noise at that point
    // ((1e6+470e3)/470e3)*1.1 = Vmax = 3.44 Volts
    // 3.44/1023 = Volts per bit = 0.003363075

    int batteryPcnt = sensorValue / 10;
#ifdef MY_DEBUG
    float batteryV = sensorValue * 0.003363075;
    Serial.print("Battery Voltage: ");
    Serial.print(batteryV);
    Serial.println(" V");

    Serial.print("Battery percent: ");
    Serial.print(batteryPcnt);
    Serial.println(" %");
#endif
    if (oldBatteryPcnt != batteryPcnt)
    {
        // Power up radio after sleep
        sendBatteryLevel(batteryPcnt);
        oldBatteryPcnt = batteryPcnt;
    }
}

void updateTemperature()
{
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t))
    {
#ifdef MY_DEBUG
        Serial.println("Failed to read from DHT sensor!");
#endif
        return;
    }

    // Compute heat index in Celsius (isFahreheit = false)
    float hic = dht.computeHeatIndex(t, h, false);

#ifdef MY_DEBUG
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" *C ");
    Serial.print("Heat index: ");
    Serial.print(hic);
    Serial.print(" *C ");
#endif

    if (!send(tempMsg.set(t, 2), true))
    {
        send(tempMsg.set(t, 2));
    }
    if (!send(humiMsg.set(h, 2), true))
    {
        send(humiMsg.set(h, 2));
    }
}

void loop()
{
    updateBatteryValue();
    updateTemperature();
    smartSleep(SLEEP_TIME);
}
