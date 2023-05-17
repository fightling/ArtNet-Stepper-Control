#include <Arduino.h>
#include "ArtnetEther.h"
#include <pt.h>
#include "protothreads.h"
#include "./Axis.cpp"

// sleep micro seconds
#define PT_SLEEP_MICROS(pt, delay)                                                                       \
    {                                                                                                    \
        do                                                                                               \
        {                                                                                                \
            static unsigned long proto_threads_sleep = micros();                                         \
            PT_WAIT_UNTIL(pt, micros() - proto_threads_sleep > delay || micros() < proto_threads_sleep); \
        } while (false);                                                                                 \
    }

#if DEBUG
#define LOG Serial.println
#define LOG_INIT() Serial.begin(115200)
#else
#define LOG
#define LOG_INIT()
#endif

// own IP address?
const IPAddress ip(192, 168, 0, 201);

// own MAC address?
uint8_t mac[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB};

// universe
const uint32_t UNIVERSE = 0; // 0 - 15

// used I/O pins
const uint8_t AXIS1_PUL_PIN = 40;
const uint8_t AXIS1_DIR_PIN = 42;
const uint8_t AXIS2_PUL_PIN = 44;
const uint8_t AXIS2_DIR_PIN = 46;

// axes movement configuration
const int AXIS_MAX_STEPS = 1000;
const int AXIS_SLEEP_TIME = 25;

// global Artnet receiver instance (automatically uses `Ethernet` via `ArtnetEther.h`?)
ArtnetReceiver artnet;

// Artnet proto thread
pt ptArtNet;

// axis accessors (based on PINs, steps and timing)
Axis axis1 = Axis(AXIS1_PUL_PIN, AXIS1_DIR_PIN, AXIS_MAX_STEPS, AXIS_SLEEP_TIME);
Axis axis2 = Axis(AXIS2_PUL_PIN, AXIS2_DIR_PIN, AXIS_MAX_STEPS, AXIS_SLEEP_TIME);

int artNetThread(struct pt *pt)
{
    PT_BEGIN(pt);

    for (;;)
    {
        // check if artnet packet has arrived (and eventually execute callback)
        artnet.parse();

        // have a stroll
        PT_SLEEP(pt, 20);
    }

    PT_END(pt);
}

/// get a message from Artnet (assume it's correct)
void callback(const uint8_t *data, const uint16_t size)
{
    LOG("Received package: %02X %02X %02X %02X", data[0], data[1], data[2], data[3]);

    // move axes to coordinates from message
    axis1.parse(data);
    axis2.parse(data + 2);
}

void setup()
{
    LOG_INIT();

    // initialize
    Ethernet.begin(mac, ip);
    artnet.begin();

    // receive messages from art net to callback
    artnet.subscribe(UNIVERSE, callback);

    // setup both axis
    axis1.setup();
    axis2.setup();
}

void loop()
{
    // process ArtNet thread
    PT_SCHEDULE(artNetThread(&ptArtNet));

    // process both axes
    axis1.loop();
    axis2.loop();
}