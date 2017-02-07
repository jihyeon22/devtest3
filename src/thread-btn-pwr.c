#include <stdio.h>
#include <unistd.h>
typedef unsigned long kernel_ulong_t;
#define BITS_PER_LONG 32
#include <linux/input.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <math.h>
// double round(double x);
#include <mdsapi/mds_api.h>

#include "uart_tools.h"
//#include <board/board_system.h>

#define EVENT0_DEV_NAME                 "/dev/input/event1"


#define DEV_BTN_TIMEOUT_SEC			3

#define IGNI_KEY_HOLD_TIME_SEC 	3
#define PWR_KEY_HOLD_TIME_SEC 	3

#define GPIO_NO_MIN                     1
#define GPIO_NO_MAX                     66
#define GPIO_SRC_NUM_POWER              6
#define GPIO_SRC_NUM_IGNITION           30

#define GPIO_SRC_NUM_POWER              6
#define POWER_SRC_DC                (1)
#define POWER_SRC_BATTERY           (0)

#define GPIO_SRC_NUM_IGNITION           30
#define POWER_IGNITION_ON       (0)
#define POWER_IGNITION_OFF      (1)

#define BTN_HOLD_SECS   3

#include "uart_parser.h"

void button1_callback(void)
{
	char send_msg[] = "$$MDS_BTN:1,1";
    printf("gtrack calback ::: button1_callback !!!\r\n");
	if ( set_test_mode == 1 )
		at_cust_write_uart_append(send_msg, strlen(send_msg), WRITE_AND_FLUSH);
}

void button2_callback(void)
{
	char send_msg[] = "$$MDS_BTN:2,1";
    printf("gtrack calback ::: button2_callback !!!\r\n");
	if ( set_test_mode == 1 )
		at_cust_write_uart_append(send_msg, strlen(send_msg), WRITE_AND_FLUSH);
}

void ignition_on_callback(void)
{
	char send_msg[] = "$$MDS_IGN:0,1";
    printf("gtrack calback ::: ignition_on_callback !!!\r\n");
	if ( set_test_mode == 1 )
		at_cust_write_uart_append(send_msg, strlen(send_msg), WRITE_AND_FLUSH);
}

void ignition_off_callback(void)
{
	char send_msg[] = "$$MDS_IGN:0,0";
    printf("gtrack calback ::: ignition_off_callback !!!\r\n");
	if ( set_test_mode == 1 )
		at_cust_write_uart_append(send_msg, strlen(send_msg), WRITE_AND_FLUSH);
}

void power_on_callback(void)
{
	char send_msg[] = "$$MDS_PWR:0,1";
    printf("gtrack calback ::: power_on_callback !!!\r\n");
	if ( set_test_mode == 1 )
		at_cust_write_uart_append(send_msg, strlen(send_msg), WRITE_AND_FLUSH);

}

void power_off_callback(void)
{
	char send_msg[] = "$$MDS_PWR:0,0";
    printf("gtrack calback ::: power_off_callback !!!\r\n");
	if ( set_test_mode == 1 )
		at_cust_write_uart_append(send_msg, strlen(send_msg), WRITE_AND_FLUSH);
}


int power_get_ignition_status(void)
{
    int value = mds_api_gpio_get_value(GPIO_SRC_NUM_IGNITION);

    if(value == POWER_IGNITION_ON)
    {
        return POWER_IGNITION_ON;
    }
    else if(value == POWER_IGNITION_OFF)
    {
        return POWER_IGNITION_OFF;
    }
    else
    {
        return -1;
    }
}

int power_get_power_source(void)
{
    int value = mds_api_gpio_get_value(GPIO_SRC_NUM_POWER);

    if(value == POWER_SRC_DC)
    {
        return POWER_SRC_DC;
    }
    else if(value == POWER_SRC_BATTERY)
    {
        return POWER_SRC_BATTERY;
    }
    else
    {
        return -1;
    }

}


static int flag_run_thread_btn_pwr = 1;

static time_t btn_1_push_time, btn_2_push_time;
static int key_led_noti;
static time_t key_led_start, key_led_end;

static time_t time_ign_hold_time;
static int ign_on = POWER_IGNITION_OFF;

static time_t time_pwr_hold_time;
static int pwr_on = POWER_SRC_DC;

