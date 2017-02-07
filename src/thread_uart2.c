#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

#include <pthread.h>

#include <logd_rpc.h>
#include <mdsapi/mds_api.h>

#include "uart_parser.h"
#include "uart_tools.h"
#include "thread_uart2.h"

#define LOG_TARGET eSVC_COMMON

extern int g_flag_keyon;
int send_uart_input_atcmd_err_msg2 = 0;


// ----------------------------------------
//  LOGD Target
// ----------------------------------------
#define LOG_TARGET eSVC_COMMON

// ----------------------------------------
// uart thread
// ----------------------------------------
static int at_cust_uart_fd2 = AT_BRIDGE_UART_INVALID_HANDLE2;

static int at_cust_uart_thread_run2 = 1;
static int uart_read_size2 = 1;

// *********************************************
// at uart api.
// *********************************************
int at_cust_uart_init2()
{
	int ret;
	
	if ( (ret = mds_api_init_uart(UART_DEVICE_NAME2, 115200)) < 0) 
	{
		printf("[bootstrap.error] uart initialize fail \r\n");
		return -1;
	}
	
	at_cust_uart_fd2 = ret;
	
	
	return ret;
}

void at_cust_uart_deinit2()
{

	if(at_cust_uart_fd2 != AT_BRIDGE_UART_INVALID_HANDLE2)
	{
		mds_api_uart_close(at_cust_uart_fd2);
		at_cust_uart_fd2 = AT_BRIDGE_UART_INVALID_HANDLE2;
	}

	mds_api_uart_close(at_cust_uart_fd2);
	at_cust_uart_fd2 = AT_BRIDGE_UART_INVALID_HANDLE2;
}

int at_cust_write_uart2(const char* buff, int buff_len, int send_cr)
{
    int err = 0;
    int ret = 0;
    char send_buff[512] = {0,};

    char* p_send_buff = (char*)buff;
    int p_send_buff_len = buff_len;


    int write_cnt = 0;
    int retry_cnt = 0;

    //configurationModel_t *conf = get_config_model();

    //int max_write_size = conf->model.pkt_uart_write_size;
    //int write_usleep = conf->model.pkt_uart_write_usleep;
    //int max_write_size = 10;
    // int write_usleep = 100;
    //char uart_write_buff[MAX_SOCK_RECEIVE_SIZE];

    //printf("max_write_size is  [%d] \r\n", max_write_size);
    //memset(uart_write_buff, 0x00, MAX_SOCK_RECEIVE_SIZE);

    if (send_cr == SEND_CR)
    {
        p_send_buff = send_buff;
        sprintf(send_buff,"%s\r\n", buff);
        p_send_buff_len = buff_len + 2;
    }

    while ( write_cnt != p_send_buff_len )
    {
        //printf("uart write ret [%s]\r\n",buff + write_cnt);
        int write_size = p_send_buff_len - write_cnt;

        /*
        if ( write_size == 0 )
        {
            fflush( NULL );
            continue;
        }*/

        ret = mds_api_uart_write(at_cust_uart_fd2, p_send_buff, write_size );

        //usleep(500);

        if ( ret > 0 )
        {
            p_send_buff += ret;

            write_cnt += ret;
            retry_cnt = 0;
        }
        else
        {
            retry_cnt ++;
            usleep(2000*10);
            //LOGE(LOG_TARGET, "at_cust_write_uart2 error -1\r\n");
        }

        // check retry cnt
        if ( retry_cnt > WRITE_UART_MAX_RETRY )
        {
            err = AT_BRIDGE_FAIL;
            LOGE(LOG_TARGET, "<atd> %s : UART write fail!!!!!", __func__ );
            break;
        }
    }

    return err ;

}

#define RET_FOUND_AT		0
#define RET_ERR				1
#define RET_ERR_NO_STRING	2
#define RET_INSERT_SUCCESS	3

