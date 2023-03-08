
#include "usertraps.h"
#include "misc.h"
#include "spawn.h"

void main (int argc, char *argv[])
{

    sem_t s_procs_completed;
    mbox_t h2o_mbox;

    if (argc != 3) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <s_procs_completed handle> <h2o_mbox handle\n"); 
    Exit();
    } 


    s_procs_completed = dstrtol(argv[1], NULL, 10);
    h2o_mbox = dstrtol(argv[2], NULL, 10);

    if (mbox_open(h2o_mbox) == MBOX_FAIL) {
      Printf("spawn_me (%d): Could not open the mailbox!\n", getpid());
      Exit();
    }


    Printf("Injection: H2O PID: %d\n", getpid());

    if (mbox_send(h2o_mbox, 2, (void *)"1") == MBOX_FAIL) {
      Printf("Could not send message to mailbox %d in %s (%d)\n", h2o_mbox, argv[0], getpid());
      Exit();
    }
    

    if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
        Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
        Exit();
    }
    
}