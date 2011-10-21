#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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

int mregister(key_t key)
{
    return 0;
}

int mderegister(key_t key)
{
    return 0;
}

int msend(message_t msg)
{
#ifdef DEBUG
    printf("msend():%d!\n", msg.src_dst, msg.buf);
#endif
    return 0;
}

int mrecv(message_t *msg)
{
#ifdef DEBUG
    printf("mrecv()!\n");
#endif
    return 0;
}

void child_proc(key_t key, int proc_num)
{
    /* */
    /*
    Declare variable to hold seconds on clock.
    */
    time_t seconds;
    message_t msg;
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

    mregister(key);
    msg.src_dst=proc_num;

    while (rand() % 10 < 9)
    {
        switch (rand()%2)
        {
        case 0:
        {
            if (mrecv(&msg)!= 0)
		printf("Child%d:Error while receiving\n",proc_num);
        }
        break;
        default:
        {
            if (msend(msg))
		printf("Child%d:Error while sending\n",proc_num);
        }
        }
        sleep(rand()%5);
    }
    mderegister(key);

}

int main(int argc, char *argv[], char *envp[])
{
    int msg_Q_id = 1;
    int num_proc = 2;
    pid_t pid[50]={0};
    int pid_count = 0;
    key_t key;
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
        //Child
        //TODO send/recv
        child_proc(key, pid_count);
        printf("Done\n");
    }
    else
    {
        //Parent
#ifdef DEBUG
        printf("Child pid=%d created.\n",pid[pid_count]);
#endif
        pid_count++;
        if (--num_proc) goto fork_again;

        //Wait for child completion
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
