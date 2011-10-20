/* 
 * Message Queue Multicast Implementation
 * CS551 Group 21  
 * 
 * Multicast Message Queue User syscall's 
 * and structures associated with it
 */
 
PUBLIC struct MsgQue {
	int token; /* This identifies the Message Queue uniquely
			    * Multiple applications can use same key to 
				* operate of single queue in multicast mode */
				
	void *queue; /* This is pointer used by kernel to point to 
				  * kernel message queue data structure, to avoid
				  * token look-up every time for all API's.
				  * msend() without minit() scenarios can 
				  * check if *queue is invalid and return */
				
}

/* Function prototypes used by MsgQue Applications 
 * See libmsgque.c for more comments on these syscall's
 */
PUBLIC int minit( int key, struct MsgQue * );
PUBLIC int msend( struct MsgQue *, void *message, int len );
PUBLIC int mrecv( struct MsgQue *, void *message, int len );
PUBLIC int mclose( struct MsgQue * );
PUBLIC int mclean( struct MsgQue * );

