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


/* This API is used to clean-up message queue resource 
 * at PM, if and only if there are NO active processes that
 * have registered the message queue.
 */
PUBLIC int mclean( struct MsgQue *msgque )
{
  message m;

  m.m1_p1 = (void *)msgque;
  return(_syscall(PM_PROC_NR, MCLEAN, &m));
}
