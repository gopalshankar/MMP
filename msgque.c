
/* Might go in misc.c
 *
 * Message Queue Multicast Implementation
 * CS551 Group 21  
 * 
 * Multicast Message Queue Kernel Functions and Structures
 * 
 */

#include <msgque.h>
#include <msgque_const.h>
#include <libmsgque.h> /* so that you can read MsgQue members here */

PRIVATE struct MQueues mQueues_;

PUBLIC int do_minit(message *m)
{
	int token = 0;
	int firstFreeQueue = -1;
	MsgQue *user_mq;
	
	printf("\nCS551 I am inside minit!\n");
	
	/* Read token and MsgQue */
	token = m_in.m1_i1;
	sys_datacopy(who, (virbytes) m_in.m1_p1, SELF, (virbytes) user_mq, sizeof(MsgQue) );
	
	/* Check if already exists MQueue with such token 
	 * If Yes, return MQueue address in MsgQueue->queue 
	 * This happens ussually when reciever comes in 
	 */
	 enterCriticalSection();
	 for( i=0; i< MQ_MAX_MSGQUES ; i++) {
		if( mQueues_[i].token == i ) {
			user_mq->queue = &mQueue_[i];
			sys_datacopy(SELF, (virbytes) user_mq, who, (virbytes)  m_in.m1_p1, sizeof(MsgQue) );
			return( MQ_SUCCESS );
		}
		if( mQueues_[i].token = -1 && firstFreeQueue == -1 )
			firstFreeQueue = i;
	 }
	
	/* Error if there are no free queues */
	if( firstFreeQueue == -1 )
		return ( MQ_ERR_MAX_MSGQUE );
		
	/* This is new request so, give him a new MQueue */
	mQueue_[ firstFreeQueue ].token = token;
	insertUser( mQueue_[i], who );
	leaveCriticalSection();
	
	user_mq->queue = &mQueue_[i];
	sys_datacopy(SELF, (virbytes) user_mq, who, (virbytes)  m_in.m1_p1, sizeof(MsgQue) );
	
	return MQ_SUCCESS;
}

PUBLIC int do_msend(message *m)
{
	int len;
	MsgQue *user_mq;
	char *message;
	printf("\nCS551 I am inside msend!\n");
	
	/* Read token and MsgQue */
	len = m_in.m1_i1;
	sys_datacopy(who, (virbytes) m_in.m1_p1, SELF, (virbytes) user_mq, sizeof(MsgQue) );
	sys_datacopy(who, (virbytes) m_in.m1_p2, SELF, (virbytes) message, len );
	
	/* Validate that such token/queue exists */
	MQueue *mq = user_mq->queue;
	if( INVALID_MQ( mq, user_mq->token ) )
		return ( ERR_INVALID_MQ );
	
	/* Block if MQueue is FULL */
	if( mq->queueLen == MQ_MAX_MESSAGES ) {
		 enterCriticalSection();
		 setUserProperty( mq, MQ_SENDER, MQ_USER_BLOCKED );
		 leaveCriticalSection();
		 
		 /* Block sender */
		 pause();
		  
		 /* Add message to mq->MsgNode */
		 enterCriticalSection();
		 removeSender( mq, who );
		 setUserProperty( mq, MQ_SENDER, MQ_USER_ACTIVE );
		 leaveCriticalSection();
		 return MQ_SUCCESS;
	}
		
	/* Add message to mq->MsgNode */
	enterCriticalSection();
	insertMessage( mq, message );
	leaveCriticalSection();
	
	/* Wake-up Recievers if they are sleeping */
	if( mq->userHead ) {
		enterCriticalSection();
		MQUser *user = mq->userHead;
		while( user && user->type==MQ_RECIEVER && user->state=MQ_USER_BLOCKED ) {
			unpause( user->procNr ); /* Wake-up */
			user = user->next;
		}
		leaveCriticalSection();
	}
	
	return MQ_SUCCESS;
}

