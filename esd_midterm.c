/* helloworld.c for TOPPERS/ATK(OSEK) */ 
#include "kernel.h"
#include "kernel_id.h"
#include "com_cfg.h"
#include "ecrobot_interface.h"
#include <stdio.h>

#define LEFT_MOTOR_PORT NXT_PORT_A
#define RIGHT_MOTOR_PORT NXT_PORT_C
#define TOUCH_SENSOR_PORT NXT_PORT_S2
#define LIGHT_SENSOR_PORT NXT_PORT_S3

DeclareCounter(SysTimerCnt);
DeclareTask(LineFollowerTask);
DeclareTask(EmergencyTask);
DeclareTask(MotorControlTask);
DeclareTask(TouchSensorTask);

void disp(int row, char* str, int val);

/* nxtOSEK hook to be invoked from an ISR in category 2 */
void user_1ms_isr_type2(void)
{
  StatusType ercd;

  /* Increment OSEK Alarm Counter */
  ercd = SignalCounter(SysTimerCnt);
  if (ercd != E_OK)
  {
    ShutdownOS(ercd);
  }
}

void ecrobot_device_intialize(void)
{
	ecrobot_set_light_sensor_active(LIGHT_SENSOR_PORT);
}

void ecrobot_device_terminate(void)
{
	ecrobot_set_light_sensor_inactive(LIGHT_SENSOR_PORT);
}

int touch_sensor = 0;
int light_sensor = 0;

MOTOR_CTR_MSG	commandSnd, commandRcv, comandSnd2;

TASK(LineFollowerTask)
{
	StartCOM(COMAPP);

	int direction = 1;

	while(1) {
		commandSnd.speed = 70 * direction;
		commandSnd.brake = 0;
		SendMessage(MotorCtrMsgSnd, &commandSnd);
		systick_wait_ms(1000);
		direction *= -1;
	}

	TerminateTask();
}

TASK(TouchSensorTask)
{
	touch_sensor = ecrobot_get_touch_sensor(NXT_PORT_S2);
	if (touch_sensor) {
		comandSnd2.speed = 0;
		comandSnd2.brake = 1;
		SendMessage(MotorCtrMsgSnd, &comandSnd2);
	}
	TerminateTask();
}

TASK(EmergencyTask)
{
	display_clear(0);
	disp(0, "EmergencyTask started ", 0);
	display_update();
	TerminateTask();
}

TASK(MotorControlTask)
{
	RecieveMessage(MotorCtrMsgRcv,&commandRcv);

	nxt_motor_set_speed(MOTOR_PORT, commandRcv.speed, commandRcv.brake);

	display_clear(0);
	disp(0, "Motor Speed ", commandRcv.speed);
	disp(1, "Motor Brake ", commandRcv.brake);
	display_update();

	TerminateTask();
}

void disp(int row, char *str, int val)
{ 
	display_goto_xy(0, row);
	display_string(str);
	display_int(val, 0);
}