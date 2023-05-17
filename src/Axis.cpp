#include <Arduino.h>
#include "protothreads.h"
#include <pt.h>
class Axis
{
public:
    // compile time configuration
    const int DATA_INTO_TARGET_FACTOR = 3;

    // run time configuration

    /// pin to trigger stepper motor to MOVE
    const uint8_t MOVE;
    /// pin to set direction
    const uint8_t DIRECTION;
    /// step range from 0
    const int MAX_STEPS;
    /// maximum sleep time??
    const int MAX_SLEEP_TIME;

    // mutable members
    int target = 0;
    int counter = 0;
    unsigned long sleep_time = 5;
    pt pt_axis;

    /// constructor
    /// - `MOVE`: data pin
    Axis(int MOVE, int dir, int max_steps, int max_sleep_time)
        : MOVE(MOVE),
          DIRECTION(dir),
          MAX_STEPS(max_steps),
          MAX_SLEEP_TIME(max_sleep_time)
    {
    }

    int doThread(struct pt *pt)
    {
        PT_BEGIN(pt);
        for (;;)
        {
            // set which way to MOVE
            if (target > counter)
            {
                // motor direction cw
                digitalWrite(DIRECTION, HIGH);
                // count steps
                counter++;
            }
            else if (target < counter)
            {
                // motor direction cw
                digitalWrite(DIRECTION, LOW);
                // count steps
                counter--;
            }

            // turn the LED on
            digitalWrite(MOVE, HIGH);
            PT_SLEEP(pt, sleep_time);

            // turn the LED off
            digitalWrite(MOVE, LOW);
            PT_SLEEP(pt, sleep_time);
        }
        PT_END(pt);
    }

    void parse(const uint8_t *data)
    {
        target = int(data[0]) * DATA_INTO_TARGET_FACTOR;
        sleep_time = (int)((float)MAX_SLEEP_TIME - round((float)data[1] / 255 * (float)MAX_SLEEP_TIME));
    }

    void setup()
    {
        // configure pins
        pinMode(MOVE, OUTPUT);
        pinMode(DIRECTION, OUTPUT);

        // initialize pins
        digitalWrite(MOVE, LOW);
        digitalWrite(DIRECTION, LOW);

        // initialize axis proto thread
        PT_INIT(&pt_axis);
    }

    void loop()
    {
        PT_SCHEDULE(doThread(&pt_axis));
    }
};