PUBLIC int do_mrecv(message *m)
{
	int len;
	MsgQue *user_mq;
	char *message;
	printf("\nCS551 I am inside mrecv!\n");
	
	/* Read token and MsgQue */
	len = m_in.m1_i1;
	sys_datacopy(who, (virbytes) m_in.m1_p1, SELF, (virbytes) user_mq, sizeof(MsgQue) );

	/* Validate that such token/queue exists */
	MQueue *mq = user_mq->queue;
	if( INVALID_MQ( mq, user_mq->token ))
		return ( ERR_INVALID_MQ );
	
	/* Block if MQueue is EMPTY */
	if( mq->MsgNode == NULL ) {
		 enterCriticalSection();
		 setUserProperty( mq, MQ_RECIEVER, MQ_USER_BLOCKED );
		 leaveCriticalSection();
		 
		 /* Block Reciever */
		 pause();
		  
		 enterCriticalSection();
		 setUserProperty( mq, MQ_RECIEVER, MQ_USER_ACTIVE );
		 leaveCriticalSection();
	}
	
	/* Read Message */
	enterCriticalSection();
	readMessage( mq, message );
	leaveCriticalSection();
	
	/* Wake-up Sender if they are sleeping */
	if( mq->shead ) {
		enterCriticalSection();
		MQUser *user = mq->userHead;
		while( user && user->type==MQ_SENDER && user->state=MQ_USER_BLOCKED ) {
			unpause( user->procNr ); /* Wake-up */
			user = user->next;
		}
		leaveCriticalSection();
	}
	
	return MQ_SUCCESS;
}

PUBLIC int do_mclose(message *m)
{
	MsgQue *user_mq;
	printf("\nCS551 I am inside mclose!\n");
	
	/* Read MsgQue */
	token = m_in.m1_i1;
	sys_datacopy(who, (virbytes) m_in.m1_p1, SELF, (virbytes) user_mq, sizeof(MsgQue) );
	
	/* Validate that such token/queue exists */
	MQueue *mq = user_mq->queue;

	if( INVALID_MQ( mq, user_mq->token ) || mq.userHead == NULL )
		return ( ERR_INVALID_MQ );
	
	/* Remove element from userHead */
	enterCriticalSection();
	removeUser( mq, who );
	if( mq.userHead == NULL ) { /* free the Message Queue */
		mq.token = MQ_FREE;
		mq.queueLen = 0; 
		removeAllMessages( mq );
	}
	leaveCriticalSection();
	
	return MQ_SUCCESS;
}

PUBLIC int do_mclean(message *m)
{
	MsgQue *user_mq;
	int rc;

	printf("\nCS551 I am inside mclean!\n");
	
	/* Read MsgQue */
	token = m_in.m1_i1;
	sys_datacopy(who, (virbytes) m_in.m1_p1, SELF, (virbytes) user_mq, sizeof(MsgQue) );
	
	/* Validate that such token/queue exists */
	MQueue *mq = user_mq->queue;
	if( INVALID_MQ( mq, user_mq->token ) || mq.userHead == NULL )
		return ( ERR_INVALID_MQ );
		
	/* Ping every one and see if they are all alive */
	MQUser *user = mq->userHead;
	while( user ) {
		rc = kill( user->proc_nr, 0 ); /* Not sure which API to use */
		if( rc != 0 ) {
			removeUser( mq, user->proc_nr );
		}
	}
	
	/* If there are active users mq->userHead will not be NULL */
	if( mq->userHead )
		return( ERR_MQ_INUSE );
		
	/* Free the MQueue */
	enterCriticalSection();
	mq.token = MQ_FREE;
	mq.queueLen = 0; 
	removeAllMessages( mq );
	enterCriticalSection();
	
	return MQ_SUCCESS;
}


PRIVATE int insertUser( MQueue *mq, int proc_nr) {
	
}


PRIVATE int removeUser( MQueue *mq, int proc_nr ) {
	
}

/* This is not supposed to fail */
PRIVATE int setUserProperty( MQueue *mq, int type, int state ) {
	
	MQUser *user = mq->userHead;
	while( user ) 	{
		if( user->proc_nr == who ) {
			user->type = type;
			user->state = state;
		}
	}
	return MQ_SUCCESS;
}

PRIVATE int insertMessage( MQueue *mq, char *message ) {
	if( mq->queueLen == MQ_MAX_MESSAGES )
		return (MQ_ERR_MAX_MESSAGES)
		
	enterCriticalSection();
	mq->queueLen++;		
	leaveCriticalSection();
	
	/* Logic to add message */
}

PRIVATE int readReciever( MQueue *mq ) {
	/* if you are the last reciever in the queue 
	 * then delete the element from MsgNode */
}

/* Called by readReciever */
PRIVATE int removeMessage( MQueue *mq ) {
	
	enterCriticalSection();
	mq->queueLen--;		
	leaveCriticalSection();
	
	
}


PRIVATE int enterCriticalSection( ) { /* takes argument - indicating different semaphore */
	
}

PRIVATE int leaveCriticalSection( ) {
}

PRIVATE int cleanOnTimer( ) { /* Need to call this periodically using timer callback */

}
