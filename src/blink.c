#include "board.h"
#include "vtimer.h"

#include "blink.h"

void blink(void) {

    while(1){
        /* do the blink */
        LED_RED_ON;
        vtimer_usleep( BLINK_DURATION );
        LED_RED_OFF;

        /* pause fixed + offset time and call ourself back*/
        vtimer_usleep( BLINK_PAUSE );
    }
}
