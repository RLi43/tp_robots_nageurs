#include <iostream>
#include <fstream>
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

const uint8_t RADIO_CHANNEL = 201;         ///< robot radio channel
const char* INTERFACE = "COM1";            ///< robot radio interface

// Tracking
const char* TRACKING_PC_NAME = "biorobpc11";   ///< host name of the tracking PC
const uint16_t TRACKING_PORT = 10502;          ///< port number of the tracking PC


int main()
{
  // Write coordinate output to file
  fstream outputfile;

  CRemoteRegs regs;
  if (!init_radio_interface(INTERFACE, RADIO_CHANNEL, regs)) {
    return 1;
  }
  // Reboots the head microcontroller to make sure it is always in the same state
  reboot_head(regs);

  CTrackingClient trk;

  // Connects to the tracking server
  if (!trk.connect(TRACKING_PC_NAME, TRACKING_PORT)) {
    return 1;
  }

  // Set params
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
  // Read required distance
  float distance = -1.f;
  while(f_amp<0 || f_amp>5){
    cout << "Required distance to move: (0-5)";
    cin >> distance;
  }
  
  // turn floats to bytes
  uint8_t amp = ENCODE_PARAM_8(f_amp, 0, 80);
  uint8_t freq = ENCODE_PARAM_8(f_freq, 0, 1.5);
  uint8_t phi = ENCODE_PARAM_8(f_phi, -1.5, 1.5);
  // send the command
  regs.set_reg_b(REG8_AMP, amp);
  regs.set_reg_b(REG8_FREQ, freq);
  regs.set_reg_b(REG8_PHI, phi);
  
  bool done = false;
  bool initialized = false;
  double start_x = 0.f, start_y = 0.f;

  time_t start, now;
  time(&start);
  while (!kbhit() && !done) {
    uint32_t frame_time;
    // Gets the current position
    if (!trk.update(frame_time)) {
      return 1;
    }
    
    double x, y;
    
    cout.precision(2);
    
    // Gets the ID of the first spot (the tracking system supports multiple ones)
    int id = trk.get_first_id();
    
    // Reads its coordinates (if (id == -1), then no spot is detected)
    if (id != -1 && trk.get_pos(id, x, y)) {
      cout << "(" << fixed << x << ", " << y << ")" << " m      \r";
      time(&now);
      // Save x, y, time to ouput.txt
      outputfile.open("output.txt", ios::app);
      outputfile << "(" << fixed << x << ", " << y << ")"
                 << " m, " << difftime(now, start) << "s" << endl;
      outputfile.close();
    } else {
      cout << "(not detected)" << '\r';
    }

    // Get initial position of robot
    if (!initialized) {
      start_x = x;
      start_y = y;
      initialized = true;
    }
    
    // Stop the robot if reached required distance
    if (sqrt(pow(x - start_x, 2) + pow(y - start_y, 2)) > distance) {
      done = true;
      break;
    }

    uint8_t r = ENCODE_PARAM_8(x, 0, 6);
    uint8_t g = 64;
    uint8_t b = ENCODE_PARAM_8(y, 0, 2);
    uint32_t rgb = ((uint32_t) r << 16) | ((uint32_t) g << 8) | b;
    regs.set_reg_dw(REG32_LED, rgb);

    // Make the robot move
    regs.set_reg_b(REG8_MODE, 2); //IMODE_SINE_DEMO     2
    // Waits 10 ms before getting the info next time (anyway the tracking runs at 15 fps)
    Sleep(10);
  }
  regs.set_reg_b(REG8_MODE, 0); // IDLE
  // Clears the console input buffer (as kbhit() doesn't)
  FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));

  regs.close();
  return 0;
}