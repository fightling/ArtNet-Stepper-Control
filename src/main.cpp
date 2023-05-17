#include <Arduino.h>
#include "ArtnetEther.h"
#include <pt.h>
#include "protothreads.h"
#include "./Axis.cpp"
// this is also valid for other platforms which can use only Ethernet
// #include <Artnet.h>

#define PT_SLEEP_MICROS(pt, delay)                                                                     \
    {                                                                                                  \
        do                                                                                             \
        {                                                                                              \
            static unsigned long protothreads_sleep = micros();                                        \
            PT_WAIT_UNTIL(pt, micros() - protothreads_sleep > delay || micros() < protothreads_sleep); \
        } while (false);                                                                               \
    }

// Ethernet stuff
const IPAddress ip(192, 168, 0, 201);
uint8_t mac[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB};

ArtnetReceiver artnet;
uint32_t universe0 = 0; // 0 - 15
uint8_t AXIS1_PUL_PIN = 40;
uint8_t AXIS1_DIR_PIN = 42;
uint8_t AXIS2_PUL_PIN = 44;
uint8_t AXIS2_DIR_PIN = 46;
pt ptArtNet;
Axis axis1 = Axis(AXIS1_PUL_PIN, AXIS1_DIR_PIN, 1000, 25);
Axis axis2 = Axis(AXIS2_PUL_PIN, AXIS2_DIR_PIN, 1000, 25);
int artNetThread(struct pt *pt)
{
    PT_BEGIN(pt);

    // Loop forever
    for (;;)
    {
        // if distanceSteps == 0 do nothing
        artnet.parse(); // check if artnet packet has come and execute callback
        PT_SLEEP(pt, 20);
    }
    PT_END(pt);
}

void callback(const uint8_t *data, const uint16_t size)
{
    // Serial.println(" Recieved package");
    axis1.setTarget(data[0]);
    axis1.setSleepTimeFromDmx(data[1]);
    axis2.setTarget(data[2]);
    axis2.setSleepTimeFromDmx(data[3]);
}

void setup()
{
    Serial.begin(115200);

    Ethernet.begin(mac, ip);
    artnet.begin();
    // artnet.subscribe_net(0);     // optionally you can change
    // artnet.subscribe_subnet(0);  // optionally you can change

    // you can also use pre-defined callbacks
    artnet.subscribe(universe0, callback);
    axis1.setup();
    axis2.setup();
}

void loop()
{
    PT_SCHEDULE(artNetThread(&ptArtNet));
    axis1.loop();
    axis2.loop();
}