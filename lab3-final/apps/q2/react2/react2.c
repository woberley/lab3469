
#include "usertraps.h"
#include "misc.h"
#include "spawn.h"

void main (int argc, char *argv[]){
    sem_t s_procs_completed;
    mbox_t h2o_mbox;
    mbox_t h2_mbox;
    mbox_t o2_mbox;

    if (argc != 5 ) { 
        Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_proc_semaphore> <handle_to_H2O_semaphore> <handle_to_H2_semaphore> <handle_to_O2_semaphore> \n"); 
        Exit();
    }     

    s_procs_completed = dstrtol(argv[1], NULL, 10);
    h2o_mbox = dstrtol(argv[2], NULL, 10);
    h2_mbox = dstrtol(argv[3], NULL, 10);
    o2_mbox = dstrtol(argv[4], NULL, 10);   

    if (mbox_open(h2o_mbox) == MBOX_FAIL) {
      Printf("spawn_me (%d): Could not open the mailbox!\n", getpid());
      Exit();
    }
    if (mbox_open(h2_mbox) == MBOX_FAIL) {
      Printf("spawn_me (%d): Could not open the mailbox!\n", getpid());
      Exit();
    }
    if (mbox_open(o2_mbox) == MBOX_FAIL) {
      Printf("spawn_me (%d): Could not open the mailbox!\n", getpid());
      Exit();
    }

    if (mbox_recv(h2o_mbox, 2, (void *)"1") == MBOX_FAIL) {
    Printf("spawn_me (%d): Could not map the virtual address to the memory!\n", getpid());
    Exit();
    }

    Printf("Reaction: 2 PID: %d\n", getpid());

    if (mbox_send(h2_mbox, 2, (void *)"1") == MBOX_FAIL) {
      Printf("Could not send message to mailbox %d in %s (%d)\n", h2_mbox, argv[0], getpid());
      Exit();
    }
    if (mbox_send(o2_mbox, 2, (void *)"1") == MBOX_FAIL) {
      Printf("Could not send message to mailbox %d in %s (%d)\n", o2_mbox, argv[0], getpid());
      Exit();
    }


    if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
    }
}