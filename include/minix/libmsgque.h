/* 
 * Message Queue Multicast Implementation
 * CS551 Group 21  
 * 
 * Multicast Message Queue User syscall's 
 * and structures associated with it
 */
 
struct MsgQue {
	int token; /* This identifies the Message Queue uniquely
			    * Multiple applications can use same key to 
				* operate of single queue in multicast mode */
				
	void *queue; /* This is pointer used by kernel to point to 
				  * kernel message queue data structure, to avoid
				  * token look-up every time for all API's.
				  * msend() without minit() scenarios can 
				  * check if *queue is invalid and return */
				
};


