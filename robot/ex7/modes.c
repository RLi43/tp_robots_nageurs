#include "config.h"
#include "modes.h"
#include "robot.h"
#include "module.h"
#include "registers.h"
#include "hardware.h"

#define MOTOR_NUM 5
const uint8_t MOTOR_ADDRS[MOTOR_NUM] = {25, 22, 24, 26, 23}; // Modify this!!! Head to tail

float phi = 1;   
float freq = 1;
float amp = 40;
int turn = 0;

// listen to the parameters command
static int8_t register_handler(uint8_t operation, uint8_t address, RadioData* radio_data)
{  
  switch (operation)
  {    
    case ROP_WRITE_8:
      if (address == REG8_AMP) {
        amp = DECODE_PARAM_8(radio_data->byte, 0, 80);
        return TRUE;
      }
      else if (address == REG8_FREQ)
      {
        freq = DECODE_PARAM_8(radio_data->byte, 0, 1.5);        
      }
      else if (address == REG8_PHI)
      {
        phi = DECODE_PARAM_8(radio_data->byte, -1.5, 1.5);    
      }
      else if (address == REG8_TURN)
      {
        turn = DECODE_PARAM_8(radio_data->byte, -30, 30);    
      }
      break;
  }
  return FALSE;
}

void sine_demo_mode()
{
  uint32_t dt, cycletimer;
  float my_time, delta_t, l;
  int8_t l_rounded;

  cycletimer = getSysTICs();
  my_time = 0;

  for(int i = 0; i<MOTOR_NUM; i++){
    init_body_module(MOTOR_ADDRS[i]);
    start_pid(MOTOR_ADDRS[i]);
  }

  do {
    // Calculates the delta_t in seconds and adds it to the current time
    dt = getElapsedSysTICs(cycletimer);
    cycletimer = getSysTICs();
    delta_t = (float) dt / sysTICSperSEC;
    my_time += delta_t;

    for(int i = 0; i<MOTOR_NUM; i++){
      // Calculates the sine wave
      l = amp * sin(M_TWOPI * (freq * my_time + (MOTOR_NUM - (float)i) *phi /MOTOR_NUM )) + turn ;
      l_rounded = (int8_t) l;

      bus_set(MOTOR_ADDRS[i], MREG_SETPOINT, DEG_TO_OUTPUT_BODY(l_rounded));
    }
    // Outputs the sine wave to the LED
    if (l >= 0) {
      set_rgb(0, l, 32);
    } else {
      set_rgb(-l, 0, 32);
    }
    

    // Make sure there is some delay, so that the timer output is not zero
    pause(ONE_MS);

  } while (reg8_table[REG8_MODE] == IMODE_SINE_DEMO);

  for(int i = 0; i<MOTOR_NUM; i++){
    bus_set(MOTOR_ADDRS[i], MREG_SETPOINT, DEG_TO_OUTPUT_BODY(0.0));
    pause(ONE_SEC);
    bus_set(MOTOR_ADDRS[i], MREG_MODE, MODE_IDLE);
  }
  // Back to the "normal" green
  set_color(2);
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
      case IMODE_SINE_DEMO:
        sine_demo_mode();
        break;
      default:
        reg8_table[REG8_MODE] = IMODE_IDLE;
    }
  }
}
