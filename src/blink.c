#include "board.h"
#include "vtimer.h"
#include "hwtimer.h"

#include "blink.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

void blink(void) {
    struct timeval tiv;
    uint32_t wait;

    /* everyone needs to wait for microseconds=0 before starting to blink */
    rtc_time(&tiv);
    vtimer_usleep( (uint32_t)(( 1000*1000 - tiv.tv_usec ) * VTIMER_FACTOR ));
    //hwtimer_spin( HWTIMER_TICKS( ( 1000*1000 - tiv.tv_usec ) * VTIMER_FACTOR ) );
    printf("blink thread synced to zero\n");

    while(1){
        //rtc_time(&tiv);
        //printf("blink time usec: %lu\n", tiv.tv_usec);
        /* do the blink */
        LED_RED_ON;
        //hwtimer_spin( HWTIMER_TICKS( BLINK_DURATION ));
        vtimer_usleep( BLINK_DURATION );
        LED_RED_OFF;

        wait = timesync_get_master_offset();
        if ( wait && ! timesync_get_synced() ){
            /* be careful with debug output here, as it affects delay as well! */
            //rtc_time(&tiv);
            //printf("(pseudo) adjusting clock from %i\n", (int) tiv.tv_usec);
            vtimer_usleep( (uint32_t)((wait+PROC_DELAY) * VTIMER_FACTOR) );
            printf("D: waited %lu us.\n", (uint32_t)((wait+PROC_DELAY) * VTIMER_FACTOR));
            timesync_set_synced(1);
            //rtc_time(&tiv);
            //printf("(pseudo) adjusting clock to %i\n", (int)tiv.tv_usec);
        }
        vtimer_usleep( BLINK_PAUSE-BLINK_DURATION);
    }
}
