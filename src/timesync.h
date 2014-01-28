#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "rtc.h"
#include "cpu.h"
#include "native_internal.h"
#include "base64.h"

#define TS "TIMESYNC:"

typedef struct {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
}__attribute__ ((packed)) tm_wrapper;

void timesync_handle_msg(void*, size_t, uint16_t);
int timesync_init();
