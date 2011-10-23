/* Might go in misc.c
 *
 * Message Queue Multicast Implementation
 * CS551 Group 21  
 * 
 * Multicast Message Queue Kernel Functions and Structures
 * 
 */

#include "msgque.h"
#include "pm.h"
#include <minix/callnr.h>
#include <signal.h>
#include <sys/svrctl.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <minix/com.h>
#include <minix/config.h>
#include <minix/sysinfo.h>
#include <minix/type.h>
#include <minix/vm.h>
#include <string.h>
#include <machine/archtypes.h>
#include <lib.h>
#include <assert.h>
#include "mproc.h"
#include "param.h"

struct MQueue mQueues_[MQ_MAX_MSGQUES];

struct timer mq_timer; /* watchdog timer for cleanup operations */


/* Should be called when PM starts boots */
PUBLIC void init_all_msg_queues( void ) {
	int i;

	for( i=0; i< MQ_MAX_MSGQUES; i++ )
	{
		mQueues_[i].token = -1;
		mQueues_[i].msgCounter = 0;
		mQueues_[i].msgHead = NULL;
		mQueues_[i].msgTail = NULL;
		mQueues_[i].userHead = NULL;
	}
}

PRIVATE void cleanOnTimer(struct timer *tp ) { /* Need to call this periodically using timer callback */
	printf("\nCS551 I am inside cleanOnTimer!\n"); /*remove once tested */
  	/* cleanup all existing message queues*/
  	set_timer(tp, MQ_CLEANUP_TIMER, cleanOnTimer, mproc[PM_PROC_NR].mp_endpoint); /*restart timer*/

	return ;
}

/* Any user calling minit() will be queued here */
PRIVATE int insertUser( struct MQueue *mq, int proc_nr) {


	/* Create New Node */
	struct MQUser *newUser = (struct MQUser*) malloc( sizeof(struct MQUser) );
	newUser->messageNo = mq->msgCounter;
	newUser->proc_nr = proc_nr;
	newUser->state = MQ_USER_BLOCKED;
	newUser->type = MQ_RECIEVER; /* read only when state is MQ_USER_BLOCKED */

	printf("\nCS551 DBG: insertUser\n");

	/* Add to list */
	newUser->next = mq->userHead;
	mq->userHead = newUser;

	return MQ_SUCCESS;
}

/* Any user calling mclose() will do below */
PRIVATE void removeUser( struct MQueue *mq, int proc_nr ) {
	struct MQUser *tmp = mq->userHead;
	struct MQUser *prev = NULL;

	printf("\nCS551 DBG: removeUser\n");

	while( tmp ) {
		if( tmp->proc_nr == proc_nr )
		{
			if( prev == NULL )
				mq->userHead = tmp->next;
			else
				prev->next = tmp->next;

			free( tmp );
			return;
		}
		prev = tmp;
		tmp = tmp->next;
	}
}

PRIVATE struct MQUser* getUserPtr( struct MQueue *mq, int proc_nr ) {
	struct MQUser *tmp = mq->userHead;

	printf("\nCS551 DBG: getUserPtr\n");

	while( tmp ) {
		if( tmp->proc_nr == proc_nr ) return tmp;
		tmp = tmp->next;
	}

	return NULL; /* never reaches here */
}

/* If there are no users */
PRIVATE int getMinUserMessageNo( struct MQueue *mq ) {
	

	int minMsgNo = 0xFFFF;
	struct MQUser *user = mq->userHead;

	printf("\nCS551 DBG: getMinUserMessageNo\n");

	while( user ) 	{
		if( minMsgNo > user->messageNo )
			minMsgNo = user->messageNo;
		user = user->next;
	}
	return minMsgNo;
}

PRIVATE void setUserMessageNo( struct MQueue *mq, int proc_nr, int msgNo ) {
	
	struct MQUser *user = mq->userHead;

	printf("\nCS551 DBG: setUserMessageNo\n");

	while( user ) 	{
		if( user->proc_nr == proc_nr ) {
			user->messageNo = msgNo;
			return;
		}
		user = user->next;
	}
}

PRIVATE void setUserProperty( struct MQueue *mq, int type, int state ) {
	
	struct MQUser *user = mq->userHead;

	printf("\nCS551 DBG: setUserProperty\n");

	while( user ) 	{
		if( user->proc_nr == who_e ) {
			user->type = type;
			user->state = state;
			return;
		}
		user = user->next;
	}
}

PRIVATE int insertMessage( struct MQueue *mq, char *message, int len ) {


	/* Create New Node */
	struct MsgNode *newMN = (struct MsgNode*) malloc( sizeof(struct MsgNode) );
	newMN->next = NULL;
	newMN->message = message;
	newMN->messageNo = mq->msgCounter;
	mq->msgCounter++;

	printf("\nCS551 DBG: insertMessage\n");

	/* Add to list */
	if( mq->msgHead == NULL ) {
		mq->msgHead = newMN;
		mq->msgTail = newMN;
	} else {
		mq->msgTail->next = newMN;	
		mq->msgTail = newMN;
	}
	mq->queueLen= mq->queueLen + 1;		
	
	return MQ_SUCCESS;
}

