#include "timesync.h"
#include <gossip.h>
#include <leader.h>
#include "vtimer.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

#define ENABLE_WARN (0)
#include "warn.h"

#define DELAY_REQ_BUFFER PREAMBLE MSG TS DELAYREQ

/* tm_wrapper *t1_master; */
/* tm_wrapper *t2_master; */
/* tm_wrapper t1_local; */
/* tm_wrapper t2_local; */

int t1_master;
int t2_master;
int t1_local;
int t2_local;

int offset;
int master_offset = 0;

int trusted = 0;

struct timeval tiv;

void timesync_set_trusted(int value) {
    trusted = value; 
}

int timesync_get_trusted() {
    return trusted;
}

int timesync_get_master_offset() {
    return master_offset;
}

/* char *timesync_write_tm(char *buf, tm_wrapper *time){ */
/*     snprintf(buf,TS_STRSIZE, "%i-%02i-%02i %02i:%02i:%02i", */
/*                             time->tm_year + 1900, */
/*                             time->tm_mon + 1, */
/*                             time->tm_mday, */
/*                             time->tm_hour, */
/*                             time->tm_min, */
/*                             time->tm_sec ); */
/*     return buf; */
/* } */

/* void timesync_copy_ts(struct tm* src, tm_wrapper *dst){ */
/*     dst->tm_sec  = src->tm_sec; */
/*     dst->tm_min  = src->tm_min; */
/*     dst->tm_hour = src->tm_hour; */
/*     dst->tm_mday = src->tm_mday; */
/*     dst->tm_mon  = src->tm_mon; */
/*     dst->tm_year = src->tm_year; */
/*     dst->tm_wday = src->tm_wday; */
/* } */


