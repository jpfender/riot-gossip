#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

#include "vtimer.h"
#include "rtc.h"
#include "board_uart0.h"
#include "board.h"
#include "transceiver.h"
#include "posix_io.h"
#include "msg.h"
#include "random.h"
#include "thread.h"
#include "hwtimer.h"

#include "gossip.h"
#include "leader.h"
#include "timesync.h"
#include "blink.h"

#include "sht11.h"

#define ENABLE_DEBUG (0)
#include <debug.h>

#define ENABLE_WARN (1)
#include <warn.h>

char leader_stack[LEADER_STACK_SIZE];
char blink_stack[BLINK_STACK_SIZE];

void handle_msg(void* msg_text, size_t size, uint16_t src){

    /* MSG is for Leader Election */
    if (strncmp(msg_text, LE, strlen(LE)) == 0) {
        DEBUG("D: received a leader election msg\n");
        if( ! leader_get_active() ){
            if( 0 > thread_create( leader_stack, LEADER_STACK_SIZE, 0,
                            CREATE_STACKTEST, leader_elect, "Leader") ){
                puts("E: leader thread creation failed");
                return;
            }
            leader_set_active(1);
        }
        leader_handle_msg(msg_text, size, src);
    }
    /* MSG is for Time Synchronization */
    else if (strncmp(msg_text, TS, strlen(TS)) == 0) {
        DEBUG("D: received a timesync msg\n");
        timesync_handle_msg((char*)msg_text + strlen(TS), size - strlen(TS), src);
    }
    /* MSG is unknown */
    else {
        WARN("W: Unknown message received.");
        // format string black hole ahead
        DEBUG("D: %s", (char*)msg_text);
    }
}

int handle_remove_neighbour(gossip_node_t* neighbour) {
    return leader_handle_remove_neighbour(neighbour);
}

int main(void)
{
    uint16_t id;
    timex_t time;
    transceiver_type_t transceiver = TRANSCEIVER_TYPE;
    gossip_node_list_t *neighbours;

    puts("\n\t\t\tWelcome to RIOT\n");
    puts("Initializing gossiping.");
    DEBUG("D: Registering sample gossip message handler.\n");
    gossip_register_msg_handler(handle_msg);

    DEBUG("D: Registering gossip on_remove_neighbour handler.\n");
    gossip_register_on_remove_neighbour_handler(handle_remove_neighbour);

    //vtimer_init();
    vtimer_now(&time);
#ifdef MODULE_SHT11
    sht11_val_t sht11_val;
    sht11_read_sensor(&sht11_val, HUMIDITY | TEMPERATURE);
    genrand_init( time.microseconds ^ (long) sht11_val.temperature*1000 ^ (long) sht11_val.relhum*1000 );
#else
    genrand_init( time.microseconds );
#endif

    id = 1+((uint16_t)genrand_uint32())%(MAX_UID-1);
    if( 0 != gossip_init(id,transceiver) ){
        DEBUG("D: gossip_init(%d) failed\n", transceiver);
        return 1;
    }
    leader_set_leader(id);

    blink_pid = thread_create( blink_stack, BLINK_STACK_SIZE , PRIORITY_MAIN-2,
                            CREATE_STACKTEST, blink, "Blink");

    while (1) {
        unsigned long delay = 2000000 * ((genrand_uint32()%10) + 1);
        vtimer_usleep(delay);

        gossip_announce();
        neighbours = gossip_get_all_neighbours();

        DEBUG("D: free space blink: %d\n",thread_measure_stack_free( blink_stack ));
        DEBUG("D: free space leader: %d\n",thread_measure_stack_free( leader_stack ));
        DEBUG("D: free space radio: %d\n",thread_measure_stack_free( gossip_radio_stack_buffer ));
        DEBUG("D: free space main: %d\n", thread_measure_stack_free( active_thread->stack_start ));

        /* print current state */
#ifdef MODULE_SHT11
        sht11_read_sensor(&sht11_val, HUMIDITY | TEMPERATURE);
        printf("[ ID: %u | Leader: %d | Net: %d | Temp: %4.1f ]\n",
                id, leader_get_leader(), neighbours->length, sht11_val.temperature);
#else
        printf("[ ID: %u | Leader: %d | Net: %d ]\n",
                id, leader_get_leader(), neighbours->length);
#endif

        if( neighbours->length && ! leader_get_initialized() ){
            printf("Starting initial leader election.\n");
            leader_set_leader(gossip_id);
            /* _try_ to initialize leader election by emitting a single packet */
            leader_init();
        }

        //Time synchronization IF I am the leader OR if I received my
        //timestamp from the leader
        //  XXX: only master initiates for now... change && to || in the future
        if ( leader_get_leader() == id && timesync_get_trusted() && ! leader_get_active() ){
            printf("Starting timesync.\n");
            timesync_init();
        }

        gossip_cleanup();
        gossip_free_node_list(neighbours);
    }
    return 0;
}