/* This procedure will check if some elements are not
 * required by any recievers and then remove such messages 
 */
PRIVATE int truncateQueue( struct MQueue *mq ) {

	int minMsgNo;
	struct MsgNode *tmp = mq->msgHead;

	printf("\nCS551 DBG: truncateQueue\n");

	minMsgNo = getMinUserMessageNo(mq);
	while( tmp && tmp->messageNo < minMsgNo ) {
			mq->msgHead = tmp->next;
			mq->queueLen--;		
			free( tmp );
			tmp = mq->msgHead;
	}
	if( mq->msgHead == NULL ) mq->msgTail = NULL;

	return MQ_SUCCESS;	
}

PUBLIC int do_minit(void)
{
	int token;
	int i;
	struct MsgQue user_mq;
	int firstFreeQueue ;
	static int mq_timer_init = 0;
	
	firstFreeQueue = -1;
	printf("\nCS551 DBG: do_minit()\n");

	if(mq_timer_init == 0)
	{
		printf("\nCS551 DBG: do_minit(): init_timer\n");	
		init_timer(&mq_timer); /* timer init happens only once */
		printf("\nCS551 DBG: do_minit(): set_timer\n");	
		set_timer(&mq_timer, MQ_CLEANUP_TIMER, cleanOnTimer, mproc[PM_PROC_NR].mp_endpoint);
		mq_timer_init = 1;
	}

	/* Read token and MsgQue */
	printf("\nCS551 DBG: do_minit(): token=%d\n",m_in.m1_i1);	
	token = m_in.m1_i1;
	printf("\nCS551 DBG: do_minit(): MsgQue=0x%x\n",m_in.m1_p1);	
	sys_datacopy(who_e, (vir_bytes) m_in.m1_p1, SELF, (vir_bytes) &user_mq, sizeof(struct MsgQue) );
	
	/* Check if already exists MQueue with such token 
	 * If Yes, return MQueue address in MsgQueue->queue 
	 * This happens ussually when reciever comes in 
	 */
	 for( i=0; i< MQ_MAX_MSGQUES ; i++) {
		if( mQueues_[i].token == token ) {
			user_mq.queue = &mQueues_[i];
			sys_datacopy(SELF, (vir_bytes) &user_mq, who_e, (vir_bytes)  m_in.m1_p1, sizeof(struct MsgQue) );
			return( MQ_SUCCESS );
		}
		if( mQueues_[i].token = -1 && firstFreeQueue == -1 )
			firstFreeQueue = i;
	 }
	
	/* Error if there are no free queues */
	if( firstFreeQueue == -1 )
		return ( MQ_ERR_MAX_MSGQUE );
		
	/* This is new request so, give him a new MQueue */
	mQueues_[ firstFreeQueue ].token = token;
	insertUser( &mQueues_[ firstFreeQueue ], who_e );
	
	user_mq.queue = &mQueues_[ firstFreeQueue ];
	sys_datacopy(SELF, (vir_bytes) &user_mq, who_e, (vir_bytes)  m_in.m1_p1, sizeof(struct MsgQue) );
	
	return MQ_SUCCESS;
}

PUBLIC int do_msend(void)
{
	int len;
	struct MsgQue *user_mq;
	char *message;
	struct MQueue *mq;

	printf("\nCS551 DBG: do_msend\n");
	
	/* Read token and MsgQue */
	len = m_in.m1_i1;
	sys_datacopy(who_e, (vir_bytes) m_in.m1_p1, SELF, (vir_bytes) user_mq, sizeof(struct MsgQue) );
	
	/* Validate that such token/queue exists */
	mq = user_mq->queue;
	if( INVALID_MQ( mq, user_mq->token ) )
		return ( ERR_INVALID_MQ );
	
	/* Block if MQueue is FULL */
	if( mq->queueLen == MQ_MAX_MESSAGES ) {
		 setUserProperty( mq, MQ_SENDER, MQ_USER_BLOCKED );
		 
		 /* Block sender */
		 pause();
		  
		 setUserProperty( mq, MQ_SENDER, MQ_USER_ACTIVE );
		 return MQ_SUCCESS;
	}
		
	/* Add message to mq->MsgNode */
	message = (char*) malloc( len );
	sys_datacopy(who_e, (vir_bytes) m_in.m1_p2, SELF, (vir_bytes) message, len );
	setUserMessageNo( mq, who_e, mq->msgCounter ); /* send is reciever THINK */
	insertMessage( mq, message, len );
	
	/* Wake-up Recievers if they are sleeping */
	if( mq->userHead ) {
		struct MQUser *user = mq->userHead;
		while( user && user->type==MQ_RECIEVER && user->state==MQ_USER_BLOCKED ) {
			/*unpause( user->proc_nr );  Wake-up */
			user = user->next;
		}
	}
	
	return MQ_SUCCESS;
}

