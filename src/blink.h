#define BLINK_DURATION  (100*1000)
#define BLINK_PAUSE     (1000*1000)
#define BLINK_STACK_SIZE MINIMUM_STACK_SIZE

/* emulate a cron-like behaviour to fire at specific time
   this is a bit silly, since we could just use sleep(DELAY)
   but we want to reset the hwtimer regularly */
void blink(void);
