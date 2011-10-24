

/* MESSAGE QUEUE LIMITS 
 * 
 * Note: can add more system calls and make these 
 * limits user defined at some level
 * 
 */

#define MQ_MAX_MSGQUES 128
#define MQ_MAX_MESSAGES 512
#define MQ_MAX_BYTES_IN_MSG 256 /* Single msg que can hold 512 * 256 bytes */
#define MQ_CLEANUP_TIMER 500 /* number of ticks after which cleanup is to be run */


/* OTHERS */
#define MQ_FREE 0xFFFF
#define MQ_USER_BLOCKED 1
#define MQ_USER_ACTIVE 2
#define MQ_SENDER 3
#define MQ_RECIEVER 4