void _check_btn_pwr_event(const struct input_event ev_input)
{
	
	time_t cur_time = 0;
	if(ev_input.type == 0x01 && ev_input.code == 0x1d3 && ev_input.value == 0x01)
	{
		int hold_time;
		cur_time = mds_api_get_kerneltime();
		hold_time = (int)round(difftime(cur_time, btn_1_push_time));
		if(hold_time > BTN_HOLD_SECS)
		{
			printf( " press button 1 on!! [%d]sec\r\n", hold_time);
			btn_1_push_time = mds_api_get_kerneltime();
			//led_noti(eLedcmd_KEY_NOTI);
			key_led_start = mds_api_get_kerneltime();
			key_led_noti = 1;

			button1_callback();

		}
	}

	else if(ev_input.type == 0x01 && ev_input.code == 0x1d2 && ev_input.value == 0x01)
	{
		int hold_time;
		cur_time = mds_api_get_kerneltime();
		hold_time = (int)round(difftime(cur_time, btn_2_push_time));
		if(hold_time > BTN_HOLD_SECS)
		{
			printf( " press button 2 on!! [%d]sec\r\n", hold_time);
			btn_2_push_time = mds_api_get_kerneltime();
			//led_noti(eLedcmd_KEY_NOTI);
			key_led_start = mds_api_get_kerneltime();
			key_led_noti = 1;

			button2_callback();
#ifdef EMUL_GPS
			if(emul_gps_speed > 0)
			{
				emul_gps_speed = 0;
			}
			else
			{
				emul_gps_speed = 200;
			}
#endif
		}
	}
	else if(ev_input.type == 0x16)
	{
		if(ev_input.code == 0x08)
		{
			if(ev_input.value == 0x01)
			{
				printf( "ignition line on!!!!!!\r\n");
				time_ign_hold_time = mds_api_get_kerneltime();
				//_check_ign_onoff();
			}
			else
			{
				printf( "ignition line off!!!!!!\r\n");
				time_ign_hold_time = mds_api_get_kerneltime();
				//_check_ign_onoff();
			}
		}
		else if(ev_input.code == 0x09)
		{
			if(ev_input.value == 0x00)
			{
				printf( "dc mode!!!!!!\r\n");
				time_pwr_hold_time = mds_api_get_kerneltime();
				//_check_pwr_onoff();
			}
			else
			{
				printf( "battery mode!!!!!!\r\n");
				time_pwr_hold_time = mds_api_get_kerneltime();
				//_check_pwr_onoff();
			}			
		}
	}
}

/*
void _check_led_noti_off(void)
{
	if(key_led_noti) {
		key_led_end = mds_api_get_kerneltime();
		if(difftime(key_led_end, key_led_start) >= BTN_LED_BLINK_SECS) {
			key_led_noti = 0;
		//	led_noti(eLedcmd_KEY_NOTI_OFF);
		}
	}
}*/

void _check_ign_onoff(void)
{
	int ign_hold_sec = IGNI_KEY_HOLD_TIME_SEC;

	if(ign_on == POWER_IGNITION_ON)
	{
		if(mds_api_get_kerneltime() - time_ign_hold_time < ign_hold_sec)
		{
//jwrho 2015.02.23 ++
			if(power_get_ignition_status() == POWER_IGNITION_ON)
				time_ign_hold_time = mds_api_get_kerneltime();
//jwrho 2015.02.23 --
			printf( "time_ign_off..[%d] sec Skip\n", ign_hold_sec);
			return;
		}

		if(power_get_ignition_status() == POWER_IGNITION_OFF)
		{
			ignition_off_callback();
			time_ign_hold_time = mds_api_get_kerneltime();
			ign_on = POWER_IGNITION_OFF;
		}
//jwrho 2015.02.23 ++
		else
		{
			time_ign_hold_time = mds_api_get_kerneltime();
		}
//jwrho 2015.02.23 --
	}
	else
	{
		if(mds_api_get_kerneltime() - time_ign_hold_time < ign_hold_sec)
		{
//jwrho 2015.02.23 ++
			if(power_get_ignition_status() == POWER_IGNITION_OFF)
				time_ign_hold_time = mds_api_get_kerneltime();
//jwrho 2015.02.23 --
			printf( "time_ign_on..[%d] sec Skip\n", ign_hold_sec);
			return;
		}

		if(power_get_ignition_status() == POWER_IGNITION_ON)
		{
			
			ignition_on_callback();
			time_ign_hold_time = mds_api_get_kerneltime();
			ign_on = POWER_IGNITION_ON;
		}
		else
		{
			time_ign_hold_time = mds_api_get_kerneltime();
		}

	}
}

