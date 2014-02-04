#include "vtimer.h"
#include "board.h"
#include "thread.h"

#include "blink.h"

void blink(void) {

    while(1) {
        /* do the blink */
        LED_RED_ON;
        vtimer_usleep(BLINK_DURATION);
        LED_RED_OFF;
        LED_GREEN_ON;
        vtimer_usleep(BLINK_DURATION);
        LED_GREEN_OFF;
        LED_RED_ON;
        vtimer_usleep(BLINK_DURATION);
        LED_RED_OFF;

        thread_sleep();
    }
}
