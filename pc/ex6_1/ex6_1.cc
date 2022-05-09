#include <iostream>
#include <cstdlib>
#include <stdint.h>
#include <windows.h>
#include "trkcli.h"
#include "utils.h"
#include "robot.h"
#include "remregs.h"
#include "regdefs.h"

using namespace std;

// Tracking
const char* TRACKING_PC_NAME = "biorobpc6";   ///< host name of the tracking PC
const uint16_t TRACKING_PORT = 10502;          ///< port number of the tracking PC

// Radio
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

  CTrackingClient trk;

  // Connects to the tracking server
  if (!trk.connect(TRACKING_PC_NAME, TRACKING_PORT)) {
    return 1;
  }

  while (!kbhit()) {
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
    } else {
      cout << "(not detected)" << '\r';
    }

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
  
  regs.close();
}
