#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "synch.h"
#include "queue.h"
#include "mbox.h"

// arrays for mboxes and mbox messages

static mbox mboxs[MBOX_NUM_MBOXES];
static mbox_message mbox_messages[MBOX_NUM_BUFFERS];


//-------------------------------------------------------
//
// void MboxModuleInit();
//
// Initialize all mailboxes.  This process does not need
// to worry about synchronization as it is called at boot
// time.  Only initialize necessary items here: you can
// initialize others in MboxCreate.  In other words,
// don't waste system resources like locks and semaphores
// on unused mailboxes.
//
//-------------------------------------------------------

void MboxModuleInit() {
  int i; // Loop Index variable
  dbprintf ('p', "MboxModuleInit: Entering MboxModuleInit\n");
  for (i=0; i<MBOX_NUM_MBOXES; i++) {
    mboxs[i].inuse = 0;
  }
  for (i=0; i<MBOX_NUM_BUFFERS; i++) {
    mbox_messages[i].inuse = 0;
  }
  dbprintf ('p', "MboxModuleInit: Leaving MboxModuleInit\n");
}

//-------------------------------------------------------
//
// mbox_t MboxCreate();
//
// Allocate an available mailbox structure for use.
//
// Returns the mailbox handle on success
// Returns MBOX_FAIL on error.
//
//-------------------------------------------------------
mbox_t MboxCreate() {
  mbox_t mbox;
  uint32 intrval;
  int i;

  intrval = DisableIntrs();
  for (mbox=0; mbox < MBOX_NUM_MBOXES; mbox++) {
    if(mboxs[mbox].inuse==0) {
      mboxs[mbox].inuse = 1;
      break;
    }
  }
  RestoreIntrs(intrval);
  if(mbox==MBOX_NUM_MBOXES) return MBOX_FAIL;

  // Initialize Lock
  if ((mboxs[mbox].m_lock = LockCreate()) == SYNC_FAIL) {
    printf("Bad lock create in MboxCreate()");
    exitsim();
  }

  // Initialize CondVars
  if ((mboxs[mbox].is_room = CondCreate(mboxs[mbox].m_lock)) == SYNC_FAIL) {
    printf("Bad cond in MboxCreate()");
    exitsim();
  }
  if ((mboxs[mbox].is_msg = CondCreate(mboxs[mbox].m_lock)) == SYNC_FAIL) {
    printf("Bad cond in MboxCreate()");
    exitsim();
  }

  // Initialize Queue
  if (AQueueInit (&mboxs[mbox].messages) != QUEUE_SUCCESS) {
    printf("FATAL ERROR: could not initialize semaphore waiting queue in MboxCreate()!\n");
    exitsim();
  }

  // Initialize Process Array
  for (i=0; i < PROCESS_MAX_PROCS; i++) {
    mboxs[mbox].procceses[i] = 0;
  }
  
  return mbox;
}

//-------------------------------------------------------
//
// void MboxOpen(mbox_t);
//
// Open the mailbox for use by the current process.  Note
// that it is assumed that the internal lock/mutex handle
// of the mailbox and the inuse flag will not be changed
// during execution.  This allows us to get the a valid
// lock handle without a need for synchronization.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxOpen(mbox_t handle) {

  // Check for Valid Handle
  if (handle < 0) return MBOX_FAIL;
  if (handle >= MBOX_NUM_MBOXES) return MBOX_FAIL;
  if (!mboxs[handle].inuse) return MBOX_FAIL;

  // Acquire Lock
  if ((LockHandleAcquire(mboxs[handle].m_lock)) == SYNC_FAIL) {
    printf("FATAL ERROR: could not acquire lock in MboxOpen()!\n");
    exitsim();
  }

  // Add the current process
  mboxs[handle].procceses[GetCurrentPid()] = 1;

  // Release Lock
  if ((LockHandleRelease(mboxs[handle].m_lock)) == SYNC_FAIL) {
    printf("FATAL ERROR: could not release lock in MboxOpen()!\n");
    exitsim();
  }

  return MBOX_SUCCESS;
}

