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

#define LOG_TARGET eSVC_COMMON
#define MAX_RET_BUFF_SIZE 512 

#include <at/at_util.h>


void at_cust_cmd_mode_callback(const char* buff, int buff_len);

int set_test_mode = 0;

AT_CMD_CONVERT_T at_cmd_convert_proc[] = 
{    
    { "AT$$MDS_SET_TESTMOD" 	    , atcmd_mds_set_testmod},  	// 모뎀의 테스트 모드 설정
	{ "AT$$MDS_SET_LED_COLOR" 	    , atcmd_mds_set_led_color},  	// 모뎀의 동작상태 설정
    { "AT$$MDS_GET_INT_BATT_LEVEL" 	, atcmd_mds_get_int_batt_level},  	// 모뎀의 동작상태 설정
    { "AT$$MDS_GET_MAIN_LEVEL" 	    , atcmd_mds_get_main_pwr_level},  	// 모뎀의 동작상태 설정
    { "AT$$MDS_GPIO" 	            , atcmd_mds_gpio_func},  	// 모뎀의 동작상태 설정
    // AT$$MDS_GET_INT_BATT_LEVEL?
    // AT$$MDS_GET_MAIN_LEVEL?
    // AT$$MDS_GET_GPS_ANT?
    // AT$$MDS_GET_IGI_LEVEL?
    // AT$$MDS_GET_BTN_1_STAT?
    // AT$$MDS_GET_BTN_2_STAT?
	{ "", NULL },
};



static int devide_argument(unsigned char* buff, int buff_len, char* argv[])
{
    unsigned char ret_buff[MAX_RET_BUFF_SIZE] = {0,};
    
    char    *base = 0;
    int     t_argc = 0;
   // char*   t_argv[10] = {0,};
    
    int i = 0;
    
    memcpy(ret_buff, buff, buff_len);
    memset(buff, 0x00, MAX_RET_BUFF_SIZE);
    
    base = buff;
    
    //t_argv[t_argc] = base;
    argv[t_argc] = base;
    t_argc++;
    
    for (i = 0 ; i < buff_len ; i++)
    {
        switch(ret_buff[i])
        {
            case ',':
                *base = '\0';
                //t_argv[t_argc] = base + 1;
                argv[t_argc] = base + 1;
                t_argc++;
                break;
            default:
                *base = ret_buff[i];
                break;
        }
        base++;
    }
    
    /*
    for( i = 0 ; i < t_argc ; i++)
    {
        printf("1: [%d]/[%d] => [%s]\r\n", i, t_argc, t_argv[i]);
    }
    */
    
    return t_argc;
}


// --------------------------------------------------
int atcmd_mds_set_testmod(const char* cmd, const char* arg)
{
    char* arg_tok = (char*) arg;
    
    char ret_buff[32] = {0,};
    char tmp_buff[32] = {0,};
    
    switch(at_cust_check_argument(cmd, &arg_tok))
    {
        case ARG_RET_SET_VALUE:
        {
            break;
        }
        case ARG_RET_READ_SETTING:
        {
            sprintf(ret_buff,"0, %d",set_test_mode);
            at_cust_ret_echo_ok(cmd, ret_buff);
            return 0;
        }
        case ARG_RET_NORMAL:
        {
            at_cust_ret_echo_err(cmd, "1");
            return 0;
        }
        case ARG_RET_FAIL:
        {
            at_cust_ret_echo_err(cmd, "1");
            return 0;
        }
        default :
        {
            at_cust_ret_echo_err(cmd, "1");
            //at_cust_ret_echo_ok(cmd, "0");
            //return 0;
        }

    }

    //printf("arg_tok is [%s]\r\n",arg_tok);
    if ( mds_api_check_is_num(arg_tok, strlen(arg_tok)) == DEFINES_MDS_API_NOK )
    {
        printf("arg_tok is not number[%s]\r\n",arg_tok);
        at_cust_ret_echo_err(cmd, "3");
        return 0;
    }
    
    set_test_mode = atoi(arg_tok);
    printf("arg_tok is [%s]\r\n",arg_tok);
    
    switch(set_test_mode)
    {
        case 0 :
        {
            send_at_cmd("AT$$LEDOFF=0");
            sprintf(ret_buff,"0,%d",set_test_mode);
            at_cust_ret_echo_ok(cmd, ret_buff);
            return 0;
            break;
        }
        case 1 :
        {
            send_at_cmd("AT$$LEDOFF=1");
            sprintf(ret_buff,"0, %d",set_test_mode);
            at_cust_ret_echo_ok(cmd, ret_buff);
            return 0;
            break;
        }
        default : 
        {
            at_cust_ret_echo_err(cmd, "5");
            return 0;
            break;
        }
    }
    
    
    at_cust_ret_echo_err(cmd, "9");
   // at_cust_ret_echo_ok(cmd, "0");

    return 0;

}

