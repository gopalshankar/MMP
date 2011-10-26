/* Might go in misc.c
 *
 * Message Queue Multicast Implementation
 * CS551 Group 21  
 * 
 * Multicast Message Queue Kernel Functions and Structures
 * 
 */

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
#include "msgque.h"

struct MQueue mQueues_[MQ_MAX_MSGQUES];

struct timer mq_timer; /* watchdog timer for cleanup operations */


PUBLIC void init_all_msg_queues( void ) {
	int i;
	
	for( i=0; i< MQ_MAX_MSGQUES; i++ )
	{
		mQueues_[i].token = MQ_FREE;
		mQueues_[i].msgCounter = 0;
		mQueues_[i].msgHead = NULL;
		mQueues_[i].msgTail = NULL;
		mQueues_[i].userHead = NULL;
	}
}

PRIVATE void cleanOnTimer(struct timer *tp ) { /* Need to call this periodically using timer callback */
	int i, rc;
	struct MQueue *mq;
	struct MQUser *user;
	struct mproc *rmp;
	printf("\nCS551 I am inside cleanOnTimer"); /*remove once tested */
  	/* cleanup all existing message queues*/
	for( i=0; i< MQ_MAX_MSGQUES; i++ )
	{
		if (mQueues_[i].token == MQ_FREE)
			continue;
		mq=&mQueues_[i];
		/* Ping every one and see if they are all alive */
		user = mq->userHead;
		while( user ) {
			rmp = &mproc[ user->proc_nr ];
			/* need to get process PID here */
			printf("Checking pid=%d\n", rmp->mp_pid);
			/*rc = kill( rmp->mp_pid, 0 );*/ /* Not sure which API to use */
			if( rmp->mp_pid == 0 ) 
				removeUser( mq, user->proc_nr );
			user = user->next;
		}
	}
	
  	set_timer(tp, MQ_CLEANUP_TIMER, cleanOnTimer, mproc[PM_PROC_NR].mp_endpoint); /*restart timer*/

	return;
}

/* Any user calling minit() will be queued here */
PRIVATE int insertUser( struct MQueue *mq, int proc_nr) {

	/* Create New Node */
	struct MQUser *newUser;
	
	if(mq==NULL)
		return MQ_FAILED;	
	
	newUser = (struct MQUser*) malloc( sizeof(struct MQUser) );
	if(newUser == NULL)
		return ERR_MALLOC_FAIL;
	newUser->messageNo = mq->msgCounter;
	newUser->proc_nr = proc_nr;
	newUser->state = MQ_USER_ACTIVE;
	newUser->type = MQ_RECIEVER; /* read only when state is MQ_USER_BLOCKED */

	printf("\nCS551 DBG: insertUser");

	/* Add to list */
	printf("\nCS551 DBG: insertUser head %x %d", mq->userHead, proc_nr);
	newUser->next = mq->userHead;
	mq->userHead = newUser;

	return MQ_SUCCESS;
}

/* First args NULL, causes search in all Q */
PRIVATE struct MQUser* getUserPtr( struct MQueue *mq, int proc_nr ) {
	struct MQUser *tmp;

	printf("\nCS551 DBG: getUserPtr %x %d", mq, proc_nr);

	tmp = mq->userHead;
	while( tmp ) {
		printf("\nCS551 DBG: getUserPtr %x %d", tmp, tmp->proc_nr);
		if( tmp->proc_nr == proc_nr ) {
		    printf("\nCS551 DBG: getUserPtr Found %x %d", tmp, proc_nr);
		    return tmp; 
		}
		tmp = tmp->next;
	}
	printf("\nCS551 DBG: getUserPtr Should NOT COME HERE %x %d", mq, proc_nr);

	return NULL; /* never reaches here */
}

