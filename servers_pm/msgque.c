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

PRIVATE MQueue mQueue_[MQ_MAX_MSGQUES];



PRIVATE int cleanOnTimer( ) { /* Need to call this periodically using timer callback */
	return 0;
}

PRIVATE int insertUser( MQueue *mq, int proc_nr) {
	return 0;
}


PRIVATE int removeUser( MQueue *mq, int proc_nr ) {
	return 0;
}

/* This is not supposed to fail */
PRIVATE int setUserProperty( MQueue *mq, int type, int state ) {
	
	MQUser *user = mq->userHead;
	while( user ) 	{
		if( user->proc_nr == who_p ) {
			user->type = type;
			user->state = state;
		}
		user = user->next;
	}
	return MQ_SUCCESS;
}

PRIVATE int insertMessage( MQueue *mq, char *message ) {
	if( mq->queueLen == MQ_MAX_MESSAGES )
		return (MQ_ERR_MAX_MESSAGES);

	mq->queueLen= mq->queueLen + 1;		
	
	/* Logic to add message */
	return MQ_SUCCESS;
}

PRIVATE int readReciever( MQueue *mq ) {
	/* if you are the last reciever in the queue 
	 * then delete the element from MsgNode */
	return MQ_SUCCESS;
}

/* Called by readReciever */
PRIVATE int removeMessage( MQueue *mq ) {

	mq->queueLen--;		
	return MQ_SUCCESS;	
}

PUBLIC int do_minit(message *m)
{
	int token = 0;
	int firstFreeQueue = -1;
	MsgQue *user_mq;
	
	printf("\nCS551 I am inside minit!\n");
	
	/* Read token and MsgQue */
	token = m_in.m1_i1;
	sys_datacopy(who_p, (virbytes) m_in.m1_p1, SELF, (virbytes) user_mq, sizeof(MsgQue) );
	
	/* Check if already exists MQueue with such token 
	 * If Yes, return MQueue address in MsgQueue->queue 
	 * This happens ussually when reciever comes in 
	 */
	 for( i=0; i< MQ_MAX_MSGQUES ; i++) {
		if( mQueues_[i].token == i ) {
			user_mq->queue = &mQueue_[i];
			sys_datacopy(SELF, (virbytes) user_mq, who_p, (virbytes)  m_in.m1_p1, sizeof(MsgQue) );
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
	insertUser( &mQueue_[i], who_p );
	
	user_mq->queue = &mQueue_[i];
	sys_datacopy(SELF, (virbytes) user_mq, who_p, (virbytes)  m_in.m1_p1, sizeof(MsgQue) );
	
	return MQ_SUCCESS;
}

PUBLIC int do_msend(message *m)
{
	int len;
	MsgQue *user_mq;
	char *message;
	MQueue *mq;
	printf("\nCS551 I am inside msend!\n");
	
	/* Read token and MsgQue */
	len = m_in.m1_i1;
	sys_datacopy(who_p, (virbytes) m_in.m1_p1, SELF, (virbytes) user_mq, sizeof(MsgQue) );
	sys_datacopy(who_p, (virbytes) m_in.m1_p2, SELF, (virbytes) message, len );
	
	/* Validate that such token/queue exists */
	mq = user_mq->queue;
	if( INVALID_MQ( mq, user_mq->token ) )
		return ( ERR_INVALID_MQ );
	
	/* Block if MQueue is FULL */
	if( mq->queueLen == MQ_MAX_MESSAGES ) {
		 setUserProperty( mq, MQ_SENDER, MQ_USER_BLOCKED );
		 
		 /* Block sender */
		 pause();
		  
		 /* Add message to mq->MsgNode */
		 removeSender( mq, who_p );
		 setUserProperty( mq, MQ_SENDER, MQ_USER_ACTIVE );
		 return MQ_SUCCESS;
	}
		
	/* Add message to mq->MsgNode */
	insertMessage( mq, message );
	
	/* Wake-up Recievers if they are sleeping */
	if( mq->userHead ) {
		MQUser *user = mq->userHead;
		while( user && user->type==MQ_RECIEVER && user->state=MQ_USER_BLOCKED ) {
			unpause( user->proc_nr ); /* Wake-up */
			user = user->next;
		}
	}
	
	return MQ_SUCCESS;
}

PUBLIC int do_mrecv(message *m)
{
	int len;
	MsgQue *user_mq;
	char *message;
	MQueue *mq;
	printf("\nCS551 I am inside mrecv!\n");
	
	/* Read token and MsgQue */
	len = m_in.m1_i1;
	sys_datacopy(who_p, (virbytes) m_in.m1_p1, SELF, (virbytes) user_mq, sizeof(MsgQue) );

	/* Validate that such token/queue exists */
	mq = user_mq->queue;
	if( INVALID_MQ( mq, user_mq->token ))
		return ( ERR_INVALID_MQ );
	
	/* Block if MQueue is EMPTY */
	if( mq->MsgNode == NULL ) {
		 setUserProperty( mq, MQ_RECIEVER, MQ_USER_BLOCKED );
		 
		 /* Block Reciever */
		 pause();
		  
		 setUserProperty( mq, MQ_RECIEVER, MQ_USER_ACTIVE );
	}
	
	/* Read Message */
	readMessage( mq, message );
	
	/* Wake-up Sender if they are sleeping */
	if( mq->shead ) {
		MQUser *user = mq->userHead;
		while( user && user->type==MQ_SENDER && user->state=MQ_USER_BLOCKED ) {
			unpause( user->proc_nr ); /* Wake-up */
			user = user->next;
		}
	}
	
	return MQ_SUCCESS;
}

PUBLIC int do_mclose(message *m)
{
	MsgQue *user_mq;
	MQueue *mq;
	printf("\nCS551 I am inside mclose!\n");
	
	/* Read MsgQue */
	token = m_in.m1_i1;
	sys_datacopy(who_p, (virbytes) m_in.m1_p1, SELF, (virbytes) user_mq, sizeof(MsgQue) );
	
	/* Validate that such token/queue exists */
	mq = user_mq->queue;

	if( INVALID_MQ( mq, user_mq->token ) || mq.userHead == NULL )
		return ( ERR_INVALID_MQ );
	
	/* Remove element from userHead */
	removeUser( mq, who_p );
	if( mq.userHead == NULL ) { /* free the Message Queue */
		mq.token = MQ_FREE;
		mq.queueLen = 0; 
		removeAllMessages( mq );
	}
	
	return MQ_SUCCESS;
}

PUBLIC int do_mclean(message *m)
{
	MsgQue *user_mq;
	int rc;
	MQueue *mq;
	MQUser *user;

	printf("\nCS551 I am inside mclean!\n");
	
	/* Read MsgQue */
	token = m_in.m1_i1;
	sys_datacopy(who_p, (virbytes) m_in.m1_p1, SELF, (virbytes) user_mq, sizeof(MsgQue) );
	
	/* Validate that such token/queue exists */
	mq = user_mq->queue;
	if( INVALID_MQ( mq, user_mq->token ) || mq.userHead == NULL )
		return ( ERR_INVALID_MQ );
		
	/* Ping every one and see if they are all alive */
	user = mq->userHead;
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
	mq.token = MQ_FREE;
	mq.queueLen = 0; 
	removeAllMessages( mq );
	
	return MQ_SUCCESS;
}