int atcmd_mds_set_led_color(const char* cmd, const char* arg)
{
    char* arg_tok = (char*) arg;
    char ret_buff[32] = {0,};
    
    char* argv[20] = {0,};
    int argc = 0;
    int i = 0;
    
    char led_name[32] = {0,};

    if (set_test_mode != 1)
    {
        at_cust_ret_echo_err(cmd, "1, Not Test Mode");
        return 0;
    }
        
    switch(at_cust_check_argument(cmd, &arg_tok))
    {
        case ARG_RET_SET_VALUE:
        {
            break;
        }
        case ARG_RET_READ_SETTING:
        {
            at_cust_ret_echo_err(cmd, "1");
            return 0;
        }
        case ARG_RET_NORMAL:
        {
            at_cust_ret_echo_err(cmd, "2");
            return 0;
        }
        case ARG_RET_FAIL:
        {
            at_cust_ret_echo_err(cmd, "3");
            return 0;
        }
        default :
        {
            at_cust_ret_echo_err(cmd, "4");
            return 0;
            //at_cust_ret_echo_ok(cmd, "0");
            //return 0;
        }
    }

    argc = devide_argument(arg_tok, strlen(arg_tok), argv);
    
    if ( argc != 2 )
    {
        at_cust_ret_echo_err(cmd, "5, invalid argument");
        return 0;
    }
    
    for( i = 0 ; i < argc ; i ++ )
    {
        printf(" argv[%d] => [%s]\r\n", i, argv[i]);
    }

    // set color
    if ( strcmp(argv[0],"1") == 0 )
    {
        if ( strcmp(argv[1],"off") != 0 )
            mds_api_led_off("pwr");
        else
            mds_api_led_on("pwr", argv[1]);
    }
    else if ( strcmp(argv[0],"2") == 0 )
    {
        if ( strcmp(argv[1],"off") != 0 )
            mds_api_led_off("wcdma");
        else
            mds_api_led_on("wcdma", argv[1]);
    }
    else if ( strcmp(argv[0],"3") == 0 )
    {
        if ( strcmp(argv[1],"off") != 0 )
            mds_api_led_off("gps");
        else
            mds_api_led_on("gps", argv[1]);
    }
    else if ( strcmp(argv[0],"all") == 0 )
    {
        if ( strcmp(argv[1],"off") != 0 )
        {
            mds_api_led_on("gps", argv[1]);
            mds_api_led_on("wcdma", argv[1]);
            mds_api_led_on("pwr", argv[1]);
        }
        else
        {
            mds_api_led_off("gps");
            mds_api_led_off("wcdma");
            mds_api_led_off("pwr");
        }
    }
    
    // set led num
    
    {
        at_cust_ret_echo_ok(cmd, "0");
    }

    return 0;

}


int atcmd_mds_get_int_batt_level(const char* cmd, const char* arg)
{
    char* arg_tok = (char*) arg;
    
    char ret_buff[32] = {0,};
    char tmp_buff[32] = {0,};
    
    switch(at_cust_check_argument(cmd, &arg_tok))
    {
        case ARG_RET_SET_VALUE:
        {
            at_cust_ret_echo_err(cmd, "1");
            return 0;
        }
        case ARG_RET_READ_SETTING:
        {
            break;
        }
        case ARG_RET_NORMAL:
        {
            at_cust_ret_echo_err(cmd, "1");
            return 0;
        }
        case ARG_RET_FAIL:
        {
            at_cust_ret_echo_err(cmd, "1");
            return 0;
        }
        default :
        {
            at_cust_ret_echo_err(cmd, "1");
            //at_cust_ret_echo_ok(cmd, "0");
            //return 0;
        }

    }

    {
        int batt_level = 0;
        mds_api_get_internal_batt_tl500(&batt_level);
        printf("internal batt level is [%d]\r\n", batt_level);
        sprintf(ret_buff, "0,%d.%d", batt_level/100, batt_level%100);
        at_cust_ret_echo_ok(cmd, ret_buff);
    }

    
   // at_cust_ret_echo_ok(cmd, "0");

    return 0;
}

