#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "rtc.h"
#include "cpu.h"
#include "base64.h"

#define TS		"TIMESYNC:"
#define SYNC		"SYNC:"
#define DELAYREQ	"DELAYREQ:"
#define DELAYRESP	"DELAYRESP:"

#define TS_STRSIZE (20)

/* magic ahead */
/* it may be that vtimer_usleep() has a non linear error of  O(n) */
#define VTIMER_FACTOR 0.947


void timesync_set_synced(int value);
int timesync_get_synced(void);
void timesync_set_trusted(int value);
int timesync_get_trusted(void);
int timesync_get_master_offset(void);
void timesync_handle_msg(void*, size_t, uint16_t);
int timesync_init(void);
void timesync_set_trusted(int);