PUBLIC int do_mrecv(void)
{
	int len;
	char *message;
	struct MsgQue *user_mq;
	struct MQueue *mq;
	struct MQUser *qUser;
	struct MsgNode *msgNode;
	int bytesToCopy ;

	printf("\nCS551 DBG: do_mrecv\n");
	
	/* Read token and MsgQue */
	len = m_in.m1_i1;
	sys_datacopy(who_e, (vir_bytes) m_in.m1_p1, SELF, (vir_bytes) user_mq, sizeof(struct MsgQue) );

	/* Validate that such token/queue exists */
	mq = user_mq->queue;
	if( INVALID_MQ( mq, user_mq->token ))
		return ( ERR_INVALID_MQ );
	
	/* Block if no new message arrived */
	qUser = getUserPtr( mq, who_e );
	if( mq->msgCounter == qUser->messageNo ) {
		 setUserProperty( mq, MQ_RECIEVER, MQ_USER_BLOCKED );
		 
		 /* Block Reciever */
		 pause();
		  
		 setUserProperty( mq, MQ_RECIEVER, MQ_USER_ACTIVE );
	}

	printf("\nCS551 DBG: readMessage\n");
	msgNode=mq->msgHead;

	/* search for next message in queue */
	while( msgNode->messageNo == qUser->messageNo ) {
		msgNode = msgNode->next;
	}

	/* Copy message to user buffer */
	bytesToCopy = (len > msgNode->len)? msgNode->len: len;
	sys_datacopy(SELF, (vir_bytes) msgNode->message, qUser->proc_nr, (vir_bytes) m_in.m1_p2, bytesToCopy );

	/* Next message to read by next mrecv() 
	 * if this is last message in queue
	 * qUser->messageNo+1 == mq->msgCounter */
	qUser->messageNo++; 
	truncateQueue( mq );
	
	/* Wake-up Sender if they are sleeping */
	if( mq->userHead ) {
		struct MQUser *user = mq->userHead;
		while( user && user->type==MQ_SENDER && user->state==MQ_USER_BLOCKED ) {
			/*unpause( user->proc_nr );  Wake-up */
			user = user->next;
		}
	}
	
	return MQ_SUCCESS;
}


PUBLIC int do_mclose(void)
{
	int token;
	struct MsgQue *user_mq;
	struct MQueue *mq;

	printf("\nCS551 DBG: do_mclose\n");
	
	/* Read MsgQue */
	token = m_in.m1_i1;
	sys_datacopy(who_e, (vir_bytes) m_in.m1_p1, SELF, (vir_bytes) user_mq, sizeof(struct MsgQue) );
	
	/* Validate that such token/queue exists */
	mq = user_mq->queue;

	if( INVALID_MQ( mq, user_mq->token ) || mq->userHead == NULL )
		return ( ERR_INVALID_MQ );
	
	/* Remove element from userHead */
	removeUser( mq, who_e );
	if( mq->userHead == NULL ) { /* free the Message Queue */
		mq->token = MQ_FREE;
		mq->queueLen = 0; 
		truncateQueue( mq );
	}
	
	return MQ_SUCCESS;
}

PUBLIC int do_mclean(void)
{
	struct MsgQue *user_mq;
	int token,rc;
	struct MQueue *mq;
	struct MQUser *user;

	printf("\nCS551 DBG: do_mclean\n");
	
	/* Read MsgQue */
	token = m_in.m1_i1;
	sys_datacopy(who_e, (vir_bytes) m_in.m1_p1, SELF, (vir_bytes) user_mq, sizeof(struct MsgQue) );
	
	/* Validate that such token/queue exists */
	mq = user_mq->queue;
	if( INVALID_MQ( mq, user_mq->token ) || mq->userHead == NULL )
		return ( ERR_INVALID_MQ );
		
	/* Ping every one and see if they are all alive */
	user = mq->userHead;
	while( user ) {
		/* need to get process PID here */
		rc = kill( user->proc_nr, 0 ); /* Not sure which API to use */
		if( rc != 0 ) {
			removeUser( mq, user->proc_nr );
		}
	}
	
	/* If there are active users mq->userHead will not be NULL */
	if( mq->userHead )
		return( ERR_MQ_INUSE );
		
	/* Free the MQueue */
	mq->token = MQ_FREE;
	mq->queueLen = 0; 
	truncateQueue( mq );
	
	return MQ_SUCCESS;
}