int atcmd_mds_get_main_pwr_level(const char* cmd, const char* arg)
{
    char* arg_tok = (char*) arg;
    
    char ret_buff[32] = {0,};
    char tmp_buff[32] = {0,};
    
    switch(at_cust_check_argument(cmd, &arg_tok))
    {
        case ARG_RET_SET_VALUE:
        {
            at_cust_ret_echo_err(cmd, "1");
            return 0;
        }
        case ARG_RET_READ_SETTING:
        {
            break;
        }
        case ARG_RET_NORMAL:
        {
            at_cust_ret_echo_err(cmd, "1");
            return 0;
        }
        case ARG_RET_FAIL:
        {
            at_cust_ret_echo_err(cmd, "1");
            return 0;
        }
        default :
        {
            at_cust_ret_echo_err(cmd, "1");
            //at_cust_ret_echo_ok(cmd, "0");
            //return 0;
        }

    }

    {
        int batt_level = 0;
        at_get_adc_main_pwr(&batt_level);
        printf("main power level is [%d]\r\n", batt_level);
        sprintf(ret_buff, "0,%d", batt_level);
        at_cust_ret_echo_ok(cmd, ret_buff);
    }

    
   // at_cust_ret_echo_ok(cmd, "0");

    return 0;
}

int atcmd_mds_get_gps_ant_stat(const char* cmd, const char* arg)
{
    char* arg_tok = (char*) arg;
    
    char ret_buff[32] = {0,};
    char tmp_buff[32] = {0,};
    
    switch(at_cust_check_argument(cmd, &arg_tok))
    {
        case ARG_RET_SET_VALUE:
        {
            at_cust_ret_echo_err(cmd, "1");
            return 0;
        }
        case ARG_RET_READ_SETTING:
        {
            break;
        }
        case ARG_RET_NORMAL:
        {
            at_cust_ret_echo_err(cmd, "1");
            return 0;
        }
        case ARG_RET_FAIL:
        {
            at_cust_ret_echo_err(cmd, "1");
            return 0;
        }
        default :
        {
            at_cust_ret_echo_err(cmd, "1");
            //at_cust_ret_echo_ok(cmd, "0");
            //return 0;
        }

    }

    {
        int gps_ant_stat = 0;
        if ( mds_api_gps_util_get_gps_ant() == DEFINES_MDS_API_OK ) 
            gps_ant_stat = 1;

        printf("gps ant stat is [%d]\r\n", gps_ant_stat);

        sprintf(ret_buff, "0,%d", gps_ant_stat);
        at_cust_ret_echo_ok(cmd, ret_buff);
    }

    
   // at_cust_ret_echo_ok(cmd, "0");

    return 0;
}

int atcmd_mds_gpio_func(const char* cmd, const char* arg)
{
    char* arg_tok = (char*) arg;
    char ret_buff[32] = {0,};
    
    char* argv[20] = {0,};
    int argc = 0;
    int i = 0;

    // ----------------
    int gpio_num = -1;
    int gpio_mode = -1;
    int gpio_set_val = -1;
    int convert_gpio_num = 0;
    // ----------------
    if (set_test_mode != 1)
    {
        at_cust_ret_echo_err(cmd, "1, Not Test Mode");
        return 0;
    }
        
    switch(at_cust_check_argument(cmd, &arg_tok))
    {
        case ARG_RET_SET_VALUE:
        {
            break;
        }
        case ARG_RET_READ_SETTING:
        {
            at_cust_ret_echo_err(cmd, "1");
            return 0;
        }
        case ARG_RET_NORMAL:
        {
            at_cust_ret_echo_err(cmd, "2");
            return 0;
        }
        case ARG_RET_FAIL:
        {
            at_cust_ret_echo_err(cmd, "3");
            return 0;
        }
        default :
        {
            at_cust_ret_echo_err(cmd, "4");
            return 0;
            //at_cust_ret_echo_ok(cmd, "0");
            //return 0;
        }
    }

    argc = devide_argument(arg_tok, strlen(arg_tok), argv);
    
    
    for( i = 0 ; i < argc ; i ++ )
    {
        printf(" argv[%d] => [%s]\r\n", i, argv[i]);
        if ( i == 0 )
        {
            gpio_num = atoi(argv[i]);
            if ( gpio_num == 1 ) { convert_gpio_num = 14; }
            else if ( gpio_num == 2 ) { convert_gpio_num = 13; }
            else if ( gpio_num == 3 ) { convert_gpio_num = 12; }
            else { at_cust_ret_echo_err(cmd, "5, invalid gpio number"); return 0; }
        }
        if ( i == 1 )
            gpio_mode = atoi(argv[i]);
        if ( i == 2 )
            gpio_set_val = atoi(argv[i]);
    }

    // eGpioInput = 0,
    // eGpioOutput
    if ( ( gpio_set_val == -1 ) && ( gpio_mode == 0 ) )// input gpio read val cmd 
    {
        int gpio_val = 0;

        mds_api_gpio_set_direction(convert_gpio_num, eGpioInput);
        gpio_val = mds_api_gpio_get_value(convert_gpio_num);

        sprintf(ret_buff, "%d,%d,%d", gpio_num, gpio_mode, gpio_val);
        at_cust_ret_echo_ok(cmd, ret_buff);
    }
    else if ( ( gpio_set_val != -1 ) && ( gpio_mode == 1 ) )// input gpio read val cmd 
    {
        mds_api_gpio_set_direction(convert_gpio_num, eGpioOutput);
        mds_api_gpio_set_value(convert_gpio_num, gpio_set_val);

        sprintf(ret_buff, "%d,%d,%d", gpio_num, gpio_mode, gpio_set_val);
        at_cust_ret_echo_ok(cmd, ret_buff);
    }
    else
        at_cust_ret_echo_err(cmd, "Invalid gpio setting");

    // mds_api_gpio_get_value(const int gpio);
    // mds_api_gpio_set_value(const int gpio, const int value);
    // mds_api_gpio_set_direction(const int gpio, gpioDirection_t direction);

    // set led num

    return 0;

}


