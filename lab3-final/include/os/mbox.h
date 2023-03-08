#ifndef __MBOX_OS__
#define __MBOX_OS__

#define MBOX_NUM_MBOXES 16           // Maximum number of mailboxes allowed in the system
#define MBOX_NUM_BUFFERS 50          // Maximum number of message buffers allowed in the system
#define MBOX_MAX_BUFFERS_PER_MBOX 10 // Maximum number of buffer slots available to any given mailbox
#define MBOX_MAX_MESSAGE_LENGTH 50   // Buffer size of 50 for each message

#define MBOX_FAIL -1
#define MBOX_SUCCESS 1

//---------------------------------------------
// Define your mailbox structures here
//--------------------------------------------

typedef struct mbox_message {
    char message[MBOX_MAX_MESSAGE_LENGTH];  // message
    int msg_size;                           // size of message
    uint32	inuse;                          // check if mbox_message is inuse
} mbox_message;

typedef struct mbox {
    Queue messages;                         // message queue
    lock_t m_lock;                          // lock
    cond_t is_room;                         // is_room CondVar
    cond_t is_msg;                          // is_msg CondVar
    uint32	inuse;                          // check if mbox is inuse
    int procceses[PROCESS_MAX_PROCS];       // array of procceses
} mbox;

typedef int mbox_t; // This is the "type" of mailbox handles

//-------------------------------------------
// Prototypes for Mbox functions you have to write
//-------------------------------------------

void MboxModuleInit();
mbox_t MboxCreate();
int MboxOpen(mbox_t m);
int MboxClose(mbox_t m);
int MboxSend(mbox_t m, int length, void *message);
int MboxRecv(mbox_t m, int maxlength, void *message);
int MboxCloseAllByPid(int pid);

#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

#endif
