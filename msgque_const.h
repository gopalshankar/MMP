

/* MESSAGE QUEUE LIMITS 
 * 
 * Note: can add more system calls and make these 
 * limits user defined at some level
 * 
 */

#define MQ_MAX_MSGQUES 128
#define MQ_MAX_MESSAGES 512
#define MQ_MAX_BYTES_IN_MSG 256 /* Single msg que can hold 512 * 256 bytes */


/* OTHERS */
#define MQ_FREE 0xFFFF
#define MQ_SUCCESS 0
#define MQ_FAILED -1

/* Error codes */

#define MQ_ERR_MAX_MSGQUE   -2
#define MQ_ERR_MAX_MESSAGES -3
#define MQ_ERR_MAX_RECIEVERS -4
#define ERR_INVALID_MQ -5
