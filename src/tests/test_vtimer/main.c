#include "stdio.h"
#include "vtimer.h"
#include "thread.h"

char s1[MINIMUM_STACK_SIZE];

void run1(void){
    while(1){
        puts("--some thread--");
        vtimer_usleep(1000*300);
    }
}

int main(void)
{
    puts("\n\t\t\tWelcome to RIOT\n");
    puts("--test_vtimer--");

    thread_create(s1, MINIMUM_STACK_SIZE, PRIORITY_MAIN+1, 0, run1, "t1");

    while(1){
        vtimer_usleep(1000*1000);
        puts("--main--");
    }
}
