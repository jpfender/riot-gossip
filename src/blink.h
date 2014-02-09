#include <rtc.h>
#include <timesync.h>

#define BLINK_DURATION  (100*1000)*VTIMER_FACTOR

/* boards are synced to <= 1sec so this is not exactly the
 * the range but only the blinking pause on one board */
#define BLINK_PAUSE     (1000*1000)*VTIMER_FACTOR
#define BLINK_STACK_SIZE KERNEL_CONF_STACKSIZE_PRINTF

#define PROC_DELAY      (0) // might be useful for empirical delay

uint16_t blink_pid;

/* emulate a cron-like behaviour to fire at specific time
   this is a bit silly, since we could just use sleep(DELAY)
   but we want to reset the hwtimer regularly */
void blink(void);

