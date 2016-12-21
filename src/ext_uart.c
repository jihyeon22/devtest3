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



int init_uart(char* dev, int baud , int *fd)
{
	struct termios newtio;
	*fd = open( dev, O_RDWR | O_NOCTTY | O_NONBLOCK );

	if ( *fd < 0 ) {
//		fprintf(log_fd,"%s> uart dev '%s' open fail [%d]n", __func__, dev, *fd);
		return -1;
	}

//	fprintf(log_fd,"init uart +++\r\n");

	memset(&newtio, 0, sizeof(newtio));
	newtio.c_iflag = IGNPAR; // non-parity
	newtio.c_oflag = OPOST | ONLCR;;
	newtio.c_cflag = CS8 | CLOCAL | CREAD; // NO-rts/cts

	switch( baud )
	{
		case 115200 : newtio.c_cflag |= B115200; break;
		case 57600 : newtio.c_cflag |= B57600; break;
		case 38400 : newtio.c_cflag |= B38400; break;
		case 19200 : newtio.c_cflag |= B19200; break;
		case 9600 : newtio.c_cflag |= B9600; break;
		case 4800 : newtio.c_cflag |= B4800; break;
		case 2400 : newtio.c_cflag |= B2400; break;
		default : newtio.c_cflag |= B115200; break;
	}

	newtio.c_lflag = 0;
	//newtio.c_cc[VTIME] = vtime; // timeout 0.1초 단위
	//newtio.c_cc[VMIN] = vmin; // 최소 n 문자 받을 때까진 대기
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 0;
	tcflush ( *fd, TCIFLUSH );
	tcsetattr( *fd, TCSANOW, &newtio );

//	fprintf(log_fd,"init uart---\r\n");
	return 0;
}

//void at_read_loop(int fd)
void at_read_loop(void * arg)
{
	int result;
	char buffer[MAX_BUF_SIZE];
	char result_string[MAX_BUF_SIZE]={0,};
	fd_set reads, temps;

	FD_ZERO(&reads);
	FD_SET(fd, &reads);

	while(1)
	{
		temps = reads;
		result = select(FD_SETSIZE, &temps, NULL, NULL, NULL);
		if (result < 0)
		{
			//          LOGE("Select error");
			exit(EXIT_FAILURE);
		}

		if (FD_ISSET(fd, &temps))
		{
			memset(buffer, 0, sizeof(buffer));

			if(read(fd, buffer, MAX_BUF_SIZE) == -1 )
			{
				
				continue;
			}
			sprintf(result_string,"input data is [%s]\r\n",buffer);
			write(fd, result_string, strlen(result_string));
			//fprintf(log_fd,"receive buffer is [%s]\r\n",buffer);
		}
	}
}

void extuart_test()
{
	int result, ret = -1;
	//log_fd = fopen("/var/log/dtg.init.log", "w");

	pthread_t p_thread;
	ret = init_uart(DEV_EXT_UART, 9600, &fd);
    printf("1\r\n");
	printf("init uart [%d], [%d]\r\n",ret,fd);
	//fflush(log_fd);

	//  set_fd(fd);
	pthread_create(&p_thread, NULL, at_read_loop, NULL);
    /*
	while(1)
	{
//		ret =    write(fd, "aaar", 4);
//		printf("write return [%d]rn",ret);
		printf(".\r\n");
		sleep(1);
	}
    */
	//  at_read_loop(fd);

}
 
