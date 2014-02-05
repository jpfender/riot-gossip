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

typedef struct {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
}__attribute__ ((packed)) tm_wrapper;

void timesync_set_trusted(int value);
int timesync_get_trusted(void);
void timesync_handle_msg(void*, size_t, uint16_t);
int timesync_init(void);
char *timesync_write_ts(char* buffer, tm_wrapper* timesource);
void timesync_copy_ts(struct tm* src, tm_wrapper* dst);

