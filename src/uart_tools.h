#ifndef __UART_TOOLS_H__
#define __UART_TOOLS_H__

#define WRITE_UART_MAX_RETRY    10

#define AT_BRIDGE_UART_INVALID_HANDLE       (0xFFABCDEF)
#define AT_BRIDGE_UART_INVALID_HANDLE2      (0xFFABCDEE)

#define AT_BRIDGE_FAIL      0
#define AT_BRIDGE_FALSE     0
#define AT_BRIDGE_SUCCESS   1
#define AT_BRIDGE_TRUE      1

// for use uart_write
#define SEND_CR     1
#define NO_SEND_CR  0

// for use uart_write_append
#define WRITE_APPEND    0
#define WRITE_AND_FLUSH 1
#define FLUSH_ONLY      2


#define ARG_RET_FAIL                -1
#define ARG_RET_NORMAL              0   // ������ ��ġ�ϴ� ���ɾ�
#define ARG_RET_READ_SETTING        1
#define ARG_RET_READ_SETTING_CHAR   '?'
#define ARG_RET_SET_VALUE           2
#define ARG_RET_SET_VALUE_CHAR      '='

int at_cust_write_uart_append(const char* buff, int buff_len, int flush);
int at_cust_ret_echo_ok(const char* cmd, const char* value);
int at_cust_ret_echo_err(const char* cmd, const char* value);
int at_cust_get_pure_cmd ( const char *cmd, const int cmd_len, char* buff);
int at_cust_check_argument(const char* cmd, char** arg);

#endif // __UART_TOOLS_H__
