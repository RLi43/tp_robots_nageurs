#include <iostream>
#include <cstdlib>
#include <stdint.h>
#include <windows.h>
#include "trkcli.h"
#include <fstream>
#include "remregs.h"
#include "robot.h"
#include "regdefs.h"
#include "utils.h"
#include <math.h>
#include <time.h>
#include <string.h>

#define REG8_AMP 2
#define REG8_FREQ 3
#define REG8_PHI 4
#define REG8_TURN 5
using namespace std;

const uint8_t RADIO_CHANNEL = 126;         ///< robot radio channel
const char* INTERFACE = "COM1";            ///< robot radio interface

// Tracking
const char* TRACKING_PC_NAME = "biorobpc6";   ///< host name of the tracking PC
const uint16_t TRACKING_PORT = 10502;          ///< port number of the tracking PC

#define TURN_AROUND

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
  int f_turn = -100;
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
  while(abs(f_turn)>30){
    cout << "Turn(-30,30):";
    cin >> f_turn;
  }
  // Read required distance
  float distance = -1.f;
  while(distance<0 || distance>500){
    cout << "Required distance to move(0-5m):";
    cin >> distance;
  }

  int trial = 0;
  cout << "Trial:";
  cin >> trial;
  string filename = "amp" + to_string(f_amp) + "_freq" + to_string(f_freq) + "_phi" + to_string(f_phi) + "_dist" + to_string(distance) 
                      + "_turn" + to_string(f_turn) + "_tial" + to_string(trial);
  
  // turn floats to bytes
  uint8_t amp = ENCODE_PARAM_8(f_amp, 0, 80);
  uint8_t freq = ENCODE_PARAM_8(f_freq, 0, 1.5);
  uint8_t phi = ENCODE_PARAM_8(f_phi, -1.5, 1.5);
  uint8_t turn = ENCODE_PARAM_8(f_turn, -30, 30);
  uint8_t turn_zeros = ENCODE_PARAM_8(0, -30, 30);
  // send the command
  regs.set_reg_b(REG8_AMP, amp);
  regs.set_reg_b(REG8_FREQ, freq);
  regs.set_reg_b(REG8_PHI, phi);
  #ifndef TURN_AROUND
  regs.set_reg_b(REG8_TURN, turn);
  #endif
  // Make the robot move
  regs.set_reg_b(REG8_MODE, 2); 
  
  bool done = false;
  bool initialized = false;
  double start_x = 0.f, start_y = 0.f;
  double cur_distance = 0.f;
  double last_x=0, last_y=0;

  clock_t start, now;
  #ifdef TURN_AROUND
  clock_t turn_start = 0;
  bool turned = false;
  #endif
  start = clock();
  while (!kbhit() && !done) {
    uint32_t frame_time;
    // Gets the current position
    if (!trk.update(frame_time)) {
      return 1;
    }    
    // Gets the ID of the first spot (the tracking system supports multiple ones)
    int id = trk.get_first_id();

    double x, y; // current position 
    
    cout.precision(2);
    // Reads its coordinates (if (id == -1), then no spot is detected)
    if (id != -1 && trk.get_pos(id, x, y)) {
      cout << "(" << fixed << x << ", " << y << ", " << cur_distance<<")" << " m      \r";
      now = clock();
      // Save x, y, time to ouput.txt
      outputfile.open("C:\\Users\\jianli\\Desktop\\tp_robots_nageurs\\" + filename + ".txt", ios::app);
      outputfile << "(" << fixed << x << ", " << y << ")"
                 << " m, " << (float)(now - start )/ CLOCKS_PER_SEC * 1000 << "ms" << endl;
      outputfile.close();
      // Get initial position of robot
      if (!initialized) {
        last_x = start_x = x;
        last_y = start_y = y;
        initialized = true;
      }

      #ifdef TURN_AROUND
      // if it's approaching the boundary
      if(x < 1.0 || x > 4.0 ){
        if(!turned){            
          // turn left for max. 4s
          regs.set_reg_b(REG8_TURN, turn);
          turn_start = now;
          turned = true;
        }
      }
      else {
        regs.set_reg_b(REG8_TURN, turn_zeros);
        turned = false;
      }
      if((float)(now - turn_start )/ CLOCKS_PER_SEC > 4){
        regs.set_reg_b(REG8_TURN, turn_zeros);
      }
      #endif
    } else {
      cout << "(not detected)" << '\r';
    }
    
    // accumulate the distance
    cur_distance += sqrt(pow(x - last_x, 2) + pow(y - last_y, 2));
    if(cur_distance >= distance) {
      done = true;
      break;
    }
    last_x = x;
    last_y = y;

    uint8_t r = ENCODE_PARAM_8(x, 0, 6);
    uint8_t g = 64;
    uint8_t b = ENCODE_PARAM_8(y, 0, 2);
    uint32_t rgb = ((uint32_t) r << 16) | ((uint32_t) g << 8) | b;
    regs.set_reg_dw(REG32_LED, rgb);

    // Waits 10 ms before getting the info next time (anyway the tracking runs at 15 fps)
    Sleep(10);
  }
  // Clears the console input buffer (as kbhit() doesn't)
  FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));

  regs.set_reg_b(REG8_MODE, 0); // IDLE
  regs.close();
  return 0;
}
