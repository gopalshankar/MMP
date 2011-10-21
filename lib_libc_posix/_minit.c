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

#include <minix/libmsgque.h>


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
PUBLIC int minit( token, msgque  )
int token;
struct MsgQue *msgque;
{
  message m;

  m.m1_i1 = token;
  m.m1_p1 = (void*) msgque;
  return(_syscall(PM_PROC_NR, MINIT, &m));
}