/* OLD DEFINITION - WILL MORE IT LATTER */
PRIVATE struct MQUser* getUserPtrOld( struct MQueue *mq, int proc_nr ) {
	struct MQueue *q;
	struct MQUser *tmp;
	int i;

	printf("\nCS551 DBG: getUserPtr %x %d", mq, proc_nr);

	for(i=0; i<MQ_MAX_MSGQUES; i++) {
		/* Skip unused Q */
		if( mQueues_[i].token == -1 ) continue;

		/* Search only in mq (from args) */
		if( mq == NULL )
			q = &mQueues_[i];
		else
			q = mq; /* Search in all Q (args=NULL) */

		/* Search in i'th Q */
		tmp = q->userHead;
		while( tmp ) {
	printf("\nCS551 DBG: getUserPtr %x %d", tmp, tmp->proc_nr);
			if( tmp->proc_nr == proc_nr ) 
			{printf("\nCS551 DBG: getUserPtr Found %x %d", tmp, proc_nr);
			    return tmp; }
			tmp = tmp->next;
		}
		if( mq ) break; 
	}
	printf("\nCS551 DBG: getUserPtr Should NOT COME HERE %x %d", mq, proc_nr);

	return NULL; /* never reaches here */
}

/* If there are no users */
PRIVATE unsigned long getMinUserMessageNo( struct MQueue *mq ) {
	

	unsigned long minMsgNo = 0xFFFF;
	struct MQUser *user;

	if(mq==NULL)
		return minMsgNo;
		
	user = mq->userHead;

	printf("\nCS551 DBG: getMinUserMessageNo");

	while( user ) 	{
		if( minMsgNo > user->messageNo )
			minMsgNo = user->messageNo;
		user = user->next;
	}
	return minMsgNo;
}

PRIVATE void setUserMessageNo( struct MQueue *mq, int proc_nr, int msgNo ) {
	
	struct MQUser *user;
	
	if(mq==NULL)
		return;
	
	user = mq->userHead;

	printf("\nCS551 DBG: setUserMessageNo");

	while( user ) 	{
		if( user->proc_nr == proc_nr ) {
			user->messageNo = msgNo;
			return;
		}
		user = user->next;
	}
}

PRIVATE int insertMessage( struct MQueue *mq, void *message, int len ) {


	/* Create New Node */
	struct MsgNode *newMN;
		
	if((mq==NULL)||(message==NULL)||(len<=0))
		return MQ_FAILED;
	
	newMN = (struct MsgNode*) malloc( sizeof(struct MsgNode) );
	if(newMN==NULL)
		return ERR_MALLOC_FAIL;
		
	mq->msgCounter++;
	newMN->next = NULL;
	newMN->len = len;
	newMN->message = message;
	newMN->messageNo = mq->msgCounter;

	printf("\nCS551 DBG: insertMessage: %20s", (char*)newMN->message);

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

	unsigned long minMsgNo;
	struct MsgNode *tmp;
	
	if(mq==NULL)
		return MQ_FAILED;
	
	tmp = mq->msgHead;

	printf("\nCS551 DBG: truncateQueue");

	minMsgNo = getMinUserMessageNo(mq);
	while( tmp && tmp->messageNo <= minMsgNo ) {
	printf("\nCS551 DBG: truncateQueue removing msg %d", tmp->messageNo);	
			mq->msgHead = tmp->next;
			mq->queueLen--;	
			free( tmp->message );
			free( tmp );
			tmp = mq->msgHead;
	}
	if( mq->msgHead == NULL ) mq->msgTail = NULL;

	return MQ_SUCCESS;	
}

/* Any user calling mclose() will do below */
PRIVATE void removeUser( struct MQueue *mq, int proc_nr ) {
	struct MQUser *tmp;
	struct MQUser *prev = NULL;

	if(mq==NULL)
		return;
		
	tmp = mq->userHead;
	printf("\nCS551 DBG: removeUser");

	while( tmp ) {
		if( tmp->proc_nr == proc_nr )
		{
			if( prev == NULL )
				mq->userHead = tmp->next;
			else
				prev->next = tmp->next;
			free( tmp );
	
			if( mq->userHead == NULL ) { 
				truncateQueue( mq );
				mq->token = MQ_FREE;
				mq->queueLen = 0; 
				mq->msgCounter = 0;
				mq->msgHead = NULL;
				mq->msgTail = NULL;
				mq->userHead = NULL;
			}
			return;
		}
		prev = tmp;
		tmp = tmp->next;
	}
}


