#ifndef __THREAD_UART_H__
#define __THREAD_UART_H__

#define MAX_UART_READ_BUFF      2048
#define MAX_AT_CMD_BUFF         64

#define UART_READ_TIMEOUT_SEC_ATCMD_MODE        5

#define UART_CHK_READ_TIMEOUT_SEC       180
#define UART_CHK_ATCMD_INSERT_ERR_CNT   5

#define UART_DEVICE_NAME2    "/dev/ttyHSL2"

int at_cust_write_uart2(const char* buff, int buff_len, int send_cr);

void *thread_at_cust_uart2(void *args);
int start_thread_at_cust_uart2(void);
void exit_thread_at_cust_uart2(void);

#endif // __THREAD_UART_H__
