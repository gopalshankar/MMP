
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
	if( mq < &mQueue[0] || mq > &mQueue[MQ_MAX_MSGQUES] || mq.token != user_mq->token )
		return ( ERR_INVALID_MQ );
	
	/* Block if MQueue is FULL */
	if( mq->queueLen == MQ_MAX_MESSAGES ) {
		 enterCriticalSection();
		 insertSender( mq, who );
		 leaveCriticalSection();
		 
		 /* Block sender */
		 pause();
		  
		 /* Add message to mq->MsgNode */
		 enterCriticalSection();
		 removeSender( mq, who );
		 insertMessage( mq, message );
		 leaveCriticalSection();
		 return MQ_SUCCESS;
	}
		
	/* Add message to mq->MsgNode */
	enterCriticalSection();
	insertMessage( mq, message );
	leaveCriticalSection();
	
	/* Wake-up Recievers if they are sleeping */
	if( mq->rhead ) {
		enterCriticalSection();
		MQReciever *recv = mq->rhead;
		while( recv ) {
			removeReciever( mq, next->procNr );
			unpause( next->procNr ); /* Wake-up */
			recv = recv->next;
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
	if( mq < &mQueue[0] || mq > &mQueue[MQ_MAX_MSGQUES] || mq.token != user_mq->token )
		return ( ERR_INVALID_MQ );
	
	/* Block if MQueue is EMPTY */
	if( mq->MsgNode == NULL ) {
		 enterCriticalSection();
		 insertReciever( mq, who );
		 leaveCriticalSection();
		 
		 /* Block Reciever */
		 pause();
		 
		 enterCriticalSection();
		 removeReciever( mq, who );
		 leaveCriticalSection();
	}
	
	/* Read Message */
	enterCriticalSection();
	readMessage( mq, message );
	leaveCriticalSection();
	
	/* Wake-up Sender if they are sleeping */
	if( mq->shead ) {
		enterCriticalSection();
		MQSender *sender = mq->shead;
		while( sender ) {
			removeSender( mq, next->procNr );
			unpause( next->procNr ); /* Wake-up */
			sender = sender->next;
		}
		leaveCriticalSection();
	}
	
	return MQ_SUCCESS;
}

PUBLIC int do_mclose(message *m)
{
	printf("\nCS551 I am inside mclose!\n");
	
	
	return MQ_SUCCESS;
}

PUBLIC int do_mclean(message *m)
{
	printf("\nCS551 I am inside mclean!\n");
	
	
	return MQ_SUCCESS;
}


PRIVATE int insertSender( MQueue *mq, int proc_nr) {
	
}


PRIVATE int removeSender( MQueue *mq, int proc_nr ) {
	
}

PRIVATE int insertReciever( MQueue *mq, int proc_nr ) {
	
}

PRIVATE int removeReciever( MQueue *mq, int proc_nr ) {
	
}

PRIVATE int insertMessage( MQueue *mq, char *message ) {
	if( mq->queueLen == MQ_MAX_MESSAGES )
		return (MQ_ERR_MAX_MESSAGES)
		
	enterCriticalSection();
	mq->queueLen++;		
	leaveCriticalSection();
	
	/* Logic to add message */
}

PRIVATE int readReciever( MQueue *mq, char *message ) {
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
