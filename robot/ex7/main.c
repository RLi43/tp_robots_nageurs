#include "hardware.h"
#include "registers.h"
#include "modes.h"
#include "can.h"
#include "module.h"

#define MOTOR_NUM 3
const uint8_t MOTOR_ADDRS[MOTOR_NUM] = {72, 73, 74};

int main(void)
{
  hardware_init();
  registers_init();

  // Changes the color of the led (green) to show the boot
  set_color_i(2, 0);
  pause(ONE_SEC);

  // turn off the led on motors
  for(int i = 0; i<MOTOR_NUM; i++){
    set_reg_value_dw(MOTOR_ADDRS[i], MREG32_LED, 0);
  }

  // Calls the main mode loop (see modes.c)
  main_mode_loop();

  return 0;
}