/* Called by mrecv() when message already present in Queue 
 * Called by msend() when unblocking reciever
 */
PUBLIC int readMessage( struct MQueue *mq, struct MQUser *qUser ) {
	struct MsgNode *msgNode;
	int bytesToCopy;
	int len = qUser->args.m1_i1;

	printf("\nCS551 DBG: readMessage");
	msgNode=mq->msgHead;

	/* search for next message in queue */

	/* Next message to read by next mrecv() 
	 * if this is last message in queue
	 * qUser->messageNo+1 == mq->msgCounter */
	qUser->messageNo++; 
	while( msgNode && msgNode->messageNo != qUser->messageNo ) {
		msgNode = msgNode->next;
	}

	/* Copy message to user buffer */
	bytesToCopy = (len > msgNode->len)? msgNode->len: len;
	sys_datacopy(SELF, (vir_bytes) msgNode->message, qUser->who_e, (vir_bytes) qUser->args.m1_p2, bytesToCopy );
	truncateQueue( mq );

	if (bytesToCopy == msgNode->len)
		return MQ_SUCCESS;
	else
		return MQ_TRUNCATED;

}

/* called by msend when Queue has space
 * called by mrecv when unblocking sender.
 */
PUBLIC int putInQueue( struct MQueue *mq, struct MQUser *qUser) {	
	int rc;
 	int len;
	void *sendMsg;

	len = qUser->args.m1_i1;
	sendMsg = (void*) malloc( len );
	if (sendMsg==NULL)
		return ERR_MALLOC_FAIL;
	
	/* Add message to mq->MsgNode */
	sys_datacopy(qUser->who_e, (vir_bytes) qUser->args.m1_p2, SELF, (vir_bytes) sendMsg, len );
	rc = insertMessage( mq, sendMsg, len );

	return rc;	
}


PUBLIC int do_minit(void)
{
	int token;
	int i, rc;
	static struct MsgQue user_mq;
	struct MQUser *qUser;
	int firstFreeQueue ;
	static int mq_init = 0;
	
	if(m_in.m1_p1==NULL)
		return ERR_INVALID_ARG;
		
	firstFreeQueue = MQ_FREE;

	if(mq_init == 0)
	{
		init_all_msg_queues();
		init_timer(&mq_timer); /* timer init happens only once */
		set_timer(&mq_timer, MQ_CLEANUP_TIMER, cleanOnTimer, mproc[PM_PROC_NR].mp_endpoint);
		mq_init = 1;
	}

	/* Read token and MsgQue */
	token = m_in.m1_i1;
	printf("\nCS551 DBG: do_minit() %d, %d %x", who_p, token, m_in.m1_p1);
		
	sys_datacopy(who_e, (vir_bytes) m_in.m1_p1, SELF, (vir_bytes) &user_mq, sizeof(struct MsgQue) );
	
	/* Check if already exists MQueue with such token 
	 * If Yes, return MQueue address in MsgQueue->queue 
	 * This happens ussually when reciever comes in 
	 */
	 for( i=0; i< MQ_MAX_MSGQUES ; i++) {
		if( mQueues_[i].token == token ) {
			/* Is this 2nd time minit on this token */
			qUser = getUserPtr( NULL, who_p );
			if (qUser!=NULL)
				return ERR_INVALID_ARG; /* ERR_INIT_TIWCE todo*/

			/* Register this user */
			rc = insertUser( &mQueues_[i], who_p );
			if (rc != MQ_SUCCESS) return rc;

			user_mq.queue = &mQueues_[i];
			user_mq.token = token;
			sys_datacopy(SELF, (vir_bytes) &user_mq, who_e, (vir_bytes)  m_in.m1_p1, sizeof(struct MsgQue) );

			return( MQ_SUCCESS );

		}
		if( mQueues_[i].token == MQ_FREE && firstFreeQueue == MQ_FREE )
			firstFreeQueue = i;
	 }
	
	/* Error if there are no free queues */
	if( firstFreeQueue == MQ_FREE )
		return ( MQ_ERR_MAX_MSGQUE );
		
	/* This is new request so, give him a new MQueue */
	mQueues_[ firstFreeQueue ].token = token;
	rc = insertUser( &mQueues_[ firstFreeQueue ], who_p );
	if (rc != MQ_SUCCESS)
		return rc;
	printf("\nCS551 DBG: end do_minit() %d, %d %x", who_p, token, m_in.m1_p1);
	
	user_mq.queue = &mQueues_[ firstFreeQueue ];
	user_mq.token = token;
	sys_datacopy(SELF, (vir_bytes) &user_mq, who_e, (vir_bytes)  m_in.m1_p1, sizeof(struct MsgQue) );
	
	return MQ_SUCCESS;
}