//-------------------------------------------------------
//
// int MboxClose(mbox_t);
//
// Close the mailbox for use to the current process.
// If the number of processes using the given mailbox
// is zero, then disable the mailbox structure and
// return it to the set of available mboxes.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxClose(mbox_t handle) {

  int i;
  int no_processes;
  Link *l;

  // Check for Valid Handle
  if (handle < 0) return MBOX_FAIL;
  if (handle >= MBOX_NUM_MBOXES) return MBOX_FAIL;
  if (!mboxs[handle].inuse) return MBOX_FAIL;

  // Acquire Lcok
  if ((LockHandleAcquire(mboxs[handle].m_lock)) == SYNC_FAIL) {
    printf("FATAL ERROR: could not acquire lock in MboxClose()!\n");
    exitsim();
  }

  // Remove the current process
  mboxs[handle].procceses[GetCurrentPid()] = 0;

  // Check if there are no other processes
  no_processes = 1;
  for (i = 0; i < PROCESS_MAX_PROCS; i++) {
    if (mboxs[handle].procceses[i] == 1) {
      no_processes = 0;
      break;
    }
  }

  // Return mailbox to available mailboxes
  if (no_processes) {
    while (!AQueueEmpty(&mboxs[handle].messages)) {
      l = AQueueFirst(&mboxs[handle].messages);
      AQueueRemove(&l);
    }
    mboxs[handle].inuse = 0;
  }

  // Release Lock
  if ((LockHandleRelease(mboxs[handle].m_lock)) == SYNC_FAIL) {
    printf("FATAL ERROR: could not release lock in MboxClose()!\n");
    exitsim();
  }

  return MBOX_SUCCESS;
}

//-------------------------------------------------------
//
// int MboxSend(mbox_t handle,int length, void* message);
//
// Send a message (pointed to by "message") of length
// "length" bytes to the specified mailbox.  Messages of
// length 0 are allowed.  The call
// blocks when there is not enough space in the mailbox.
// Messages cannot be longer than MBOX_MAX_MESSAGE_LENGTH.
// Note that the calling process must have opened the
// mailbox via MboxOpen.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxSend(mbox_t handle, int length, void* message) {

  uint32 intrval;
  int message_handle;
  Link *l;

  // Check for Valid Handle
  if (handle < 0) return MBOX_FAIL;
  if (handle >= MBOX_NUM_MBOXES) return MBOX_FAIL;
  if (!mboxs[handle].inuse) return MBOX_FAIL;

  // Acquire the lock
  if ((LockHandleAcquire(mboxs[handle].m_lock)) == SYNC_FAIL) {
    printf("FATAL ERROR: could not acquire lock in MboxSend()!\n");
    exitsim();
  }

  // Check if process has the mailbox open
  if (mboxs[handle].procceses[GetCurrentPid()] == 0) {
    return MBOX_FAIL;
  }

  // Check message length
  if (length <= 0 || length > MBOX_MAX_MESSAGE_LENGTH) {
    return MBOX_FAIL;
  }

  // Check cond
  if (AQueueLength(&mboxs[handle].messages) >= MBOX_MAX_BUFFERS_PER_MBOX) {
    if (CondHandleWait(mboxs[handle].is_room) != SYNC_SUCCESS) {
      printf("Bad conditional is_room in MboxCreate()!\n");
      exitsim();
    }    
  }

  // disableIntrs()
  intrval = DisableIntrs();

  // Get free mbox message buffer
  for (message_handle=0; message_handle < MBOX_NUM_BUFFERS; message_handle++) {
    if(mbox_messages[message_handle].inuse==0) {
      mbox_messages[message_handle].inuse = 1;
      break;
    }
  }
  
  // Store message to mbox message array
  bcopy(message, mbox_messages[message_handle].message, length);
  mbox_messages[message_handle].msg_size = length;
  mbox_messages[message_handle].inuse = 1;

  // RestoreIntrs()
  RestoreIntrs(intrval);

  // Insert message to queue
  if ((l = AQueueAllocLink(&mbox_messages[message_handle])) == NULL) {
    printf("FATAL ERROR: could not allocate link for message queue in MboxSend()!\n");
    exitsim();
  }

  AQueueInsertLast(&mboxs[handle].messages, l);

  // condSignal(notEmpty)
  if (CondHandleSignal(mboxs[handle].is_msg) != SYNC_SUCCESS) {
    printf("Bad conditional is_msg in MboxSend()!\n");
    exitsim();
  }

  // Release the lock
  if ((LockHandleRelease(mboxs[handle].m_lock)) == SYNC_FAIL) {
    printf("FATAL ERROR: could not release lock in MboxSend()!\n");
    exitsim();
  }

  return MBOX_SUCCESS;
}

