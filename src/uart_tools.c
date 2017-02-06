#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

#include <logd_rpc.h>
#include <mdsapi/mds_api.h>

#include "uart_parser.h"
#include "uart_tools.h"
#include "thread_uart.h"

#include <at/at_util.h>


#define LOG_TARGET eSVC_COMMON



int at_cust_write_uart_append(const char* buff, int buff_len, int flush)
{
    static char write_buff[512] = {0,};
    static int  write_cnt = 0;

    if (flush != FLUSH_ONLY)
    {
        if ( (write_cnt + buff_len) <=  512 )
        {
            strncpy(write_buff + write_cnt, buff, buff_len);
            write_cnt += buff_len;
        }
        else
        {
            LOGE(LOG_TARGET, "%s: err!!! read len is over\r\n",__FUNCTION__);
        }
    }

    if ( (flush == WRITE_AND_FLUSH) || (flush == FLUSH_ONLY) )
    {
        at_cust_write_uart(write_buff,write_cnt,SEND_CR);

        memset(write_buff, 0x00, 512);
        write_cnt = 0;
    }

    return buff_len;
}

int at_cust_ret_echo_ok(const char* cmd, const char* value)
{
    int offset = 2; // "at" strlen is 2
    char buff[512] = {0,};

    sprintf(buff, "\r\n%s: %s\r\n\r\n",cmd + offset, value);
    at_cust_write_uart_append(buff, strlen(buff), WRITE_APPEND);
    at_cust_write_uart_append("OK\r\n", strlen("OK\r\n") , WRITE_AND_FLUSH);
    //at_cust_write_uart(buff, strlen(buff), SEND_CR);
    //at_cust_write_uart("OK\r\n", strlen("OK\r\n"), SEND_CR);
    return 0;
}

int at_cust_ret_echo_err(const char* cmd, const char* value)
{
    int offset = 2; // "at" strlen is 2
    char buff[512] = {0,};

    sprintf(buff, "\r\n%s: %s\r\n\r\n",cmd+offset, value);
    at_cust_write_uart_append(buff, strlen(buff), WRITE_APPEND);
    at_cust_write_uart_append("ERROR\r\n", strlen("ERROR\r\n") , WRITE_AND_FLUSH);
    return 0;
}

int at_cust_get_pure_cmd ( const char *cmd, const int cmd_len, char* buff)
{
    int i = 0;
    int j = 0;

    for ( i = 0 ; i < cmd_len ; i ++)
    {
        if ( (cmd[i] == '\r') || (cmd[i] == '\n') || (cmd[i] == ':') || (cmd[i] == '=') || (cmd[i] == '?') )
        {
            break;
        }
        buff[j++] = cmd[i];
    }

    printf("purecmd : buff -> [%s]\r\n", buff);
    printf("purecmd : cmd -> [%s]\r\n", cmd);
    printf("purecmd : j -> [%d]\r\n", j);
    return j;
}

int at_cust_check_argument(const char* cmd, char** arg)
{
    char tok;
    int cmd_len = strlen(cmd);
    int ret;

    // argument check.
    ret = strncasecmp(*arg, cmd, cmd_len);

    if ( ret != 0 )
    {
        return ARG_RET_FAIL;
    }

    // 명령어가 완전히 일치. 즉 다른 명령어없이... 온전히 해당 명령어.
    if ( mds_api_strlen_without_cr(*arg) == strlen(cmd) )
    {
        return ARG_RET_NORMAL;
    }

    tok = *(*arg + cmd_len);

    switch(tok)
    {
        case ARG_RET_READ_SETTING_CHAR:
        {
            //printf("arg parse : ARG_RET_READ_SETTING_CHAR\r\n");
            *arg += cmd_len + 1;
            ret = ARG_RET_READ_SETTING;
            break;
        }
        case ARG_RET_SET_VALUE_CHAR:
        {
            //printf("arg parse : ARG_RET_SET_VALUE_CHAR\r\n");
            *arg += cmd_len + 1;
            ret = ARG_RET_SET_VALUE;
            break;
        }
        default :
        {
            //printf("arg parse : fail??? [%c]\r\n",tok);
            ret = ARG_RET_FAIL;
        }
    }
    return ret;
}