PUBLIC int do_msend(void)
{
	int len, rc;
	static struct MsgQue user_mq;
	void *sendMsg;
	struct MQueue *mq;
	struct MQUser *qUser;
	struct MQUser *user;
	struct mproc *rmp;


	printf("\nCS551 DBG: do_msend %d", who_p);

	if((m_in.m1_i1<=0)||(m_in.m1_p1==NULL)||(m_in.m1_p2==NULL))
		return ERR_INVALID_ARG;
		
	/* Read token and MsgQue */
	len = m_in.m1_i1;
	sys_datacopy(who_e, (vir_bytes) m_in.m1_p1, SELF, (vir_bytes) &user_mq, sizeof(struct MsgQue) );
	
	/* Validate that such token/queue exists */
	mq = user_mq.queue;
	if( INVALID_MQ( mq, user_mq.token ) )
		return ( ERR_INVALID_MQ );

	/* Get User Details */	
	qUser = getUserPtr( mq, who_p );
	qUser->who_e = who_e;
	qUser->args = m_in;

	/* Block if MQueue is FULL */
	if( mq->queueLen == MQ_MAX_MESSAGES ) {
		 free(sendMsg);
		 /* Block sender */
		 rmp = &mproc[ who_p ];
		 rmp->mp_flags |= WAITING;
		 qUser->type= MQ_SENDER;
		 qUser->state= MQ_USER_BLOCKED;
 	 	 qUser->args = m_in;
		 return (SUSPEND); 
	}
	
	/* There is no one else who called minit() 
	 * Messages are ignored, if there is no one else to read it.
	 */
	if( mq->userHead->next == NULL )
		return MQ_ERR_NO_RECIEVERS ;
	
	rc = putInQueue( mq, qUser);

	/* Wake-up Recievers if they are sleeping */
	user = mq->userHead;
	while( user ) {
		printf("\nCS551 DBG: loop unblock %d", user->state);
		if( user->type==MQ_RECIEVER && user->state==MQ_USER_BLOCKED ){
			printf("\nCS551 DBG: unblock rcv do_msend %d", user->proc_nr);
			rmp = &mproc[ user->proc_nr ];
			rmp->mp_flags &= ~WAITING;
			
			user->state=MQ_USER_ACTIVE;
			
			rc = readMessage( mq, user );
			setreply(user->proc_nr, rc ); 
		}
		user = user->next;
	}
	
	return rc;
}

