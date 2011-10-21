
/* CS551 Group 21  
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


/* This API is used to de-register from particular
 * message queue. It does not destroy the message queue, 
 * as there could be other users using the message queue.
 * The last user calling mclose() will cause message queue
 * to be destroyed.
 */
PUBLIC int mclose( msgque )
struct MsgQue *msgque;
{
  message m;

  m.m1_p1 = (void*) msgque;
  return(_syscall(PM_PROC_NR, MCLOSE, &m));
}
