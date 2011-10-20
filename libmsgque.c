/* Message Queue Multicast Implementation
 * CS551 Group 21  
 * 
 * 
 * Message Queue User Library Interface function
 * definitions are here. These definitions go into
 * libmsend.so, which is expected to be linked with 
 * user executable. 
 */
#include <lib.h>
#include <unistd.h>

#include <libmsend.h>
#include <msgque_const.h>


/* Application callable interface functions
 * 
 * Return Values
 * 0 = Success
 * -N = Error Code - See msgque_const.h
 */


/* Applications should call this function 
 * to get a message queue or to register to 
 * already created message queue.
 */
PUBLIC int minit( token, msgque * )
int token;
struct MsgQue msgque *;
{
  message m;

  m.m1_i1 = token;
  m.m1_p1 = (void*) msgque;
  return(_syscall(PM_PROC_NR, MINIT, &m));
}

/* This API is used to send message to PM
 */
PUBLIC int msend( *msgque, *message, len )
struct MsgQue *msgque;
void *message;
int len;
{
  message m;

  m.m1_i1 = len;
  m.m1_p1 = (void*) msgque;
  m.m1_p2 = (void*) message;
  return(_syscall(PM_PROC_NR, MSEND, &m));
}

/* This API is used to recieve message from PM
 */
PUBLIC int mrecv( *msgque , *message, len )
struct MsgQue *msgque;
void *message;
int len;
{
  message m;

  m.m1_i1 = len;
  m.m1_p1 = (void*) msgque;
  m.m1_p2 = (void*) message;
  return(_syscall(PM_PROC_NR, MRECV, &m));
}

/* This API is used to de-register from particular
 * message queue. It does not destroy the message queue, 
 * as there could be other users using the message queue.
 * The last user calling mclose() will cause message queue
 * to be destroyed.
 */
PUBLIC int mclose( msgque * )
struct MsgQue *msgque;
{
  message m;

  m.m1_i1 = token;
  m.m1_p1 = (void*) msgque;
  return(_syscall(PM_PROC_NR, MCLOSE, &m));
}

/* This API is used to clean-up message queue resource 
 * at PM, if and only if there are NO active processes that
 * have registered the message queue.
 */
PUBLIC int mclean( msgque * )
struct MsgQue *msgque;
{
  message m;

  m.m1_i1 = msgque;
  return(_syscall(PM_PROC_NR, MCLEAN, &m));
}
