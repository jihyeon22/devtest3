#ifndef __UART_PARSER_H__
#define __UART_PARSER_H__


typedef struct AT_CMD_CONVERT
{
    char * at_cmd;
    int (*proc_func)(const char* cmd, const char* arg);
}AT_CMD_CONVERT_T;

void at_cust_cmd_mode_callback(const char* buff, int buff_len);

int atcmd_mds_set_led_color(const char* cmd, const char* arg);
int atcmd_mds_set_testmod(const char* cmd, const char* arg);

#endif // __UART_PARSER_H__