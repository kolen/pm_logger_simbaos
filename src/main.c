#include "simba.h"
#include "sds011.h"

static THRD_STACK(sds011_stack, 1024);

struct log_object_t main_log;

int main()
{
    struct pin_driver_t led;
    sys_start();

    log_object_init(&main_log, "main_module", LOG_UPTO(INFO));

    pin_init(&led, &pin_led_dev, PIN_OUTPUT);
    pin_write(&led, 1);

    log_object_print(&main_log, LOG_INFO, OSTR("Starting SDS011 thread"));

    thrd_spawn(sds011_main, NULL, 10, sds011_stack, sizeof(sds011_stack));

    log_object_print(&main_log, LOG_INFO, OSTR("Starting blinking"));

    while (1) {
        /* Wait half a second. */
        thrd_sleep(2);

        /* Toggle the LED on/off. */
        pin_toggle(&led);
    }

    return (0);
}
