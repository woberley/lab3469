
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  //int numprocs = 0;               // Used to store number of processes to create
  //int i;                          // Loop index variable
  //missile_code *mc;               // Used to get address of shared memory page
  //uint32 h_mem;                   // Used to hold handle to shared memory page
  //sem_t s_procs_completed;        // Semaphore used to wait until all spawned processes have completed
  //char h_mem_str[10];             // Used as command-line argument to pass mem_handle to new processes
  //char s_procs_completed_str[10]; // Used as command-line argument to pass page_mapped handle to new processes

  //run_os_tests(); // Use for testing purpose

  //if (argc != 3) {
  //  Printf("Usage: "); Printf(argv[0]); Printf(" <N2 amount> <H2O amount>\n");
  //  Exit();
  //}

  sem_t s_procs_completed;
  mbox_t no_mbox;
  mbox_t h2o_mbox;
  mbox_t o2_mbox;
  mbox_t n2_mbox;
  mbox_t h2_mbox;
  mbox_t hno3_mbox;
  int no_;
  int h2o;
  int maxCount;
  int no_2;
  int h2o2;
  int n2;
  int o2;
  int h2;
  int i;

  char s_procs_completed_str[10]; 
  char no_mbox_str[10];
  char n2_mbox_str[10];
  char o2_mbox_str[10];
  char h2o_mbox_str[10];
  char h2_mbox_str[10];
  char hno3_mbox_str[10];
  char no_str[10];
  char h2o_str[10];
  char maxCount_str[10];

  char my_msg[] = "1";

  no_ = dstrtol(argv[1], NULL, 10);
  h2o = dstrtol(argv[2], NULL, 10); /////change these??
  Printf("Creating processes\n");

  no_2 = no_;
  h2o2 = h2o;
  while (no_2 >= 2){
    n2 += 1;
    o2 += 1;
    no_2 -= 2;
  }
  while (h2o2 >= 2){
    o2 += 1;
    h2 += 2;
    h2o2 -= 2;
  }
  maxCount = 0;
  while((h2 > 0) && (n2 > 0) && (o2 >= 3)){
      maxCount += 1;
      h2 -= 1;
      n2 -= 1;
      o2 -= 3;
  }
  // Convert string from ascii command line argument to integer number

  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.  To do this, we will initialize
  // the semaphore to (-1) * (number of signals), where "number of signals"
  // should be equal to the number of processes we're spawning - 1.  Once 
  // each of the processes has signaled, the semaphore should be back to
  // zero and the final sem_wait below will return.
  if ((s_procs_completed = sem_create(-(maxCount + no_ + h2o + no_/2 + h2o/2 -1))) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if ((no_mbox = mbox_create()) == MBOX_FAIL) {
    Printf("makeprocs (%d): ERROR: could not allocate mailbox!", getpid());
    Exit();
  }
  if ((h2o_mbox = mbox_create()) == MBOX_FAIL) {
    Printf("makeprocs (%d): ERROR: could not allocate mailbox!", getpid());
    Exit();
  }
  if ((h2_mbox = mbox_create()) == MBOX_FAIL) {
    Printf("makeprocs (%d): ERROR: could not allocate mailbox!", getpid());
    Exit();
  }
  if ((n2_mbox = mbox_create()) == MBOX_FAIL) {
    Printf("makeprocs (%d): ERROR: could not allocate mailbox!", getpid());
    Exit();
  }
  if ((o2_mbox = mbox_create()) == MBOX_FAIL) {
    Printf("makeprocs (%d): ERROR: could not allocate mailbox!", getpid());
    Exit();
  }
  if ((hno3_mbox = mbox_create()) == MBOX_FAIL) {
    Printf("makeprocs (%d): ERROR: could not allocate mailbox!", getpid());
    Exit();
  }
  if (mbox_open(no_mbox) == MBOX_FAIL) {
    Printf("makeprocs (%d): Could not open mailbox %d!\n", getpid(), no_mbox);
    Exit();
  }
  if (mbox_open(h2o_mbox) == MBOX_FAIL) {
    Printf("makeprocs (%d): Could not open mailbox %d!\n", getpid(), h2o_mbox);
    Exit();
  }
  if (mbox_open(h2_mbox) == MBOX_FAIL) {
    Printf("makeprocs (%d): Could not open mailbox %d!\n", getpid(), h2_mbox);
    Exit();
  }
  if (mbox_open(n2_mbox) == MBOX_FAIL) {
    Printf("makeprocs (%d): Could not open mailbox %d!\n", getpid(), n2_mbox);
    Exit();
  }
  if (mbox_open(o2_mbox) == MBOX_FAIL) {
    Printf("makeprocs (%d): Could not open mailbox %d!\n", getpid(), o2_mbox);
    Exit();
  }
  if (mbox_open(hno3_mbox) == MBOX_FAIL) {
    Printf("makeprocs (%d): Could not open mailbox %d!\n", getpid(), hno3_mbox);
    Exit();
  }
  // Setup the command-line arguments for the new process.  We're going to
  // pass the handles to the shared memory page and the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  //ditoa(h_mem, h_mem_str);
  ditoa(s_procs_completed, s_procs_completed_str);
  ditoa(no_mbox, no_mbox_str);
  ditoa(n2_mbox, n2_mbox_str);
  ditoa(h2o_mbox, h2o_mbox_str);
  ditoa(h2_mbox, h2_mbox_str);
  ditoa(o2_mbox, o2_mbox_str);
  ditoa(no_, no_str);
  ditoa(h2o, h2o_str);
  ditoa(hno3_mbox, hno3_mbox_str);
  ditoa(maxCount, maxCount_str);

  // Now we can create the proesses.  Note that you MUST end your call to
  // process_create with a NULL argument so that the operating system
  // knows how many arguments you are sending.
  for(i=0; i<no_; i++) {
    process_create(FILENAME_NO,0,0, s_procs_completed_str, no_mbox_str, NULL);
    // Printf("makeprocs (%d): Process %d created\n", getpid(), i);
  }
  for(i=0; i<h2o; i++) {
    process_create(FILENAME_H2O,0,0, s_procs_completed_str, h2o_mbox_str, NULL);
    // Printf("makeprocs (%d): Process %d created\n", getpid(), i);
  }
  for(i=0; i<no_/2; i++) {
    process_create(FILENAME_REACT1,0,0, s_procs_completed_str, no_mbox_str, n2_mbox_str, o2_mbox_str, NULL);
    // Printf("makeprocs (%d): Process %d created\n", getpid(), i);
  }
  for(i=0; i<h2o/2; i++) {
    process_create(FILENAME_REACT2,0,0, s_procs_completed_str, h2o_mbox_str, h2_mbox_str, o2_mbox_str, NULL);
    // Printf("makeprocs (%d): Process %d created\n", getpid(), i);
  }
  for(i=0; i<maxCount; i++) {
    process_create(FILENAME_REACT3,0,0, s_procs_completed_str, h2_mbox_str, n2_mbox_str, o2_mbox_str, hno3_mbox_str, NULL);
    // Printf("makeprocs (%d): Process %d created\n", getpid(), i);
  }

  // And finally, wait until all spawned processes have finished.
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_open(no_mbox) == MBOX_FAIL) {
    Printf("spawn_me (%d): Could not open the mailbox!\n", getpid());
    Exit();
  }
  if (mbox_open(h2o_mbox) == MBOX_FAIL) {
    Printf("spawn_me (%d): Could not open the mailbox!\n", getpid());
    Exit();
  }
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
  Printf("Remain: %d NO, %d N2, %d H2O, %d H2, %d O2, %d HNO3\n", no_2, n2, h2o2, h2, o2, (2 * maxCount));
}
