#include <iostream>
#include "remregs.h"
#include "robot.h"
#include "regdefs.h"
#include "utils.h"
#include <math.h>

#define REG8_AMP 2
#define REG8_FREQ 3
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

  float f_amp = -1, f_freq = -1;
  while(f_amp<0 || f_amp>60){
    cout << "Amp(0-60):";
    cin >> f_amp;
  }
  while(f_freq<0 || f_freq>2){
    cout << "Freq(0-2):";
    cin >> f_freq;
  }
  
  // turn floats to bytes
  uint8_t amp = ENCODE_PARAM_8(f_amp, 0, 60);
  uint8_t freq = ENCODE_PARAM_8(f_freq, 0, 2);

  // send the command
  regs.set_reg_b(REG8_MODE, 2);
  regs.set_reg_b(REG8_AMP, amp);
  regs.set_reg_b(REG8_FREQ, freq);
  ext_key(); // wait to quit
  regs.set_reg_b(REG8_MODE, 0);

  regs.close();
  return 0;
}