PUBLIC int do_mrecv(void)
{
	int len,rc;
	static struct MsgQue user_mq;
	struct MQueue *mq;
	struct MQUser *qUser;
	struct MQUser *user;
	struct mproc *rmp;
	struct MsgNode *msgNode;
	int bytesToCopy ;

	printf("\nCS551 DBG: do_mrecv %d", who_p);

	if((m_in.m1_i1<=0)||(m_in.m1_p1==NULL)||(m_in.m1_p2==NULL))
		return ERR_INVALID_ARG;
		
	printf("\nCS551 DBG: do_mrecv %d TEST2", who_p);

	/* Read token and MsgQue */
	len = m_in.m1_i1;
	sys_datacopy(who_e, (vir_bytes) m_in.m1_p1, SELF, (vir_bytes) &user_mq, sizeof(struct MsgQue) );

	printf("\nCS551 DBG: do_mrecv %d TEST3", who_p);
	/* Validate that such token/queue exists */
	mq = user_mq.queue;
	if( INVALID_MQ( mq, user_mq.token ))
		return ( ERR_INVALID_MQ );
	
	/* Get User Details */	
	qUser = getUserPtr( mq, who_p );
	qUser->args = m_in;
	qUser->who_e = who_e;

	/* Block if no new message arrived */
	if( mq->msgCounter == qUser->messageNo ) {
		
		/* If there are no messages in Queue, reset mq->msgCounter & all user->messageNo = 0 */
		if( qUser->messageNo == getMinUserMessageNo( mq ) ) {
			mq->msgCounter = 0;
			user = mq->userHead;
			while( user ) {
				user->messageNo = 0;
				user = user->next;
			}
		}

		 printf("\nCS551 DBG: do_mrecv %d TEST4 blocking", who_p);
		 /* Block Reciever */
		 rmp = &mproc[ who_p ];
		 rmp->mp_flags |= WAITING;
		 qUser->type= MQ_RECIEVER;
		 qUser->state= MQ_USER_BLOCKED;
		 
		 return (SUSPEND); 
	}

	printf("\nCS551 DBG: readMessage");
	rc = readMessage( mq, qUser );
	
	/* Wake-up Sender if they are sleeping */
	user = mq->userHead;
	while(( user )&&(mq->queueLen < MQ_MAX_MESSAGES )) {
		printf("\nCS551 DBG: do_mrecv loop %d", user->state);
		if( user->type==MQ_SENDER && user->state==MQ_USER_BLOCKED ) { 
			printf("\nCS551 DBG: do_mrecv unblocking sender %d", user->proc_nr);
			rmp = &mproc[ user->proc_nr ];
			rmp->mp_flags &= ~WAITING;

			user->state=MQ_USER_ACTIVE;

			rc = putInQueue( mq, user );
			setreply(user->proc_nr,  rc); 
		}
		user = user->next;
	}

	return rc;
}


PUBLIC int do_mclose(void)
{
	static struct MsgQue user_mq;
	struct MQueue *mq;

	printf("\nCS551 DBG: do_mclose %d", who_p);
	
	if(m_in.m1_p1==NULL)
		return ERR_INVALID_ARG;
	/* Read MsgQue */
	sys_datacopy(who_e, (vir_bytes) m_in.m1_p1, SELF, (vir_bytes) &user_mq, sizeof(struct MsgQue) );
	
	/* Validate that such token/queue exists */
	mq = user_mq.queue;

	if( INVALID_MQ( mq, user_mq.token ) || mq->userHead == NULL )
		return ( ERR_INVALID_MQ );
	
	/* Remove element from userHead */
	removeUser( mq, who_p );
	
	return MQ_SUCCESS;
}

PUBLIC int do_mclean(void)
{
	static struct MsgQue user_mq;
	int rc;
	struct MQueue *mq;
	struct MQUser *user;
	struct mproc *rmp;

	printf("\nCS551 DBG: do_mclean");
	
	if(m_in.m1_p1==NULL)
		return ERR_INVALID_ARG;
		
	/* Read MsgQue */
	sys_datacopy(who_e, (vir_bytes) m_in.m1_p1, SELF, (vir_bytes) &user_mq, sizeof(struct MsgQue) );
	
	/* Validate that such token/queue exists */
	mq = user_mq.queue;
	if( INVALID_MQ( mq, user_mq.token ) || mq->userHead == NULL )
		return ( ERR_INVALID_MQ );
		
	/* Ping every one and see if they are all alive */
	user = mq->userHead;
	while( user ) {
		rmp = &mproc[ user->proc_nr ];
		/* need to get process PID here */
		printf("Checking pid=%d\n", rmp->mp_pid);
		/*rc = kill( rmp->mp_pid, 0 );*/ /* Not sure which API to use */
		if( rmp->mp_pid == 0 ) 
			removeUser( mq, user->proc_nr );
		user = user->next;
	}
	
	/* If there are active users mq->userHead will not be NULL */
	if( mq->userHead )
		return( ERR_MQ_INUSE );
		
	/* Free the MQueue */
	truncateQueue( mq );
	mq->token = MQ_FREE;
	mq->queueLen = 0; 
	mq->msgCounter = 0;
	mq->msgHead = NULL;
	mq->msgTail = NULL;
	mq->userHead = NULL;	
	return MQ_SUCCESS;
}

