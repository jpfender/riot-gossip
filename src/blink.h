#include <rtc.h>
#include <timesync.h>

#define BLINK_DURATION  (100*1000)
#define BLINK_PAUSE     (1000*1000)
#define BLINK_STACK_SIZE KERNEL_CONF_STACKSIZE_PRINTF

uint16_t blink_pid;

/* emulate a cron-like behaviour to fire at specific time
   this is a bit silly, since we could just use sleep(DELAY)
   but we want to reset the hwtimer regularly */
void blink(void);

