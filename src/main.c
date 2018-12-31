#include "simba.h"
#include "sds011.h"

THRD_STACK(sds011_stack, 128);

int main()
{
    struct pin_driver_t led;

    sys_start();


    pin_init(&led, &pin_led_dev, PIN_OUTPUT);
    pin_write(&led, 1);

    thrd_spawn(sds011_main, NULL, 10, sds011_stack, sizeof(sds011_stack));

    while (1) {
        /* Wait half a second. */
        thrd_sleep_us(500000);

        /* Toggle the LED on/off. */
        pin_toggle(&led);
    }

    return (0);
}