int at_cust_atcmd_insert2(char* output_buff, char* input_buff, int input_len)
{
	static char atcmd_buff[MAX_AT_CMD_BUFF] = {0,};
	static int atcmd_idx = 0;
	
	int ret = RET_ERR;
	int i = 0 ;
	
	int echo_setting = AT_BRIDGE_TRUE;

	const char* check_str = input_buff;
	char* tmp_chek_str = NULL;

	int substring_cnt = 0;
	int todo_reopen = 0;

	// 연속적인 at cmd 가 한번에 들어오는지 확인한다. 
	// 이유는 연속적인 at cmd 가 들어오면 uart 자체가 좀 이상하다고 판단;;;;

	// printf("at_cust_atcmd_insert2 [%s]\r\n",input_buff);

	tmp_chek_str = strstr(check_str, "AT");

	while(tmp_chek_str != NULL)
	{
		substring_cnt += 1;

		if (substring_cnt >= 3)
		{
			todo_reopen = 1;
			break;
		}

		tmp_chek_str = strstr(tmp_chek_str+2, "AT");

		if(tmp_chek_str != NULL) {
			printf("tmp_chek_str = [%s]\n", tmp_chek_str);
		}
	}

	i = 0;

	
	for ( i = 0 ; i < input_len ; i++ )
	{
		// err check... buffer check..
		if (atcmd_idx > MAX_AT_CMD_BUFF)
		{
			atcmd_idx = 0;
			memset(atcmd_buff, 0x00, MAX_AT_CMD_BUFF);
			ret = RET_ERR;
			//	printf("%s : insert error - max char\r\n", __FUNCTION__);
			break;
		}
		
		// echo 가 세팅되어있을때만 echo 한다.
		// ate0 일때는 echo 하지 않음.
		if (echo_setting == AT_BRIDGE_TRUE )
			at_cust_write_uart2(&input_buff[i], 1, NO_SEND_CR);
			
		if ( (input_buff[i] == '\r') || (input_buff[i] == '\n') )
		{
			//printf("%s : insert cpy [%s]\r\n", __FUNCTION__, atcmd_buff);
			
			if (atcmd_idx > 1)
			{
				atcmd_buff[atcmd_idx] = input_buff[i];
				memset(output_buff, 0x00, MAX_AT_CMD_BUFF);
			
				strncpy(output_buff, atcmd_buff, atcmd_idx + 1	);
			
				//printf("%s : insert success [%s]\r\n", __FUNCTION__, output_buff);
			
				atcmd_idx = 0;
				memset(atcmd_buff, 0x00, MAX_AT_CMD_BUFF);
				
				ret = RET_FOUND_AT;
			}
			else
			{
				atcmd_idx = 0;
				memset(atcmd_buff, 0x00, MAX_AT_CMD_BUFF);
				ret = RET_ERR_NO_STRING;
			}
			
			break;
		}
		else
		{
			// 특수 문자등은 error 를 리턴한다.
			if  ( !( (input_buff[i] >= 32 ) && (input_buff[i] <= 126) ) )
			{
				ret = RET_ERR;
				LOGI(LOG_TARGET, "%s:  insert error - no char [%c] \r\n", __FUNCTION__, input_buff[i]);
			
				//break;
				continue;
			}

			atcmd_buff[atcmd_idx] = input_buff[i];
			
			/*
			printf("--------------------------------------------\r\n");
			printf("atcmd_idx is [%d]\r\n",atcmd_idx);
			printf("atcmd_buff[atcmd_idx] [%c]\r\n",atcmd_buff[atcmd_idx]);
			printf("input_buff[i] [%c]\r\n",input_buff[i]);
			printf("atcmd_buff[atcmd_idx] -- [%s] --\r\n", atcmd_buff);
			printf("--------------------------------------------\r\n");
			*/
			
			if (atcmd_idx++ > MAX_AT_CMD_BUFF )
			{
				atcmd_idx = 0;
				memset(atcmd_buff, 0x00, MAX_AT_CMD_BUFF);
				ret = RET_ERR;
			//	printf("%s : insert error - max char\r\n", __FUNCTION__);
				break;
			}
			
			// printf("%s : insert ok \r\n", __FUNCTION__);
			ret = RET_INSERT_SUCCESS;
		}
	}
	
	return ret;
}


