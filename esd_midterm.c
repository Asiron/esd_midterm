/* helloworld.c for TOPPERS/ATK(OSEK) */ 
#include "kernel.h"
#include "kernel_id.h"
#include "com_cfg.h"
#include "ecrobot_interface.h"
#include <stdio.h>

#define LEFT_MOTOR_PORT NXT_PORT_A
#define RIGHT_MOTOR_PORT NXT_PORT_C
#define TOUCH_SENSOR_PORT NXT_PORT_S4
#define LIGHT_SENSOR_PORT NXT_PORT_S1

#define MIN_SPEED 57
#define MAX_SPEED 67

DeclareCounter(SysTimerCnt);
DeclareEvent(TouchSensorOnEvent);
DeclareTask(LineFollowerTask);
DeclareTask(EmergencyTask);
DeclareTask(MotorControlTask);
DeclareTask(TouchSensorTask);
DeclareTask(LightSensorTask);
DeclareTask(LCDTask);

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

void ecrobot_device_initialize(void)
{
	ecrobot_set_light_sensor_active(LIGHT_SENSOR_PORT);
}

void ecrobot_device_terminate(void)
{
	ecrobot_set_light_sensor_inactive(LIGHT_SENSOR_PORT);
}

int touch_sensor = 0;
int light_sensor = 0;
int left_brake   = 0;
int right_brake  = 0;
int current_left_motor_speed  = 0;
int current_right_motor_speed = 0;
int initial_brightness = 0;
int emergency = 0;

MOTOR_CTR_MSG	commandSnd, comandSnd2, commandRcv;

TASK(LineFollowerTask)
{
	StartCOM(COMAPP);

	systick_wait_ms(300);

	initial_brightness = light_sensor;
	int newBrightness = initial_brightness;
	int beforeBrightness = initial_brightness;

	while(1) {



		if ( newBrightness < (initial_brightness * 0.77) ) {
			// Start to lose the black line
			if ( beforeBrightness >= newBrightness ) {
				// Turn to the right
				commandSnd.speed = MIN_SPEED;
				commandSnd.brake = 0;
				commandSnd.motor = RIGHT_MOTOR_PORT;
				SendMessage(MotorCtrMsgSnd, &commandSnd);

				commandSnd.speed = MAX_SPEED;
				commandSnd.brake = 0;
				commandSnd.motor = LEFT_MOTOR_PORT;
				SendMessage(MotorCtrMsgSnd, &commandSnd);

			}
			beforeBrightness = newBrightness;
		}
		else{
				commandSnd.speed = MAX_SPEED;
				commandSnd.brake = 0;
				commandSnd.motor = RIGHT_MOTOR_PORT;
				SendMessage(MotorCtrMsgSnd, &commandSnd);

				commandSnd.speed = MIN_SPEED;
				commandSnd.brake = 0;
				commandSnd.motor = LEFT_MOTOR_PORT;
				SendMessage(MotorCtrMsgSnd, &commandSnd);

		}
		newBrightness = light_sensor;
	}
	TerminateTask();
}

TASK(TouchSensorTask)
{
	static UINT8 old_touch_status = 0;

	touch_sensor = ecrobot_get_touch_sensor(TOUCH_SENSOR_PORT);
	
	if (old_touch_status == 0 && touch_sensor == 1) {
		SetEvent(EmergencyTask, TouchSensorOnEvent);
	}

	old_touch_status = touch_sensor;

	TerminateTask();
}

TASK(LightSensorTask)
{
	light_sensor = ecrobot_get_light_sensor(LIGHT_SENSOR_PORT);
	TerminateTask();
}

TASK(LCDTask)
{
	display_clear(0);
	disp(0, "Left speed ", current_left_motor_speed);
	disp(1, "Left brake ", left_brake);
	disp(2, "Right speed ", current_right_motor_speed);
	disp(3, "Right brake ", right_brake);
	disp(4, "Light ", light_sensor);
	disp(5, "Touch ", touch_sensor);
	disp(6, "Inital light ", initial_brightness);
	disp(7, "Emergency ", emergency);
	display_update();
	TerminateTask();
}

TASK(EmergencyTask)
{
	WaitEvent(TouchSensorOnEvent);
	ClearEvent(TouchSensorOnEvent);

	GetEvent(EmergencyTask, &emergency);
	//emergency++; 

	TerminateTask();
}

TASK(MotorControlTask)
{
	RecieveMessage(MotorCtrMsgRcv,&commandRcv);

	nxt_motor_set_speed(commandRcv.motor, commandRcv.speed, commandRcv.brake);

	if (commandRcv.motor == RIGHT_MOTOR_PORT) {
		current_right_motor_speed = commandRcv.speed;
		right_brake = commandRcv.brake;
	} else if (commandRcv.motor == LEFT_MOTOR_PORT) {
		current_left_motor_speed  = commandRcv.speed;
		left_brake  = commandRcv.brake;
	}
	TerminateTask();
}

void disp(int row, char *str, int val)
{ 
	display_goto_xy(0, row);
	display_string(str);
	display_int(val, 0);
}