//-------------------------------------------------------
//
// int MboxRecv(mbox_t handle, int maxlength, void* message);
//
// Receive a message from the specified mailbox.  The call
// blocks when there is no message in the buffer.  Maxlength
// should indicate the maximum number of bytes that can be
// copied from the buffer into the address of "message".
// An error occurs if the message is larger than maxlength.
// Note that the calling process must have opened the mailbox
// via MboxOpen.
//
// Returns MBOX_FAIL on failure.
// Returns number of bytes written into message on success.
//
//-------------------------------------------------------
int MboxRecv(mbox_t handle, int maxlength, void* message) {

  Link *l;
  mbox_message *m;

  // Check for Valid Handle
  if (handle < 0) return MBOX_FAIL;
  if (handle >= MBOX_NUM_MBOXES) return MBOX_FAIL;
  if (!mboxs[handle].inuse) return MBOX_FAIL;
  
  // Acquire the lock
  if ((LockHandleAcquire(mboxs[handle].m_lock)) == SYNC_FAIL) {
    printf("FATAL ERROR: could not acquire lock in MboxRecv()!\n");
    exitsim();
  }

  // Check if process has the mailbox open
  if (mboxs[handle].procceses[GetCurrentPid()]  == 0) {
    return MBOX_FAIL;
  }

  // Check input message length

  //????

  // Check cond;
  if (AQueueEmpty(&mboxs[handle].messages)) { 
    if (CondHandleWait(mboxs[handle].is_msg) != SYNC_SUCCESS) {
      printf("Bad conditional is_msg in MboxRecv()!\n");
      exitsim();
    }     
  }

  // Get the message from message queue
  l = AQueueFirst(&mboxs[handle].messages);
  m = (mbox_message *) l->object;

  if (m->msg_size > maxlength) {
    printf("FATAL ERROR: size of message is greater than the maximum length in MboxRecv()!\n");
    return MBOX_FAIL;
  }

  // Copy message
  bcopy(m->message, message, m->msg_size);

  // Remove message from queue
  AQueueRemove(&l);

  // condSignal(notFull)
  if (CondHandleSignal(mboxs[handle].is_room) != SYNC_SUCCESS) {
    printf("Bad conditional is_room in MboxRecv()!\n");
    exitsim();
  }

  // Release the lock
  if ((LockHandleRelease(mboxs[handle].m_lock)) == SYNC_FAIL) {
    printf("FATAL ERROR: could not release lock in MboxRecv()!\n");
    exitsim();
  }

  return m->msg_size;
}

//--------------------------------------------------------------------------------
//
// int MboxCloseAllByPid(int pid);
//
// Scans through all mailboxes and removes this pid from their "open procs" list.
// If this was the only open process, then it makes the mailbox available.  Call
// this function in ProcessFreeResources in process.c.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//--------------------------------------------------------------------------------
int MboxCloseAllByPid(int pid) {

  int mbox;

  for (mbox=0; mbox < MBOX_NUM_MBOXES; mbox++) {
    MboxClose(mbox);
  }

  return MBOX_SUCCESS;
}
