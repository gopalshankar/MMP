/* 
 * Message Queue Multicast Implementation
 * CS551 Group 21  
 * 
 * Multicast Message Queue User syscall's 
 * and structures associated with it
 */
 
 /* Return codes */
#define MQ_SUCCESS 0
#define MQ_FAILED -1
#define MQ_ERR_MAX_MSGQUE   -2
#define MQ_ERR_MAX_MESSAGES -3
#define MQ_ERR_MAX_RECIEVERS -4
#define ERR_INVALID_MQ -5
#define ERR_MQ_INUSE -6

PUBLIC struct MsgQue {
	int token; /* This identifies the Message Queue uniquely
			    * Multiple applications can use same key to 
				* operate of single queue in multicast mode */
				
	void *queue; /* This is pointer used by kernel to point to 
				  * kernel message queue data structure, to avoid
				  * token look-up every time for all API's.
				  * msend() without minit() scenarios can 
				  * check if *queue is invalid and return */
				
};