void _check_pwr_onoff(void)
{
	int pwr_hold_sec = PWR_KEY_HOLD_TIME_SEC;

	if(pwr_on == POWER_SRC_DC)
	{
		if(mds_api_get_kerneltime() - time_pwr_hold_time < pwr_hold_sec)
		{
//jwrho 2015.02.23 ++
			if(power_get_power_source() == POWER_SRC_DC)
				time_pwr_hold_time = mds_api_get_kerneltime();
//jwrho 2015.02.23 --
			printf( "time_pwr_off..[%d] sec Skip\n", pwr_hold_sec);
			return;
		}

		if(power_get_power_source() == POWER_SRC_BATTERY)
		{
			
			power_off_callback();

			time_pwr_hold_time = mds_api_get_kerneltime();
			pwr_on = POWER_SRC_BATTERY;
		}
		else
		{
			time_pwr_hold_time = mds_api_get_kerneltime();
		}
	}
	else
	{
		if(mds_api_get_kerneltime() - time_pwr_hold_time < pwr_hold_sec)
		{
			if(power_get_power_source() == POWER_SRC_BATTERY)
				time_pwr_hold_time = mds_api_get_kerneltime();
			return;
		}

		if(power_get_power_source() == POWER_SRC_DC)
		{
			power_on_callback();
			time_pwr_hold_time = mds_api_get_kerneltime();
			pwr_on = POWER_SRC_DC;
		}
		else
		{
			time_pwr_hold_time = mds_api_get_kerneltime();
		}
	}
}

void *thread_btn_pwr(void *args)
{
	int fd_evt = 0;
//	int fd_cnt = 0;
	int rc;
	fd_set  readSet;
	struct  timeval tv;

	struct input_event ev_input;

	printf( "PID %s : %d\n", __FUNCTION__, getpid());

	if(!flag_run_thread_btn_pwr)
	{
		return NULL;
	}
/*
	if(power_get_ignition_status() == POWER_IGNITION_OFF)
	{
		
		time_ign_hold_time = mds_api_get_kerneltime();
		ign_on = POWER_IGNITION_OFF;
		ignition_off_callback();
	}
	else
	{
		
		time_ign_hold_time = mds_api_get_kerneltime();
		ign_on = POWER_IGNITION_ON;
		ignition_on_callback();
	}
*/
	//if(conf->common.first_pwr_status_on == 1)
	if (0)
	{
		time_pwr_hold_time = mds_api_get_kerneltime();
		pwr_on = POWER_SRC_DC;

		power_on_callback();
	}
	else
	{
		if(power_get_power_source() == POWER_SRC_BATTERY)
		{
			
			time_pwr_hold_time = mds_api_get_kerneltime();
			pwr_on = POWER_SRC_BATTERY;
			power_off_callback();
		}
		else
		{
			
			time_pwr_hold_time = mds_api_get_kerneltime();
			pwr_on = POWER_SRC_DC;
			power_on_callback();
		}
	}

	btn_1_push_time = btn_2_push_time = mds_api_get_kerneltime();

	fd_evt = open(EVENT0_DEV_NAME, O_RDONLY | O_NONBLOCK);
	if(fd_evt < 0) {
		printf( "%s(): open() error\n", __func__);
		//error_critical(eERROR_REBOOT, "thread_btn_pwr open Error");
	}

	int val;

	val = fcntl(fd_evt, F_GETFD, 0);
	val |= FD_CLOEXEC;
	fcntl(fd_evt, F_SETFD, val);


	while(flag_run_thread_btn_pwr)
	{
	
		FD_ZERO(&readSet);
		FD_SET(fd_evt, &readSet);

		tv.tv_sec = DEV_BTN_TIMEOUT_SEC;
		tv.tv_usec = 0;
		
		rc = select(fd_evt + 1 , &readSet, NULL, NULL, &tv);

		if(rc < 0)
		{
			if(errno != EINTR)
			{
				printf( "%s(): select error\n", __FUNCTION__);
				//error_critical(eERROR_EXIT, "btn_pwr select Error");
			}
			continue;
		}

		if(rc > 0)
		{
			int chk_fd = 0;

			if( FD_ISSET(fd_evt, &readSet) )
			{
				if(read(fd_evt, &ev_input, sizeof(ev_input)) == sizeof(ev_input))
				{
					printf( "type: 0x%x, code: 0x%x, value: 0x%x\n", ev_input.type, ev_input.code, ev_input.value);
					_check_btn_pwr_event(ev_input);
				}
				
				chk_fd = 1;

			}

			if (chk_fd == 0)
			{
				continue;
			}

		}
		//_check_led_noti_off();
		_check_ign_onoff();
		_check_pwr_onoff();
	}

	close(fd_evt);


	return NULL;
}

void exit_thread_btn_pwr(void)
{
	flag_run_thread_btn_pwr = 0;
}

int start_thread_btn(void)
{
    pthread_attr_t attr;
    pthread_t tid = 0;;
    
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, (512 * 1024));
    
    if(pthread_create(&tid, &attr, thread_btn_pwr, NULL) != 0)
    {
        //LOGE(LOG_TARGET, "Create Thread Fail.. exit Program...\r\n");
        printf("[error] pthread_create Error");
        return -1;
    }
    
	flag_run_thread_btn_pwr = 1;
    return 0;
}
