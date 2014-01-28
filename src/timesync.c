#include "timesync.h"
#include <gossip.h>

#define ENABLE_DEBUG (1)
#include "debug.h"

#define ENABLE_WARN (1)
#include "warn.h"

void timesync_handle_msg(void* msg_text, size_t size, uint16_t src){
    char ts_src_id[6];
    char enc[size];

    // Extract source ID of timestamp
    strncpy(ts_src_id, (char *) msg_text + strlen(TS), 5);
    ts_src_id[5] = '\0';

    // Base64 decode timestamp and cast to tm struct
    strncpy(enc, (char *) msg_text + strlen(TS) + 6, sizeof(enc));
    unsigned char timestamp[255];
    base64_decode(timestamp, enc, (uint16_t) BASE64_ENCODE_LENGTH(sizeof(tm_wrapper)) + 3);
    tm_wrapper *localt = (tm_wrapper *) timestamp;

    // This is where the timesync magic should happen -- for now just
    // print the received timestamp
    DEBUG("Received timestamp from %i: %i-%02i-%02i %02i:%02i:%02i\n",
            atoi(ts_src_id),
            localt->tm_year + 1900,
            localt->tm_mon + 1,
            localt->tm_mday,
            localt->tm_hour,
            localt->tm_min,
            localt->tm_sec
        );
}

int timesync_init() {
    gossip_node_t* node;

    node = gossip_get_neighbour(RANDOM);

    if(!node){
        WARN("W: no neighbours, timesync init failed.\n");
        return 1;
    }

    // Get current time
    struct tm ltime;
    rtc_get_localtime(&ltime);

    //Wrap localtime into packed wrapper
    tm_wrapper localt;
    localt.tm_sec = ltime.tm_sec;
    localt.tm_min = ltime.tm_min;
    localt.tm_hour = ltime.tm_hour;
    localt.tm_mday = ltime.tm_mday;
    localt.tm_mon = ltime.tm_mon;
    localt.tm_year = ltime.tm_year;
    localt.tm_wday = ltime.tm_wday;

    DEBUG("Current localtime: %i-%i-%i %i:%i:%i\n",
            localt.tm_year + 1900,
            localt.tm_mon + 1,
            localt.tm_mday,
            localt.tm_hour,
            localt.tm_min,
            localt.tm_sec
        );

    // Base64 encode timestamp
    char enc[BASE64_ENCODE_LENGTH(sizeof(tm_wrapper)) + 1];
    base64_encode(enc, (unsigned char *) &localt, sizeof(tm_wrapper));

    char msg_buffer[strlen(PREAMBLE)
                    + strlen(MSG)
                    + strlen(TS)
                    + strlen(enc)
                    + 100];
    sprintf(msg_buffer, "%s%s%s%05i$%s", PREAMBLE, MSG, TS, gossip_id, enc);

    return gossip_send(node, msg_buffer, strlen(msg_buffer));
}
