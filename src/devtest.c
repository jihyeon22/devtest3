#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include <string.h>
#include <sys/types.h>
#include <termios.h>

#include <pthread.h>

#include "ext_uart.h"

static int fd = 0;


struct cmd_options {

    /* FLAGS */
    unsigned short e;       /* Ext uart Test mode */
    unsigned short l;       /* led Test Mode */
    
};

struct cmd_options option = {0,};

void getoptions(int argc, char* argv[]) 
{
    int opt;
    
    printf("ext uart is opt\r\n");
    
    while ((opt = getopt(argc, argv, "ed")) != -1)
    {
        //printf("opt is [%c] [%d]\r\n",opt);
        switch (opt)
        {
        case 'e':
            option.e = 1;
            break;
        default:
            fprintf(stderr, "Usage: -e : extuart (/dev/ttyEXT) echo test : 9600 \n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[])
{
	int result, ret = -1;
    int i = 0;
	int sck;
    int opt;

	// ----------------------------------------------------
	// fd redirection
	// ----------------------------------------------------
#if 0
	close(0);
	close(1);
	close(2);

	stdin = freopen("/dev/null", "r", stdin);
	stdout = freopen("/dev/null", "w", stdout);
	stderr = freopen("/dev/null", "rw", stderr);
#endif
    //log_fd = fopen("/var/log/test.app", "w");
    
    getoptions(argc, argv);
    
    
    
#if 0
	int pid, sid;

	// make deamon Process...
	pid = fork();
	while (pid < 0 )
	{
		perror("fork error : ");
		pid = fork();
		if (count == 10)
			exit(0);
		sleep(10);
		count++;
	}
	if (pid > 0) 
		exit(EXIT_SUCCESS);

	sid = setsid();
	if (sid < 0) 
		exit(EXIT_FAILURE);


	chdir("/");
#endif


    if (option.e)
    {
        printf("ext uart test start....\r\n");
        extuart_test();
    }
    /*
    else if (option.x)
    {
    
    }*/
    
    else
    {
    
    }


	// ------------------------------------------------
	// get network stat
	// ------------------------------------------------
	while(1)
	{
        printf(".");
		sleep(10);
	}

}