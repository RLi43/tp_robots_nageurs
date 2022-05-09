#include <iostream>
#include "remregs.h"
#include "robot.h"
#include "regdefs.h"
#include "utils.h"
#include <math.h>

#define REG8_SETPOINT 2
using namespace std;

const uint8_t RADIO_CHANNEL = 201;         ///< robot radio channel
const char* INTERFACE = "COM1";            ///< robot radio interface

int main()
{
  CRemoteRegs regs;

  if (!init_radio_interface(INTERFACE, RADIO_CHANNEL, regs)) {
    return 1;
  }

  // Reboots the head microcontroller to make sure it is always in the same state
  reboot_head(regs);
  
  regs.set_reg_b(REG8_MODE, 1); // start demo(listener)
  
  double starter = time_d();
  double cnt = starter;
  while(!kbhit()){//quit when key hit 
      if (time_d() - cnt > 0.001){//send command at frequency of 1kHz(expected)
          regs.set_reg_b(REG8_SETPOINT, (int) 40*sin((time_d() - starter)*2*M_PI*1));
          cnt = time_d();
      }
  }

  // ext_key(); // wait to quit
  regs.set_reg_b(REG8_MODE, 0);

  regs.close();
  return 0;
}
