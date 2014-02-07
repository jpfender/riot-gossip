#include "board.h"
#include "vtimer.h"

#include "blink.h"

void blink(void) {

    while(1){
        /* do the blink */
        LED_RED_ON;
        vtimer_usleep( BLINK_DURATION );
        LED_RED_OFF;

        struct timeval tiv;
        rtc_time(&tiv);

        /* pause fixed + offset time and call ourself back*/
        vtimer_usleep( (1000 * 1000) - tiv.tv_usec );

        int wait = timesync_get_master_offset();
        if (wait)
            vtimer_usleep(wait);
    }
}
