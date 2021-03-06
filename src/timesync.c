#include "timesync.h"
#include <gossip.h>
#include <leader.h>
#include "vtimer.h"
#include "hwtimer.h"
#include "blink.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

#define ENABLE_WARN (0)
#include "warn.h"

#define DELAY_REQ_BUFFER PREAMBLE MSG TS DELAYREQ

#define ABS(X) (X>0?X:-X)

uint32_t t1_master,t2_master,rtt_master;
uint32_t t1_local,t2_local,rtt_local;
uint32_t tt;
uint32_t master_offset=0;
int precision;
int trusted = 0;

int synced=0;

void timesync_set_trusted(int value) {
    trusted = value;
}

int timesync_get_trusted() {
    return trusted;
}

uint32_t timesync_get_master_offset() {
    return master_offset;
}

void timesync_set_synced(char value){
    synced=value;
}
char timesync_get_synced(){
    return synced;
}



void timesync_handle_msg(void* msg_text, size_t size, uint16_t src){
    char rcv_buffer[size+1];
    rcv_buffer[size]=0;
    char ts_src_id[UID_LEN+1];
    char *msg_buffer;
    int new_precision;
    struct timeval tiv;

    rtc_time(&tiv);

    strncpy(rcv_buffer, (char *) msg_text, size);
    puts(rcv_buffer);


    if (!strncmp(rcv_buffer, SYNC, strlen(SYNC))) {

        if( leader_is_leader() ){
            // master does not want to update its time
            return;
        }
        t1_local = tiv.tv_usec;
        DEBUG("SYNC usec: %lu\n", tiv.tv_usec );

        // Extract source ID of timestamp
        strncpy(ts_src_id, rcv_buffer + strlen(SYNC), UID_LEN);
        ts_src_id[UID_LEN] = '\0';
        int ts_src = atoi(ts_src_id);

        // If message comes from the leader (directly or indirectly),
        // record master and local timestamps and initiate ping-pong
        if (ts_src == leader_get_leader()) {

            t1_master = atoi(rcv_buffer + strlen(SYNC) + UID_LEN);
            DEBUG("D: Received T1_MASTER usec from %i: %ld\n", ts_src, t1_master);

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

            gossip_node_t* node = gossip_find_node_by_id(src);
            gossip_send(node, DELAY_REQ_BUFFER, strlen(DELAY_REQ_BUFFER));

        }

    } else if (!strncmp(rcv_buffer, DELAYREQ, strlen(DELAYREQ))) {

        DEBUG("DELAYREQ usec: %lu\n", tiv.tv_usec );

        msg_buffer = malloc(strlen(PREAMBLE) + strlen(MSG) + strlen(TS) + strlen(DELAYRESP) + 8);
        if(!msg_buffer)
            printf("E: malloc failed.\n");

        sprintf(msg_buffer, "%s%s%s%s%08lu", PREAMBLE, MSG, TS, DELAYRESP, tiv.tv_usec);

        gossip_node_t* node = gossip_find_node_by_id(src);
        gossip_send(node, msg_buffer, strlen(msg_buffer));
        free(msg_buffer);


    } else if (!strncmp(rcv_buffer, DELAYRESP, strlen(DELAYRESP))) {

        t2_local = tiv.tv_usec;
        t2_master = atoi(rcv_buffer + strlen(DELAYRESP));


        // Calculate offset (half RTT + our local time at first arrival of sync msg) - really?
        rtt_local   = t2_local > t1_local ? t2_local - t1_local : (1000000 - t1_local) + t2_local;
        rtt_master  = t2_master > t1_master ? t2_master - t1_master : (1000000 - t1_master) + t2_master;

        /* average over both observed roundtrips and assuming both direction took the same time
         * divide by two to get the offset of master and local */
        tt = (rtt_local + rtt_master)/2/2;
        master_offset = (t1_local > tt ? t1_local - tt : (1000*1000) + t1_local - tt);

        DEBUG("DELAYRESP usec: %lu\n", tiv.tv_usec );
        DEBUG("Received T2_MASTER timestamp: %lu\n", t2_master);
        DEBUG("our RTT: %lu , master RTT: %lu\n", rtt_local, rtt_master);
        DEBUG("estimated master offset: %lu\n", master_offset);

        new_precision = ABS( (t1_local - t2_local ) - (t2_master - t1_master) );
        if( new_precision < precision ){
            // fancy TODO: update blinking delay for we have a better offset now
        }

        timesync_set_trusted(1);

    }
}

int timesync_init() {
    gossip_node_t* node;
    char *msg_buffer;
    struct timeval tiv;

    node = gossip_get_neighbour(RANDOM);

    if(!node){
        WARN("W: no neighbours, timesync init failed.\n");
        return 1;
    }

    msg_buffer = malloc(strlen(PREAMBLE) + strlen(MSG) + strlen(TS) + strlen(SYNC) + UID_LEN + 8);
    if( !msg_buffer ){
        puts("E: malloc failed");
        return 1;
    }


    sprintf(msg_buffer, "%s%s%s%s%0" UID_LEN_STR "i%08lu", PREAMBLE, MSG, TS, SYNC, (int) gossip_id,
            master_offset);

    /* timesync init and blink loop need to be synced locally, wait for usec=0 */
    rtc_time(&tiv);
    vtimer_usleep( (uint32_t)(( 1000*1000 - tiv.tv_usec ) * VTIMER_FACTOR ));

    gossip_send(node, msg_buffer, strlen(msg_buffer));
    free(msg_buffer);

    rtc_time(&tiv);
    DEBUG("Current usec: %lu\n", tiv.tv_usec );
    printf("time sync init synced zero\n");
    return 0;
}
