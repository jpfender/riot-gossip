/*
 * Copyright (C) 2013 Ludwig Ortmann <ludwig.ortmann@fu-berlin.de>
 */

#include <stdio.h>
#include <time.h>
#include <debug.h>

#include "vtimer.h"
#include "rtc.h"
#include "board_uart0.h"
#include "shell.h"
#include "shell_commands.h"
#include "board.h"
#include "transceiver.h"
#include "posix_io.h"
#include "nativenet.h"
#include "msg.h"
#include <thread.h>

#include "gossip.h"

void handle_msg(void* data, size_t len){
    size_t i;

    // assume we were sending a simple string for now
    puts("got a message: ");
    for( i=0 ; i<len ; i++){
        putchar( ((char*)data)[i] );
    }
}

int main(void)
{
    transceiver_type_t transceiver = TRANSCEIVER_NATIVE;
    int main_pid = thread_getpid();

    // TODO: figure out what this does:
    posix_open(uart0_handler_pid, 0);

    printf("\n\t\t\tWelcome to RIOT\n\n");

    printf("Initializing gossiping.");
    if( 0 != gossip_init(transceiver) ){
        DEBUG("gossip_init(%d) failed\n", transceiver);
        return 1;
    }

    DEBUG("Sending gossip hello to the world.\n");
    if( 0 != gossip_announce() ){
        DEBUG("gossip_announce() failed\n");
        return 1;
    }

    puts("Registering sample gossip message handler.");
    gossip_register_msg_handler(handle_msg);

    // TODO: sleep for now, should receive IPC logger msg and printf here
    thread_sleep();

    return 0;
}
