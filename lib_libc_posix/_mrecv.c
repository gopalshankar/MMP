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


/* This API is used to recieve message from PM
 */
PUBLIC int mrecv( msgque , msg, len )
struct MsgQue *msgque;
void *msg;
int len;
{
  message m;

  m.m1_i1 = len;
  m.m1_p1 = (void*) msgque;
  m.m1_p2 = (void*) msg;
  return(_syscall(PM_PROC_NR, MRECV, &m));
}