void timesync_handle_msg(void* msg_text, size_t size, uint16_t src){
    char rcv_buffer[size];
    char ts_src_id[6];
    char enc[size];
    char *ts_buffer = malloc(TS_STRSIZE*sizeof(char));

    strncpy(rcv_buffer, (char *) msg_text, size);

    if (!strncmp(rcv_buffer, SYNC, strlen(SYNC))) {

        // Extract source ID of timestamp
        strncpy(ts_src_id, rcv_buffer + strlen(SYNC), UID_LEN);
        ts_src_id[UID_LEN] = '\0';
        int ts_src = atoi(ts_src_id);

        // If message comes from the leader (directly or indirectly),
        // record master and local timestamps and initiate ping-pong
        if (ts_src == leader_get_leader()) {

            // Base64 decode timestamp and cast to tm struct
            /* strncpy(enc, rcv_buffer + strlen(SYNC) + 6, sizeof(enc)); */
            /* unsigned char timestamp[255]; */
            /* base64_decode(timestamp, enc, (uint16_t) BASE64_ENCODE_LENGTH(sizeof(tm_wrapper)) + 3); */
            /* t1_master = (tm_wrapper *) timestamp; */

            t1_master = atoi(rcv_buffer + strlen(SYNC) + UID_LEN);

            DEBUG("D: Received T1_MASTER from %i: %i\n", ts_src, t1_master);

            /* DEBUG("D: Received T1_MASTER timestamp from %i: %s\n", */
            /*         ts_src, timesync_write_tm(ts_buffer, t1_master) ); */

            // Get current time
            /* struct tm ltime; */
            /* rtc_get_localtime(&ltime); */
            rtc_time(&tiv);

            //Wrap localtime into packed wrapper
            /* timesync_copy_ts(&ltime, &t1_local); */

            /* DEBUG("D: Current T1_LOCAL timestamp: %s\n", timesync_write_tm(ts_buffer, &t1_local) ); */
            DEBUG("D: Current T1_LOCAL: %i\n", t1_local);
            
            // Sleep for 2 seconds, then record T2_LOCAL and send
            // DELAY_REQ
            /* vtimer_usleep(1000*1000*2); */

            /* rtc_get_localtime(&ltime); */

            //Wrap localtime into packed wrapper
            /* timesync_copy_ts(&ltime, &t2_local); */

            /* DEBUG("D: Current T2_LOCAL timestamp: %s\n", timesync_write_tm(ts_buffer, &t2_local) ); */

            // Send DELAY_REQ message to sender
            /* char msg_buffer[strlen(PREAMBLE) + strlen(MSG) + strlen(TS) + strlen(DELAYREQ)]; */
            /* sprintf(msg_buffer, "%s%s%s%s", PREAMBLE, MSG, TS, DELAYREQ); */
            gossip_node_t* node = gossip_find_node_by_id(src);
            gossip_send(node, DELAY_REQ_BUFFER, strlen(DELAY_REQ_BUFFER));

            t1_local = tiv.tv_usec;
        }

    } else if (!strncmp(rcv_buffer, DELAYREQ, strlen(DELAYREQ))) {

        // Record current timestamp and send it to node

        /* struct tm ltime; */
        /* rtc_get_localtime(&ltime); */

        rtc_time(&tiv);

        //Wrap localtime into packed wrapper
        /* tm_wrapper localt; */
        /* timesync_copy_ts(&ltime, &localt); */

        /* DEBUG("Current localtime: %s\n", timesync_write_tm(ts_buffer, &localt) ); */
        DEBUG("Current usec: %i\n", tiv.tv_usec );

        // Base64 encode timestamp
        /* char enc_r[BASE64_ENCODE_LENGTH(sizeof(tm_wrapper)) + 1]; */
        /* base64_encode(enc_r, (unsigned char *) &localt, sizeof(tm_wrapper)); */

        char msg_buffer[strlen(PREAMBLE)
                        + strlen(MSG)
                        + strlen(TS)
                        + strlen(DELAYRESP)
                        + 8];
        sprintf(msg_buffer, "%s%s%s%s%08i", PREAMBLE, MSG, TS, DELAYRESP, tiv.tv_usec);

        gossip_node_t* node = gossip_find_node_by_id(src);
        gossip_send(node, msg_buffer, strlen(msg_buffer));


    } else if (!strncmp(rcv_buffer, DELAYRESP, strlen(DELAYRESP))) {

        // Base64 decode timestamp and cast to tm struct
        /* strncpy(enc, rcv_buffer + strlen(DELAYRESP), sizeof(enc)); */
        /* unsigned char timestamp[255]; */
        /* base64_decode(timestamp, enc, (uint16_t) BASE64_ENCODE_LENGTH(sizeof(tm_wrapper)) + 3); */
        /* t2_master = (tm_wrapper *) timestamp; */

        rtc_time(&tiv);
        t2_local = tiv.tv_usec;

        t2_master = atoi(rcv_buffer + strlen(DELAYRESP));

        DEBUG("Received T2_MASTER timestamp: %s\n", t2_master);

        // Calculate offset
        /* offset = (t1_local.tm_sec - t1_master->tm_sec - t2_local.tm_sec + t2_master->tm_sec) / 2; */
        offset = (t2_local > t1_local ? t2_local - t1_local : (1000000 - t1_local) + t2_local) / 2;
        master_offset = t1_local - offset > 0 ? t1_local - offset : 1000000 - t1_local;

        DEBUG("Current offset & master offset: %i; %i\n", offset, master_offset);

        // Apply offset to localtime
        /* struct tm ltime; */
        /* rtc_get_localtime(&ltime); */
        /* ltime.tm_sec += offset; */
        /* rtc_set_localtime(&ltime); */
        /* // upstream error is missing a newline :O */
        /* printf("\n"); */

        // Indicate that we have a trusted timestamp
        timesync_set_trusted(1);

        /* tm_wrapper localt; */
        /* timesync_copy_ts(&ltime, &localt); */

        /* DEBUG("Adjusted offset by %i seconds, timestamp is now: %s\n", */
        /*         offset, timesync_write_tm(ts_buffer, &localt) ); */
    }
    /* free(ts_buffer); */
}

int timesync_init() {
    gossip_node_t* node;

    node = gossip_get_neighbour(RANDOM);

    if(!node){
        WARN("W: no neighbours, timesync init failed.\n");
        return 1;
    }

    /* char *ts_buffer = malloc(TS_STRSIZE*sizeof(char)); */

    // Get current time
    /* struct tm ltime; */
    /* rtc_get_localtime(&ltime); */

    struct timeval tiv;
    rtc_time(&tiv);

    /* pause fixed + offset time and call ourself back*/
    vtimer_usleep( (1000 * 1000) - tiv.tv_usec );

    //Wrap localtime into packed wrapper
    /* tm_wrapper localt; */
    /* timesync_copy_ts(&ltime, &localt); */

    /* DEBUG("Current localtime: %s\n", timesync_write_tm(ts_buffer, &localt) ); */

    // Base64 encode timestamp
    /* char enc[BASE64_ENCODE_LENGTH(sizeof(tm_wrapper)) + 1]; */
    /* base64_encode(enc, (unsigned char *) &localt, sizeof(tm_wrapper)); */

    char msg_buffer[strlen(PREAMBLE)
                    + strlen(MSG)
                    + strlen(TS)
                    + strlen(SYNC)
                    + UID_LEN
                    + 8];
    sprintf(msg_buffer, "%s%s%s%s%0" UID_LEN_STR "u%08i", PREAMBLE, MSG, TS, SYNC, gossip_id, master_offset);

    /* free(ts_buffer); */
    return gossip_send(node, msg_buffer, strlen(msg_buffer));
}
