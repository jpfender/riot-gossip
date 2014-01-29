#include "timesync.h"
#include <gossip.h>
#include <leader.h>
#include "vtimer.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

#define ENABLE_WARN (1)
#include "warn.h"

tm_wrapper *t1_master;
tm_wrapper *t2_master;
tm_wrapper t1_local;
tm_wrapper t2_local;

int offset;

int trusted = 0;

void timesync_set_trusted(int value) {
    trusted = value; 
}

int timesync_get_trusted() {
    return trusted;
}

void timesync_handle_msg(void* msg_text, size_t size, uint16_t src){
    char rcv_buffer[size];
    char ts_src_id[6];
    char enc[size];

    strncpy(rcv_buffer, (char *) msg_text, size);

    if (!strncmp(rcv_buffer, SYNC, strlen(SYNC))) {

        // Extract source ID of timestamp
        strncpy(ts_src_id, rcv_buffer + strlen(SYNC), 5);
        ts_src_id[5] = '\0';
        int ts_src = atoi(ts_src_id);

        // If message comes from the leader (directly or indirectly),
        // record master and local timestamps and initiate ping-pong
        if (ts_src == leader_get_leader()) {

            // Base64 decode timestamp and cast to tm struct
            strncpy(enc, rcv_buffer + strlen(SYNC) + 6, sizeof(enc));
            unsigned char timestamp[255];
            base64_decode(timestamp, enc, (uint16_t) BASE64_ENCODE_LENGTH(sizeof(tm_wrapper)) + 3);
            t1_master = (tm_wrapper *) timestamp;

            DEBUG("Received T1_MASTER timestamp from %i: %i-%02i-%02i %02i:%02i:%02i\n",
                    ts_src,
                    t1_master->tm_year + 1900,
                    t1_master->tm_mon + 1,
                    t1_master->tm_mday,
                    t1_master->tm_hour,
                    t1_master->tm_min,
                    t1_master->tm_sec
                );

            // Get current time
            struct tm ltime;
            rtc_get_localtime(&ltime);

            //Wrap localtime into packed wrapper
            t1_local.tm_sec  = ltime.tm_sec;
            t1_local.tm_min  = ltime.tm_min;
            t1_local.tm_hour = ltime.tm_hour;
            t1_local.tm_mday = ltime.tm_mday;
            t1_local.tm_mon  = ltime.tm_mon;
            t1_local.tm_year = ltime.tm_year;
            t1_local.tm_wday = ltime.tm_wday;

            DEBUG("Current T1_LOCAL timestamp: %i-%02i-%02i %02i:%02i:%02i\n",
                    t1_local.tm_year + 1900,
                    t1_local.tm_mon + 1,
                    t1_local.tm_mday,
                    t1_local.tm_hour,
                    t1_local.tm_min,
                    t1_local.tm_sec
                );
            
            // Sleep for 2 seconds, then record T2_LOCAL and send
            // DELAY_REQ
            vtimer_usleep(1000*1000*2);

            rtc_get_localtime(&ltime);

            //Wrap localtime into packed wrapper
            t2_local.tm_sec  = ltime.tm_sec;
            t2_local.tm_min  = ltime.tm_min;
            t2_local.tm_hour = ltime.tm_hour;
            t2_local.tm_mday = ltime.tm_mday;
            t2_local.tm_mon  = ltime.tm_mon;
            t2_local.tm_year = ltime.tm_year;
            t2_local.tm_wday = ltime.tm_wday;

            DEBUG("Current T2_LOCAL timestamp: %i-%02i-%02i %02i:%02i:%02i\n",
                    t2_local.tm_year + 1900,
                    t2_local.tm_mon + 1,
                    t2_local.tm_mday,
                    t2_local.tm_hour,
                    t2_local.tm_min,
                    t2_local.tm_sec
                );

            // Send DELAY_REQ message to sender
            char msg_buffer[strlen(PREAMBLE)
                            + strlen(MSG)
                            + strlen(TS)
                            + strlen(DELAYREQ)
                            + 100];
            sprintf(msg_buffer, "%s%s%s%s", PREAMBLE, MSG, TS, DELAYREQ);
            gossip_node_t* node = gossip_find_node_by_id(src);
            gossip_send(node, msg_buffer, strlen(msg_buffer));
        }

    } else if (!strncmp(rcv_buffer, DELAYREQ, strlen(DELAYREQ))) {

        // Record current timestamp and send it to node

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

        DEBUG("Current localtime: %i-%02i-%02i %02i:%02i:%02i\n",
                localt.tm_year + 1900,
                localt.tm_mon + 1,
                localt.tm_mday,
                localt.tm_hour,
                localt.tm_min,
                localt.tm_sec
            );

        // Base64 encode timestamp
        char enc_r[BASE64_ENCODE_LENGTH(sizeof(tm_wrapper)) + 1];
        base64_encode(enc_r, (unsigned char *) &localt, sizeof(tm_wrapper));

        char msg_buffer[strlen(PREAMBLE)
                        + strlen(MSG)
                        + strlen(TS)
                        + strlen(DELAYRESP)
                        + strlen(enc_r)
                        + 100];
        sprintf(msg_buffer, "%s%s%s%s%s", PREAMBLE, MSG, TS, DELAYRESP, enc_r);

        gossip_node_t* node = gossip_find_node_by_id(src);
        gossip_send(node, msg_buffer, strlen(msg_buffer));

    } else if (!strncmp(rcv_buffer, DELAYRESP, strlen(DELAYRESP))) {

        // Base64 decode timestamp and cast to tm struct
        strncpy(enc, rcv_buffer + strlen(DELAYRESP), sizeof(enc));
        unsigned char timestamp[255];
        base64_decode(timestamp, enc, (uint16_t) BASE64_ENCODE_LENGTH(sizeof(tm_wrapper)) + 3);
        t2_master = (tm_wrapper *) timestamp;

        DEBUG("Received T2_MASTER timestamp: %i-%02i-%02i %02i:%02i:%02i\n",
                t2_master->tm_year + 1900,
                t2_master->tm_mon + 1,
                t2_master->tm_mday,
                t2_master->tm_hour,
                t2_master->tm_min,
                t2_master->tm_sec
            );

        // Calculate offset
        offset = (t1_local.tm_sec - t1_master->tm_sec - t2_local.tm_sec + t2_master->tm_sec) / 2;

        // Apply offset to localtime
        struct tm ltime;
        rtc_get_localtime(&ltime);
        ltime.tm_sec += offset;
        rtc_set_localtime(&ltime);

        // Indicate that we have a trusted timestamp
        timesync_set_trusted(1);

        DEBUG("Adjusted offset by %i seconds, timestamp is now: %i-%02i-%02i %02i:%02i:%02i\n",
                offset,
                ltime.tm_year + 1900,
                ltime.tm_mon + 1,
                ltime.tm_mday,
                ltime.tm_hour,
                ltime.tm_min,
                ltime.tm_sec
            );
    }
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

    DEBUG("Current localtime: %i-%02i-%02i %02i:%02i:%02i\n",
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
                    + strlen(SYNC)
                    + strlen(enc)
                    + 100];
    sprintf(msg_buffer, "%s%s%s%s%05i$%s", PREAMBLE, MSG, TS, SYNC, gossip_id, enc);

    return gossip_send(node, msg_buffer, strlen(msg_buffer));
}