// -------------------------------
// -------------------------------
// -------------------------------


void at_cust_cmd_mode_callback(const char* buff, int buff_len)
{
    int i = 0;
    int found = 0 ;
    int ret = 0;

    char pure_cmd[512] = {0,};
    int pure_cmd_len = 0;

    
//    printf("\r\nat_cust_cmd_mode_callback [%s]\r\n",buff);
    if ( buff_len == 0 )
        return;
//  LOGI(LOG_TARGET, "%s: call \r\n",__FUNCTION__);

    // at 로 시작하지 않는 커맨드는 모두 에러다.
    ret  = strncasecmp ( buff, "AT", 2);

    if (ret != 0)
    {
        if (mds_api_strlen_without_cr(buff) > 0 )
        {
            LOGE(LOG_TARGET, "%s: invalid cmd [%s] \r\n",__FUNCTION__, buff);
            at_cust_write_uart("\r\nERROR", 7, SEND_CR);
        }
        return;
    }

    // 실제 커맨드를 얻어온다.
    if ( ( pure_cmd_len = at_cust_get_pure_cmd(buff, buff_len, pure_cmd) ) <= 0)
    {
        LOGE(LOG_TARGET, "%s: invalid cmd [%s] \r\n",__FUNCTION__, buff);
        at_cust_write_uart("\r\nERROR", 7, SEND_CR);
    }

    //printf("\r\n1: [%s] of pure cmd is [%s]/%d\r\n",buff, pure_cmd, pure_cmd_len);

    while(1)
    {
        if (!(strlen(at_cmd_convert_proc[i].at_cmd)))
        {
            break;
        }

        // '?' '='  등등의 전까지의 실제 at cmd 를 얻은후..
        // 해당 at cmd 의 길이를 비교...        
        if ( pure_cmd_len == mds_api_strlen_without_cr(at_cmd_convert_proc[i].at_cmd) )
        {
            ret  = strncasecmp ( pure_cmd, at_cmd_convert_proc[i].at_cmd, strlen(at_cmd_convert_proc[i].at_cmd));
            printf("[%s]/[%s] ==> ret [%d]\r\n",pure_cmd, at_cmd_convert_proc[i].at_cmd, ret);
        }
        else
        {
            ret = -1;
        }

        // check found.
        if ( ret  == 0 )
        {
            found = 1;
            break;
        }

        i++;
    }

    if (found)
    {
        char tmp_buff[MAX_AT_CMD_BUFF]={0,};
        mds_api_remove_cr(buff,tmp_buff,MAX_AT_CMD_BUFF);
        at_cmd_convert_proc[i].proc_func(at_cmd_convert_proc[i].at_cmd, tmp_buff);
    }
    else
    {
        LOGI(LOG_TARGET, "%s: [%s] cmd not found!! \r\n",__FUNCTION__, buff);
        at_cust_write_uart("ERROR\r\n", strlen("ERROR\r\n"), NO_SEND_CR);
        // todo error..
    }
}


        