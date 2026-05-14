#include "main.h"
#include "user.h"

void user_led(int8_t red_en, int8_t green_en, int8_t blue_en){
	if(red_en){
		HAL_GPIO_WritePin(USER_LED_R_GPIO_Port, USER_LED_R_Pin, GPIO_PIN_RESET);
	}else{
		HAL_GPIO_WritePin(USER_LED_R_GPIO_Port, USER_LED_R_Pin, GPIO_PIN_SET);
	}
	if(green_en){
		HAL_GPIO_WritePin(USER_LED_G_GPIO_Port, USER_LED_G_Pin, GPIO_PIN_RESET);
	}else{
		HAL_GPIO_WritePin(USER_LED_G_GPIO_Port, USER_LED_G_Pin, GPIO_PIN_SET);
	}
	if(blue_en){
		HAL_GPIO_WritePin(USER_LED_B_GPIO_Port, USER_LED_B_Pin, GPIO_PIN_RESET);
	}else{
		HAL_GPIO_WritePin(USER_LED_B_GPIO_Port, USER_LED_B_Pin, GPIO_PIN_SET);
	}	
}

SYS_STAMP_ sys_stamp_duration(SYS_STAMP_* sys_stamp_now, SYS_STAMP_* sys_stamp_last){
	SYS_STAMP_ sys_stamp_duration = {.sys_stamp_sec = sys_stamp_now->sys_stamp_sec-sys_stamp_last->sys_stamp_sec, .sys_stamp_msec = sys_stamp_now->sys_stamp_msec-sys_stamp_last->sys_stamp_msec};
	if(sys_stamp_duration.sys_stamp_msec < 0){
		sys_stamp_duration.sys_stamp_sec -= 1;
		sys_stamp_duration.sys_stamp_msec += 1000;
	}
	return sys_stamp_duration;
}
