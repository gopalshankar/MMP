#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <minix/libmsgque.h>

void printUsage(void)
{
    printf("Usage: \t\t\targ[1]=Msg Queue id\n\t\t\targ[2]=Type of process[1=receiver, 2=sender]\n\t\t\targ[3]=Number of times to repeat [Optional:default=1]\n\t\t\targ[4]=Sleep duration between repeats [Optional:default=0]\n");
}


int main(int argc, char *argv[], char *envp[])
{
    int msg_Q_id = 1;
    int proc_num = 2;
    struct MsgQue msgque;
    pid_t pid[50]={0};
    int pid_count = 0;
    int key;
    int i, rc;
    char msg[50];
    int msg_num = 1;
	int repeat = 1;
	int sleep_time = 0;
	int choice;

    if (argc>=3)
    {
        if ((atoi(argv[1])>0)&&(atoi(argv[2])>0)&&(atoi(argv[2])<50))
        {
            key = atoi(argv[1]);
            proc_num = atoi(argv[2]);
			if (argc>=4)
				repeat = atoi(argv[3]);
			if (argc>=5)
				sleep_time = atoi(argv[4]);
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

    minit(key, &msgque);
	
	while(repeat>0)
	{
	if((proc_num<1)||(proc_num>2))
	{
		printf("\nEnter choice[1-recv, 2-send]");
		scanf("%d",choice);
	}
	else
	{
		choice = proc_num;
	}
    if(choice==1){
	    rc = mrecv(&msgque, msg, 50);
	    if ( rc != MQ_SUCCESS)
		    printf("%d:Error while receiving\n",rc);
	    else
		    printf("Recved:%d %s\n",rc, msg);
    } else if(choice==2)
    {
    	sprintf(msg,"Process[%d] MsgNo %d", getpid(), msg_num++);
	    rc = msend(&msgque, msg, 50);
	    if (rc != MQ_SUCCESS)
		    printf("Sender: %d Error while sending\n",rc);
	    else
		    printf("Sender got:%d\n",rc);
    }
	repeat--;
	mclean(&msgque);
	if (sleep_time>0)
		sleep(sleep_time);
	}
    mclose(&msgque);

    return 0;
}