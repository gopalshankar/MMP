#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <minix/libmsgque.h>

#ifndef DEBUG
#define DEBUG //comment out to remove debug traces from default compilation
#endif

typedef struct
{
  int src_dst;
  char buf[5];
} message_t;

void printUsage(void)
{
    printf("Usage: \t\t\targ[1]=Msg Queue id\n\t\t\targ[2]=Number of processes\n");
}


void child_proc(int key, int proc_num)
{
    /* */
    /*
    Declare variable to hold seconds on clock.
    */
	struct MsgQue msgque;
    time_t seconds;
    char msg[50];
	int msg_num = 1;
    /*
    Get value from system clock and
    place in seconds variable.
    */
    time(&seconds);
    /*
    Convert seconds to a unsigned
    integer.
    */
    srand((unsigned int) seconds);

	printf("Child%d:msgque=0x%x\n",proc_num, &msgque);

    minit(key, &msgque);

    while (rand() % 10 < 9)
    {
        switch (rand()%2)
        {
        case 0:
        {
            if (mrecv(&msgque, msg, 50)!= MQ_SUCCESS)
				printf("Child%d:Error while receiving\n",proc_num);
			else
				printf("Child%d:%s\n",proc_num, msg);
        }
        break;
        default:
        {
			sprintf(msg,"[Child%d] says msg %d",proc_num, msg_num++);
            if (msend(&msgque, msg, 50)!= MQ_SUCCESS)
				printf("Child%d:Error while sending\n",proc_num);
        }
        }
        sleep(rand()%5);
    }
    mclose(&msgque);

}

int main(int argc, char *argv[], char *envp[])
{
    int msg_Q_id = 1;
    int num_proc = 2;
    pid_t pid[50]={0};
    int pid_count = 0;
    int key;
    int i;

    if (argc==3)
    {
        if ((atoi(argv[1])>0)&&(atoi(argv[2])>0)&&(atoi(argv[2])<50))
        {
            msg_Q_id = atoi(argv[1]);
            num_proc = atoi(argv[2]);
        }
        else
        {
            printUsage();
            return -1;
        }
    }
    else
    {
        printUsage();
        return -1;
    }
    key = ftok(argv[0],msg_Q_id);
#ifdef DEBUG
    printf("key=%d\n",key);
#endif


fork_again:
    pid[pid_count] = fork();
    if (pid[pid_count] == 0)
    {
        /*Child*/
        child_proc(key, pid_count);
        printf("Done\n");
    }
    else
    {
        /*Parent*/
#ifdef DEBUG
        printf("Child pid=%d created.\n",pid[pid_count]);
#endif
        pid_count++;
        if (--num_proc) goto fork_again;

        /*Wait for child completion*/
        for (i = 0; i < pid_count; ++i)
        {
            int status;
            while (-1 == waitpid(pid[i], &status, 0));
        }
#ifdef DEBUG
        printf("All child completed. Exiting now..\n");
#endif
    }
    return 0;
}
