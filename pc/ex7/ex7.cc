#include <iostream>
#include "remregs.h"
#include "robot.h"
#include "regdefs.h"
#include "utils.h"
#include <math.h>
#include <time.h>

#define REG8_AMP 2
#define REG8_FREQ 3
#define REG8_PHI 4
using namespace std;

const uint8_t RADIO_CHANNEL = 126;         ///< robot radio channel
const char* INTERFACE = "COM1";            ///< robot radio interface

int main()
{
  CRemoteRegs regs;
  if (!init_radio_interface(INTERFACE, RADIO_CHANNEL, regs)) {
    return 1;
  }
  // Reboots the head microcontroller to make sure it is always in the same state
  reboot_head(regs);

  while (1)
  {
    float f_amp = -1, f_freq = -1, f_phi = -100;
    while(f_amp<0 || f_amp>80){
      cout << "Amp(0-80):";
      cin >> f_amp;
    }
    while(f_freq<0 || f_freq>1.5){
      cout << "Freq(0-1.5):";
      cin >> f_freq;
    }
    while(abs(f_phi)<0.5 || abs(f_phi)>1.5){
      cout << "Phi(0.5,1.5):";
      cin >> f_phi;
    }
    
    // turn floats to bytes
    uint8_t amp = ENCODE_PARAM_8(f_amp, 0, 80);
    uint8_t freq = ENCODE_PARAM_8(f_freq, 0, 1.5);
    uint8_t phi = ENCODE_PARAM_8(f_phi, -1.5, 1.5);

    // send the command
    regs.set_reg_b(REG8_AMP, amp);
    regs.set_reg_b(REG8_FREQ, freq);
    regs.set_reg_b(REG8_PHI, phi);
    regs.set_reg_b(REG8_MODE, 2); //IMODE_SINE_DEMO     2
    
    // Escape by key
    DWORD a = ext_key();
    break;
    // // Escape by time
    // time_t starter, timer;
    // time(&starter);
    // while(1){
    //   time(&timer);
    //   if(difftime(timer, starter) > 15)break;
    // }
    // break;
  }
  
  //ext_key(); // wait to quit
  regs.set_reg_b(REG8_MODE, 0); // IDLE

  regs.close();
  return 0;
}
