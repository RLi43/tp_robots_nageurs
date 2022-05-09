#include "hardware.h"
#include "module.h"
#include "robot.h"
#include "registers.h"

// the addresses of all the motors
const uint8_t MOTOR_ADDRS[4] = {21, 72, 73, 74};
// the positions of all the motors
int8_t pos[4];

static int8_t register_handler(uint8_t operation, uint8_t address, RadioData* radio_data)
{
  uint8_t i;
  switch (operation)
  {
    case ROP_READ_MB:
      if (address == 2) {  
        radio_data->multibyte.size = 4;
        for (i = 0; i < 4; i++) {
          radio_data->multibyte.data[i] = pos[i];
        }
        return TRUE;
      }
      break;
  }
  return FALSE;
}

int main(void)
{

  hardware_init();
  
  // Changes the color of the led (red) to show the boot
  set_color_i(4, 0);

  // Initialises the body module with the specified address (but do not start
  // the PD controller)  
  for(int i = 0; i < 4; i++){
    init_body_module(MOTOR_ADDRS[i]);
  }
  
  // Registers the register handler callback function
  radio_add_reg_callback(register_handler);
  
  // query the positions one by one
  while (1) {
    for(int i = 0; i < 4; i++){
      pos[i] = bus_get(MOTOR_ADDRS[i], MREG_POSITION);
      /*
      if (pos > 0) {
        set_rgb(pos, 32, 0);
      } else {
        pos = -pos;
        set_rgb(0, 32, pos);
      }*/
    }
  }

  return 0;
}