void *thread_at_cust_uart2(void *args)
{
	char outbuf[MAX_UART_READ_BUFF]={0,};
	
	char atcmd_buff[MAX_AT_CMD_BUFF]={0,};
	//char atcmd_buff_first[MAX_AT_CMD_BUFF]={0,};
	
	int readcnt = 0;
	int ret = 0;

	int chk_atcmd_input_err_cnt = 0;

	int uart_timeout_sec = UART_READ_TIMEOUT_SEC_ATCMD_MODE;
	at_cust_uart_init2();
	
	while(at_cust_uart_thread_run2)
	{
		memset(outbuf, 0x00, MAX_UART_READ_BUFF);
        
		uart_timeout_sec = UART_READ_TIMEOUT_SEC_ATCMD_MODE;
		readcnt = mds_api_uart_read(at_cust_uart_fd2, outbuf , uart_read_size2, uart_timeout_sec);
        
		if (readcnt <= 0) 
		{
			printf("uart2 %d sec timeout!\r\n", uart_timeout_sec);
            continue;
		}

		ret = at_cust_atcmd_insert2(atcmd_buff, outbuf, readcnt);

		if ( ret == RET_FOUND_AT )
		{
			at_cust_cmd_mode_callback(atcmd_buff, strlen(atcmd_buff));
			memset(atcmd_buff, 0x00, MAX_AT_CMD_BUFF);

			chk_atcmd_input_err_cnt = 0;
		}
		else if ( ret == RET_ERR )
		{
			LOGI(LOG_TARGET, "%s: insert Error!! - RET_ERR \r\n", __FUNCTION__);
			memset(atcmd_buff, 0x00, MAX_AT_CMD_BUFF);
            
			at_cust_write_uart2("ERROR\r\n", strlen("ERROR\r\n"), NO_SEND_CR);

			chk_atcmd_input_err_cnt ++;
		}
		else if ( ret == RET_ERR_NO_STRING)
		{
			LOGI(LOG_TARGET, "%s: insert Error!! - RET_ERR_NO_STRING\r\n", __FUNCTION__);
			memset(atcmd_buff, 0x00, MAX_AT_CMD_BUFF);

			chk_atcmd_input_err_cnt ++;
		}

		// 일정 횟수동안, insert 가 fail 나면 webdm 으로 경고메시지를 보낸다.
		if ( chk_atcmd_input_err_cnt > UART_CHK_ATCMD_INSERT_ERR_CNT )
		{
			if ( send_uart_input_atcmd_err_msg2 < 2 )
			{
				//devel_webdm_send_log("Critical Error : uart err : cannot insert atcmd");
				send_uart_input_atcmd_err_msg2++;
			}

			at_cust_uart_deinit2();
			sleep(3);
			at_cust_uart_init2();

			chk_atcmd_input_err_cnt = 0;
		}
	}
    
	at_cust_uart_deinit2();

	return NULL;
}

int start_thread_at_cust_uart2(void)
{
    pthread_attr_t attr;
    pthread_t tid = 0;;
    
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, (512 * 1024));
    
    if(pthread_create(&tid, &attr, thread_at_cust_uart2, NULL) != 0)
    {
        LOGE(LOG_TARGET, "Create Thread 2 Fail.. exit Program...\r\n");
        printf("[error] pthread_create 2 Error");
        return -1;
    }
    
	at_cust_uart_thread_run2 = 1;
    return 0;
}

void exit_thread_at_cust_uart2(void)
{
	at_cust_uart_thread_run2 = 0;
}




