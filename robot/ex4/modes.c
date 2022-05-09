#include "config.h"
#include "modes.h"
#include "robot.h"
#include "module.h"
#include "registers.h"
#include "hardware.h"

const uint8_t MOTOR_ADDR = 21;

int8_t mysetpoint = 0;

void motor_demo_mode()
{
  init_body_module(MOTOR_ADDR);
  start_pid(MOTOR_ADDR);
  set_color(4);
  while (reg8_table[REG8_MODE] == IMODE_MOTOR_DEMO) {
    bus_set(MOTOR_ADDR, MREG_SETPOINT, DEG_TO_OUTPUT_BODY(mysetpoint));
    //pause(ONE_MS);
    //pause(ONE_SEC);
    //bus_set(MOTOR_ADDR, MREG_SETPOINT, DEG_TO_OUTPUT_BODY(-21.0));
    //pause(ONE_SEC);
  }
  bus_set(MOTOR_ADDR, MREG_SETPOINT, DEG_TO_OUTPUT_BODY(0.0));
  pause(ONE_SEC);
  bus_set(MOTOR_ADDR, MREG_MODE, MODE_IDLE);
  set_color(2);
}

// listen to the setpoint command
static int8_t register_handler(uint8_t operation, uint8_t address, RadioData* radio_data)
{  
  switch (operation)
  {    
    case ROP_WRITE_8:
      if (address == REG8_SETPOINT) {
        mysetpoint = (int8_t) radio_data->byte;
        return TRUE;
      }
      break;
  }
  return FALSE;
}

void main_mode_loop()
{  
  radio_add_reg_callback(register_handler);
  reg8_table[REG8_MODE] = IMODE_IDLE;

  while (1)
  {
    switch(reg8_table[REG8_MODE])
    {
      case IMODE_IDLE:
        break;
      case IMODE_MOTOR_DEMO:
        motor_demo_mode();
        break;
      default:
        reg8_table[REG8_MODE] = IMODE_IDLE;
    }
  }
}
