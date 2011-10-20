/* 
 * Message Queue Multicast Implementation
 * CS551 Group 21  
 * 
 * Multicast Message Queue Kernel Functions and Structures
 * 
 */
 #include <libmsgque.h>


/* This structure hold receivers who are waiting 
 * for messages to arrive
 */
struct MQReciever {   /* Dynamically allocated */
	int messageNo; 			 /* This is the last message recieve used */
	int procNr: 			 /* Receiver's process number */
	struct MQReciever *next; /* If many are waiting to read message */
}

/* This structure hold senders who are waiting 
 * for messages to be removed and queue gets freed.
 */
struct MQSender {   /* Dynamically allocated */
	int procNr: 			/* Sender's process number */
	struct MQSender *next;  /* If many are waiting to write message */
}

 /* 
  * Linked list maintaining all the message posted by msend
  * 
  * Elements are removed by mrecv() if there are no more
  * recievers pending to read the message.
  * 
  * There exist maximum of MQ_MAX_MESSAGES MsgNode per MQueue.
  * 
  */
struct MsgNode {
	int messageNo;
	char *message;   
	struct MsgNode *next;
}
  
struct MQueue {	  
	int token;	/* Unique identifier for this message queue, user gives this */
	int queueLen;
	struct MsgNode *mhead; 

	struct MQSender *shead;
	struct MQReciever *rhead;
}
 
 /* 
  * struct MsgQues holds list of all message queue that are 
  * created by applications.
  * 
  * Data structure that is initialized by PM when it starts.
  * queues[*].token should be initialized with MQ_FREE, if its free.
  * 
  * There can only be MQ_MAX_MSGQUES created in Minix
  * 
  * &queue[*].token with MQ_FREE is returned to user in MsgQue->queue by
  * minit(). Future calls to msend(), mrecv() and mclose() will
  * use MsgQue->queue to get its MQueue.
  * 
  */
 
 struct MQueues {
	 struct MQueue queues[MQ_MAX_MSGQUES]; /* 16 * MQ_MAX_MSGQUES bytes */
 }


/* might go into proto.h
 * 
 * These functions handle message queue operations at PM.
 * See msgque.c for more comments on these syscall's
 */
 
_PROTOTYPE( int do_minit, (message *mess_ptr));
_PROTOTYPE( int do_msend, (message *mess_ptr));
_PROTOTYPE( int do_mrecv, (message *mess_ptr));
_PROTOTYPE( int do_mclose, (message *mess_ptr));
_PROTOTYPE( int do_mclean, (message *mess_ptr));


/* might go in table.h
 * 
	do_minit,		/* 65 = unused	*
	do_mrecv,		/* 67 = unused	*
	do_msend,		/* 66 = unused  *
	do_mclose,		/* 68 = unused	*
	do_mclean,	    /* 69 = unused  */

 

