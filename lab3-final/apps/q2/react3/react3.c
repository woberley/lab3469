
#include "usertraps.h"
#include "misc.h"
#include "spawn.h"

void main (int argc, char *argv[]){
    sem_t s_procs_completed;
    mbox_t h2_mbox;
    mbox_t n2_mbox;
    mbox_t o2_mbox;
    mbox_t hno3_mbox;

    if (argc != 6 ) { 
        Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_proc_semaphore> <handle_to_H2_semaphore> <handle_to_N2_semaphore> <handle_to_O2_semaphore> <handle_to_HNO3_semaphore>  \n"); 
        Exit();
    }     

    s_procs_completed = dstrtol(argv[1], NULL, 10);
    h2_mbox = dstrtol(argv[2], NULL, 10);
    n2_mbox = dstrtol(argv[3], NULL, 10);
    o2_mbox = dstrtol(argv[4], NULL, 10);
    hno3_mbox = dstrtol(argv[5], NULL, 10);    

    if (mbox_open(h2_mbox) == MBOX_FAIL) {
      Printf("spawn_me (%d): Could not open the mailbox!\n", getpid());
      Exit();
    }
    if (mbox_open(n2_mbox) == MBOX_FAIL) {
      Printf("spawn_me (%d): Could not open the mailbox!\n", getpid());
      Exit();
    }
    if (mbox_open(o2_mbox) == MBOX_FAIL) {
      Printf("spawn_me (%d): Could not open the mailbox!\n", getpid());
      Exit();
    }
    if (mbox_open(hno3_mbox) == MBOX_FAIL) {
      Printf("spawn_me (%d): Could not open the mailbox!\n", getpid());
      Exit();
    }

    if (mbox_recv(h2_mbox, 2, (void *)"1") == MBOX_FAIL) {
    Printf("spawn_me (%d): Could not map the virtual address to the memory!\n", getpid());
    Exit();
    }
    if (mbox_recv(n2_mbox, 2, (void *)"1") == MBOX_FAIL) {
    Printf("spawn_me (%d): Could not map the virtual address to the memory!\n", getpid());
    Exit();
    }
    if (mbox_recv(o2_mbox, 2, (void *)"1") == MBOX_FAIL) {
    Printf("spawn_me (%d): Could not map the virtual address to the memory!\n", getpid());
    Exit();
    }

    Printf("Reaction: 3 PID: %d\n", getpid());

    if (mbox_send(hno3_mbox, 2, (void *)"1") == MBOX_FAIL) {
      Printf("Could not send message to mailbox %d in %s (%d)\n", hno3_mbox, argv[0], getpid());
      Exit();
    }
    


    if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
    }
}